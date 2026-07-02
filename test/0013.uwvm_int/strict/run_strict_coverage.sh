#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}/../../.." && pwd)"
cd -- "${ROOT_DIR}"

STRICT_DIR="test/0013.uwvm_int/strict"

mkdir -p -- "${ROOT_DIR}/build"
LOCK_DIR="${UWVM_STRICT_LOCK_DIR:-${ROOT_DIR}/build/uwvm_int_strict.lock}"
if ! mkdir -- "${LOCK_DIR}" 2>/dev/null; then
  echo "ERR: another uwvm_int strict run appears to be active: ${LOCK_DIR}" >&2
  echo "ERR: remove the lock only after confirming no xmake/uwvm_int test process is still running." >&2
  exit 9
fi
printf '%s\n' "$$" > "${LOCK_DIR}/pid"
cleanup_lock() {
  rm -rf -- "${LOCK_DIR}"
}
trap cleanup_lock EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

# Optional: limit xmake parallel jobs (useful on memory-limited machines, e.g. macOS).
# Example: export UWVM_XMAKE_JOBS=4
if [[ -n "${UWVM_XMAKE_JOBS:-}" ]]; then
  if [[ "${UWVM_XMAKE_JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "INFO: xmake jobs limited via UWVM_XMAKE_JOBS=${UWVM_XMAKE_JOBS}"
  else
    echo "ERR: UWVM_XMAKE_JOBS must be a positive integer, got: ${UWVM_XMAKE_JOBS}" >&2
    exit 2
  fi
fi

xmake_build() {
  if [[ -n "${UWVM_XMAKE_JOBS:-}" ]]; then
    xmake b -v -j "${UWVM_XMAKE_JOBS}" "$@"
  else
    xmake b -v "$@"
  fi
}

find_llvm_tool() {
  local tool="$1"
  local candidate=""
  if [[ -n "${UWVM_LLVM_BIN:-}" ]]; then
    candidate="${UWVM_LLVM_BIN}/${tool}"
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  fi
  if [[ -n "${SYSROOT:-}" ]]; then
    local toolchain_root
    toolchain_root="$(cd -- "$(dirname -- "${SYSROOT}")" && pwd)"
    candidate="${toolchain_root}/llvm/bin/${tool}"
    if [[ -x "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  fi
  command -v "${tool}" 2>/dev/null || true
}

find_clang_bin() {
  if [[ -n "${UWVM_CLANG_BIN:-}" && -x "${UWVM_CLANG_BIN}" ]]; then
    printf '%s\n' "${UWVM_CLANG_BIN}"
    return 0
  fi
  find_llvm_tool clang
}

LLVM_PROFDATA="${UWVM_LLVM_PROFDATA:-$(find_llvm_tool llvm-profdata)}"
LLVM_COV="${UWVM_LLVM_COV:-$(find_llvm_tool llvm-cov)}"
CLANG_BIN="$(find_clang_bin)"

if [[ ! -x "${LLVM_PROFDATA}" || ! -x "${LLVM_COV}" ]]; then
  echo "ERR: llvm-profdata/llvm-cov not found. Set UWVM_LLVM_BIN, UWVM_LLVM_PROFDATA/UWVM_LLVM_COV, or put them in PATH." >&2
  exit 3
fi

# macOS: dyld may not find clang runtime libraries (profile runtime, etc).
if [[ "$(uname -s)" == "Darwin" ]]; then
  if [[ -x "${CLANG_BIN}" ]]; then
    CLANG_RUNTIME_DIR="$("${CLANG_BIN}" --print-runtime-dir 2>/dev/null || true)"
    if [[ -n "${CLANG_RUNTIME_DIR}" ]]; then
      export DYLD_LIBRARY_PATH="${CLANG_RUNTIME_DIR}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"
    fi
  fi
fi

COMMON_F_FLAGS=(
  -m debug
  --use-llvm-compiler=y
  --ccache=n
  --test-libfuzzer=y
  --enable-test-uwvm-int=y
  --use-cxx-module=n
  --static=none
  --execution-int=uwvm-int
  "--enable-uwvm-int-combine-ops=${UWVM_STRICT_COVERAGE_COMBINE_MODE:-heavy}"
  "--enable-uwvm-int-delay-local=${UWVM_STRICT_COVERAGE_DELAY_MODE:-heavy}"
)

if [[ -n "${SYSROOT:-}" ]]; then
  COMMON_F_FLAGS+=("--sysroot=${SYSROOT}")
fi

export UWVM2TEST_ABI_MODES="${UWVM_STRICT_ABI_MODES:-all}"
export UWVM2TEST_MATRIX_LEVEL="${UWVM_STRICT_MATRIX_LEVEL:-coverage}"
COVER_INCLUDE_REGEX="${UWVM_STRICT_COVERAGE_REGEX:-.*src/uwvm2/runtime/compiler/uwvm_int/(compile_all_from_uwvm|optable)/.*}"

echo "INFO: strict coverage ABI modes = ${UWVM2TEST_ABI_MODES}"
echo "INFO: strict coverage matrix level = ${UWVM2TEST_MATRIX_LEVEL}"
if [[ -n "${SYSROOT:-}" ]]; then
  echo "INFO: strict coverage sysroot = ${SYSROOT}"
else
  echo "INFO: strict coverage sysroot = <toolchain default>"
fi
echo "INFO: strict coverage llvm-profdata = ${LLVM_PROFDATA}"
echo "INFO: strict coverage llvm-cov = ${LLVM_COV}"
echo "INFO: strict coverage regex = ${COVER_INCLUDE_REGEX}"
echo "INFO: strict coverage min line = ${UWVM_STRICT_COVERAGE_MIN_LINE:-90}"
echo "INFO: strict coverage min function = ${UWVM_STRICT_COVERAGE_MIN_FUNCTION:-90}"
if [[ -n "${UWVM_STRICT_COVERAGE_MIN_BRANCH:-}" ]]; then
  echo "INFO: strict coverage min branch = ${UWVM_STRICT_COVERAGE_MIN_BRANCH}"
else
  echo "INFO: strict coverage min branch = <report-only>"
fi

STRICT_NO_WERROR_CXFLAGS="-Wno-error"
COVER_CXFLAGS="${STRICT_NO_WERROR_CXFLAGS} -fprofile-instr-generate -fcoverage-mapping"
COVER_LDFLAGS="-fprofile-instr-generate -fcoverage-mapping"

COVER_BATCH_SIZE="${UWVM_STRICT_COVERAGE_BATCH_SIZE:-8}"
if [[ ! "${COVER_BATCH_SIZE}" =~ ^[0-9]+$ ]]; then
  echo "ERR: UWVM_STRICT_COVERAGE_BATCH_SIZE must be a non-negative integer, got: ${COVER_BATCH_SIZE}" >&2
  exit 4
fi

STRICT_TARGETS=()
FULL_TARGET_SET=false
SKIP_TARGET_REGEX="${UWVM_STRICT_COVERAGE_SKIP_TARGET_REGEX:-^uwvm_int_translate_validator_parity_strict$}"
if [[ "$#" -gt 0 ]]; then
  STRICT_TARGETS=("$@")
else
  FULL_TARGET_SET=true
  while IFS= read -r f; do
    target_name="$(basename -- "${f}" .cc)"
    if [[ -n "${SKIP_TARGET_REGEX}" && "${target_name}" =~ ${SKIP_TARGET_REGEX} ]]; then
      echo "INFO: strict coverage skip target = ${target_name}"
      continue
    fi
    STRICT_TARGETS+=("${target_name}")
  done < <(find "${STRICT_DIR}" -type f -name '*.cc' | sort)
fi

if [[ "${#STRICT_TARGETS[@]}" -eq 0 ]]; then
  echo "ERR: no strict targets found." >&2
  exit 4
fi

if [[ -n "${UWVM_STRICT_COVERAGE_START_AT:-}" && -n "${UWVM_STRICT_COVERAGE_START_AFTER:-}" ]]; then
  echo "ERR: set only one of UWVM_STRICT_COVERAGE_START_AT or UWVM_STRICT_COVERAGE_START_AFTER" >&2
  exit 4
fi

if [[ -n "${UWVM_STRICT_COVERAGE_START_AT:-}" || -n "${UWVM_STRICT_COVERAGE_START_AFTER:-}" ]]; then
  START_TARGET="${UWVM_STRICT_COVERAGE_START_AT:-${UWVM_STRICT_COVERAGE_START_AFTER}}"
  START_MATCHED=false
  FILTERED_TARGETS=()
  for t in "${STRICT_TARGETS[@]}"; do
    if [[ "${START_MATCHED}" == "false" ]]; then
      if [[ "${t}" == "${START_TARGET}" ]]; then
        START_MATCHED=true
        if [[ -n "${UWVM_STRICT_COVERAGE_START_AT:-}" ]]; then
          FILTERED_TARGETS+=("${t}")
        fi
      fi
      continue
    fi
    FILTERED_TARGETS+=("${t}")
  done
  if [[ "${START_MATCHED}" == "false" ]]; then
    echo "ERR: start target not found: ${START_TARGET}" >&2
    exit 4
  fi
  STRICT_TARGETS=("${FILTERED_TARGETS[@]}")
fi

if [[ -n "${UWVM_STRICT_COVERAGE_LIMIT:-}" ]]; then
  if [[ ! "${UWVM_STRICT_COVERAGE_LIMIT}" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERR: UWVM_STRICT_COVERAGE_LIMIT must be a positive integer, got: ${UWVM_STRICT_COVERAGE_LIMIT}" >&2
    exit 4
  fi
  if [[ "${#STRICT_TARGETS[@]}" -gt "${UWVM_STRICT_COVERAGE_LIMIT}" ]]; then
    STRICT_TARGETS=("${STRICT_TARGETS[@]:0:UWVM_STRICT_COVERAGE_LIMIT}")
  fi
fi

if [[ "${#STRICT_TARGETS[@]}" -eq 0 ]]; then
  echo "ERR: no strict targets selected after start/limit filters." >&2
  exit 4
fi

if [[ -n "${UWVM_STRICT_COVERAGE_START_AT:-}" ]]; then
  echo "INFO: strict coverage start at = ${UWVM_STRICT_COVERAGE_START_AT}"
fi
if [[ -n "${UWVM_STRICT_COVERAGE_START_AFTER:-}" ]]; then
  echo "INFO: strict coverage start after = ${UWVM_STRICT_COVERAGE_START_AFTER}"
fi
if [[ -n "${UWVM_STRICT_COVERAGE_LIMIT:-}" ]]; then
  echo "INFO: strict coverage target limit = ${UWVM_STRICT_COVERAGE_LIMIT}"
fi
echo "INFO: strict coverage selected targets = ${#STRICT_TARGETS[@]}"
echo "INFO: strict coverage batch size = ${COVER_BATCH_SIZE}"
if [[ "${FULL_TARGET_SET}" == "true" && -n "${SKIP_TARGET_REGEX}" ]]; then
  echo "INFO: strict coverage skip regex = ${SKIP_TARGET_REGEX}"
fi
CLEAN_OBJS_AFTER_BATCH="${UWVM_STRICT_COVERAGE_CLEAN_OBJS_AFTER_BATCH:-1}"
if [[ "${CLEAN_OBJS_AFTER_BATCH}" != "0" && "${CLEAN_OBJS_AFTER_BATCH}" != "1" ]]; then
  echo "ERR: UWVM_STRICT_COVERAGE_CLEAN_OBJS_AFTER_BATCH must be 0 or 1, got: ${CLEAN_OBJS_AFTER_BATCH}" >&2
  exit 4
fi
echo "INFO: strict coverage clean objs/cache after batch = ${CLEAN_OBJS_AFTER_BATCH}"
STREAMING_COVERAGE="${UWVM_STRICT_COVERAGE_STREAMING:-auto}"
if [[ "${STREAMING_COVERAGE}" == "auto" ]]; then
  if [[ "${FULL_TARGET_SET}" == "true" ]]; then
    STREAMING_COVERAGE=1
  else
    STREAMING_COVERAGE=0
  fi
fi
if [[ "${STREAMING_COVERAGE}" != "0" && "${STREAMING_COVERAGE}" != "1" ]]; then
  echo "ERR: UWVM_STRICT_COVERAGE_STREAMING must be 0, 1, or auto, got: ${UWVM_STRICT_COVERAGE_STREAMING:-auto}" >&2
  exit 4
fi
echo "INFO: strict coverage streaming aggregation = ${STREAMING_COVERAGE}"
GROUP_BY_DIR="${UWVM_STRICT_COVERAGE_GROUP_BY_DIR:-auto}"
if [[ "${GROUP_BY_DIR}" == "auto" ]]; then
  GROUP_BY_DIR=0
fi
if [[ "${GROUP_BY_DIR}" != "0" && "${GROUP_BY_DIR}" != "1" ]]; then
  echo "ERR: UWVM_STRICT_COVERAGE_GROUP_BY_DIR must be 0, 1, or auto, got: ${UWVM_STRICT_COVERAGE_GROUP_BY_DIR:-auto}" >&2
  exit 4
fi
GROUP_TARGET_MAX="${UWVM_STRICT_COVERAGE_GROUP_TARGET_MAX:-16}"
if [[ ! "${GROUP_TARGET_MAX}" =~ ^[1-9][0-9]*$ ]]; then
  echo "ERR: UWVM_STRICT_COVERAGE_GROUP_TARGET_MAX must be a positive integer, got: ${GROUP_TARGET_MAX}" >&2
  exit 4
fi
echo "INFO: strict coverage group by directory = ${GROUP_BY_DIR}"
echo "INFO: strict coverage group target max = ${GROUP_TARGET_MAX}"

COVER_DIR="${UWVM_STRICT_COVERAGE_DIR:-${ROOT_DIR}/build/uwvm_int_strict_coverage}"
PROFRAW_DIR="${COVER_DIR}/profraw"
BATCH_EXPORT_DIR="${COVER_DIR}/batch_exports"
BATCH_PROFDATA_DIR="${COVER_DIR}/batch_profdata"
HTML_DIR="${COVER_DIR}/html"
PROFDATA_FILE="${COVER_DIR}/merged.profdata"

rm -rf -- "${COVER_DIR}"
mkdir -p -- "${PROFRAW_DIR}" "${BATCH_EXPORT_DIR}" "${BATCH_PROFDATA_DIR}"

echo "=== uwvm_int strict coverage: configure (combine=${UWVM_STRICT_COVERAGE_COMBINE_MODE:-heavy}, delay=${UWVM_STRICT_COVERAGE_DELAY_MODE:-heavy}) ==="
xmake f -c
xmake f "${COMMON_F_FLAGS[@]}" --cxflags="${COVER_CXFLAGS}" --ldflags="${COVER_LDFLAGS}"

export LLVM_PROFILE_FILE="${PROFRAW_DIR}/%p.profraw"

echo "=== uwvm_int strict coverage: build+run strict targets (profile) ==="
targetfile_for() {
  local target="$1"
  local tf
  tf="$(xmake show -t "${target}" | perl -pe 's/\e\[[0-9;]*m//g' | sed -n 's/^[[:space:]]*targetfile:[[:space:]]*//p' | head -n1 || true)"
  if [[ -z "${tf}" ]]; then
    echo "ERR: failed to query targetfile for: ${target}" >&2
    exit 6
  fi
  if [[ ! -f "${ROOT_DIR}/${tf}" ]]; then
    echo "ERR: targetfile not found: ${ROOT_DIR}/${tf}" >&2
    exit 7
  fi
  printf '%s\n' "${ROOT_DIR}/${tf}"
}

run_strict_target() {
  local target="$1"
  local targetfile
  local profile_dir="${CURRENT_PROFRAW_DIR:-${PROFRAW_DIR}}"
  targetfile="$(targetfile_for "${target}")"
  LLVM_PROFILE_FILE="${profile_dir}/${target}_%p.profraw" "${targetfile}"
}

clean_after_batch() {
  if [[ "${CLEAN_OBJS_AFTER_BATCH}" == "1" ]]; then
    rm -rf -- "${ROOT_DIR}/build/.objs" "${ROOT_DIR}/build/.build_cache"
  fi
}

build_run_batch() {
  local batch_idx="$1"
  shift
  local batch=("$@")
  echo "--- strict coverage build batch ${batch_idx}: ${#batch[@]} targets ---"
  for t in "${batch[@]}"; do
    xmake_build "${t}"
  done
  echo "--- strict coverage run batch ${batch_idx}: ${#batch[@]} targets ---"
  for t in "${batch[@]}"; do
    run_strict_target "${t}"
  done
  clean_after_batch
}

stream_build_run_export_batch() {
  local batch_idx="$1"
  shift
  local batch=("$@")
  local batch_tag
  batch_tag="$(printf '%04d' "${batch_idx}")"
  local batch_profraw_dir="${PROFRAW_DIR}/batch_${batch_tag}"
  local batch_profdata="${BATCH_PROFDATA_DIR}/batch_${batch_tag}.profdata"
  local batch_export="${BATCH_EXPORT_DIR}/batch_${batch_tag}.json"
  local batch_report="${BATCH_EXPORT_DIR}/batch_${batch_tag}.txt"
  local batch_objects=()

  rm -rf -- "${batch_profraw_dir}"
  mkdir -p -- "${batch_profraw_dir}"

  echo "--- strict coverage build batch ${batch_idx}: ${#batch[@]} targets ---"
  for t in "${batch[@]}"; do
    xmake_build "${t}"
  done

  for t in "${batch[@]}"; do
    batch_objects+=(--object "$(targetfile_for "${t}")")
  done

  echo "--- strict coverage run batch ${batch_idx}: ${#batch[@]} targets ---"
  CURRENT_PROFRAW_DIR="${batch_profraw_dir}"
  export CURRENT_PROFRAW_DIR
  for t in "${batch[@]}"; do
    run_strict_target "${t}"
  done
  unset CURRENT_PROFRAW_DIR

  if ! ls "${batch_profraw_dir}"/*.profraw >/dev/null 2>&1; then
    echo "ERR: no .profraw generated for batch ${batch_idx} at: ${batch_profraw_dir}" >&2
    exit 5
  fi

  echo "--- strict coverage export batch ${batch_idx}: ${#batch[@]} targets ---"
  "${LLVM_PROFDATA}" merge -sparse "${batch_profraw_dir}"/*.profraw -o "${batch_profdata}"
  "${LLVM_COV}" export \
    -instr-profile="${batch_profdata}" \
    --include-filename-regex="${COVER_INCLUDE_REGEX}" \
    "${batch_objects[@]}" > "${batch_export}"
  "${LLVM_COV}" report \
    -instr-profile="${batch_profdata}" \
    --include-filename-regex="${COVER_INCLUDE_REGEX}" \
    "${batch_objects[@]}" > "${batch_report}"

  for ((j = 0; j < ${#batch_objects[@]}; j += 2)); do
    rm -f -- "${batch_objects[j + 1]}"
  done
  rm -rf -- "${batch_profraw_dir}"
  clean_after_batch
}

stream_build_run_export_group_batch() {
  local batch_idx="$1"
  local group_dir="$2"
  shift 2
  local batch=("$@")
  local batch_tag
  batch_tag="$(printf '%04d' "${batch_idx}")"
  local batch_profraw_dir="${PROFRAW_DIR}/batch_${batch_tag}"
  local batch_profdata="${BATCH_PROFDATA_DIR}/batch_${batch_tag}.profdata"
  local batch_export="${BATCH_EXPORT_DIR}/batch_${batch_tag}.json"
  local batch_report="${BATCH_EXPORT_DIR}/batch_${batch_tag}.txt"
  local batch_objects=()

  rm -rf -- "${batch_profraw_dir}"
  mkdir -p -- "${batch_profraw_dir}"

  echo "--- strict coverage build group batch ${batch_idx}: ${group_dir} (${#batch[@]} targets) ---"
  xmake_build -g "${group_dir}/*"

  for t in "${batch[@]}"; do
    batch_objects+=(--object "$(targetfile_for "${t}")")
  done

  echo "--- strict coverage run group batch ${batch_idx}: ${#batch[@]} targets ---"
  CURRENT_PROFRAW_DIR="${batch_profraw_dir}"
  export CURRENT_PROFRAW_DIR
  for t in "${batch[@]}"; do
    run_strict_target "${t}"
  done
  unset CURRENT_PROFRAW_DIR

  if ! ls "${batch_profraw_dir}"/*.profraw >/dev/null 2>&1; then
    echo "ERR: no .profraw generated for batch ${batch_idx} at: ${batch_profraw_dir}" >&2
    exit 5
  fi

  echo "--- strict coverage export group batch ${batch_idx}: ${#batch[@]} targets ---"
  "${LLVM_PROFDATA}" merge -sparse "${batch_profraw_dir}"/*.profraw -o "${batch_profdata}"
  "${LLVM_COV}" export \
    -instr-profile="${batch_profdata}" \
    --include-filename-regex="${COVER_INCLUDE_REGEX}" \
    "${batch_objects[@]}" > "${batch_export}"
  "${LLVM_COV}" report \
    -instr-profile="${batch_profdata}" \
    --include-filename-regex="${COVER_INCLUDE_REGEX}" \
    "${batch_objects[@]}" > "${batch_report}"

  for ((j = 0; j < ${#batch_objects[@]}; j += 2)); do
    rm -f -- "${batch_objects[j + 1]}"
  done
  rm -rf -- "${batch_profraw_dir}"
  clean_after_batch
}

iterate_batches() {
  local runner="$1"
  if [[ "${COVER_BATCH_SIZE}" == "0" || "${#STRICT_TARGETS[@]}" -le "${COVER_BATCH_SIZE}" ]]; then
    "${runner}" 1 "${STRICT_TARGETS[@]}"
  else
    batch_idx=0
    for ((i = 0; i < ${#STRICT_TARGETS[@]}; i += COVER_BATCH_SIZE)); do
      batch_idx=$((batch_idx + 1))
      batch=("${STRICT_TARGETS[@]:i:COVER_BATCH_SIZE}")
      "${runner}" "${batch_idx}" "${batch[@]}"
    done
  fi
}

iterate_streaming_full_by_dir() {
  local batch_idx=0
  local dir
  while IFS= read -r dir; do
    local dir_all=()
    local dir_targets=()
    local f
    while IFS= read -r f; do
      local target_name
      target_name="$(basename -- "${f}" .cc)"
      dir_all+=("${target_name}")
      if [[ -n "${SKIP_TARGET_REGEX}" && "${target_name}" =~ ${SKIP_TARGET_REGEX} ]]; then
        continue
      fi
      dir_targets+=("${target_name}")
    done < <(find "${dir}" -type f -name '*.cc' | sort)

    if [[ "${#dir_targets[@]}" -eq 0 ]]; then
      continue
    fi

    if [[ "${#dir_all[@]}" -eq "${#dir_targets[@]}" && "${#dir_targets[@]}" -le "${GROUP_TARGET_MAX}" ]]; then
      batch_idx=$((batch_idx + 1))
      stream_build_run_export_group_batch "${batch_idx}" "${dir}" "${dir_targets[@]}"
    elif [[ "${COVER_BATCH_SIZE}" == "0" || "${#dir_targets[@]}" -le "${COVER_BATCH_SIZE}" ]]; then
      batch_idx=$((batch_idx + 1))
      stream_build_run_export_batch "${batch_idx}" "${dir_targets[@]}"
    else
      local i
      for ((i = 0; i < ${#dir_targets[@]}; i += COVER_BATCH_SIZE)); do
        batch_idx=$((batch_idx + 1))
        batch=("${dir_targets[@]:i:COVER_BATCH_SIZE}")
        stream_build_run_export_batch "${batch_idx}" "${batch[@]}"
      done
    fi
  done < <(find "${STRICT_DIR}" -mindepth 1 -maxdepth 1 -type d | sort)
}

if [[ "${STREAMING_COVERAGE}" == "1" ]]; then
  if [[ "${FULL_TARGET_SET}" == "true" && "${GROUP_BY_DIR}" == "1" ]]; then
    iterate_streaming_full_by_dir
  else
    iterate_batches stream_build_run_export_batch
  fi

  echo "=== uwvm_int strict coverage: aggregate streaming batch exports ==="
  python3 - "${BATCH_EXPORT_DIR}" \
    "${COVER_DIR}/uwvm_int_compiler_export.json" \
    "${COVER_DIR}/uwvm_int_compiler_report.txt" \
    "${COVER_DIR}/uwvm_int_compiler_totals.json" \
    "${COVER_DIR}/uwvm_int_compiler_totals.txt" \
    "${UWVM_STRICT_COVERAGE_MIN_LINE:-90}" \
    "${UWVM_STRICT_COVERAGE_MIN_FUNCTION:-90}" \
    "${UWVM_STRICT_COVERAGE_MIN_BRANCH:-}" \
    "${COVER_INCLUDE_REGEX}" <<'PY'
import glob
import json
import math
import os
import re
import sys

(
    batch_dir,
    export_out,
    report_out,
    totals_json_out,
    totals_txt_out,
    min_line_s,
    min_func_s,
    min_branch_s,
    include_regex,
) = sys.argv[1:10]

include_re = re.compile(include_regex)

def parse_min(name: str, text: str, default: float | None) -> float | None:
    if text == "":
        return default
    try:
        value = float(text)
    except ValueError:
        raise SystemExit(f"ERR: {name} must be a number, got: {text!r}")
    if not math.isfinite(value) or value < 0.0 or value > 100.0:
        raise SystemExit(f"ERR: {name} must be in [0, 100], got: {text!r}")
    return value

min_line = parse_min("UWVM_STRICT_COVERAGE_MIN_LINE", min_line_s, 90.0)
min_func = parse_min("UWVM_STRICT_COVERAGE_MIN_FUNCTION", min_func_s, 90.0)
min_branch = parse_min("UWVM_STRICT_COVERAGE_MIN_BRANCH", min_branch_s, None)

class FileCov:
    __slots__ = ("line_count", "max_line_covered", "covered_lines", "branch_outcomes")

    def __init__(self) -> None:
        self.line_count = 0
        self.max_line_covered = 0
        self.covered_lines: set[int] = set()
        self.branch_outcomes: dict[tuple[int, int, int, int], list[bool]] = {}

files: dict[str, FileCov] = {}
functions: dict[tuple, bool] = {}

def mark_lines_from_segments(dst: FileCov, segments: list[list]) -> None:
    if not segments:
        return
    ordered = sorted(segments, key=lambda s: (int(s[0]), int(s[1])))
    prev = None
    for seg in ordered:
        line = int(seg[0])
        col = int(seg[1])
        if prev is not None:
            pline, pcol, pcount, phas_count, pis_gap = prev
            if phas_count and not pis_gap:
                end_line = line
                if end_line >= pline:
                    for covered_line in range(pline, end_line + 1):
                        if pcount > 0:
                            dst.covered_lines.add(covered_line)
        has_count = bool(seg[3]) if len(seg) > 3 else False
        is_gap = bool(seg[5]) if len(seg) > 5 else False
        prev = (line, col, int(seg[2]), has_count, is_gap)

def summary_count(summary: dict, group: str, key: str) -> int:
    entry = summary.get(group)
    if isinstance(entry, dict):
        return int(entry.get(key, 0))
    return 0

batch_paths = sorted(glob.glob(os.path.join(batch_dir, "batch_*.json")))
if not batch_paths:
    raise SystemExit(f"ERR: no batch coverage exports found in: {batch_dir}")

for path in batch_paths:
    with open(path, "r", encoding="utf-8") as f:
        doc = json.load(f)
    data = doc.get("data", [{}])[0]
    for fdoc in data.get("files", []):
        filename = fdoc.get("filename")
        if not isinstance(filename, str) or not include_re.search(filename):
            continue
        dst = files.setdefault(filename, FileCov())
        summary = fdoc.get("summary", {})
        dst.line_count = max(dst.line_count, summary_count(summary, "lines", "count"))
        dst.max_line_covered = max(dst.max_line_covered, summary_count(summary, "lines", "covered"))
        mark_lines_from_segments(dst, fdoc.get("segments", []))
        for br in fdoc.get("branches", []):
            if len(br) < 6:
                continue
            key = (int(br[0]), int(br[1]), int(br[2]), int(br[3]))
            state = dst.branch_outcomes.setdefault(key, [False, False])
            state[0] = state[0] or int(br[4]) > 0
            state[1] = state[1] or int(br[5]) > 0
    for fn in data.get("functions", []):
        filenames = tuple(x for x in fn.get("filenames", []) if isinstance(x, str) and include_re.search(x))
        if not filenames:
            continue
        regions = fn.get("regions", [])
        # Match llvm-cov report's function granularity: source file + function source range.
        # Function names include template arguments and can explode into instantiation coverage.
        first_region = tuple(int(v) for v in regions[0][:4]) if regions else ()
        key = (filenames, first_region)
        functions[key] = functions.get(key, False) or int(fn.get("count", 0)) > 0

file_rows = []
total_lines = 0
covered_lines = 0
total_branches = 0
covered_branches = 0

for filename, cov in sorted(files.items()):
    file_line_count = cov.line_count
    file_covered = max(len(cov.covered_lines), cov.max_line_covered)
    if file_line_count:
        file_covered = min(file_covered, file_line_count)
    total_lines += file_line_count
    covered_lines += file_covered
    file_branch_total = 2 * len(cov.branch_outcomes)
    file_branch_covered = sum(int(state[0]) + int(state[1]) for state in cov.branch_outcomes.values())
    total_branches += file_branch_total
    covered_branches += file_branch_covered
    file_rows.append((filename, file_line_count, file_covered, file_branch_total, file_branch_covered))

total_functions = len(functions)
covered_functions = sum(1 for covered in functions.values() if covered)

def pct(covered: int, total: int) -> float:
    return 100.0 if total == 0 else covered * 100.0 / total

line_pct = pct(covered_lines, total_lines)
func_pct = pct(covered_functions, total_functions)
branch_pct = pct(covered_branches, total_branches)

totals = {
    "lines": {
        "count": total_lines,
        "covered": covered_lines,
        "notcovered": max(total_lines - covered_lines, 0),
        "percent": line_pct,
    },
    "functions": {
        "count": total_functions,
        "covered": covered_functions,
        "notcovered": max(total_functions - covered_functions, 0),
        "percent": func_pct,
    },
    "branches": {
        "count": total_branches,
        "covered": covered_branches,
        "notcovered": max(total_branches - covered_branches, 0),
        "percent": branch_pct,
    },
    "thresholds": {
        "line": min_line,
        "function": min_func,
        "branch": min_branch,
    },
    "streaming_batches": len(batch_paths),
    "coverage_method": "streaming_union",
}

with open(totals_json_out, "w", encoding="utf-8") as f:
    json.dump(totals, f, indent=2, sort_keys=True)
    f.write("\n")

with open(totals_txt_out, "w", encoding="utf-8") as f:
    f.write(f"lines={line_pct:.2f}% min={min_line:.2f}% covered={covered_lines} count={total_lines}\n")
    f.write(f"functions={func_pct:.2f}% min={min_func:.2f}% covered={covered_functions} count={total_functions}\n")
    if min_branch is None:
        f.write(f"branches={branch_pct:.2f}% min=<report-only> covered={covered_branches} count={total_branches}\n")
    else:
        f.write(f"branches={branch_pct:.2f}% min={min_branch:.2f}% covered={covered_branches} count={total_branches}\n")
    f.write(f"streaming_batches={len(batch_paths)}\n")

with open(report_out, "w", encoding="utf-8") as f:
    f.write("Filename,Lines,CoveredLines,LinePercent,Branches,CoveredBranches,BranchPercent\n")
    for filename, line_count, line_covered, branch_count, branch_covered in file_rows:
        f.write(
            f"{filename},{line_count},{line_covered},{pct(line_covered, line_count):.2f}%,"
            f"{branch_count},{branch_covered},{pct(branch_covered, branch_count):.2f}%\n"
        )
    f.write(
        f"TOTAL,{total_lines},{covered_lines},{line_pct:.2f}%,"
        f"{total_branches},{covered_branches},{branch_pct:.2f}%\n"
    )

with open(export_out, "w", encoding="utf-8") as f:
    json.dump({"type": "uwvm-int-strict-streaming-coverage", "totals": totals}, f, indent=2, sort_keys=True)
    f.write("\n")

print(f"coverage lines={line_pct:.2f}% functions={func_pct:.2f}% branches={branch_pct:.2f}%")
failures = []
if line_pct + 1e-9 < min_line:
    failures.append(f"line coverage {line_pct:.2f}% < {min_line:.2f}%")
if func_pct + 1e-9 < min_func:
    failures.append(f"function coverage {func_pct:.2f}% < {min_func:.2f}%")
if min_branch is not None and branch_pct + 1e-9 < min_branch:
    failures.append(f"branch coverage {branch_pct:.2f}% < {min_branch:.2f}%")
if failures:
    for failure in failures:
        print(f"ERR: {failure}", file=sys.stderr)
    raise SystemExit(8)
PY
  echo "OK: streaming coverage report written to: ${COVER_DIR}/uwvm_int_compiler_report.txt"
  echo "OK: streaming coverage totals written to: ${COVER_DIR}/uwvm_int_compiler_totals.txt"
  exit 0
fi

iterate_batches build_run_batch

if ! ls "${PROFRAW_DIR}"/*.profraw >/dev/null 2>&1; then
  echo "ERR: no .profraw generated at: ${PROFRAW_DIR}" >&2
  exit 5
fi

echo "=== uwvm_int strict coverage: merge profraw -> profdata ==="
"${LLVM_PROFDATA}" merge -sparse "${PROFRAW_DIR}"/*.profraw -o "${PROFDATA_FILE}"

echo "=== uwvm_int strict coverage: generate html for compile_all_from_uwvm + optable ==="
mkdir -p -- "${HTML_DIR}"

OBJECT_ARGS=()
for t in "${STRICT_TARGETS[@]}"; do
  OBJECT_ARGS+=(--object "$(targetfile_for "${t}")")
done

"${LLVM_COV}" show \
  -instr-profile="${PROFDATA_FILE}" \
  --format=html \
  --output-dir="${HTML_DIR}" \
  --show-region-summary \
  --show-branch-summary \
  --include-filename-regex="${COVER_INCLUDE_REGEX}" \
  "${OBJECT_ARGS[@]}" >/dev/null

echo "OK: coverage html written to: ${HTML_DIR}"

echo "=== uwvm_int strict coverage: export json for compile_all_from_uwvm + optable ==="
"${LLVM_COV}" export \
  -instr-profile="${PROFDATA_FILE}" \
  --include-filename-regex="${COVER_INCLUDE_REGEX}" \
  "${OBJECT_ARGS[@]}" > "${COVER_DIR}/uwvm_int_compiler_export.json"

echo "OK: coverage json written to: ${COVER_DIR}/uwvm_int_compiler_export.json"

echo "=== uwvm_int strict coverage: text summary for compile_all_from_uwvm + optable ==="
"${LLVM_COV}" report \
  -instr-profile="${PROFDATA_FILE}" \
  --include-filename-regex="${COVER_INCLUDE_REGEX}" \
  "${OBJECT_ARGS[@]}" \
  > "${COVER_DIR}/uwvm_int_compiler_report.txt"

echo "OK: coverage report written to: ${COVER_DIR}/uwvm_int_compiler_report.txt"

echo "=== uwvm_int strict coverage: enforce coverage gate ==="
python3 - "${COVER_DIR}/uwvm_int_compiler_export.json" \
  "${COVER_DIR}/uwvm_int_compiler_totals.json" \
  "${COVER_DIR}/uwvm_int_compiler_totals.txt" \
  "${UWVM_STRICT_COVERAGE_MIN_LINE:-90}" \
  "${UWVM_STRICT_COVERAGE_MIN_FUNCTION:-90}" \
  "${UWVM_STRICT_COVERAGE_MIN_BRANCH:-}" <<'PY'
import json
import math
import sys

export_path, totals_json_path, totals_txt_path, min_line_s, min_func_s, min_branch_s = sys.argv[1:7]

def parse_min(name: str, text: str, default: float | None) -> float | None:
    if text == "":
        return default
    try:
        value = float(text)
    except ValueError:
        raise SystemExit(f"ERR: {name} must be a number, got: {text!r}")
    if not math.isfinite(value) or value < 0.0 or value > 100.0:
        raise SystemExit(f"ERR: {name} must be in [0, 100], got: {text!r}")
    return value

min_line = parse_min("UWVM_STRICT_COVERAGE_MIN_LINE", min_line_s, 90.0)
min_func = parse_min("UWVM_STRICT_COVERAGE_MIN_FUNCTION", min_func_s, 90.0)
min_branch = parse_min("UWVM_STRICT_COVERAGE_MIN_BRANCH", min_branch_s, None)

with open(export_path, "r", encoding="utf-8") as f:
    doc = json.load(f)

totals = doc.get("data", [{}])[0].get("totals")
if not isinstance(totals, dict):
    raise SystemExit("ERR: llvm-cov export did not contain data[0].totals")

def pct(group: str) -> float:
    entry = totals.get(group)
    if not isinstance(entry, dict):
        raise SystemExit(f"ERR: llvm-cov totals missing {group!r}")
    value = entry.get("percent")
    if value is None:
        count = float(entry.get("count", 0))
        covered = float(entry.get("covered", 0))
        value = 100.0 if count == 0.0 else covered * 100.0 / count
    return float(value)

summary = {
    "regions": totals.get("regions", {}),
    "functions": totals.get("functions", {}),
    "instantiations": totals.get("instantiations", {}),
    "lines": totals.get("lines", {}),
    "branches": totals.get("branches", {}),
    "thresholds": {
        "line": min_line,
        "function": min_func,
        "branch": min_branch,
    },
}

with open(totals_json_path, "w", encoding="utf-8") as f:
    json.dump(summary, f, indent=2, sort_keys=True)
    f.write("\n")

line_pct = pct("lines")
func_pct = pct("functions")
branch_pct = pct("branches")

with open(totals_txt_path, "w", encoding="utf-8") as f:
    f.write(f"lines={line_pct:.2f}% min={min_line:.2f}%\n")
    f.write(f"functions={func_pct:.2f}% min={min_func:.2f}%\n")
    if min_branch is None:
        f.write(f"branches={branch_pct:.2f}% min=<report-only>\n")
    else:
        f.write(f"branches={branch_pct:.2f}% min={min_branch:.2f}%\n")

failures = []
if line_pct + 1e-9 < min_line:
    failures.append(f"line coverage {line_pct:.2f}% < {min_line:.2f}%")
if func_pct + 1e-9 < min_func:
    failures.append(f"function coverage {func_pct:.2f}% < {min_func:.2f}%")
if min_branch is not None and branch_pct + 1e-9 < min_branch:
    failures.append(f"branch coverage {branch_pct:.2f}% < {min_branch:.2f}%")

print(f"coverage lines={line_pct:.2f}% functions={func_pct:.2f}% branches={branch_pct:.2f}%")
if failures:
    for failure in failures:
        print(f"ERR: {failure}", file=sys.stderr)
    raise SystemExit(8)
PY

echo "OK: coverage totals written to: ${COVER_DIR}/uwvm_int_compiler_totals.txt"
