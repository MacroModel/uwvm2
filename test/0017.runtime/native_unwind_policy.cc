#include <uwvm2/runtime/compiler/llvm_jit/native_unwind_platform.h>

#if defined(UWVM_RUNTIME_LLVM_JIT) && UWVM2_RUNTIME_LLVM_JIT_WIN64_SEH_PLATFORM_SUPPORTED
# define UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND 1
#else
# define UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND 0
#endif

#pragma pop_macro("UWVM2_RUNTIME_LLVM_JIT_NATIVE_UNWIND_PLATFORM_SUPPORTED")
#pragma pop_macro("UWVM2_RUNTIME_LLVM_JIT_WIN64_SEH_PLATFORM_SUPPORTED")

#include <uwvm2/runtime/lib/uwvm_runtime_native_unwind.h>

// POSIX/native unwinding is auxiliary. It must never replace the logical Wasm
// stack or reintroduce a generated frame-pointer/raw-stack reconstruction path.
static_assert(UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE == UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND);
static_assert(UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES == UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND);
static_assert(UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN == UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND);

#undef UWVM2_TEST_EXPECT_AUTHORITATIVE_NATIVE_UNWIND

int main() {}
