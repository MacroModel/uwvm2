#!/usr/bin/env bash

# Lightweight policy/capability regression checks for run_linux_qemu_matrix.sh.
# All target and wat2wasm executions are mocks; this test never compiles or runs
# QEMU and is therefore safe to use while a real -j1 build is in progress.

set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
RUNNER="${SCRIPT_DIR}/run_linux_qemu_matrix.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/uwvm-qemu-matrix-test.XXXXXX")
trap 'rm -rf -- "$TEST_TMP"' EXIT

fail()
{
    printf 'test_run_linux_qemu_matrix: FAIL: %s\n' "$*" >&2
    exit 1
}

assert_tsv_value()
{
    local file=$1
    local key=$2
    local expected=$3
    local actual
    actual=$(awk -F '\t' -v key="$key" '$1 == key { print $2; exit }' "$file")
    [[ $actual == "$expected" ]] || fail "${file}: ${key}: expected ${expected}, got ${actual:-<missing>}"
}

assert_tsv_columns()
{
    local file=$1
    ! awk -F '\t' 'NF != 20 { bad = 1 } END { exit !bad }' "$file" ||
        fail "unexpected TSV column count in ${file}"
}

assert_no_fail_rows()
{
    local file=$1
    assert_tsv_columns "$file"
    ! awk -F '\t' 'NR > 1 && $15 == "FAIL" { found = 1 } END { exit !found }' "$file" ||
        fail "unexpected FAIL row in ${file}"
}

MOCK_WAT2WASM="${TEST_TMP}/wat2wasm"
MOCK_UWVM="${TEST_TMP}/uwvm"
MOCK_REALPATH="${TEST_TMP}/realpath"

cat >"$MOCK_REALPATH" <<'MOCK'
#!/usr/bin/env bash
set -euo pipefail
if [[ ${1:-} == -m ]]; then shift; fi
/bin/realpath "$@"
MOCK

cat >"$MOCK_WAT2WASM" <<'MOCK'
#!/usr/bin/env bash
set -euo pipefail
if [[ ${1:-} == --version ]]; then
    printf 'mock wat2wasm 1.0\n'
    exit 0
fi
output=
while (($# != 0)); do
    if [[ $1 == -o ]]; then
        output=$2
        shift 2
    else
        shift
    fi
done
[[ -n $output ]]
: >"$output"
MOCK

cat >"$MOCK_UWVM" <<'MOCK'
#!/usr/bin/env bash
set -euo pipefail

profile=${MOCK_PROFILE:?}
if [[ ${1:-} == --version ]]; then
    printf 'Version: mock-1\n'
    printf 'Architecture: x86_64\n'
    printf 'OS: Linux\n'
    printf 'C Library: glibc\n'
    printf 'WASM Memory Model: Memory Map\n'
    case $profile in
        full-live|full-bad|full-aux-authority|full-tiered*|lazy-only) printf 'Runtime Compiler: LLVM-JIT\n' ;;
        int-*) printf 'Runtime Compiler: UWVM2-Interpreter\n' ;;
        *) printf 'Runtime Compiler: LLVM-AOT\n' ;;
    esac
    case $profile in
        full-tiered-native|full-live|full-bad|full-aux-authority) printf 'Call Stack Modes Support: instruction, unwind, unwind-uncheck\n' ;;
        ros-instruction|full-tiered*|lazy-only) printf 'Call Stack Modes Support: instruction, unwind-uncheck (auxiliary)\n' ;;
        auto-none) printf 'Call Stack Modes Support: instruction\n' ;;
    esac
    exit 0
fi

if [[ ${1:-} == --help && ${2:-} == runtime ]]; then
    if [[ $profile == int-* ]]; then
        printf '%s\n' '--runtime-int'
        printf '%s\n' '--runtime-compile-threads'
        printf '%s\n' '--runtime-compiler-log'
    else
        if [[ $profile == lazy-only ]]; then
            printf '%s\n' '--runtime-jit'
        else
            printf '%s\n' '--runtime-aot'
        fi
        if [[ $profile == full-tiered* ]]; then
            printf '%s\n' '--runtime-tiered'
        fi
        if [[ $profile == full-live || $profile == full-bad || $profile == full-aux-authority ]]; then
            printf '%s\n' '--runtime-custom-mode'
            printf '%s\n' '--runtime-custom-compiler'
        fi
        if [[ $profile == ros-instruction ]]; then
            # These longer options must not make the exact --runtime-tiered token
            # appear advertised in a source-pruned runtime.
            printf '%s\n' '--runtime-tiered-disable-uwvm-int-lazy-interpreter'
            printf '%s\n' '--runtime-tiered-disable-llvm-full-jit'
        fi
        printf '%s\n' '--runtime-llvm-jit-call-stack'
        case $profile in
            full-tiered-native|full-live|full-bad|full-aux-authority) printf 'Usage: -Rllvm-call-stack [auto|instruction|none|unwind|unwind-uncheck]\n' ;;
            ros-instruction|full-tiered*|lazy-only) printf 'Usage: -Rllvm-call-stack [auto|instruction|none|unwind-uncheck]\n' ;;
            auto-none) printf 'Usage: -Rllvm-call-stack [auto|instruction|none]\n' ;;
        esac
    fi
    exit 0
fi
if [[ ${1:-} == --help && ${2:-} == wasm ]]; then
    if [[ $profile == int-* || $profile == full-tiered* || $profile == lazy-only ]]; then
        printf '%s\n' '--wasm-feature-wasm2'
    else
        printf 'mock wasm help\n'
    fi
    exit 0
fi

policy=auto
compiler_log=
wasm=
int_mode=0
while (($# != 0)); do
    if [[ $profile == int-* && $1 == -Rllvm-* ]]; then
        printf 'int-only mock received forbidden LLVM argument: %s\n' "$1" >&2
        exit 97
    fi
    case $1 in
        -Rint)
            int_mode=1
            shift
            ;;
        -Rllvm-call-stack)
            policy=$2
            shift 2
            ;;
        -Rclog)
            [[ ${2:-} == file ]]
            compiler_log=$3
            shift 3
            ;;
        --run)
            wasm=$2
            shift 2
            ;;
        *) shift ;;
    esac
done

[[ -n $compiler_log ]]
if [[ $profile == int-* ]]; then
    ((int_mode)) || {
        printf 'int-only mock did not receive -Rint\n' >&2
        exit 98
    }
    effective=logical
else
    case $profile:$policy in
        ros-instruction:unwind)
            printf 'call-stack mode unwind is not supported\n' >&2
            exit 2
            ;;
        auto-none:unwind|auto-none:unwind-uncheck)
            printf 'call-stack mode %s is not supported\n' "$policy" >&2
            exit 2
            ;;
    esac

    effective=$policy
    if [[ $policy == auto ]]; then
        case $profile in
            full-tiered-native|full-live|full-bad|full-aux-authority) effective=unwind ;;
            ros-instruction|full-tiered*) effective=instruction ;;
            auto-none) effective=none ;;
        esac
    fi

    case $effective in
        instruction)
            backend=unwind.h
            check=off
            replace=no
            frames=emit
            ;;
        unwind-uncheck)
            backend=unwind.h
            check=static
            replace=no
            frames=emit
            ;;
        unwind)
            if [[ $profile == full-bad ]]; then
                backend=unwind.h
                check=static
                replace=no
                frames=emit
            elif [[ $profile == full-aux-authority ]]; then
                backend=unwind.h
                check=live
                replace=yes
                frames=omit
            else
                backend=win64-seh
                check=live
                replace=yes
                frames=omit
            fi
            ;;
        none)
            backend=unavailable
            check=off
            replace=no
            frames=omit
            ;;
    esac

    printf '[llvm-jit-full] optimize-start fn=0 call_stack=%s unwind_backend=%s unwind_check=%s unwind_replace_frames=%s call_stack_frames=%s\n' \
        "$effective" "$backend" "$check" "$replace" "$frames" >>"$compiler_log"
fi

fixture=$(basename -- "$wasm")
trap_kind=
stack=
case $fixture in
    oob_load.wasm)
        trap_kind='memory access out of bounds'
        stack='0 1 2 3'
        ;;
    float_to_int.wasm)
        trap_kind='invalid conversion to integer'
        stack='0 1 2 3'
        ;;
    wasm2_bulk_oob.wasm)
        trap_kind='memory access out of bounds'
        stack='0 1 2'
        ;;
    wasm2_table_oob.wasm)
        trap_kind='table access out of bounds'
        stack='0 1 2'
        ;;
    tiered_osr_oob.wasm)
        trap_kind='memory access out of bounds'
        stack='0 1 2'
        printf '[llvm-jit-lazy] tiered-osr-request fn=0 cu=0 lane=inline\n' >>"$compiler_log"
        printf '[llvm-jit-lazy] compile-end fn=0 cu=0 state=compiled\n' >>"$compiler_log"
        ;;
    tiered_full_ready_oob.wasm)
        trap_kind='memory access out of bounds'
        stack='0 1'
        printf '[llvm-jit-lazy] tiered-full-request module="mock" module_id=0 priority=0 reason=switch\n' >>"$compiler_log"
        if [[ $profile != full-tiered-no-ready ]]; then
            # Deliberately no T2 entry event: readiness must not be promoted to
            # execution/unwind coverage by the matrix's PASS classification.
            printf '[llvm-jit-lazy] tiered-full-ready module="mock" module_id=0 functions=2\n' >>"$compiler_log"
        fi
        ;;
esac

if [[ -n $trap_kind ]]; then
    if [[ $profile == int-bad-stack && $fixture == oob_load.wasm ]]; then
        stack='0 2 3'
    fi
    printf 'Runtime crash (%s)\n' "$trap_kind" >&2
    if [[ $effective != none ]]; then
        for index in $stack; do
            printf 'func_idx=%s\n' "$index" >&2
        done
    fi
    exit 1
fi
exit 0
MOCK

chmod +x "$MOCK_REALPATH" "$MOCK_WAT2WASM" "$MOCK_UWVM"

run_profile()
{
    local profile=$1
    local policies=$2
    local expected_rc=$3
    local modes=${4:-auto}
    local allow_unsupported=${5:-1}
    local case_name=${6:-$profile}
    local compile_threads=${7:-0}
    local output="${TEST_TMP}/out-${case_name}"
    local rc=0
    local -a args=(
        --uwvm "$MOCK_UWVM"
        --wat2wasm "$MOCK_WAT2WASM"
        --output "$output"
        --label "$case_name"
        --modes "$modes"
        --jobs 1
        --compile-threads "$compile_threads"
        --timeout 5
        --max-rss-mib 0
        --memory-budget-mib 0
    )
    if [[ $policies != default ]]; then
        args+=(--policies "$policies")
    fi
    if ((allow_unsupported)); then
        args+=(--allow-unsupported)
    fi
    if PATH="${TEST_TMP}:$PATH" MOCK_PROFILE=$profile "$RUNNER" "${args[@]}" \
        >"${TEST_TMP}/${case_name}.log" 2>&1; then
        rc=0
    else
        rc=$?
    fi
    [[ $rc == "$expected_rc" ]] || {
        sed -n '1,200p' "${TEST_TMP}/${case_name}.log" >&2
        fail "${case_name}: expected exit ${expected_rc}, got ${rc}"
    }
    assert_tsv_columns "$output/results.tsv"
    printf '%s' "$output"
}

[[ -x $RUNNER ]] || fail "runner is not executable: $RUNNER"
! grep -Eq 'seeded-libunwind|resolved_jit_caller' "$RUNNER" || fail 'obsolete seeded-unwind requirement remains'

int_output=$(run_profile int-only default 0 auto 0 int-only-default)
assert_tsv_value "$int_output/metadata.tsv" selected_modes int
assert_tsv_value "$int_output/metadata.tsv" compile_threads 0
assert_tsv_value "$int_output/metadata.tsv" supported_policies logical
assert_tsv_value "$int_output/metadata.tsv" policy_probe_mode unavailable
assert_tsv_value "$int_output/metadata.tsv" auto_effective_policy unavailable
assert_tsv_value "$int_output/metadata.tsv" unwind_uncheck_auxiliary_status unavailable
grep -q $'\tint\tlogical\tlogical\tmvp_smoke\tnormal\tok\t0\tPASS\t' "$int_output/results.tsv" ||
    fail 'int-only MVP correctness row did not pass with logical stack policy'
grep -q $'\tint\tlogical\tlogical\toob_load\ttrap\tmemory access out of bounds\t1\tPASS\t' "$int_output/results.tsv" ||
    fail 'int-only trap/logical-stack row did not pass'
awk -F '\t' 'NR > 1 && $8 == "int" && $9 == "logical" && $15 == "PASS" { count++ } END { exit count != 8 }' \
    "$int_output/results.tsv" || fail 'int-only default matrix did not run all eight applicable fixtures'
if grep -R -E -- '-Rllvm-' "$int_output/logs" >/dev/null 2>&1; then
    fail 'int-only command or probe received a forbidden -Rllvm-* argument'
fi
assert_no_fail_rows "$int_output/results.tsv"

int_bad_stack_output=$(run_profile int-bad-stack logical 1 int 0)
assert_tsv_columns "$int_bad_stack_output/results.tsv"
grep -q $'\tint\tlogical\tlogical\toob_load\ttrap\tmemory access out of bounds\t1\tFAIL\tcall stack mismatch: expected 0,1,2,3, actual 0,2,3' \
    "$int_bad_stack_output/results.tsv" || fail 'missing interpreter logical frame was not rejected'

int_fail_output=$(run_profile int-only instruction 1 int 0 int-only-instruction-fail)
assert_tsv_columns "$int_fail_output/results.tsv"
grep -q $'\tint\tinstruction\t-\tcapability\tcapability\tan applicable policy\t-\tFAIL\tinterpreter mode requires the logical policy' \
    "$int_fail_output/results.tsv" || fail 'explicit int/instruction mismatch was not reported as FAIL'

int_skip_output=$(run_profile int-only instruction 0 int 1 int-only-instruction-skip)
grep -q $'\tint\tinstruction\t-\tcapability\tcapability\tan applicable policy\t-\tSKIP\tinterpreter mode requires the logical policy' \
    "$int_skip_output/results.tsv" || fail 'allowed int/instruction mismatch was not reported as SKIP'
assert_no_fail_rows "$int_skip_output/results.tsv"

llvm_skip_output=$(run_profile full-live logical 0 full 1 full-logical-skip)
grep -q $'\tfull\tlogical\t-\tcapability\tcapability\tan applicable policy\t-\tSKIP\tLLVM runtime mode requires instruction' \
    "$llvm_skip_output/results.tsv" || fail 'allowed full/logical mismatch was not reported as SKIP'
assert_no_fail_rows "$llvm_skip_output/results.tsv"

lazy_probe_output=$(run_profile lazy-only unwind-uncheck 1 lazy 0)
assert_tsv_columns "$lazy_probe_output/results.tsv"
grep -q $'\tunwind-uncheck\tunavailable\tunwind-uncheck-auxiliary-probe\tcapability\tan advertised AOT/full policy-probe mode\t-\tFAIL\t' \
    "$lazy_probe_output/results.tsv" || fail 'missing auxiliary probe did not produce an explicit failure'

tiered_serial_output=$(run_profile full-tiered instruction 0 tiered 0)
assert_no_fail_rows "$tiered_serial_output/results.tsv"
grep -q $'\ttiered\tinstruction\t-\ttiered_full_ready_oob\ttrap\tmemory access out of bounds\t-\tSKIP\ttier-2 background readiness requires --compile-threads >= 2' \
    "$tiered_serial_output/results.tsv" || fail 'tier-2 witness did not preserve the requested zero-worker cap'
if grep -R -E -- '-Rct [1-9]' "$tiered_serial_output/logs" >/dev/null 2>&1; then
    fail 'tiered matrix exceeded the requested zero-worker cap'
fi

tiered_ready_output=$(run_profile full-tiered-ready instruction 0 tiered 0 tiered-ready-only 2)
assert_no_fail_rows "$tiered_ready_output/results.tsv"
assert_tsv_value "$tiered_ready_output/metadata.tsv" compile_threads 2
assert_tsv_value "$tiered_ready_output/metadata.tsv" tier2_witness_scope publication-readiness-and-trap
assert_tsv_value "$tiered_ready_output/metadata.tsv" t2_entry_execution unverified
assert_tsv_value "$tiered_ready_output/metadata.tsv" t2_unwind_coverage unverified
grep -q $'\ttiered_full_ready_oob\ttrap\tmemory access out of bounds\t1\tPASS\t.*t2_publication=ready;t2_entry_execution=unverified;t2_unwind_coverage=unverified' \
    "$tiered_ready_output/results.tsv" || fail 'ready-only evidence was not explicitly limited to publication and trap'

tiered_no_ready_output=$(run_profile full-tiered-no-ready instruction 1 tiered 0 tiered-missing-ready 2)
grep -q $'\ttiered_full_ready_oob\ttrap\tmemory access out of bounds\t1\tFAIL\trequired tier-2 request/readiness evidence is missing' \
    "$tiered_no_ready_output/results.tsv" || fail 'trap without T2 readiness was accepted'

tiered_native_only_output=$(run_profile full-tiered-ready unwind-uncheck 0 tiered 0 tiered-native-only 2)
assert_no_fail_rows "$tiered_native_only_output/results.tsv"
assert_tsv_value "$tiered_native_only_output/metadata.tsv" tier2_witness_policy unavailable
grep -q $'\ttiered\t-\t-\ttiered_full_ready_oob\ttrap\tmemory access out of bounds\t-\tSKIP\tno compatible instruction policy' \
    "$tiered_native_only_output/results.tsv" || fail 'native unwind was treated as permitting background T2'

tiered_native_auto_output=$(run_profile full-tiered-native auto,instruction 0 tiered 0 tiered-native-auto 2)
assert_no_fail_rows "$tiered_native_auto_output/results.tsv"
assert_tsv_value "$tiered_native_auto_output/metadata.tsv" auto_effective_policy unwind
assert_tsv_value "$tiered_native_auto_output/metadata.tsv" tier2_witness_policy instruction
if grep -q $'\ttiered\tauto\t.*\ttiered_full_ready_oob\t' "$tiered_native_auto_output/results.tsv"; then
    fail 'auto resolved to native unwind was selected for T2 readiness'
fi

ros_output=$(run_profile ros-instruction auto,unwind,unwind-uncheck 0)
assert_tsv_value "$ros_output/metadata.tsv" selected_modes aot
assert_tsv_value "$ros_output/metadata.tsv" auto_effective_policy instruction
assert_tsv_value "$ros_output/metadata.tsv" unwind_uncheck_auxiliary_status verified
grep -q $'^capabilities\t.*unwind=0,unwind_uncheck=1' "$ros_output/metadata.tsv" ||
    fail 'unwind and unwind-uncheck capabilities were not separated'
grep -q $'unwind\tunsupported\tunwind-availability\tcapability\trejected\t2\tPASS' "$ros_output/results.tsv" ||
    fail 'unsupported unwind was not rejection-probed'
assert_no_fail_rows "$ros_output/results.tsv"

live_output=$(run_profile full-live auto 0)
assert_tsv_value "$live_output/metadata.tsv" selected_modes full
assert_tsv_value "$live_output/metadata.tsv" auto_effective_policy unwind
grep -q 'unwind_check=live;unwind_replace_frames=yes' "$live_output/results.tsv" ||
    fail 'authoritative auto unwind was not accepted from structured fields'
assert_no_fail_rows "$live_output/results.tsv"

bad_output=$(run_profile full-bad auto 1)
assert_tsv_value "$bad_output/metadata.tsv" auto_effective_policy invalid
grep -q $'auto-live-probe\ttrap\tinstruction-or-checked-unwind\t1\tFAIL' "$bad_output/results.tsv" ||
    fail 'unchecked/non-replacing auto unwind was not rejected'

aux_authority_output=$(run_profile full-aux-authority auto 1)
assert_tsv_value "$aux_authority_output/metadata.tsv" auto_effective_policy invalid
grep -q 'auto unwind lacks live checked native unwind with frame replacement' "$aux_authority_output/results.tsv" ||
    fail 'auxiliary unwind.h backend was accepted as authoritative'

none_output=$(run_profile auto-none auto 1)
assert_tsv_value "$none_output/metadata.tsv" selected_modes aot
assert_tsv_value "$none_output/metadata.tsv" auto_effective_policy invalid
grep -q $'auto-live-probe\ttrap\tinstruction-or-checked-unwind\t1\tFAIL' "$none_output/results.tsv" ||
    fail 'impossible auto none resolution was not rejected'
grep -q 'invalid auto resolution to none' "$none_output/results.tsv" ||
    fail 'auto none rejection did not explain the invalid runtime state'

printf 'test_run_linux_qemu_matrix: PASS\n'
