This directory contains small LLVM JIT smoke cases for `uwvm`.

Purpose:
- verify that `-Rcc int -Rcm full`
- `--runtime-jit`
- `--runtime-aot`

all accept the same small modules for a few key paths:
- local direct call chains
- memory grow + load/store roundtrip
- `call_indirect`
- imported direct calls through the runtime initializer

Usage:
- `python3 test/0014.llvm_jit/compile_wat.py`
- `python3 test/0014.llvm_jit/run_llvm_jit_smoke.py`

Notes:
- The current LLVM backend is a hybrid backend: native wrappers + `uwvm_int` semantic fallback.
- These are smoke tests only; they do not prove full native lowering coverage.
