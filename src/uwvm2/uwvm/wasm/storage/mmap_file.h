/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @brief       Imported wasm modules
 * @details     "--wasm-load-wasm" or "-Wlw"
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-28
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

#ifndef UWVM_MODULE

// import
# include <fast_io.h>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/utils/hash/impl.h>
# include <uwvm2/parser/wasm/concepts/impl.h>
# include <uwvm2/parser/wasm/standard/wasm1/type/impl.h>
# include <uwvm2/uwvm/wasm/base/impl.h>
# include <uwvm2/uwvm/wasm/feature/impl.h>
# include <uwvm2/uwvm/wasm/type/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::uwvm::wasm::storage
{
    struct mmap_file_results
    {
        ::uwvm2::utils::container::u8string_view export_name{};
        ::uwvm2::utils::container::u8cstring_view file_path{};
    };

    struct mmap_file_results_hash
    {
        using is_transparent = void;

        inline constexpr ::std::size_t operator() (mmap_file_results const& v) const noexcept
        { return ::std::hash<::uwvm2::utils::container::u8string_view>{}(v.export_name); }

        inline constexpr ::std::size_t operator() (::uwvm2::utils::container::u8string_view v) const noexcept
        { return ::std::hash<::uwvm2::utils::container::u8string_view>{}(v); }
    };

    struct mmap_file_results_equal
    {
        using is_transparent = void;

        inline constexpr bool operator() (mmap_file_results const& a, mmap_file_results const& b) const noexcept { return a.export_name == b.export_name; }

        inline constexpr bool operator() (mmap_file_results const& a, ::uwvm2::utils::container::u8string_view b) const noexcept { return a.export_name == b; }

        inline constexpr bool operator() (::uwvm2::utils::container::u8string_view a, mmap_file_results const& b) const noexcept { return a == b.export_name; }
    };

    using mmap_file_results_set = ::uwvm2::utils::container::unordered_flat_set<mmap_file_results, mmap_file_results_hash, mmap_file_results_equal>;

    // module name -> export memories
    // Initialize to `wasm_mmap_memory_modules` first, merge modules with the same name, then uniformly initialize to all modules.
    inline ::uwvm2::utils::container::unordered_flat_map<::uwvm2::utils::container::u8string_view, mmap_file_results_set>
        wasm_mmap_memory_modules{};  // [global]

}  // namespace uwvm2::uwvm::wasm::storage
