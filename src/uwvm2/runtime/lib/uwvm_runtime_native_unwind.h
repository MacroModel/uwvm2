/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

#pragma once

// Keep native-unwind detection shared by the header and module builds. A POSIX <unwind.h> walk is diagnostic-only: it starts from
// the runtime helper and must not replace instruction-emitted Wasm frames. Only the Win64 SEH path receives an explicit generated
// caller CONTEXT and is allowed to replace the logical stack.
#if defined(UWVM_RUNTIME_LLVM_JIT) && defined(_WIN64) && !(defined(__arm64ec__) || defined(_M_ARM64EC)) && !defined(__CYGWIN__) &&                              \
    (defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64))
# define UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE 1
#else
# define UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE 0
#endif

#if defined(UWVM_RUNTIME_LLVM_JIT) && defined(__APPLE__) && !defined(_WIN32)
# define UWVM2_RUNTIME_LLVM_JIT_ENABLE_NATIVE_UNWIND_BACKTRACE 1
#elif UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE
# define UWVM2_RUNTIME_LLVM_JIT_ENABLE_NATIVE_UNWIND_BACKTRACE 1
#elif defined(UWVM_RUNTIME_LLVM_JIT) && (defined(__linux__) || defined(__FreeBSD__)) &&                                                                        \
    ((defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)) && !defined(__ILP32__))
# define UWVM2_RUNTIME_LLVM_JIT_ENABLE_NATIVE_UNWIND_BACKTRACE 1
#else
# define UWVM2_RUNTIME_LLVM_JIT_ENABLE_NATIVE_UNWIND_BACKTRACE 0
#endif

#if defined(UWVM_RUNTIME_LLVM_JIT) && UWVM2_RUNTIME_LLVM_JIT_ENABLE_NATIVE_UNWIND_BACKTRACE && !defined(_WIN32) && __has_include(<unwind.h>)
# include <unwind.h>
# define UWVM2_RUNTIME_LLVM_JIT_HAS_UNWIND_H_BACKTRACE 1
# if defined(__APPLE__)
extern "C" void __register_frame(void const*);
extern "C" void __deregister_frame(void const*);
# endif
#else
# define UWVM2_RUNTIME_LLVM_JIT_HAS_UNWIND_H_BACKTRACE 0
#endif

#if UWVM2_RUNTIME_LLVM_JIT_HAS_UNWIND_H_BACKTRACE || UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE
# define UWVM2_RUNTIME_LLVM_JIT_HAS_UNWIND_BACKTRACE 1
#else
# define UWVM2_RUNTIME_LLVM_JIT_HAS_UNWIND_BACKTRACE 0
#endif

#if UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE
# define UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES 1
# define UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN 1
#else
# define UWVM2_RUNTIME_LLVM_JIT_UNWIND_REPLACES_INSTRUCTION_FRAMES 0
# define UWVM2_RUNTIME_LLVM_JIT_HAS_TRAP_FRAME_POINTER_CHAIN 0
#endif
