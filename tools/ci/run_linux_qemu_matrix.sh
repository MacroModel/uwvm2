#!/usr/bin/env bash

# Run an already-built uwvm Linux binary, optionally through qemu-user or a
# wrapper. This script deliberately does not build uwvm, WABT, sysroots, or
# container images.

set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd -- "${SCRIPT_DIR}/../.." && pwd)

usage()
{
    cat <<'EOF'
Usage:
  tools/ci/run_linux_qemu_matrix.sh --uwvm <path> [options]

Required:
  --uwvm <path>                 Existing target uwvm binary.

Target execution:
  --wrapper <words>             Prefix target commands, for example
                                  --wrapper 'qemu-aarch64 -L /opt/sysroot'
                                Shell operators and expansion are not evaluated.
  --wrapper-arg <arg>           Append one exact wrapper argument. Repeatable;
                                useful when an argument contains whitespace.
  --uwvm-arg <arg>              Add one uwvm argument before --run. Repeatable.
  --label <name>                Platform/build label used in TSV output.

Fixture and output:
  --wat2wasm <path>             Existing wat2wasm. Defaults to WAT2WASM, PATH,
                                or an existing build/test WABT checkout.
  --output <dir>                Output directory. Default:
                                  build/test/linux-qemu-matrix/<label>
  --expect-memory-model <name>  Assert version output reports this model. The
                                aliases mmap, multi-thread-alloc, and
                                single-thread-alloc are accepted.

Matrix and resource controls:
  --policies <csv>              instruction,auto,unwind (default: all three).
                                Explicit unwind is rejected-probed and skipped
                                when the target does not advertise libunwind.
  --jobs <n>                    Concurrent target processes (default: 1).
  --compile-threads <n>         -Rct value for each target (default: 1).
  --timeout <seconds>           Per probe/fixture/run timeout (default: 120).
  --max-rss-mib <MiB>           Sampled per-process-group soft RSS cap (default:
                                4096; 0 disables). Use a container/cgroup memory
                                limit as the hard cap for builds and QEMU runs.
  --memory-budget-mib <MiB>     Reject jobs * max-RSS above this (default: 8192;
                                0 disables the aggregate check).
  --allow-unsupported           Record unavailable required capabilities as
                                SKIP instead of FAIL.
  -h, --help                    Show this help.

The fixed mode matrix is full (-Rcm full -Rcc jit), lazy (-Rjit), tiered,
tiered-no-t0, tiered-no-t2, and tiered-no-t0-no-t2. Normal fixtures must exit
zero. Trap fixtures must exit nonzero and report the exact Wasm trap kind.
Metadata, results.tsv, and per-run logs are written below build/test by default.
EOF
}

die()
{
    printf 'run_linux_qemu_matrix: error: %s\n' "$*" >&2
    exit 2
}

is_uint()
{
    [[ $1 =~ ^[0-9]+$ ]]
}

UWVM=
WAT2WASM_BIN=${WAT2WASM:-}
OUTPUT_DIR=
LABEL=
EXPECT_MEMORY_MODEL=
POLICY_CSV=instruction,auto,unwind
JOBS=${UWVM_QEMU_JOBS:-1}
COMPILE_THREADS=${UWVM_QEMU_COMPILE_THREADS:-1}
TIMEOUT_SECONDS=${UWVM_QEMU_TIMEOUT:-120}
MAX_RSS_MIB=${UWVM_QEMU_MAX_RSS_MIB:-4096}
MEMORY_BUDGET_MIB=${UWVM_QEMU_MEMORY_BUDGET_MIB:-8192}
ALLOW_UNSUPPORTED=0
RUN_PREFIX=()
EXTRA_UWVM_ARGS=()

while (($# != 0)); do
    case $1 in
        --uwvm)
            (($# >= 2)) || die "--uwvm requires a path"
            UWVM=$2
            shift 2
            ;;
        --wrapper|--qemu)
            (($# >= 2)) || die "$1 requires a command prefix"
            read -r -a RUN_PREFIX <<<"$2"
            ((${#RUN_PREFIX[@]} != 0)) || die "$1 must not be empty"
            shift 2
            ;;
        --wrapper-arg)
            (($# >= 2)) || die "--wrapper-arg requires an argument"
            RUN_PREFIX+=("$2")
            shift 2
            ;;
        --uwvm-arg)
            (($# >= 2)) || die "--uwvm-arg requires an argument"
            EXTRA_UWVM_ARGS+=("$2")
            shift 2
            ;;
        --wat2wasm)
            (($# >= 2)) || die "--wat2wasm requires a path"
            WAT2WASM_BIN=$2
            shift 2
            ;;
        --output)
            (($# >= 2)) || die "--output requires a directory"
            OUTPUT_DIR=$2
            shift 2
            ;;
        --label)
            (($# >= 2)) || die "--label requires a value"
            LABEL=$2
            shift 2
            ;;
        --expect-memory-model)
            (($# >= 2)) || die "--expect-memory-model requires a value"
            EXPECT_MEMORY_MODEL=$2
            shift 2
            ;;
        --policies)
            (($# >= 2)) || die "--policies requires a comma-separated list"
            POLICY_CSV=$2
            shift 2
            ;;
        --jobs)
            (($# >= 2)) || die "--jobs requires a value"
            JOBS=$2
            shift 2
            ;;
        --compile-threads)
            (($# >= 2)) || die "--compile-threads requires a value"
            COMPILE_THREADS=$2
            shift 2
            ;;
        --timeout)
            (($# >= 2)) || die "--timeout requires a value"
            TIMEOUT_SECONDS=$2
            shift 2
            ;;
        --max-rss-mib)
            (($# >= 2)) || die "--max-rss-mib requires a value"
            MAX_RSS_MIB=$2
            shift 2
            ;;
        --memory-budget-mib)
            (($# >= 2)) || die "--memory-budget-mib requires a value"
            MEMORY_BUDGET_MIB=$2
            shift 2
            ;;
        --allow-unsupported)
            ALLOW_UNSUPPORTED=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            die "unknown option: $1"
            ;;
    esac
done

(($# == 0)) || die "unexpected positional arguments: $*"
[[ -n $UWVM ]] || die "--uwvm is required"
[[ -f $UWVM ]] || die "uwvm binary does not exist: $UWVM"

for numeric in "$JOBS" "$COMPILE_THREADS" "$TIMEOUT_SECONDS" "$MAX_RSS_MIB" "$MEMORY_BUDGET_MIB"; do
    is_uint "$numeric" || die "resource-control values must be non-negative integers"
done
((JOBS >= 1)) || die "--jobs must be at least 1"
((COMPILE_THREADS >= 1)) || die "--compile-threads must be at least 1"
((TIMEOUT_SECONDS >= 1)) || die "--timeout must be at least 1"
if ((MAX_RSS_MIB != 0 && MEMORY_BUDGET_MIB != 0 && JOBS * MAX_RSS_MIB > MEMORY_BUDGET_MIB)); then
    die "jobs * max RSS exceeds --memory-budget-mib (${JOBS} * ${MAX_RSS_MIB} > ${MEMORY_BUDGET_MIB})"
fi

if ((${#RUN_PREFIX[@]} == 0)); then
    [[ -x $UWVM ]] || die "uwvm is not executable and no wrapper was supplied: $UWVM"
else
    command -v -- "${RUN_PREFIX[0]}" >/dev/null 2>&1 || die "wrapper executable not found: ${RUN_PREFIX[0]}"
fi

if [[ -z $WAT2WASM_BIN ]]; then
    if command -v wat2wasm >/dev/null 2>&1; then
        WAT2WASM_BIN=$(command -v wat2wasm)
    else
        for candidate in \
            "${ROOT_DIR}/build/test/third-parties/wabt/build/wat2wasm" \
            "${ROOT_DIR}/build/test/third-parties/wabt/build/bin/wat2wasm" \
            "${ROOT_DIR}/build/test/third-parties/wabt/build/Release/wat2wasm" \
            "${ROOT_DIR}/wabt/build/wat2wasm" \
            "${ROOT_DIR}/wabt/build/bin/wat2wasm"; do
            if [[ -x $candidate ]]; then
                WAT2WASM_BIN=$candidate
                break
            fi
        done
    fi
fi
[[ -n $WAT2WASM_BIN && -x $WAT2WASM_BIN ]] || die "wat2wasm not found; pass --wat2wasm or set WAT2WASM"

UWVM=$(realpath -m -- "$UWVM")
WAT2WASM_BIN=$(realpath -m -- "$WAT2WASM_BIN")

if [[ -z $LABEL ]]; then
    LABEL=$(basename -- "$(dirname -- "$UWVM")")
    case $LABEL in
        debug|release|releasedbg)
            LABEL=$(basename -- "$(dirname -- "$(dirname -- "$UWVM")")")
            ;;
    esac
fi
LABEL=$(printf '%s' "$LABEL" | tr -c 'A-Za-z0-9._-' '_')
[[ -n $LABEL ]] || LABEL=target

if [[ -z $OUTPUT_DIR ]]; then
    OUTPUT_DIR="${ROOT_DIR}/build/test/linux-qemu-matrix/${LABEL}"
elif [[ $OUTPUT_DIR != /* ]]; then
    OUTPUT_DIR="${ROOT_DIR}/${OUTPUT_DIR}"
fi

LOG_DIR="${OUTPUT_DIR}/logs"
FIXTURE_DIR="${OUTPUT_DIR}/fixtures"
FRAGMENT_DIR="${OUTPUT_DIR}/.result-fragments"
RESULTS_TSV="${OUTPUT_DIR}/results.tsv"
METADATA_TSV="${OUTPUT_DIR}/metadata.tsv"
mkdir -p -- "$LOG_DIR" "$FIXTURE_DIR" "$FRAGMENT_DIR"
find "$FRAGMENT_DIR" -maxdepth 1 -type f -name '*.tsv' -delete

RUN_RC=0
RUN_PEAK_RSS_KIB=0
RUN_ELAPSED_MS=0
RUN_LIMIT_REASON=exit

terminate_process_tree()
{
    local pid=$1
    local isolated=$2

    if ((isolated)); then
        kill -TERM -- "-${pid}" 2>/dev/null || true
        sleep 1
        kill -KILL -- "-${pid}" 2>/dev/null || true
    else
        if command -v pkill >/dev/null 2>&1; then
            pkill -TERM -P "$pid" 2>/dev/null || true
        fi
        kill -TERM "$pid" 2>/dev/null || true
        sleep 1
        if command -v pkill >/dev/null 2>&1; then
            pkill -KILL -P "$pid" 2>/dev/null || true
        fi
        kill -KILL "$pid" 2>/dev/null || true
    fi
}

run_limited()
{
    local output_log=$1
    shift
    local start_ms now_ms pid state rss_kib=0 peak_kib=0
    local isolated=0
    local forced_rc=
    local max_rss_kib=$((MAX_RSS_MIB * 1024))

    : >"$output_log"
    {
        printf '# command:'
        printf ' %q' "$@"
        printf '\n'
    } >>"$output_log"

    start_ms=$(date +%s%3N)
    if command -v setsid >/dev/null 2>&1; then
        # Keep a shell as the session leader so a target trap signal becomes a
        # normal numeric exit status. Its diagnostic stays in the case log
        # instead of leaking from this script's job-control reporting.
        setsid bash -c '
            "$@" &
            child=$!
            if wait "$child"; then
                exit 0
            else
                rc=$?
                exit "$rc"
            fi
        ' uwvm-limited-runner "$@" >>"$output_log" 2>&1 &
        pid=$!
        isolated=1
    else
        "$@" >>"$output_log" 2>&1 &
        pid=$!
    fi

    while :; do
        state=$(ps -o stat= -p "$pid" 2>/dev/null || true)
        if [[ -z $state || $state == Z* ]]; then
            break
        fi

        if ((isolated)); then
            rss_kib=$(ps -o rss= --sid "$pid" 2>/dev/null | awk '{ total += $1 } END { print total + 0 }' || true)
        else
            rss_kib=$(ps -o rss= -p "$pid" 2>/dev/null | awk '{ total += $1 } END { print total + 0 }' || true)
        fi
        rss_kib=${rss_kib:-0}
        if ((rss_kib > peak_kib)); then
            peak_kib=$rss_kib
        fi

        now_ms=$(date +%s%3N)
        if ((max_rss_kib != 0 && rss_kib > max_rss_kib)); then
            forced_rc=125
            RUN_LIMIT_REASON=rss-limit
            terminate_process_tree "$pid" "$isolated"
            break
        fi
        if ((now_ms - start_ms >= TIMEOUT_SECONDS * 1000)); then
            forced_rc=124
            RUN_LIMIT_REASON=timeout
            terminate_process_tree "$pid" "$isolated"
            break
        fi
        sleep 0.25
    done

    if wait "$pid"; then
        RUN_RC=0
    else
        RUN_RC=$?
    fi
    if [[ -n $forced_rc ]]; then
        RUN_RC=$forced_rc
    else
        RUN_LIMIT_REASON=exit
    fi
    now_ms=$(date +%s%3N)
    RUN_ELAPSED_MS=$((now_ms - start_ms))
    RUN_PEAK_RSS_KIB=$peak_kib
    return 0
}

strip_ansi_file()
{
    local input=$1
    local output=$2
    LC_ALL=C sed $'s/\033\\[[0-9;?]*[ -\\/]*[@-~]//g' "$input" >"$output"
}

line_value()
{
    local key=$1
    local input=$2
    awk -v key="$key" '
        {
            prefix = key ":"
            position = index($0, prefix)
            if (position != 0) {
                line = substr($0, position + length(prefix))
                sub(/^[[:space:]]*/, "", line)
                print line
                exit
            }
        }
    ' "$input"
}

probe_target()
{
    local name=$1
    shift
    local log="${LOG_DIR}/probe-${name}.log"
    run_limited "$log" "${RUN_PREFIX[@]}" "$UWVM" "$@"
    if ((RUN_RC != 0)); then
        die "target probe ${name} failed (rc=${RUN_RC}, reason=${RUN_LIMIT_REASON}, log=${log})"
    fi
    strip_ansi_file "$log" "${LOG_DIR}/probe-${name}.txt"
}

printf '[qemu-matrix] target=%s label=%s jobs=%s max_rss_mib=%s timeout=%ss\n' \
    "$UWVM" "$LABEL" "$JOBS" "$MAX_RSS_MIB" "$TIMEOUT_SECONDS"

probe_target version --version
probe_target runtime-help --help runtime
probe_target wasm-help --help wasm
run_limited "${LOG_DIR}/probe-wat2wasm.log" "$WAT2WASM_BIN" --version
((RUN_RC == 0)) || die "wat2wasm --version failed (log=${LOG_DIR}/probe-wat2wasm.log)"

VERSION_TEXT="${LOG_DIR}/probe-version.txt"
RUNTIME_HELP_TEXT="${LOG_DIR}/probe-runtime-help.txt"
WASM_HELP_TEXT="${LOG_DIR}/probe-wasm-help.txt"
VERSION=$(line_value Version "$VERSION_TEXT")
ARCHITECTURE=$(line_value Architecture "$VERSION_TEXT")
TARGET_OS=$(line_value OS "$VERSION_TEXT")
C_LIBRARY=$(line_value 'C Library' "$VERSION_TEXT")
MEMORY_MODEL=$(line_value 'WASM Memory Model' "$VERSION_TEXT")
RUNTIME_COMPILER=$(line_value 'Runtime Compiler' "$VERSION_TEXT")
LLVM_VERSION=$(line_value 'LLVM Version' "$VERSION_TEXT")
CALL_STACK_MODES=$(line_value 'Call Stack Modes Support' "$VERSION_TEXT")
WAT2WASM_VERSION=$(tail -n +2 "${LOG_DIR}/probe-wat2wasm.log" | tr '\t\r\n' '   ')

VERSION=${VERSION:-unknown}
ARCHITECTURE=${ARCHITECTURE:-unknown}
TARGET_OS=${TARGET_OS:-unknown}
C_LIBRARY=${C_LIBRARY:-unknown}
MEMORY_MODEL=${MEMORY_MODEL:-unknown}
RUNTIME_COMPILER=${RUNTIME_COMPILER:-unknown}
LLVM_VERSION=${LLVM_VERSION:-unknown}
CALL_STACK_MODES=${CALL_STACK_MODES:-none}
WAT2WASM_VERSION=${WAT2WASM_VERSION:-unknown}

CAP_FULL=0
CAP_LAZY=0
CAP_TIERED=0
CAP_TIERED_NO_T0=0
CAP_TIERED_NO_T2=0
CAP_CALL_STACK=0
CAP_INSTRUCTION=0
CAP_AUTO=0
CAP_UNWIND=0
CAP_WASM2=0

if [[ $RUNTIME_COMPILER == *LLVM-JIT* ]] && grep -q -- '--runtime-custom-mode' "$RUNTIME_HELP_TEXT" && \
    grep -q -- '--runtime-custom-compiler' "$RUNTIME_HELP_TEXT"; then
    CAP_FULL=1
fi
if grep -q -- '--runtime-jit' "$RUNTIME_HELP_TEXT"; then CAP_LAZY=1; fi
if grep -q -- '--runtime-tiered' "$RUNTIME_HELP_TEXT"; then CAP_TIERED=1; fi
if grep -q -- '--runtime-tiered-disable-uwvm-int-lazy-interpreter' "$RUNTIME_HELP_TEXT"; then CAP_TIERED_NO_T0=1; fi
if grep -q -- '--runtime-tiered-disable-llvm-full-jit' "$RUNTIME_HELP_TEXT"; then CAP_TIERED_NO_T2=1; fi
if grep -q -- '--runtime-llvm-jit-call-stack' "$RUNTIME_HELP_TEXT"; then CAP_CALL_STACK=1; fi
if ((CAP_CALL_STACK)) && grep -Eq '\[[^]]*instruction' "$RUNTIME_HELP_TEXT"; then CAP_INSTRUCTION=1; fi
if ((CAP_CALL_STACK)) && grep -Eq '\[[^]]*auto' "$RUNTIME_HELP_TEXT"; then CAP_AUTO=1; fi
if ((CAP_CALL_STACK)) && grep -Eq '\[[^]]*unwind' "$RUNTIME_HELP_TEXT"; then CAP_UNWIND=1; fi
if grep -q -- '--wasm-feature-wasm2' "$WASM_HELP_TEXT"; then CAP_WASM2=1; fi

CAPABILITIES="full=${CAP_FULL},lazy=${CAP_LAZY},tiered=${CAP_TIERED},tiered_no_t0=${CAP_TIERED_NO_T0},tiered_no_t2=${CAP_TIERED_NO_T2},instruction=${CAP_INSTRUCTION},auto=${CAP_AUTO},unwind=${CAP_UNWIND},wasm2=${CAP_WASM2}"
WRAPPER_DISPLAY='<none>'
EXTRA_ARGS_DISPLAY='<none>'
if ((${#RUN_PREFIX[@]} != 0)); then printf -v WRAPPER_DISPLAY '%q ' "${RUN_PREFIX[@]}"; fi
if ((${#EXTRA_UWVM_ARGS[@]} != 0)); then printf -v EXTRA_ARGS_DISPLAY '%q ' "${EXTRA_UWVM_ARGS[@]}"; fi

{
    printf 'key\tvalue\n'
    printf 'label\t%s\n' "$LABEL"
    printf 'uwvm\t%s\n' "$UWVM"
    printf 'wrapper\t%s\n' "$WRAPPER_DISPLAY"
    printf 'extra_uwvm_args\t%s\n' "$EXTRA_ARGS_DISPLAY"
    printf 'version\t%s\n' "$VERSION"
    printf 'architecture\t%s\n' "$ARCHITECTURE"
    printf 'os\t%s\n' "$TARGET_OS"
    printf 'c_library\t%s\n' "$C_LIBRARY"
    printf 'wasm_memory_model\t%s\n' "$MEMORY_MODEL"
    printf 'runtime_compiler\t%s\n' "$RUNTIME_COMPILER"
    printf 'llvm_version\t%s\n' "$LLVM_VERSION"
    printf 'call_stack_modes\t%s\n' "$CALL_STACK_MODES"
    printf 'capabilities\t%s\n' "$CAPABILITIES"
    printf 'wat2wasm\t%s\n' "$WAT2WASM_BIN"
    printf 'wat2wasm_version\t%s\n' "$WAT2WASM_VERSION"
    printf 'jobs\t%s\n' "$JOBS"
    printf 'compile_threads\t%s\n' "$COMPILE_THREADS"
    printf 'timeout_seconds\t%s\n' "$TIMEOUT_SECONDS"
    printf 'max_rss_mib\t%s\n' "$MAX_RSS_MIB"
    printf 'memory_budget_mib\t%s\n' "$MEMORY_BUDGET_MIB"
} >"$METADATA_TSV"

printf '[qemu-matrix] arch=%s libc=%s memory_model=%s capabilities=%s\n' \
    "$ARCHITECTURE" "$C_LIBRARY" "$MEMORY_MODEL" "$CAPABILITIES"

case $EXPECT_MEMORY_MODEL in
    '') ;;
    mmap) EXPECT_MEMORY_MODEL='Memory Map' ;;
    multi-thread-alloc) EXPECT_MEMORY_MODEL='Multi-threaded Memory Allocator' ;;
    single-thread-alloc) EXPECT_MEMORY_MODEL='Single-threaded Memory Allocator' ;;
esac

IFS=',' read -r -a POLICIES <<<"$POLICY_CSV"
((${#POLICIES[@]} != 0)) || die "--policies must not be empty"
for index in "${!POLICIES[@]}"; do
    POLICIES[$index]=${POLICIES[$index]//[[:space:]]/}
    case ${POLICIES[$index]} in
        instruction|auto|unwind) ;;
        *) die "unsupported call-stack policy: ${POLICIES[$index]}" ;;
    esac
done

FIXTURE_NAMES=(
    mvp_smoke
    wasm2_bulk_memory
    wasm2_multiple_tables
    wasm2_multivalue
    oob_load
    float_to_int
    wasm2_bulk_oob
    wasm2_table_oob
    tiered_osr_oob
    tiered_full_ready_oob
)
FIXTURE_CLASSES=(normal normal normal normal trap trap trap trap trap trap)
FIXTURE_FEATURES=(core wasm2 wasm2 wasm2 core core wasm2 wasm2 core core)
FIXTURE_EXPECTED=(
    ok
    ok
    ok
    ok
    'memory access out of bounds'
    'invalid conversion to integer'
    'memory access out of bounds'
    'table access out of bounds'
    'memory access out of bounds'
    'memory access out of bounds'
)
FIXTURE_STACKS=(- - - - '0,1,2,3' '0,1,2,3' '0,1,2' '0,1,2' '0,1,2' '0,1')

TIERED_OSR_TEMPLATE="${ROOT_DIR}/test/0014.llvm_jit/wat/tiered_osr_direct_trap.wat"
TIERED_OSR_GENERATED="${FIXTURE_DIR}/tiered_osr_direct_trap.padded.wat"
[[ -f $TIERED_OSR_TEMPLATE ]] || die "fixture source missing: $TIERED_OSR_TEMPLATE"
awk '
    /UWVM_TIERED_OSR_PAD_NOPS/ {
        for (i = 0; i != 1600; ++i) print "    nop"
        next
    }
    { print }
' "$TIERED_OSR_TEMPLATE" >"$TIERED_OSR_GENERATED"

FIXTURE_SOURCES=(
    "${ROOT_DIR}/test/0014.llvm_jit/nontrivial_start.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/wasm2_bulk_memory_native.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/wasm2_multiple_tables.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/wasm2_multivalue_typed.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/trap_matrix/oob_load.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/trap_matrix/invalid_conversion.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/trap_matrix/wasm2_memory_copy_oob.wat"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/trap_matrix/wasm2_table_copy_oob.wat"
    "$TIERED_OSR_GENERATED"
    "${ROOT_DIR}/test/0014.llvm_jit/wat/tiered_full_ready_oob.wat"
)
FIXTURE_MODE_SCOPES=(all all all all all all all all tiered-all tiered-t2)
FIXTURE_WASMS=()

for index in "${!FIXTURE_NAMES[@]}"; do
    source_wat=${FIXTURE_SOURCES[$index]}
    output_wasm="${FIXTURE_DIR}/${FIXTURE_NAMES[$index]}.wasm"
    compile_log="${LOG_DIR}/wat2wasm-${FIXTURE_NAMES[$index]}.log"
    [[ -f $source_wat ]] || die "fixture source missing: $source_wat"
    if [[ ! -s $output_wasm || $source_wat -nt $output_wasm ]]; then
        run_limited "$compile_log" "$WAT2WASM_BIN" "$source_wat" -o "$output_wasm"
        ((RUN_RC == 0)) || die "wat2wasm failed for ${FIXTURE_NAMES[$index]} (rc=${RUN_RC}, log=${compile_log})"
    else
        printf '# reused %s (newer than %s)\n' "$output_wasm" "$source_wat" >"$compile_log"
    fi
    FIXTURE_WASMS+=("$output_wasm")
done

MODE_NAMES=(full lazy tiered tiered-no-t0 tiered-no-t2 tiered-no-t0-no-t2)

mode_supported()
{
    case $1 in
        full) ((CAP_FULL)) ;;
        lazy) ((CAP_LAZY)) ;;
        tiered) ((CAP_TIERED)) ;;
        tiered-no-t0) ((CAP_TIERED && CAP_TIERED_NO_T0)) ;;
        tiered-no-t2) ((CAP_TIERED && CAP_TIERED_NO_T2)) ;;
        tiered-no-t0-no-t2) ((CAP_TIERED && CAP_TIERED_NO_T0 && CAP_TIERED_NO_T2)) ;;
        *) return 1 ;;
    esac
}

fixture_applicable_to_mode()
{
    local fixture_index=$1
    local mode=$2
    case ${FIXTURE_MODE_SCOPES[$fixture_index]} in
        all) return 0 ;;
        tiered-all)
            case $mode in tiered|tiered-no-t0|tiered-no-t2|tiered-no-t0-no-t2) return 0 ;; esac
            ;;
        tiered-t2) [[ $mode == tiered ]] && return 0 ;;
    esac
    return 1
}

policy_supported()
{
    case $1 in
        instruction) ((CAP_INSTRUCTION)) ;;
        auto) ((CAP_AUTO)) ;;
        unwind) ((CAP_UNWIND)) ;;
        *) return 1 ;;
    esac
}

clean_field()
{
    local value=$1
    value=${value//$'\t'/ }
    value=${value//$'\r'/ }
    value=${value//$'\n'/ }
    printf '%s' "$value"
}

write_result()
{
    local id=$1
    local mode=$2
    local policy=$3
    local actual_policy=$4
    local fixture=$5
    local fixture_class=$6
    local expected=$7
    local exit_code=$8
    local outcome=$9
    shift 9
    local detail=$1
    local peak_rss_kib=$2
    local elapsed_ms=$3
    local output_log=$4
    local compiler_log=$5
    local fragment
    fragment=$(printf '%s/%06d.tsv' "$FRAGMENT_DIR" "$id")

    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$id" \
        "$(clean_field "$LABEL")" \
        "$(clean_field "$VERSION")" \
        "$(clean_field "$ARCHITECTURE")" \
        "$(clean_field "$C_LIBRARY")" \
        "$(clean_field "$MEMORY_MODEL")" \
        "$(clean_field "$CAPABILITIES")" \
        "$(clean_field "$mode")" \
        "$(clean_field "$policy")" \
        "$(clean_field "$actual_policy")" \
        "$(clean_field "$fixture")" \
        "$(clean_field "$fixture_class")" \
        "$(clean_field "$expected")" \
        "$(clean_field "$exit_code")" \
        "$(clean_field "$outcome")" \
        "$(clean_field "$detail")" \
        "$peak_rss_kib" \
        "$elapsed_ms" \
        "$(clean_field "$output_log")" \
        "$(clean_field "$compiler_log")" >"$fragment"
}

next_id=0
allocate_id()
{
    next_id=$((next_id + 1))
    ALLOCATED_ID=$next_id
}

unsupported_outcome()
{
    if ((ALLOW_UNSUPPORTED)); then
        printf SKIP
    else
        printf FAIL
    fi
}

for mode in "${MODE_NAMES[@]}"; do
    if ! mode_supported "$mode"; then
        allocate_id
        write_result "$ALLOCATED_ID" "$mode" - - capability capability supported - "$(unsupported_outcome)" \
            "required runtime mode is not advertised" 0 0 - -
    fi
done

if ((CAP_WASM2 == 0)); then
    for index in "${!FIXTURE_NAMES[@]}"; do
        if [[ ${FIXTURE_FEATURES[$index]} == wasm2 ]]; then
            allocate_id
            write_result "$ALLOCATED_ID" - - - "${FIXTURE_NAMES[$index]}" capability wasm2 - "$(unsupported_outcome)" \
                "--wasm-feature-wasm2 is not advertised" 0 0 - -
        fi
    done
fi

for policy in "${POLICIES[@]}"; do
    if [[ $policy != unwind ]] && ! policy_supported "$policy"; then
        allocate_id
        write_result "$ALLOCATED_ID" - "$policy" - capability capability supported - "$(unsupported_outcome)" \
            "call-stack policy is not advertised" 0 0 - -
    fi
done

if [[ -n $EXPECT_MEMORY_MODEL && $MEMORY_MODEL != "$EXPECT_MEMORY_MODEL" ]]; then
    allocate_id
    write_result "$ALLOCATED_ID" capability - - memory-model capability "$EXPECT_MEMORY_MODEL" - FAIL \
        "target reports ${MEMORY_MODEL}" 0 0 "${LOG_DIR}/probe-version.log" -
fi

extract_trap_kind()
{
    local output_log=$1
    local line
    line=$(grep -a -m1 'Runtime crash (' "$output_log" || true)
    if [[ $line == *'Runtime crash ('* ]]; then
        line=${line#*Runtime crash (}
        printf '%s' "${line%%)*}"
    fi
}

extract_actual_policy()
{
    local compiler_log=$1
    local token=
    if [[ -s $compiler_log ]]; then
        while IFS= read -r match; do
            token=${match#call_stack=}
            token=${token%%[[:space:],;]*}
        done < <(grep -ao 'call_stack=[^[:space:],;]*' "$compiler_log" 2>/dev/null || true)
    fi
    printf '%s' "${token:-unknown}"
}

extract_call_stack_frames()
{
    local compiler_log=$1
    local token=
    if [[ -s $compiler_log ]]; then
        while IFS= read -r match; do
            token=${match#call_stack_frames=}
            token=${token%%[[:space:],;]*}
        done < <(grep -ao 'call_stack_frames=[^[:space:],;]*' "$compiler_log" 2>/dev/null || true)
    fi
    printf '%s' "${token:-unknown}"
}

has_strict_native_capture()
{
    local compiler_log=$1
    grep -aEq '\[llvm-jit-unwind\] capture_source=seeded-libunwind backend=libunwind resolved_jit_caller=yes' "$compiler_log" 2>/dev/null
}

regular_case_executes_llvm()
{
    local mode=$1
    local fixture=${2:-}
    case $fixture in
        tiered_osr_oob|tiered_full_ready_oob) return 0 ;;
    esac
    case $mode in
        full|lazy|tiered-no-t0|tiered-no-t0-no-t2) return 0 ;;
        tiered|tiered-no-t2) return 1 ;;
        *) return 1 ;;
    esac
}

has_tiered_witness_log()
{
    local mode=$1
    local fixture=$2
    local compiler_log=$3
    local request_marker request_line request_fn request_cu
    case $fixture in
        tiered_osr_oob)
            case $mode in
                tiered|tiered-no-t2)
                    request_marker='[llvm-jit-lazy] tiered-osr-request'
                    ;;
                tiered-no-t0|tiered-no-t0-no-t2)
                    request_marker='[llvm-jit-lazy] demand-request'
                    ;;
                *) return 1 ;;
            esac
            request_line=$(grep -aF -m1 "$request_marker" "$compiler_log" 2>/dev/null || true)
            [[ $request_line == *' lane=inline'* ]] || return 1
            request_fn=$(grep -o ' fn=[0-9][0-9]*' <<<"$request_line" | head -n1 | cut -d= -f2)
            request_cu=$(grep -o ' cu=[0-9][0-9]*' <<<"$request_line" | head -n1 | cut -d= -f2)
            [[ -n $request_fn && -n $request_cu ]] || return 1
            grep -aF '[llvm-jit-lazy] compile-end' "$compiler_log" |
                grep -Eq " fn=${request_fn} cu=${request_cu} state=compiled" || return 1
            if [[ $mode == tiered-no-t0 || $mode == tiered-no-t0-no-t2 ]]; then
                ! grep -aFq '[uwvm-int-lazy] demand-request' "$compiler_log" || return 1
            fi
            if [[ $mode == tiered-no-t2 || $mode == tiered-no-t0-no-t2 ]]; then
                ! grep -aEq 'tiered-full-(request|ready)' "$compiler_log" || return 1
            fi
            if [[ $mode == tiered-no-t0-no-t2 ]]; then
                ! grep -aEq 'tiered-(demand|osr)-request' "$compiler_log" || return 1
            fi
            return 0
            ;;
        tiered_full_ready_oob)
            grep -aFq '[llvm-jit-lazy] tiered-full-request' "$compiler_log" &&
                grep -aFq '[llvm-jit-lazy] tiered-full-ready' "$compiler_log"
            ;;
        *) return 1 ;;
    esac
}

has_expected_llvm_log()
{
    local mode=$1
    local compiler_log=$2
    local fixture=${3:-}
    case $fixture in
        tiered_osr_oob|tiered_full_ready_oob)
            has_tiered_witness_log "$mode" "$fixture" "$compiler_log"
            return
            ;;
    esac
    case $mode in
        full) grep -aFq '[llvm-jit-full] optimize-start' "$compiler_log" ;;
        lazy|tiered-no-t0|tiered-no-t0-no-t2) grep -aFq '[llvm-jit-lazy] compile-end' "$compiler_log" ;;
        tiered|tiered-no-t2) return 0 ;;
        *) return 1 ;;
    esac
}

extract_func_indices()
{
    local output_log=$1
    local match
    local indices=
    while IFS= read -r match; do
        if [[ -n $indices ]]; then indices+=,; fi
        indices+=${match#func_idx=}
    done < <(grep -ao 'func_idx=[0-9][0-9]*' "$output_log" 2>/dev/null || true)
    printf '%s' "$indices"
}

AUTO_EFFECTIVE_POLICY=unavailable
if ((CAP_AUTO && CAP_FULL && CAP_CALL_STACK)); then
    allocate_id
    auto_probe_id=$ALLOCATED_ID
    auto_probe_log="${LOG_DIR}/$(printf '%06d' "$auto_probe_id")-auto-live-probe.log"
    auto_probe_compiler_log="${LOG_DIR}/$(printf '%06d' "$auto_probe_id")-auto-live-probe.compiler.log"
    : >"$auto_probe_compiler_log"
    run_limited "$auto_probe_log" \
        "${RUN_PREFIX[@]}" "$UWVM" -Rcm full -Rcc jit -Rct "$COMPILE_THREADS" \
        -Rllvm-cache-path disable -Rllvm-call-stack auto -Rclog file "$auto_probe_compiler_log" \
        "${EXTRA_UWVM_ARGS[@]}" --run "${FIXTURE_WASMS[4]}"

    AUTO_EFFECTIVE_POLICY=$(extract_actual_policy "$auto_probe_compiler_log")
    auto_probe_frames=$(extract_call_stack_frames "$auto_probe_compiler_log")
    auto_probe_stack=$(extract_func_indices "$auto_probe_log")
    auto_probe_trap=$(extract_trap_kind "$auto_probe_log")
    auto_probe_outcome=PASS
    auto_probe_detail="auto=${AUTO_EFFECTIVE_POLICY}"
    if [[ $RUN_LIMIT_REASON != exit ]]; then
        auto_probe_outcome=FAIL
        auto_probe_detail="auto probe hit ${RUN_LIMIT_REASON}"
    elif ((RUN_RC == 0)) || [[ $auto_probe_trap != 'memory access out of bounds' ]]; then
        auto_probe_outcome=FAIL
        auto_probe_detail="auto probe did not produce the expected OOB trap"
    else
        case $AUTO_EFFECTIVE_POLICY in
            unwind)
                if [[ $auto_probe_stack != '0,1,2,3' ]]; then
                    auto_probe_outcome=FAIL
                    auto_probe_detail="auto unwind stack mismatch: ${auto_probe_stack:-missing}"
                elif ! has_strict_native_capture "$auto_probe_compiler_log"; then
                    auto_probe_outcome=FAIL
                    auto_probe_detail='auto unwind did not use seeded libunwind to resolve a JIT caller'
                else
                    auto_probe_detail='auto=unwind;capture=seeded-libunwind;resolved_jit_caller=yes'
                fi
                ;;
            none)
                if [[ $auto_probe_frames != omit ]]; then
                    auto_probe_outcome=FAIL
                    auto_probe_detail="auto none did not omit JIT call-stack frames: ${auto_probe_frames}"
                elif [[ -n $auto_probe_stack ]]; then
                    auto_probe_outcome=FAIL
                    auto_probe_detail="auto none emitted Instruction frames: ${auto_probe_stack}"
                else
                    auto_probe_detail='auto=none;call_stack_frames=omit'
                fi
                ;;
            instruction)
                auto_probe_outcome=FAIL
                auto_probe_detail='forbidden auto fallback to Instruction'
                ;;
            *)
                auto_probe_outcome=FAIL
                auto_probe_detail="auto effective policy was not logged: ${AUTO_EFFECTIVE_POLICY}"
                ;;
        esac
    fi
    write_result "$auto_probe_id" capability auto "$AUTO_EFFECTIVE_POLICY" auto-live-probe trap \
        'unwind-or-none' "$RUN_RC" "$auto_probe_outcome" "$auto_probe_detail" \
        "$RUN_PEAK_RSS_KIB" "$RUN_ELAPSED_MS" "$auto_probe_log" "$auto_probe_compiler_log"
    if [[ $auto_probe_outcome != PASS ]]; then AUTO_EFFECTIVE_POLICY=invalid; fi
fi
printf 'auto_effective_policy\t%s\n' "$AUTO_EFFECTIVE_POLICY" >>"$METADATA_TSV"

TIER2_WITNESS_POLICY=
for preferred_policy in auto unwind instruction; do
    for selected_policy in "${POLICIES[@]}"; do
        if [[ $selected_policy != "$preferred_policy" ]]; then continue; fi
        if ! policy_supported "$selected_policy"; then continue; fi
        if [[ $selected_policy == auto && $AUTO_EFFECTIVE_POLICY == invalid ]]; then continue; fi
        TIER2_WITNESS_POLICY=$selected_policy
        break 2
    done
done
printf 'tier2_witness_policy\t%s\n' "${TIER2_WITNESS_POLICY:-unavailable}" >>"$METADATA_TSV"

run_matrix_case()
{
    local id=$1
    local mode=$2
    local policy=$3
    local fixture_index=$4
    local fixture=${FIXTURE_NAMES[$fixture_index]}
    local fixture_class=${FIXTURE_CLASSES[$fixture_index]}
    local feature=${FIXTURE_FEATURES[$fixture_index]}
    local expected=${FIXTURE_EXPECTED[$fixture_index]}
    local expected_stack=${FIXTURE_STACKS[$fixture_index]}
    local wasm=${FIXTURE_WASMS[$fixture_index]}
    local stem
    local output_log compiler_log trap_kind func_indices observed_policy actual_policy call_stack_frames outcome detail
    local expected_effective_policy expect_stack=1 case_compile_threads=$COMPILE_THREADS
    local mode_args=()
    local command=()

    case $mode in
        full) mode_args=(-Rcm full -Rcc jit) ;;
        lazy) mode_args=(-Rjit) ;;
        tiered) mode_args=(-Rtiered) ;;
        tiered-no-t0) mode_args=(-Rtiered -Rtiered-disable-t0) ;;
        tiered-no-t2) mode_args=(-Rtiered -Rtiered-disable-t2) ;;
        tiered-no-t0-no-t2) mode_args=(-Rtiered -Rtiered-disable-t0 -Rtiered-disable-t2) ;;
        *)
            write_result "$id" "$mode" "$policy" - "$fixture" "$fixture_class" "$expected" - FAIL \
                "internal unknown mode" 0 0 - -
            return 0
            ;;
    esac
    if [[ $fixture == tiered_full_ready_oob ]]; then
        mode_args+=(-Rllvm-policy max)
        if ((case_compile_threads < 2)); then case_compile_threads=2; fi
    fi

    stem=$(printf '%06d-%s-%s-%s' "$id" "$mode" "$policy" "$fixture")
    output_log="${LOG_DIR}/${stem}.log"
    compiler_log="${LOG_DIR}/${stem}.compiler.log"
    : >"$compiler_log"
    command=(
        "${RUN_PREFIX[@]}" "$UWVM"
        "${mode_args[@]}"
        -Rct "$case_compile_threads"
        -Rllvm-cache-path disable
        -Rllvm-call-stack "$policy"
        -Rclog file "$compiler_log"
        "${EXTRA_UWVM_ARGS[@]}"
    )
    if [[ $feature == wasm2 ]]; then
        command+=(--wasm-feature-wasm2)
    fi
    command+=(--run "$wasm")

    run_limited "$output_log" "${command[@]}"
    trap_kind=$(extract_trap_kind "$output_log")
    func_indices=$(extract_func_indices "$output_log")
    observed_policy=$(extract_actual_policy "$compiler_log")
    call_stack_frames=$(extract_call_stack_frames "$compiler_log")
    if [[ $observed_policy != unknown ]]; then
        actual_policy=$observed_policy
    elif [[ $policy == auto ]]; then
        actual_policy=$AUTO_EFFECTIVE_POLICY
    else
        actual_policy=$policy
    fi
    if [[ $policy == auto ]]; then
        expected_effective_policy=$AUTO_EFFECTIVE_POLICY
    else
        expected_effective_policy=$policy
    fi
    outcome=PASS
    detail=ok

    if [[ $RUN_LIMIT_REASON == timeout ]]; then
        outcome=FAIL
        detail=timeout
    elif [[ $RUN_LIMIT_REASON == rss-limit ]]; then
        outcome=FAIL
        detail=rss-limit
    elif regular_case_executes_llvm "$mode" "$fixture" && ! has_expected_llvm_log "$mode" "$compiler_log" "$fixture"; then
        outcome=FAIL
        detail='required LLVM JIT execution marker is missing'
    elif [[ $actual_policy == invalid || $actual_policy == unavailable ]]; then
        outcome=FAIL
        detail="call-stack policy is unavailable: ${actual_policy}"
    elif [[ $mode == full && $observed_policy == unknown ]]; then
        outcome=FAIL
        detail='full JIT did not log the effective call-stack policy'
    elif [[ $observed_policy != unknown && $observed_policy != "$expected_effective_policy" ]]; then
        outcome=FAIL
        detail="call-stack policy mismatch: expected ${expected_effective_policy}, actual ${observed_policy}"
    elif [[ $fixture_class == normal ]]; then
        if ((RUN_RC != 0)); then
            outcome=FAIL
            detail="normal case exited nonzero"
        fi
    else
        if ((RUN_RC == 0)); then
            outcome=FAIL
            detail="trap case exited zero"
        elif [[ $trap_kind != "$expected" ]]; then
            outcome=FAIL
            detail="trap kind mismatch: ${trap_kind:-missing}"
        else
            if regular_case_executes_llvm "$mode" "$fixture" && [[ $actual_policy == none ]]; then
                expect_stack=0
            fi
            if ((expect_stack)) && [[ $func_indices != "$expected_stack" ]]; then
                outcome=FAIL
                detail="call stack mismatch: expected ${expected_stack}, actual ${func_indices:-missing}"
            elif ((expect_stack == 0)) && [[ -n $func_indices ]]; then
                outcome=FAIL
                detail="auto none emitted forbidden Instruction frames: ${func_indices}"
            elif regular_case_executes_llvm "$mode" "$fixture" && [[ $actual_policy == unwind ]] && ! has_strict_native_capture "$compiler_log"; then
                outcome=FAIL
                detail='native stack did not use seeded libunwind to resolve a JIT caller'
            else
                detail="trap=${trap_kind};funcs=${func_indices:-omitted};frames=${call_stack_frames}"
            fi
        fi
    fi

    write_result "$id" "$mode" "$policy" "$actual_policy" "$fixture" "$fixture_class" "$expected" "$RUN_RC" "$outcome" "$detail" \
        "$RUN_PEAK_RSS_KIB" "$RUN_ELAPSED_MS" "$output_log" "$compiler_log"
    return 0
}

unwind_requested=0
for policy in "${POLICIES[@]}"; do
    if [[ $policy == unwind ]]; then unwind_requested=1; fi
done

if ((unwind_requested && CAP_UNWIND == 0)); then
    for unavailable_policy in unwind unwind-uncheck; do
        allocate_id
        guard_id=$ALLOCATED_ID
        guard_log="${LOG_DIR}/$(printf '%06d' "$guard_id")-unavailable-${unavailable_policy}-guard.log"
        guard_compiler_log="${LOG_DIR}/$(printf '%06d' "$guard_id")-unavailable-${unavailable_policy}-guard.compiler.log"
        : >"$guard_compiler_log"
        if ((CAP_FULL && CAP_CALL_STACK)); then
            run_limited "$guard_log" \
                "${RUN_PREFIX[@]}" "$UWVM" -Rcm full -Rcc jit -Rct "$COMPILE_THREADS" \
                -Rllvm-cache-path disable -Rllvm-call-stack "$unavailable_policy" -Rclog file "$guard_compiler_log" \
                "${EXTRA_UWVM_ARGS[@]}" --run "${FIXTURE_WASMS[0]}"
            guard_output=$(tail -n +2 "$guard_log")
            guard_outcome=PASS
            guard_detail="explicit unavailable ${unavailable_policy} was rejected"
            if [[ $RUN_LIMIT_REASON != exit ]]; then
                guard_outcome=FAIL
                guard_detail="${unavailable_policy} rejection probe hit ${RUN_LIMIT_REASON}"
            elif ((RUN_RC == 0)); then
                guard_outcome=FAIL
                guard_detail="unavailable ${unavailable_policy} was silently accepted"
            elif [[ $guard_output == *'Runtime crash ('* ]]; then
                guard_outcome=FAIL
                guard_detail="unavailable ${unavailable_policy} reached runtime instead of CLI rejection"
            elif ! grep -Eqi 'unwind|call-stack|invalid|usage|support|available' <<<"$guard_output"; then
                guard_outcome=FAIL
                guard_detail="unavailable ${unavailable_policy} rejection diagnostic was not recognized"
            fi
            write_result "$guard_id" capability "$unavailable_policy" unsupported unwind-availability capability rejected "$RUN_RC" \
                "$guard_outcome" "$guard_detail" "$RUN_PEAK_RSS_KIB" "$RUN_ELAPSED_MS" "$guard_log" "$guard_compiler_log"
        else
            write_result "$guard_id" capability "$unavailable_policy" unsupported unwind-availability capability rejected - "$(unsupported_outcome)" \
                'cannot run rejection guard without full JIT and call-stack option' 0 0 - -
        fi
    done
fi

active_pids=()
reap_first_task()
{
    local pid=${active_pids[0]}
    if ! wait "$pid"; then
        die "internal matrix worker failed: pid=${pid}"
    fi
    active_pids=("${active_pids[@]:1}")
}

schedule_case()
{
    run_matrix_case "$@" &
    active_pids+=("$!")
    if ((${#active_pids[@]} >= JOBS)); then
        reap_first_task
    fi
}

for mode in "${MODE_NAMES[@]}"; do
    if ! mode_supported "$mode"; then continue; fi
    for policy in "${POLICIES[@]}"; do
        if ! policy_supported "$policy"; then continue; fi
        if [[ $policy == auto && $AUTO_EFFECTIVE_POLICY == invalid ]]; then continue; fi
        for fixture_index in "${!FIXTURE_NAMES[@]}"; do
            if [[ ${FIXTURE_FEATURES[$fixture_index]} == wasm2 && $CAP_WASM2 == 0 ]]; then continue; fi
            if ! fixture_applicable_to_mode "$fixture_index" "$mode"; then continue; fi
            if [[ ${FIXTURE_NAMES[$fixture_index]} == tiered_full_ready_oob && $policy != "$TIER2_WITNESS_POLICY" ]]; then continue; fi
            allocate_id
            schedule_case "$ALLOCATED_ID" "$mode" "$policy" "$fixture_index"
        done
    done
done

while ((${#active_pids[@]} != 0)); do
    reap_first_task
done

{
    printf 'id\tlabel\tversion\tarchitecture\tc_library\tmemory_model\tcapabilities\tmode\tpolicy\tactual_policy\tfixture\tclass\texpected\texit_code\toutcome\tdetail\tpeak_rss_kib\telapsed_ms\tlog\tcompiler_log\n'
    shopt -s nullglob
    fragments=("${FRAGMENT_DIR}"/*.tsv)
    if ((${#fragments[@]} != 0)); then
        cat "${fragments[@]}"
    fi
    shopt -u nullglob
} >"$RESULTS_TSV"

PASS_COUNT=$(awk -F '\t' 'NR > 1 && $15 == "PASS" { count++ } END { print count + 0 }' "$RESULTS_TSV")
FAIL_COUNT=$(awk -F '\t' 'NR > 1 && $15 == "FAIL" { count++ } END { print count + 0 }' "$RESULTS_TSV")
SKIP_COUNT=$(awk -F '\t' 'NR > 1 && $15 == "SKIP" { count++ } END { print count + 0 }' "$RESULTS_TSV")

printf '[qemu-matrix] PASS=%s FAIL=%s SKIP=%s results=%s metadata=%s\n' \
    "$PASS_COUNT" "$FAIL_COUNT" "$SKIP_COUNT" "$RESULTS_TSV" "$METADATA_TSV"

if ((FAIL_COUNT != 0)); then
    exit 1
fi
