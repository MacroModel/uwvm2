/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2026-03-16
 * @copyright   APL-2.0 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

#pragma once

#include <cstddef>

#ifndef UWVM_MODULE
# include <uwvm2/utils/container/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::llvm_jit
{
    struct run_config
    {
        /// @brief The first function index to enter in the main module.
        /// @note  This is the WASM function index space (imports first, then local-defined).
        ::std::size_t entry_function_index{};
    };

    /// @brief Execute the main module with the LLVM JIT backend in lazy mode.
    /// @note  The backend compiles native wrappers on demand and reuses uwvm_int as the complete semantic fallback.
    extern "C++" void lazy_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, run_config) noexcept;

    /// @brief Execute the main module with the LLVM JIT backend in full-compile mode.
    /// @note  The backend compiles all module-local wrappers up front, then enters through the native entry wrapper when possible.
    extern "C++" void full_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, run_config) noexcept;
}  // namespace uwvm2::runtime::llvm_jit
