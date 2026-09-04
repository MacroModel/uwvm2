#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"
cd -- "${ROOT_DIR}"

mkdir -p -- "${ROOT_DIR}/build"
LOCK_DIR="${UWVM_LLVM_JIT_LOCK_DIR:-${ROOT_DIR}/build/llvm_jit.lock}"
if ! mkdir -- "${LOCK_DIR}" 2>/dev/null; then
  echo "ERR: another llvm-jit run appears to be active: ${LOCK_DIR}" >&2
  exit 9
fi
printf '%s\n' "$$" > "${LOCK_DIR}/pid"
cleanup_lock() { rm -rf -- "${LOCK_DIR}"; }
trap cleanup_lock EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

if [[ -n "${UWVM_XMAKE_JOBS:-}" ]]; then
  if [[ ! "${UWVM_XMAKE_JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERR: UWVM_XMAKE_JOBS must be a positive integer, got: ${UWVM_XMAKE_JOBS}" >&2
    exit 2
  fi
  echo "INFO: xmake jobs limited via UWVM_XMAKE_JOBS=${UWVM_XMAKE_JOBS}"
fi

UWVM_XMAKE_MODE="${UWVM_XMAKE_MODE:-debug}"
case "${UWVM_XMAKE_MODE}" in
  debug|release|releasedbg|minsizerel) ;;
  *)
    echo "ERR: UWVM_XMAKE_MODE must be debug, release, releasedbg, or minsizerel; got: ${UWVM_XMAKE_MODE}" >&2
    exit 2
    ;;
esac
echo "INFO: xmake mode = ${UWVM_XMAKE_MODE}"

xmake_build() {
  if [[ -n "${UWVM_XMAKE_JOBS:-}" ]]; then
    xmake b -v -j "${UWVM_XMAKE_JOBS}" "$@"
  else
    xmake b -v "$@"
  fi
}

if [[ "$(uname -s)" == "Darwin" ]]; then
  CLANG_BIN=""
  if [[ -n "${SYSROOT:-}" ]]; then
    TOOLCHAIN_ROOT="$(cd -- "$(dirname -- "${SYSROOT}")" && pwd)"
    CLANG_BIN="${TOOLCHAIN_ROOT}/llvm/bin/clang"
  fi
  if [[ ! -x "${CLANG_BIN}" ]]; then
    CLANG_BIN="$(command -v clang || true)"
  fi
  if [[ -x "${CLANG_BIN}" ]]; then
    CLANG_RUNTIME_DIR="$("${CLANG_BIN}" --print-runtime-dir 2>/dev/null || true)"
    if [[ -n "${CLANG_RUNTIME_DIR}" ]]; then
      export DYLD_LIBRARY_PATH="${CLANG_RUNTIME_DIR}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"
    fi
  fi
fi

COMMON_F_FLAGS=(
  -m "${UWVM_XMAKE_MODE}"
  --use-llvm-compiler=y
  --ccache=n
  --cxflags=-Wno-error
  --enable-test-llvm-jit=y
  --use-cxx-module=n
  --static=none
  --execution-int=uwvm-int
  --execution-jit=llvm
)

if [[ -n "${SYSROOT:-}" ]]; then
  COMMON_F_FLAGS+=("--sysroot=${SYSROOT}")
fi

TARGETS=()
if [[ "$#" -gt 0 ]]; then
  TARGETS=("$@")
else
  while IFS= read -r name; do
    TARGETS+=("${name}")
  done < <(
    {
      printf '%s\n' llvm_jit_verify_compile
      printf '%s\n' call_indirect_encoding_parity
      printf '%s\n' llvm_jit_imported_bulk_memory
      printf '%s\n' llvm_jit_trap_matrix_wat
      printf '%s\n' llvm_jit_unwind_call_stack_wat
      index=0
      find test/0013.uwvm_int/strict -type f -name '*.cc' | sort | while IFS= read -r file; do
        # Bash 3.2 misparses a nested `case ... pattern)` while this loop lives inside the process substitution above.
        # Keep the portable `[[ ... ]]` form so the directed-suite generator also works on the supported Darwin shell.
        if [[ "${file}" == */wasm1p1/uwvm_int_translate_wasm1p1_full_interpreter_strict.cc ||
              "${file}" == */memory/uwvm_int_translate_wasm1p1_bulk_memory_strict.cc ||
              "${file}" == */table/uwvm_int_translate_wasm1p1_externref_table_strict.cc ||
              "${file}" == */table/uwvm_int_translate_wasm1p1_table_ref_bulk_strict.cc ||
              "${file}" == */cf/uwvm_int_translate_if_no_else_identity_strict.cc ||
              "${file}" == */validate/uwvm_int_translate_wasm1p1_simd_basic_strict.cc ||
              "${file}" == */validate/uwvm_int_validate_wasm1p1_validator_alignment_strict.cc ]]; then
          continue
        fi
        index=$((index + 1))
        printf 'lj13s_%03d\n' "${index}"
      done
    } | awk '!seen[$0]++'
  )
fi

if [[ "${#TARGETS[@]}" -eq 0 ]]; then
  echo "ERR: no llvm-jit targets found." >&2
  exit 3
fi

echo "INFO: llvm-jit target count = ${#TARGETS[@]}"
echo "=== llvm-jit full: configure ==="
xmake f -c
xmake f "${COMMON_F_FLAGS[@]}"

for i in "${!TARGETS[@]}"; do
  t="${TARGETS[$i]}"
  printf '=== [%03d/%03d] build %s ===\n' "$((i + 1))" "${#TARGETS[@]}" "$t"
  xmake_build "$t"
  exe="$(xmake show -t "$t" | perl -pe 's/\e\[[0-9;]*m//g' | sed -n 's/^[[:space:]]*targetfile:[[:space:]]*//p' | head -n1 || true)"
  if [[ -z "${exe}" || ! -f "${ROOT_DIR}/${exe}" ]]; then
    echo "ERR: targetfile not found for: ${t}" >&2
    exit 4
  fi
  printf '=== [%03d/%03d] run %s ===\n' "$((i + 1))" "${#TARGETS[@]}" "$t"
  "${ROOT_DIR}/${exe}"
done

echo "OK: llvm-jit full run completed"
