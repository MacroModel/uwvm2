#include <uwvm2/runtime/lib/uwvm_runtime_native_unwind.h>

#if defined(UWVM_RUNTIME_LLVM_JIT) && defined(_WIN64) && !(defined(__arm64ec__) || defined(_M_ARM64EC)) && !defined(__CYGWIN__) &&                           \
    (defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64))
static_assert(UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES == 1);
static_assert(UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN == 1);
#else
// POSIX/native unwinding is auxiliary. It must never replace the logical Wasm
// stack or reintroduce a generated frame-pointer/raw-stack reconstruction path.
static_assert(UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES == 0);
static_assert(UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN == 0);
#endif

int main() {}
