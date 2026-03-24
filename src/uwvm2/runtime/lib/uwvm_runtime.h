/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
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
#include <cstdint>

#ifndef UWVM_MODULE
# include <uwvm2/utils/container/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::uwvm::wasm::type { struct uwvm_preload_memory_descriptor_t; }
UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::uwvm_int::optable { struct compiled_defined_call_info; }

UWVM_MODULE_EXPORT namespace uwvm2::runtime::uwvm_int
{
    struct full_compile_run_config
    {
        /// @brief The first function index to enter in the main module.
        /// @note  This is the WASM function index space (imports first, then local-defined).
        /// @note  If this points to an imported function, the runtime will fast_terminate().
        ::std::size_t entry_function_index{};
    };

    /// @brief Full-compile and run the main module using the uwvm_int interpreter backend.
    /// @note  This expects uwvm runtime initialization to be complete (runtime storages + import resolution).
    extern "C++" void full_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, full_compile_run_config) noexcept;

    /// @brief Ensure the uwvm_int runtime metadata/cache is ready for bridge-style execution.
    /// @note  This is used by other backends (for example the LLVM JIT wrapper backend) to reuse uwvm_int's
    ///        complete runtime semantics as a fallback path.
    extern "C++" void ensure_runtime_ready() noexcept;

    /// @brief Resolve the internal uwvm_int runtime module id from a module name.
    /// @return Returns `SIZE_MAX` if the module is unknown.
    extern "C++" [[nodiscard]] ::std::size_t runtime_module_id_from_name(::uwvm2::utils::container::u8string_view module_name) noexcept;

    /// @brief Query the compiler metadata for a local-defined function in the uwvm_int runtime cache.
    extern "C++" [[nodiscard]] bool runtime_defined_call_info_at(
        ::std::size_t module_id,
        ::std::size_t local_function_index,
        ::uwvm2::runtime::compiler::uwvm_int::optable::compiled_defined_call_info* out) noexcept;

    /// @brief Execute a function through uwvm_int's bridge layer.
    /// @note  This preserves the full uwvm_int semantics (imports, traps, call stack, preload host API context).
    extern "C++" void invoke_function_bridge(::std::size_t module_id, ::std::size_t function_index, ::std::byte** stack_top_ptr) noexcept;

    /// @brief Execute an indirect call through uwvm_int's bridge layer.
    extern "C++" void invoke_function_call_indirect_bridge(::std::size_t module_id,
                                                           ::std::size_t type_index,
                                                           ::std::size_t table_index,
                                                           ::std::byte** stack_top_ptr) noexcept;

    /// @brief Execute `memory.grow` through uwvm_int's memory bridge layer.
    /// @note  This preserves uwvm_int's memory semantics for mmap / allocator backends and their thread-safety rules.
    extern "C++" void invoke_memory_grow_bridge(::std::size_t module_id, ::std::size_t memory_index, ::std::byte** stack_top_ptr) noexcept;

    /// @brief Execute `i32.load` through uwvm_int's memory bridge layer.
    extern "C++" void
        invoke_memory_i32_load_bridge(::std::size_t module_id, ::std::size_t memory_index, ::std::size_t static_offset, ::std::byte** stack_top_ptr) noexcept;

    /// @brief Execute `i32.store` through uwvm_int's memory bridge layer.
    extern "C++" void
        invoke_memory_i32_store_bridge(::std::size_t module_id, ::std::size_t memory_index, ::std::size_t static_offset, ::std::byte** stack_top_ptr) noexcept;

    [[nodiscard]] ::std::size_t preload_memory_descriptor_count_host_api() noexcept;
    [[nodiscard]] bool preload_memory_descriptor_at_host_api(::std::size_t descriptor_index,
                                                             ::uwvm2::uwvm::wasm::type::uwvm_preload_memory_descriptor_t* out) noexcept;
    [[nodiscard]] bool preload_memory_read_host_api(::std::size_t memory_index, ::std::uint_least64_t offset, void* destination, ::std::size_t size) noexcept;
    [[nodiscard]] bool preload_memory_write_host_api(::std::size_t memory_index, ::std::uint_least64_t offset, void const* source, ::std::size_t size) noexcept;
}  // namespace uwvm2::runtime::uwvm_int
