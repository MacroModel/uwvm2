#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
runtime_lib="$repo_root/src/uwvm2/runtime/lib"
signal_handler="$repo_root/src/uwvm2/object/memory/signal/signal.h"
production_source_boundary=(
    "$repo_root/src/uwvm2/runtime"
    "$repo_root/src/uwvm2/parser"
    "$repo_root/src/uwvm2/validation"
    "$repo_root/src/uwvm2/uwvm/cmdline"
    "$repo_root/src/uwvm2/uwvm/run"
    "$repo_root/src/uwvm2/uwvm/runtime"
    "$repo_root/src/uwvm2/uwvm/wasm/loader"
)
build_surface=(
    "$repo_root/xmake.lua"
    "$repo_root/xmake"
)
test_script_surface=(
    "$repo_root/.github"
    "$repo_root/test"
    "$repo_root/tools"
)

fail() {
    printf 'reduced-runtime audit: %s\n' "$*" >&2
    exit 1
}

# Deleted execution strategies must not leak back through build selection, CLI/parser surfaces, validation/initialization,
# loaders, compiler/runtime dispatch, module interfaces, or runnable test tooling. Keep this list precise: the retained LLVM
# translator's `llvm_jit_instruction_emitted_inline` flag means "emitted directly in the opcode validation arm" and is not
# cross-Wasm-function inlining.
forbidden_runtime_pattern='compile_cu_from_lazy_validator|lazy_compile_(and_run|stop|scheduler)|runtime_mode_t::(lazy|auto_compile)|uwvm_interpreter_llvm_jit_tiered|tiered_(runtime|full|entry|osr)|llvm_jit_call_interpreter_defined_raw_api|interpreter_fallback|runtime_custom_(mode|compiler)|llvm_jit_lazy_policy|runtime_tiered_disable|lazy_defined_(raw_call|typed_entry)_target|get_llvm_lazy|tiered_loop|osr_(entry|loop|target)|debug_interpreter|runtime_debug_int|UWVM_(RUNTIME_DEBUG_INTERPRETER|RUNTIME_HAS_DEBUGGER_BACKEND|ENABLE_DEBUG_INT|DISABLE_DEBUG_INT)|--runtime-(jit|tiered|custom-mode|custom-compiler|llvm-jit-lazy-policy|debug-int)'
if rg -n --glob '*.{h,cpp,cppm}' "$forbidden_runtime_pattern" "${production_source_boundary[@]}"; then
    fail 'found a deleted lazy/tiered/LLVM-to-interpreter production reference'
fi
if rg -n --glob '*.lua' "$forbidden_runtime_pattern" "${build_surface[@]}"; then
    fail 'found a deleted execution strategy in the build configuration'
fi
if rg -n --glob '*.{h,cpp,cppm,cc,lua,sh,py,yml,yaml}' --glob '!check_reduced_runtime_surface.sh' \
    "$forbidden_runtime_pattern" "${test_script_surface[@]}"; then
    fail 'found a deleted execution strategy in runnable test/tooling configuration'
fi

# A production AOT build has no switches that can bypass generated-IR verification or authenticated native caching.
safety_bypass_pattern='runtime_llvm_jit_(disable_ir_verifaction|cache_no_sign|cache_no_verify)|--runtime-llvm-jit-(disable-ir-verifaction|cache-no-sign|cache-no-verify)|-Rllvm-(noverify|cache-nosign|cache-noverify)|unwind[-_]uncheck(ed)?'
if rg -n --glob '*.{h,cpp,cppm,cc,lua,md,sh,py,yml,yaml}' --glob '!check_reduced_runtime_surface.sh' \
    "$safety_bypass_pattern" "$repo_root/src" "$repo_root/documents" "$repo_root/test" "$repo_root/tools" "$repo_root/xmake.lua" "$repo_root/xmake"; then
    fail 'found a removed LLVM safety-bypass policy'
fi

removed_paths=(
    src/uwvm2/runtime/compiler/uwvm_int/compile_cu_from_lazy_validator
    src/uwvm2/runtime/compiler/llvm_jit/compile_cu_from_lazy_validator
    src/uwvm2/runtime/compiler/uwvm_int/optable/lazy.h
    src/uwvm2/runtime/compiler/uwvm_int/optable/lazy.cppm
    src/uwvm2/runtime/compiler/uwvm_int/utils
    src/uwvm2/runtime/compiler/debug_int/readme.md
    src/uwvm2/runtime/compiler/debug_int/compile_all_from_uwvm/.keep
    src/uwvm2/runtime/compiler/tiered.md
    src/uwvm2/uwvm/cmdline/callback/runtime_jit.h
    src/uwvm2/uwvm/cmdline/callback/runtime_jit.cppm
    src/uwvm2/uwvm/cmdline/callback/runtime_tiered.h
    src/uwvm2/uwvm/cmdline/callback/runtime_tiered.cppm
    src/uwvm2/uwvm/cmdline/callback/runtime_custom_mode.h
    src/uwvm2/uwvm/cmdline/callback/runtime_custom_mode.cppm
    src/uwvm2/uwvm/cmdline/callback/runtime_custom_compiler.h
    src/uwvm2/uwvm/cmdline/callback/runtime_custom_compiler.cppm
    src/uwvm2/uwvm/cmdline/callback/runtime_llvm_jit_lazy_policy.h
    src/uwvm2/uwvm/cmdline/callback/runtime_llvm_jit_lazy_policy.cppm
    src/uwvm2/uwvm/cmdline/callback/runtime_debug_int.h
    src/uwvm2/uwvm/cmdline/callback/runtime_debug_int.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_jit.h
    src/uwvm2/uwvm/cmdline/params/runtime_jit.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_tiered.h
    src/uwvm2/uwvm/cmdline/params/runtime_tiered.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_custom_mode.h
    src/uwvm2/uwvm/cmdline/params/runtime_custom_mode.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_custom_compiler.h
    src/uwvm2/uwvm/cmdline/params/runtime_custom_compiler.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_lazy_policy.h
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_lazy_policy.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_debug_int.h
    src/uwvm2/uwvm/cmdline/params/runtime_debug_int.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_disable_ir_verifaction.h
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_disable_ir_verifaction.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_cache_no_sign.h
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_cache_no_sign.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_cache_no_verify.h
    src/uwvm2/uwvm/cmdline/params/runtime_llvm_jit_cache_no_verify.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_tiered_disable_llvm_full_jit.h
    src/uwvm2/uwvm/cmdline/params/runtime_tiered_disable_llvm_full_jit.cppm
    src/uwvm2/uwvm/cmdline/params/runtime_tiered_disable_uwvm_int_lazy_interpreter.h
    src/uwvm2/uwvm/cmdline/params/runtime_tiered_disable_uwvm_int_lazy_interpreter.cppm
    test/0014.llvm_jit/tiered_osr_call_stack_wat.cc
    test/0014.llvm_jit/tiered_strategy_unwind_wat.cc
    test/0014.llvm_jit/wat/tiered_osr_call_indirect_trap.wat
    test/0014.llvm_jit/wat/tiered_osr_direct_trap.wat
    tools/unwind_fuzz/uwvm_osr_unwind_fuzzer.py
)
for relative_path in "${removed_paths[@]}"; do
    [[ ! -e "$repo_root/$relative_path" ]] || fail "deleted source path exists: $relative_path"
done

if [[ -d "$repo_root/test/0013.uwvm_int/lazy" ]] && find "$repo_root/test/0013.uwvm_int/lazy" -type f -print -quit | grep -q .; then
    fail 'deleted uwvm-int lazy tests still contain source or scripts'
fi
if [[ -d "$repo_root/src/uwvm2/runtime/compiler/debug_int" ]] && find "$repo_root/src/uwvm2/runtime/compiler/debug_int" -type f -print -quit | grep -q .; then
    fail 'deleted debug interpreter directory still contains source'
fi

# Max/O3 remains available for function-local optimization, but generated Wasm/public/raw functions must stay separate.
# Direct opcode-to-IR emission inside the validator switch is legal and deliberately not matched by this check.
llvm_translate="$repo_root/src/uwvm2/runtime/compiler/llvm_jit/compile_all_from_uwvm/translate"
rg -q 'function\.addFnAttr\(::llvm::Attribute::NoInline\);' "$llvm_translate/single_func_emit.h" ||
    fail 'generated LLVM AOT functions are not permanently marked NoInline'
rg -q 'It is not LLVM function inlining' "$llvm_translate/single_func_validation_dispatch.h" ||
    fail 'direct opcode IR emission is not distinguished from function inlining'
if rg -n 'Attribute::AlwaysInline|AlwaysInlinerPass|ModuleInlinerWrapperPass|createFunctionInliningPass|InlinerPass' "$llvm_translate"; then
    fail 'found an LLVM function-inlining pass or attribute in the retained AOT translator'
fi

rg -q 'llvm_jit_opt\.verify_llvm_jit_ir = true;' "$runtime_lib/uwvm_runtime.default.cpp" ||
    fail 'generated LLVM IR verification is not fixed on'
rg -q 'policy\.generate_signature = true;' "$repo_root/src/uwvm2/runtime/llvm_jit_cache/environment.h" ||
    fail 'LLVM native cache signature generation is not fixed on'
rg -q 'policy\.verify_signature = true;' "$repo_root/src/uwvm2/runtime/llvm_jit_cache/environment.h" ||
    fail 'LLVM native cache signature verification is not fixed on'

# Host-specific code generation is opt-in. The default inherits the configured toolchain baseline.
march_option_block="$(sed -n '/^option("march"/,/^end)$/p' "$repo_root/xmake/option.lua")"
grep -q 'set_default("none")' <<<"$march_option_block" || fail 'march does not default to the toolchain baseline'
grep -q 'arch ~= "no" and arch ~= "none"' "$repo_root/xmake/utility/utility.lua" ||
    fail 'march=none is not handled as a no-flag setting'
rg -q '\*\*Default:\*\* `none`' "$repo_root/documents/xmake-options.md" ||
    fail 'xmake option documentation does not record the safe march default'

# POSIX native unwind is only an auxiliary compiler <unwind.h> backtrace. Dedicated libunwind cursors,
# signal-context seeding, and frame/raw-stack scanning must not return.
if rg -n 'get_signal_(frame_address|stack_pointer)|<libunwind\.h>|UNW_LOCAL_ONLY|unw_(getcontext|init_local2?|step|get_reg)|UNW_INIT_SIGNAL_FRAME|raw[_ -]stack|seeded[_ -]unwind|__builtin_frame_address|Intrinsic::frameaddress' \
    "$runtime_lib" "$signal_handler" "$llvm_translate"; then
    fail 'found seeded/signal/raw-stack POSIX unwind reconstruction'
fi

cxx="${CXX:-c++}"
command -v "$cxx" >/dev/null 2>&1 || fail "C++ preprocessor is unavailable: $cxx"
linux_unwind_macros="$({
    printf '%s\n' '#define UWVM_RUNTIME_LLVM_JIT 1'
    printf '%s\n' '#include "uwvm_runtime_native_unwind.h"'
} | "$cxx" -E -dM -x c++ -U__APPLE__ -U__MACH__ -U_WIN32 -U_WIN64 -D__linux__=1 -D__x86_64__=1 '-D__has_include(x)=0' \
    -I"$runtime_lib" - 2>/dev/null)"
grep -q '^#define UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES 0$' <<<"$linux_unwind_macros" ||
    fail 'Linux native unwind claims it can replace logical Wasm frames'
grep -q '^#define UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN 0$' <<<"$linux_unwind_macros" ||
    fail 'Linux native unwind enables the trap frame-pointer chain'

# Module builds must compile only selected backend partitions; the backend-neutral runtime interface remains unconditional.
if rg -n 'add_files\("src/uwvm2/runtime/\*\*\.cppm"' "$repo_root/xmake.lua"; then
    fail 'runtime module wildcard compiles disabled backends'
fi
rg -q 'local uwvm_uses_uwvm_int = ' "$repo_root/xmake.lua" || fail 'missing uwvm-int backend selection predicate'
rg -q 'add_files\("src/uwvm2/runtime/lib/\*\*\.cppm"' "$repo_root/xmake.lua" || fail 'missing backend-neutral runtime module interface'
rg -q 'add_files\("src/uwvm2/runtime/compiler/uwvm_int/\*\*\.cppm"' "$repo_root/xmake.lua" || fail 'missing guarded uwvm-int module partition'
rg -q 'add_files\("src/uwvm2/runtime/compiler/llvm_jit/\*\*\.cppm"' "$repo_root/xmake.lua" || fail 'missing guarded LLVM module partition'
rg -q 'add_files\("src/uwvm2/runtime/llvm_jit_cache/\*\*\.cppm"' "$repo_root/xmake.lua" || fail 'missing guarded LLVM cache module partition'

# Reload invalidation is backend-neutral: int-only runs must not retain compiled_all with stale runtime-storage pointers.
rg -q 'void reset_runtime_state_host_api\(\) noexcept' "$runtime_lib/uwvm_runtime.h" || fail 'missing generic runtime reset API declaration'
rg -q 'reset_runtime_state_host_api\(\);' "$repo_root/src/uwvm2/uwvm/run/run.h" || fail 'normal run teardown does not call the generic reset API'

printf 'reduced-runtime audit: ok\n'
