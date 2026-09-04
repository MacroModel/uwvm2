#!/usr/bin/env python3
"""Validate or execute Wasmtime WAT fixtures through uwvm-int-full and LLVM-full."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path


ANSI_RE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")
ENTRY_RE = re.compile(r'\((?:start\b|export\s+"(?:_start|main)")')
UWVM_TRAP_RE = re.compile(r"Runtime crash \(([^)]+)\)", re.IGNORECASE)

MVP_VALIDATION_FLAGS = (
    "--disable-mutable-globals",
    "--disable-saturating-float-to-int",
    "--disable-sign-extension",
    "--disable-simd",
    "--disable-multi-value",
    "--disable-bulk-memory",
    "--disable-reference-types",
)

# WABT enables the proposals integrated by WebAssembly 2.0 core by default.
# Experimental tracks such as threads, GC, memory64, multi-memory, tail calls,
# and relaxed SIMD still require an explicit --enable-* (or --enable-all).
VALIDATION_PROFILES: dict[str, tuple[str, ...]] = {
    "mvp": MVP_VALIDATION_FLAGS,
    "wasm2-core": (),
    "all": ("--enable-all",),
}

# Keep the runtime grammar aligned with the WABT profile used to select each
# case. The Wasm 2.0 profile is materially different from Wasm 1.1 for encoded
# immediates such as call_indirect's table index, even when both profiles enable
# the same proposal families.
UWVM_FEATURE_ARGS: dict[str, tuple[str, ...]] = {
    "mvp": ("--wasm-feature-mvp",),
    "wasm2-core": ("--wasm-feature-wasm2",),
    "all": ("--wasm-feature-wasm2",),
}


@dataclass
class ProcessResult:
    argv: list[str]
    returncode: int
    timed_out: bool
    elapsed_ms: float
    stdout: str
    stderr: str
    outcome: str


def strip_ansi(text: str) -> str:
    return ANSI_RE.sub("", text)


def canonical_trap(text: str) -> str | None:
    plain = strip_ansi(text).lower()
    uwvm_match = UWVM_TRAP_RE.search(strip_ansi(text))
    if uwvm_match:
        plain = uwvm_match.group(1).lower()

    patterns = (
        ("unreachable", ("unreachable",)),
        ("memory-oob", ("out of bounds memory", "memory out of bounds", "out-of-bounds memory")),
        ("divide-by-zero", ("divide by zero", "division by zero", "divided by zero")),
        ("integer-overflow", ("integer overflow",)),
        ("invalid-conversion", ("invalid conversion", "invalid float-to-int")),
        ("indirect-null", ("uninitialized element", "null function", "uninitialized table")),
        ("indirect-oob", ("out of bounds table", "undefined element", "table index is out of bounds")),
        ("indirect-type", ("indirect call type mismatch", "function signature mismatch")),
        ("stack-overflow", ("call stack exhausted", "stack overflow")),
    )
    for name, needles in patterns:
        if any(needle in plain for needle in needles):
            return name
    return None


def classify(returncode: int, timed_out: bool, stdout: str, stderr: str) -> str:
    if timed_out:
        return "timeout"
    if returncode == 0:
        return "ok"
    trap = canonical_trap(stdout + "\n" + stderr)
    if trap:
        return f"trap:{trap}"
    if 0 < returncode < 128:
        return f"exit:{returncode}"
    if returncode < 0:
        return f"signal:{-returncode}"
    return f"error:{returncode}"


def run_process(argv: list[str], cwd: Path, timeout: float) -> ProcessResult:
    started = time.perf_counter_ns()
    timed_out = False
    try:
        completed = subprocess.run(
            argv,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            timeout=timeout,
        )
        returncode = completed.returncode
        stdout = completed.stdout.decode("utf-8", errors="replace")
        stderr = completed.stderr.decode("utf-8", errors="replace")
    except subprocess.TimeoutExpired as error:
        timed_out = True
        returncode = 124
        stdout = (error.stdout or b"").decode("utf-8", errors="replace")
        stderr = (error.stderr or b"").decode("utf-8", errors="replace")
    elapsed_ms = (time.perf_counter_ns() - started) / 1_000_000.0
    return ProcessResult(
        argv=argv,
        returncode=returncode,
        timed_out=timed_out,
        elapsed_ms=elapsed_ms,
        stdout=stdout,
        stderr=stderr,
        outcome=classify(returncode, timed_out, stdout, stderr),
    )


def executable_path(raw: str) -> Path:
    path = Path(raw).expanduser().resolve()
    if not path.is_file() or not os.access(path, os.X_OK):
        raise argparse.ArgumentTypeError(f"not an executable: {path}")
    return path


def same_result(lhs: ProcessResult, rhs: ProcessResult) -> bool:
    if lhs.outcome != rhs.outcome:
        return False
    return lhs.outcome != "ok" or lhs.stdout == rhs.stdout


def has_binary_entry(objdump_text: str) -> bool:
    if "Start:" in objdump_text:
        return True

    void_signatures = set(re.findall(r"- type\[(\d+)\] \(\) -> nil", objdump_text))
    function_signatures = dict(re.findall(r"- func\[(\d+)\] sig=(\d+)", objdump_text))
    exported_entries = re.findall(r'- func\[(\d+)\].*-> "(?:_start|main)"$', objdump_text, re.MULTILINE)
    return any(function_signatures.get(function_index) in void_signatures for function_index in exported_entries)


def artifact_stem(relative: str) -> str:
    readable = re.sub(r"[^A-Za-z0-9_.-]+", "_", relative).strip("_")[-120:]
    digest = hashlib.sha256(relative.encode("utf-8")).hexdigest()[:12]
    return f"{digest}-{readable}"


def inherited_cpu_affinity() -> list[int] | None:
    if not hasattr(os, "sched_getaffinity"):
        return None
    return sorted(os.sched_getaffinity(0))


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--uwvm", type=executable_path, help="combined-backend executable used for both uwvm modes")
    parser.add_argument("--uwvm-int", type=executable_path, help="pure or combined uwvm-int executable")
    parser.add_argument("--uwvm-llvm", type=executable_path, help="pure or combined LLVM-full executable")
    parser.add_argument("--wasmtime", type=executable_path, required=True)
    parser.add_argument("--wat2wasm", type=executable_path, required=True)
    parser.add_argument("--wasm-validate", type=executable_path, required=True)
    parser.add_argument("--wasm-objdump", type=executable_path, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=20.0)
    parser.add_argument(
        "--mode",
        choices=("execute", "validate"),
        default="execute",
        help="execute entry-point fixtures, or validate every profile-compatible WAT without running it",
    )
    parser.add_argument(
        "--uwvm-feature-arg",
        action="append",
        default=[],
        help=(
            "override the profile-derived uwvm feature switch; repeat for multiple arguments "
            "(use --uwvm-feature-arg=--wasm-feature-wasm2 when the value begins with '-')"
        ),
    )
    parser.add_argument(
        "--feature-profile",
        choices=tuple(VALIDATION_PROFILES),
        default="mvp",
        help="WABT validation profile used to select runnable modules",
    )
    parser.add_argument(
        "--only-profile-delta",
        action="store_true",
        help="exclude modules already valid under the preceding profile (MVP for wasm2-core, wasm2-core for all)",
    )
    parser.add_argument("--include", action="append", default=[])
    parser.add_argument("--exclude", action="append", default=[])
    parser.add_argument("--max-cases", type=int, default=0)
    parser.add_argument("--allow-reference-mismatch", action="store_true")
    args = parser.parse_args(argv)

    if args.timeout <= 0.0:
        parser.error("--timeout must be positive")
    if args.max_cases < 0:
        parser.error("--max-cases must not be negative")
    if args.only_profile_delta and args.feature_profile == "mvp":
        parser.error("--only-profile-delta requires wasm2-core or all")

    uwvm_int = args.uwvm_int or args.uwvm
    uwvm_llvm = args.uwvm_llvm or args.uwvm
    if uwvm_int is None or uwvm_llvm is None:
        parser.error("provide --uwvm, or provide both --uwvm-int and --uwvm-llvm")

    source_root = args.source_root.expanduser().resolve()
    if not source_root.is_dir():
        parser.error(f"source root does not exist: {source_root}")
    work_dir = args.work_dir.expanduser().resolve()
    artifacts_dir = work_dir / "artifacts"
    artifacts_dir.mkdir(parents=True, exist_ok=True)

    include = [re.compile(value) for value in args.include]
    exclude = [re.compile(value) for value in args.exclude]
    selected: list[tuple[Path, str]] = []
    selection_counts = {"wat_total": 0, "no_entry": 0, "include_filtered": 0, "excluded": 0}
    for path in sorted(source_root.rglob("*.wat")):
        selection_counts["wat_total"] += 1
        relative = path.relative_to(source_root).as_posix()
        if include and not any(pattern.search(relative) for pattern in include):
            selection_counts["include_filtered"] += 1
            continue
        if any(pattern.search(relative) for pattern in exclude):
            selection_counts["excluded"] += 1
            continue
        if args.mode == "execute":
            text = path.read_text(encoding="utf-8", errors="replace")
            if not ENTRY_RE.search(text):
                selection_counts["no_entry"] += 1
                continue
        selected.append((path, relative))
    if args.max_cases:
        selected = selected[: args.max_cases]
    if not selected:
        parser.error("no executable WAT fixtures selected")

    cases: list[dict[str, object]] = []
    counters = {
        "selected": len(selected),
        "wat_compile_skipped": 0,
        "profile_validation_skipped": 0,
        "preceding_profile_skipped": 0,
        "no_binary_entry_skipped": 0,
        "passed": 0,
        "backend_mismatch": 0,
        "reference_mismatch": 0,
        "validation_rejected": 0,
    }
    started = time.time()

    for index, (wat_path, relative) in enumerate(selected, start=1):
        stem = artifact_stem(relative)
        case_dir = artifacts_dir / stem
        case_dir.mkdir(parents=True, exist_ok=True)
        wasm_path = case_dir / "input.wasm"

        compile_result = run_process(
            [str(args.wat2wasm), "--enable-all", str(wat_path), "-o", str(wasm_path)],
            source_root,
            args.timeout,
        )
        case: dict[str, object] = {
            "relative_path": relative,
            "wat_path": str(wat_path),
            "wasm_path": str(wasm_path),
            "wat2wasm": asdict(compile_result),
        }
        if compile_result.outcome != "ok":
            counters["wat_compile_skipped"] += 1
            case["status"] = "wat-compile-skipped"
            cases.append(case)
            continue

        profile_flags = VALIDATION_PROFILES[args.feature_profile]
        validate_result = run_process(
            [str(args.wasm_validate), *profile_flags, str(wasm_path)],
            source_root,
            args.timeout,
        )
        case["profile_validation"] = {
            "profile": args.feature_profile,
            "flags": list(profile_flags),
            "result": asdict(validate_result),
        }
        if validate_result.outcome != "ok":
            counters["profile_validation_skipped"] += 1
            case["status"] = "profile-validation-skipped"
            cases.append(case)
            continue

        if args.only_profile_delta:
            preceding_profile = "mvp" if args.feature_profile == "wasm2-core" else "wasm2-core"
            preceding_flags = VALIDATION_PROFILES[preceding_profile]
            preceding_result = run_process(
                [str(args.wasm_validate), *preceding_flags, str(wasm_path)],
                source_root,
                args.timeout,
            )
            case["preceding_profile_validation"] = {
                "profile": preceding_profile,
                "flags": list(preceding_flags),
                "result": asdict(preceding_result),
            }
            if preceding_result.outcome == "ok":
                counters["preceding_profile_skipped"] += 1
                case["status"] = "preceding-profile-skipped"
                cases.append(case)
                continue

        uwvm_feature_args = tuple(args.uwvm_feature_arg) or UWVM_FEATURE_ARGS[args.feature_profile]
        if args.mode == "validate":
            engines = {
                "uwvm_int_validation": [
                    str(uwvm_int),
                    *uwvm_feature_args,
                    "--mode",
                    "validation",
                    "--run",
                    str(wasm_path),
                ],
                "uwvm_llvm_validation": [
                    str(uwvm_llvm),
                    *uwvm_feature_args,
                    "--mode",
                    "validation",
                    "--run",
                    str(wasm_path),
                ],
            }
            results = {name: run_process(command, wat_path.parent, args.timeout) for name, command in engines.items()}
            case["engines"] = {name: asdict(result) for name, result in results.items()}
            int_result = results["uwvm_int_validation"]
            llvm_result = results["uwvm_llvm_validation"]
            if int_result.outcome != llvm_result.outcome:
                status = "backend-mismatch"
                counters["backend_mismatch"] += 1
            elif int_result.outcome != "ok":
                status = "validation-rejected"
                counters["validation_rejected"] += 1
            else:
                status = "passed"
                counters["passed"] += 1
            case["status"] = status
            cases.append(case)
            print(
                f"[{index}/{len(selected)}] {status} {relative} "
                f"int={int_result.outcome} llvm={llvm_result.outcome}",
                flush=True,
            )
            continue

        objdump_result = run_process([str(args.wasm_objdump), "-x", str(wasm_path)], source_root, args.timeout)
        case["wasm_objdump"] = asdict(objdump_result)
        objdump_text = objdump_result.stdout + "\n" + objdump_result.stderr
        if objdump_result.outcome != "ok" or not has_binary_entry(objdump_text):
            counters["no_binary_entry_skipped"] += 1
            case["status"] = "no-binary-entry-skipped"
            cases.append(case)
            continue

        engines = {
            "wasmtime": [str(args.wasmtime), str(wasm_path)],
            "uwvm_int_full": [str(uwvm_int), "-Rint", *uwvm_feature_args, "--run", str(wasm_path)],
            "uwvm_llvm_full": [
                str(uwvm_llvm),
                "-Raot",
                "-Rllvm-cache-path",
                "disable",
                *uwvm_feature_args,
                "--run",
                str(wasm_path),
            ],
        }
        results = {name: run_process(command, wat_path.parent, args.timeout) for name, command in engines.items()}
        case["engines"] = {name: asdict(result) for name, result in results.items()}
        int_result = results["uwvm_int_full"]
        llvm_result = results["uwvm_llvm_full"]
        wasmtime_result = results["wasmtime"]

        if not same_result(int_result, llvm_result):
            status = "backend-mismatch"
            counters["backend_mismatch"] += 1
        elif not same_result(int_result, wasmtime_result):
            status = "reference-mismatch"
            counters["reference_mismatch"] += 1
        else:
            status = "passed"
            counters["passed"] += 1
        case["status"] = status
        cases.append(case)

        print(
            f"[{index}/{len(selected)}] {status} {relative} "
            f"wasmtime={wasmtime_result.outcome} int={int_result.outcome} llvm={llvm_result.outcome}",
            flush=True,
        )

    report = {
        "schema": "uwvm2-wasmtime-full-backend-matrix-v2",
        "cpu_affinity": inherited_cpu_affinity(),
        "source_root": str(source_root),
        "uwvm_int": str(uwvm_int),
        "uwvm_llvm": str(uwvm_llvm),
        "mode": args.mode,
        "feature_profile": args.feature_profile,
        "uwvm_feature_args": list(tuple(args.uwvm_feature_arg) or UWVM_FEATURE_ARGS[args.feature_profile]),
        "only_profile_delta": args.only_profile_delta,
        "selection": selection_counts,
        "counters": counters,
        "elapsed_seconds": time.time() - started,
        "cases": cases,
    }
    report_path = work_dir / "report.json"
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"report={report_path}")
    print(json.dumps(counters, sort_keys=True))

    if counters["backend_mismatch"] or counters["validation_rejected"]:
        return 1
    if counters["reference_mismatch"] and not args.allow_reference_mismatch:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
