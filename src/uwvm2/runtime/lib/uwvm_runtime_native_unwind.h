/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

#pragma once

#include <uwvm2/runtime/compiler/llvm_jit/native_unwind_platform.h>

// Keep native-unwind detection identical for the traditional aggregation translation unit and the C++ module consumer.
// Backend availability is separate from authority: a normal POSIX <unwind.h> walk is diagnostic-only, while the explicit Win64 SEH
// caller context is the sole native path allowed to replace instruction-emitted logical Wasm frames.
#if defined(UWVM_RUNTIME_LLVM_JIT) && UWVM2_RUNTIME_LLVM_JIT_WIN64_SEH_PLATFORM_SUPPORTED
# define UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE 1
#else
# define UWVM2_RUNTIME_LLVM_JIT_HAS_WIN64_SEH_BACKTRACE 0
#endif

#if defined(UWVM_RUNTIME_LLVM_JIT) && UWVM2_RUNTIME_LLVM_JIT_NATIVE_UNWIND_PLATFORM_SUPPORTED
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

#pragma pop_macro("UWVM2_RUNTIME_LLVM_JIT_NATIVE_UNWIND_PLATFORM_SUPPORTED")
#pragma pop_macro("UWVM2_RUNTIME_LLVM_JIT_WIN64_SEH_PLATFORM_SUPPORTED")
