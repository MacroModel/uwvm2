#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Case:
    name: str
    args: tuple[str, ...]


@dataclass(frozen=True)
class Mode:
    name: str
    args: tuple[str, ...]


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _case_root() -> Path:
    return Path(__file__).resolve().parent


def _compile_wat(case_root: Path) -> None:
    script = case_root / "compile_wat.py"
    subprocess.run([sys.executable, str(script)], cwd=case_root, check=True)


def _build_uwvm(repo_root: Path) -> None:
    subprocess.run(["xmake", "build", "uwvm"], cwd=repo_root, check=True)


def _run_uwvm(repo_root: Path, uwvm: str | None, mode: Mode, case: Case) -> subprocess.CompletedProcess[str]:
    if uwvm:
        args = [uwvm, *mode.args, *case.args]
    else:
        args = ["xmake", "run", "uwvm", *mode.args, *case.args]
    return subprocess.run(args, cwd=repo_root, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run small LLVM JIT smoke cases against uwvm int/jit/aot backends.")
    parser.add_argument("--case", default="all", help='Case name, or "all" (default)')
    parser.add_argument(
        "--uwvm",
        default=None,
        help="Path to an existing uwvm binary. Default: use `xmake run uwvm -- ...`.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip `xmake build uwvm` before running cases.",
    )
    args = parser.parse_args()

    repo_root = _repo_root()
    case_root = _case_root()
    wat_dir = case_root / "wat"

    _compile_wat(case_root)
    if not args.skip_build and args.uwvm is None:
        _build_uwvm(repo_root)

    cases = [
        Case(
            name="direct-call-chain",
            args=(
                "--run",
                str((wat_dir / "direct_call_chain.wasm").resolve()),
            ),
        ),
        Case(
            name="memory-grow-roundtrip",
            args=(
                "--run",
                str((wat_dir / "memory_grow_roundtrip.wasm").resolve()),
            ),
        ),
        Case(
            name="call-indirect-dispatch",
            args=(
                "--run",
                str((wat_dir / "call_indirect_dispatch.wasm").resolve()),
            ),
        ),
        Case(
            name="imported-direct-call",
            args=(
                "--wasm-preload-library",
                str((wat_dir / "jit_provider.wasm").resolve()),
                "jit_provider",
                "--wasm-set-main-module-name",
                "jit_consumer",
                "--run",
                str((wat_dir / "jit_consumer.wasm").resolve()),
            ),
        ),
    ]

    modes = [
        Mode(name="int", args=("-Rcc", "int", "-Rcm", "full")),
        Mode(name="runtime-jit", args=("--runtime-jit",)),
        Mode(name="runtime-aot", args=("--runtime-aot",)),
    ]

    selected = cases if args.case == "all" else [c for c in cases if c.name == args.case]
    if not selected:
        sys.stderr.write(f"Unknown --case: {args.case}\n")
        sys.stderr.write("Available:\n")
        for case in cases:
            sys.stderr.write(f"  - {case.name}\n")
        return 2

    failed = 0
    for case in selected:
        for mode in modes:
            proc = _run_uwvm(repo_root=repo_root, uwvm=args.uwvm, mode=mode, case=case)
            if proc.returncode != 0:
                failed += 1
                sys.stderr.write(f"[FAIL] case={case.name} mode={mode.name} returncode={proc.returncode}\n")
                if proc.stdout:
                    sys.stderr.write("---- stdout ----\n")
                    sys.stderr.write(proc.stdout)
                    if not proc.stdout.endswith("\n"):
                        sys.stderr.write("\n")
                if proc.stderr:
                    sys.stderr.write("---- stderr ----\n")
                    sys.stderr.write(proc.stderr)
                    if not proc.stderr.endswith("\n"):
                        sys.stderr.write("\n")
            else:
                sys.stdout.write(f"[OK] case={case.name} mode={mode.name}\n")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
