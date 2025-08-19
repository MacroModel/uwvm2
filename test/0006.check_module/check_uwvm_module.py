#!/usr/bin/env python3
"""
Check UWVM_MODULE imports across the src tree.

Rules:
- For each pair (.cppm, .h) in the same directory with the same basename,
  their UWVM_MODULE imports must match in content and order.
- For each pair (*.module.cpp, *.default.cpp) in the same directory with the same basename,
  their imports must match in content and order.
- Import order must respect category sequence:
  1) uwvm_predefine/*
  2) utils/* (including fast_io)
  3) parser/*
  4) uwvm/io/*
  5) uwvm/utils/*
  After these categories, the order of any remaining headers is not constrained.

Notes:
- In .cppm/.module.cpp, we read `import ...;` lines.
- In .h/.default.cpp, we read `#include <.../impl.h>`, `#include <fast_io.h>`, and relative `#include "*.h"`
  within the `#ifndef UWVM_MODULE` guarded region, ignoring other includes (e.g., macros, std headers).
- We normalize imports to canonical names so that, for example:
  `#include <uwvm2/utils/container/impl.h>` == `import uwvm2.utils.container;`
  `#include "def.h"` == `import :def;`
  `#include <fast_io.h>` == `import fast_io;`
"""

from __future__ import annotations

import os
import re
import sys
from dataclasses import dataclass
from typing import Dict, List, Tuple, Iterable


# Root directories
THIS_DIR = os.path.abspath(os.path.dirname(__file__))
SRC_ROOT = os.path.abspath(os.path.join(THIS_DIR, "..", "..", "src"))


IMPORT_LINE_RE = re.compile(r"^\s*(?:export\s+)?import\s+([^;\s]+)\s*;\s*$")
MODULE_NAME_RE = re.compile(r"^\s*export\s+module\s+([^;\s]+)\s*;\s*$", re.MULTILINE)
INCLUDE_RE = re.compile(r"^\s*#\s*include\s+[<\"]([^>\"]+)[>\"]")


@dataclass
class Pair:
    a: str
    b: str


def debug(msg: str) -> None:
    # Toggle for verbose debugging if needed
    # print(f"[DEBUG] {msg}")
    pass


def is_cppm(path: str) -> bool:
    return path.endswith(".cppm")


def is_header(path: str) -> bool:
    return path.endswith(".h") and not path.endswith("impl.h")


def is_default_cpp(path: str) -> bool:
    return path.endswith(".default.cpp")


def is_module_cpp(path: str) -> bool:
    return path.endswith(".module.cpp")


def read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def list_files(root: str) -> List[str]:
    paths: List[str] = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            if fn.startswith("."):
                continue
            paths.append(os.path.join(dirpath, fn))
    return paths


def normalize_header_to_import_name(header: str) -> str | None:
    """Map header include path to canonical import name.

    Examples:
    - "uwvm2/utils/container/impl.h" -> "uwvm2.utils.container"
    - "uwvm2/uwvm/wasm/storage/impl.h" -> "uwvm2.uwvm.wasm.storage"
    - "fast_io.h" -> "fast_io"
    - "def.h" -> ":def" (relative header, treated as partition)

    Returns None for headers to be ignored (e.g., std headers, macro push/pop).
    """

    # Ignore known macro helpers and pop macros
    if "macro/push_macros.h" in header or "macro/pop_macros.h" in header:
        return None
    if "uwvm_color_push_macro.h" in header or "uwvm_color_pop_macro.h" in header:
        return None

    # Ignore STL and C headers
    if not ("/" in header or header.endswith(".h")):
        return None

    # Special-case fast_io
    if header == "fast_io.h" or header.startswith("fast_io/"):
        return "fast_io"

    # Only consider impl.h and local .h as module-correlated
    if header.endswith("/impl.h"):
        core = header[:-len("/impl.h")]
        return core.replace("/", ".")

    # Local relative header like "def.h" or nested like "types/abc.h"
    if "/" not in header and header.endswith(".h"):
        base = header[:-2]
        return f":{base}"

    # Other tree headers (non-impl.h) are considered non-UWVM_MODULE imports
    return None


def extract_imports_from_cppm_or_module_cpp(text: str) -> List[str]:
    """Extract ordered list of canonical import names from .cppm or .module.cpp.

    - import uwvm2.utils.container; -> "uwvm2.utils.container"
    - import :def; -> ":def"
    - Duplicates are preserved to catch ordering mistakes; final comparison will dedupe in a stable way.
    """
    imports: List[str] = []
    for line in text.splitlines():
        m = IMPORT_LINE_RE.match(line)
        if not m:
            continue
        name = m.group(1).strip()
        # Filter out internal leader keywords (e.g., import module;) – not expected here
        if name == "module":
            continue
        imports.append(name)
    debug(f"imports-from-cppm/module: {imports}")
    return imports


def extract_guarded_includes(text: str, *, base_module: str | None = None, restrict_to: List[str] | None = None) -> List[str]:
    """Extract ordered list of canonical import names from the #ifndef UWVM_MODULE region.

    Only keep includes that map to module-like imports:
    - <.../impl.h>
    - <fast_io.h>
    - "*.h" (local headers)
    """
    lines = text.splitlines()

    # Find the first #ifndef UWVM_MODULE and match until its corresponding #endif
    start_idx = None
    depth = 0
    for i, ln in enumerate(lines):
        if re.match(r"^\s*#\s*ifndef\s+UWVM_MODULE\b", ln):
            start_idx = i
            depth = 1
            break
    if start_idx is None:
        return []

    includes: List[str] = []
    i = start_idx + 1
    while i < len(lines) and depth > 0:
        ln = lines[i]
        if re.match(r"^\s*#\s*if(n?def|)\b", ln):
            depth += 1
        elif re.match(r"^\s*#\s*endif\b", ln):
            depth -= 1
            i += 1
            continue
        elif depth > 0:
            m = INCLUDE_RE.match(ln)
            if m:
                hdr = m.group(1).strip()
                norm = None
                # Special mapping for relative submodule aggregator paths like "sub/impl.h"
                if base_module and (not ("/" in hdr and hdr.startswith("uwvm2/"))):
                    # relative include
                    if hdr.endswith("/impl.h") and "/" in hdr:
                        sub = hdr[: -len("/impl.h")]
                        # Map to fully-qualified dotted name using base module
                        norm = f"{base_module}.{sub.replace('/', '.')}"
                    elif hdr.endswith(".h") and "/" not in hdr:
                        # local partition header, map to partition
                        norm = f":{hdr[:-2]}"
                if norm is None:
                    norm = normalize_header_to_import_name(hdr)
                if norm is not None:
                    includes.append(norm)
        i += 1

    debug(f"includes-from-guard: {includes}")
    # If restrict_to provided, filter to only those present in restrict_to (preserving order)
    if restrict_to is not None:
        restrict_set = set(restrict_to)
        includes = [x for x in includes if x in restrict_set]

    return includes


def stable_unique(seq: Iterable[str]) -> List[str]:
    seen = set()
    out: List[str] = []
    for x in seq:
        if x in seen:
            continue
        seen.add(x)
        out.append(x)
    return out


def _strip_prefix(name: str) -> str:
    return name[6:] if name.startswith("uwvm2.") else name


def _is_predefine(name: str) -> bool:
    n = _strip_prefix(name)
    return n.startswith("uwvm_predefine.")


def _is_utils(name: str) -> bool:
    n = _strip_prefix(name)
    # Exclude uwvm_predefine subtree entirely from utils detection, even though it contains "utils" segment
    if n.startswith("uwvm_predefine."):
        return False
    return name == "fast_io" or name.startswith("fast_io.") or n.startswith("utils.")


def _is_parser(name: str) -> bool:
    n = _strip_prefix(name)
    return n.startswith("parser.")


def _is_uwvm_io(name: str) -> bool:
    n = _strip_prefix(name)
    return n == "uwvm.io" or n.startswith("uwvm.io.")


def _is_uwvm_utils(name: str) -> bool:
    n = _strip_prefix(name)
    return n == "uwvm.utils" or n.startswith("uwvm.utils.")


def check_order(seq: List[str]) -> Tuple[bool, str | None]:
    """Enforce category ordering constraints with a late utils window.

    Required relative order of blocks (if present):
      predefine -> utils(early) -> parser -> uwvm/io -> uwvm/utils -> utils(late)
    After that, the order of any remaining headers is unconstrained.
    Each block may be absent; constraints only apply when both sides exist.
    All early-utils must appear before the first parser; all parser before first uwvm/io; etc.
    """
    if not seq:
        return True, None

    # Build index lists
    idx_pre: List[int] = []
    idx_utils: List[int] = []
    idx_parser: List[int] = []
    idx_uio: List[int] = []
    idx_uutil: List[int] = []

    for i, name in enumerate(seq):
        if name.startswith(":"):
            continue  # local headers treated as unconstrained later
        if _is_predefine(name):
            idx_pre.append(i)
        elif _is_utils(name):
            idx_utils.append(i)
        elif _is_parser(name):
            idx_parser.append(i)
        elif _is_uwvm_io(name):
            idx_uio.append(i)
        elif _is_uwvm_utils(name):
            idx_uutil.append(i)

    pre0 = idx_pre[0] if idx_pre else None
    utils0 = idx_utils[0] if idx_utils else None
    parser0 = idx_parser[0] if idx_parser else None
    uio0 = idx_uio[0] if idx_uio else None
    uutil0 = idx_uutil[0] if idx_uutil else None

    # Partition utils into early and late based on uutil0 (if exists)
    early_utils = [i for i in idx_utils if uutil0 is None or i < uutil0]
    late_utils = [i for i in idx_utils if uutil0 is not None and i > uutil0]

    def fail(msg: str) -> Tuple[bool, str]:
        return False, msg

    # predefine < utils(early)
    if pre0 is not None and early_utils:
        if pre0 > min(early_utils):
            return fail(f"uwvm_predefine/* must appear before utils/* (found pre at {pre0}, utils at {min(early_utils)})")

    # utils(early) < parser
    if early_utils and parser0 is not None:
        if max(early_utils) > parser0:
            return fail(f"utils/* (early) must appear before parser/* (last early utils at {max(early_utils)}, parser at {parser0})")

    # parser < uwvm/io
    if parser0 is not None and uio0 is not None:
        if parser0 > uio0:
            return fail(f"parser/* must appear before uwvm/io/* (parser at {parser0}, uwvm/io at {uio0})")

    # uwvm/io < uwvm/utils
    if uio0 is not None and uutil0 is not None:
        if uio0 > uutil0:
            return fail(f"uwvm/io/* must appear before uwvm/utils/* (uwvm/io at {uio0}, uwvm/utils at {uutil0})")

    # uwvm/utils < utils(late) if both present
    if uutil0 is not None and late_utils:
        if uutil0 > min(late_utils):
            return fail(
                f"uwvm/utils/* must appear before late utils/* (uwvm/utils at {uutil0}, late utils at {min(late_utils)})"
            )

    return True, None


def compare_sequences(ref: List[str], other: List[str]) -> Tuple[bool, List[str]]:
    """Compare two sequences for equality, after stable-deduping.

    We dedupe to avoid false positives when one side repeats the same import.
    """
    a = stable_unique(ref)
    b = stable_unique(other)
    if a == b:
        return True, []
    diffs: List[str] = []
    diffs.append("Expected (A): " + ", ".join(a))
    diffs.append("Found    (B): " + ", ".join(b))
    # Also show missing and extra for clarity
    missing = [x for x in a if x not in b]
    extra = [x for x in b if x not in a]
    if missing:
        diffs.append("Missing in B: " + ", ".join(missing))
    if extra:
        diffs.append("Extra in B: " + ", ".join(extra))
    return False, diffs


def find_cppm_h_pairs(files: List[str]) -> List[Pair]:
    pairs: List[Pair] = []
    # Group by directory + stem
    by_dir: Dict[Tuple[str, str], Dict[str, str]] = {}
    for p in files:
        d = os.path.dirname(p)
        base = os.path.basename(p)
        stem, ext = os.path.splitext(base)
        key = (d, stem)
        by_dir.setdefault(key, {})[ext] = p
    for (d, stem), mp in by_dir.items():
        if ".cppm" in mp and ".h" in mp:
            pairs.append(Pair(mp[".cppm"], mp[".h"]))
    return pairs


def is_impl_pair(pair: Pair) -> bool:
    return os.path.basename(pair.a) == "impl.cppm" and os.path.basename(pair.b) == "impl.h"


def extract_module_name(text: str) -> str | None:
    m = MODULE_NAME_RE.search(text)
    if m:
        return m.group(1).strip()
    return None


def find_module_default_pairs(files: List[str]) -> List[Pair]:
    pairs: List[Pair] = []
    by_dir: Dict[str, Dict[str, str]] = {}
    for p in files:
        if not (is_module_cpp(p) or is_default_cpp(p)):
            continue
        d = os.path.dirname(p)
        by_dir.setdefault(d, {})[os.path.basename(p)] = p

    for d, mp in by_dir.items():
        # Build basenames without suffix
        names = list(mp.keys())
        stems = set()
        for n in names:
            if n.endswith(".module.cpp"):
                stems.add(n[:-len(".module.cpp")])
            elif n.endswith(".default.cpp"):
                stems.add(n[:-len(".default.cpp")])
        for stem in stems:
            mod = os.path.join(d, stem + ".module.cpp")
            dfl = os.path.join(d, stem + ".default.cpp")
            if os.path.exists(mod) and os.path.exists(dfl):
                pairs.append(Pair(mod, dfl))
    return pairs


def main() -> int:
    print(f"Scanning src root: {SRC_ROOT}")
    files = list_files(SRC_ROOT)

    cppm_h_pairs = find_cppm_h_pairs(files)
    mod_def_pairs = find_module_default_pairs(files)

    problems: List[str] = []

    # Check .cppm <-> .h
    for pair in cppm_h_pairs:
        cppm_txt = read_text(pair.a)
        h_txt = read_text(pair.b)

        imports_cppm = extract_imports_from_cppm_or_module_cpp(cppm_txt)

        if is_impl_pair(pair):
            # Special logic for impl.cppm <-> impl.h
            base_module = extract_module_name(cppm_txt)
            if base_module is None:
                base_module = ""

            includes_h = extract_guarded_includes(h_txt, base_module=base_module, restrict_to=imports_cppm)
        else:
            includes_h = extract_guarded_includes(h_txt)

        # Enforce category order only on header-side includes, not on module import lines
        ok_order_h, msg_h = check_order(includes_h)
        if not ok_order_h:
            problems.append(f"[ORDER] {pair.b}: {msg_h}")

        ok_eq, diffs = compare_sequences(imports_cppm, includes_h)
        if not ok_eq:
            problems.append(f"[MISMATCH] {pair.a} <-> {pair.b}")
            problems.extend(["  " + d for d in diffs])

    # Check *.module.cpp <-> *.default.cpp
    for pair in mod_def_pairs:
        mod_txt = read_text(pair.a)
        def_txt = read_text(pair.b)

        imports_mod = extract_imports_from_cppm_or_module_cpp(mod_txt)
        includes_def = extract_guarded_includes(def_txt)

        # Enforce category order only on default.cpp includes, not on module.cpp import lines
        ok_order_def, msg_def = check_order(includes_def)
        if not ok_order_def:
            problems.append(f"[ORDER] {pair.b}: {msg_def}")

        ok_eq, diffs = compare_sequences(imports_mod, includes_def)
        if not ok_eq:
            problems.append(f"[MISMATCH] {pair.a} <-> {pair.b}")
            problems.extend(["  " + d for d in diffs])

    if problems:
        print("Found issues:")
        for p in problems:
            print(p)
        return 1
    else:
        print("All UWVM_MODULE imports are consistent and ordered correctly.")
        return 0


if __name__ == "__main__":
    sys.exit(main())


