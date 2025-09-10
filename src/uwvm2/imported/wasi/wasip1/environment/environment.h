﻿
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

#ifndef UWVM_MODULE
// std
# include <cstddef>
# include <cstdint>
# include <cstring>
# include <climits>
# include <version>
# include <limits>
# include <concepts>
# include <bit>
// macro
# include <uwvm2/utils/macro/push_macros.h>
// import
# include <fast_io.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/parser/wasm/standard/wasm1/type/impl.h>
# include <uwvm2/object/memory/impl.h>
# include <uwvm2/imported/wasi/wasip1/abi/impl.h>
# include <uwvm2/imported/wasi/wasip1/fd_manager/impl.h>
# include <uwvm2/imported/wasi/wasip1/memory/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::imported::wasi::wasip1::environment
{
    template <typename memory_type>
    concept wasip1_memory = requires(memory_type& mem, ::std::size_t offset, ::std::byte* begin, ::std::byte* end) {
        { ::uwvm2::imported::wasi::wasip1::memory::check_memory_bounds(mem, offset, 0uz) };

        { ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(mem, offset) };
        { ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64>(mem, offset) };
        {
            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(
                mem,
                offset,
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32{})
        };
        {
            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64>(
                mem,
                offset,
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64{})
        };
        { ::uwvm2::imported::wasi::wasip1::memory::read_all_from_memory(mem, offset, begin, end) };
        { ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory(mem, offset, begin, end) };

        {
            ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_unchecked<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(mem,
                                                                                                                                                       offset)
        };
        {
            ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_unchecked<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64>(mem,
                                                                                                                                                       offset)
        };
        {
            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_unchecked<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(
                mem,
                offset,
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32{})
        };
        {
            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_unchecked<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64>(
                mem,
                offset,
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64{})
        };
        { ::uwvm2::imported::wasi::wasip1::memory::read_all_from_memory_unchecked(mem, offset, begin, end) };
        { ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_unchecked(mem, offset, begin, end) };
    };
    
#if __cpp_lib_atomic_wait >= 201907L
    static_assert(wasip1_memory<::uwvm2::object::memory::linear::allocator_memory_t>);
#endif

#if defined(UWVM_SUPPORT_MMAP)
    static_assert(wasip1_memory<::uwvm2::object::memory::linear::mmap_memory_t>);
#endif

    template <wasip1_memory memory_type>
    struct wasip1_environment
    {
        // wasip1 only support only one memory (memory[0])
        memory_type& wasip1_memory;
        // allow user custom argv, generate argv from system api is not supported
        ::uwvm2::utils::container::vector<::uwvm2::utils::container::u8string_view> argv;
        // allow user custom envs, use current vm envs by default
        ::uwvm2::utils::container::vector<::uwvm2::utils::container::u8string_view> envs;
        bool trace_wasip1_call;
    };

}  // namespace uwvm2::imported::wasi::wasip1::environment

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
#endif
