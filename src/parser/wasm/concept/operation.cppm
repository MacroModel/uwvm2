/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-04-06
 * @copyright   APL-2 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

module;

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <utility>
#include <algorithm>
#include <vector>

#if !(__cpp_pack_indexing >= 202311L)
# error "UWVM requires at least C++26 standard compiler."
#endif

export module parser.wasm.concepts:operation;

import fast_io;
import parser.wasm.standard.wasm1.type;
import :root;

export namespace parser::wasm::concepts
{
    namespace details
    {
        /// @brief Determine if parameters can be passed through registers under the current abi.
        template <typename T>
        concept abi_transferable_value = ::std::is_trivially_copyable_v<T> &&
#if (defined(_WIN32) && !defined(__WINE__)) || defined(__CYGWIN__)
                                         sizeof(T) <= 8u
#else
                                         sizeof(T) <= (sizeof(::std::size_t) * 2)
#endif
            ;
    }  // namespace details

    namespace operation
    {
        /// @brief      Get the version of binfmt
        /// @details    Need to satisfy the concept has_wasm_binfmt_version
        template <::parser::wasm::concepts::wasm_feature FeatureType>
            requires (::parser::wasm::concepts::has_wasm_binfmt_version<FeatureType>)
        inline consteval ::parser::wasm::standard::wasm1::type::wasm_u32 get_binfmt_version() noexcept
        {
            constexpr ::parser::wasm::standard::wasm1::type::wasm_u32 binfmt_version{::std::remove_cvref_t<FeatureType>::binfmt_version};
            // Can't be 0.
            if constexpr(binfmt_version == 0) { ::fast_io::fast_terminate(); }
            return binfmt_version;
        }

        /// @brief Checking for duplicate binfmt version handler functions
        template <::parser::wasm::concepts::wasm_feature... Fs>
        inline consteval void check_has_duplicate_binfmt_handler() noexcept
        {
            // Define the binfmt needed for the feature. Duplicates not eliminated.
            ::std::vector<::parser::wasm::standard::wasm1::type::wasm_u32> binfmt_vers_uneliminated{};

            // Get all required binfmt versions from the variant templates
            [&]<::std::size_t... I>(::std::index_sequence<I...>) constexpr noexcept
            { ((binfmt_vers_uneliminated.push_back(get_binfmt_version<Fs...[I]>())), ...); }(::std::make_index_sequence<sizeof...(Fs)>{});

            // Sorting for easy follow-up
            ::std::ranges::sort(binfmt_vers_uneliminated);

            // Constructing a version that eliminates duplicates.
            ::std::vector<::parser::wasm::standard::wasm1::type::wasm_u32> binfmt_vers{};
            for(::parser::wasm::standard::wasm1::type::wasm_u32 tmp{0u}; auto curr: binfmt_vers_uneliminated)
            {
                if(curr == tmp) { continue; }
                tmp = curr;
                binfmt_vers.push_back(curr);
            }

            // Define the binfmt needed to process the feature.
            ::std::vector<::parser::wasm::standard::wasm1::type::wasm_u32> binfmt_handlers{};

            // Cannot define a parsing policy that differs from your wasm version.
            [&]<::std::size_t... I>(::std::index_sequence<I...>) constexpr noexcept
            {
                ((
                     [&]<::parser::wasm::concepts::wasm_feature FeatureType>() constexpr noexcept
                     {
                         if constexpr(::parser::wasm::concepts::has_wasm_binfmt_parsering_strategy<FeatureType>)
                         {
                             constexpr auto binfmt_ver{get_binfmt_version<FeatureType>()};
                             binfmt_handlers.push_back(binfmt_ver);
                         }
                     }.template operator()<Fs...[I]>()),
                 ...);
            }(::std::make_index_sequence<sizeof...(Fs)>{});

            // Determine if there is a corresponding parsing scheme by the existing binfmt version.
            ::std::ranges::sort(binfmt_handlers);

            // Check for duplicates
            for(::parser::wasm::standard::wasm1::type::wasm_u32 tmp{0u}; auto curr: binfmt_handlers)
            {
                if(curr == tmp) { ::fast_io::fast_terminate(); }
                tmp = curr;
            }
        }

        /// @brief      Checking for duplicate binfmt version handler functions from tuple
        /// @details    You can pass values directly when passing registers.
        template <::parser::wasm::concepts::wasm_feature... Fs>
            requires (::parser::wasm::concepts::details::abi_transferable_value<::fast_io::tuple<Fs...>>)
        inline consteval void check_has_duplicate_binfmt_handler_from_tuple(::fast_io::tuple<Fs...>) noexcept
        {
            check_has_duplicate_binfmt_handler<Fs...>();
        }

        /// @brief      Checking for duplicate binfmt version handler functions from tuple
        /// @details    You can't pass references on register passes.
        template <::parser::wasm::concepts::wasm_feature... Fs>
            requires (!::parser::wasm::concepts::details::abi_transferable_value<::fast_io::tuple<Fs...>>)
        inline consteval void check_has_duplicate_binfmt_handler_from_tuple(::fast_io::tuple<Fs...> const&) noexcept
        {
            check_has_duplicate_binfmt_handler<Fs...>();
        }

        
    }  // namespace operation
}  // namespace parser::wasm::concepts
