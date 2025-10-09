﻿/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      GPT-5
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

#include <cstddef>
#include <cstdint>

#include <fast_io.h>

#include <uwvm2/imported/wasi/wasip1/func/clock_res_get.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::clockid_t;
    using ::uwvm2::imported::wasi::wasip1::abi::errno_t;
    using ::uwvm2::imported::wasi::wasip1::abi::timestamp_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory), .argv = {}, .envs = {}, .fd_storage = {}, .mount_dir_roots={}, .trace_wasip1_call = false};

    constexpr wasi_void_ptr_t res_ptr{4096u};

    auto const ret{::uwvm2::imported::wasi::wasip1::func::clock_res_get(env, clockid_t::clock_monotonic, res_ptr)};
    if(ret != errno_t::esuccess) { ::fast_io::fast_terminate(); }

    auto const got_ns{::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm32<std::underlying_type_t<timestamp_t>>(memory, res_ptr)};

    // Sanity: resolution should be > 0 and < 1 second
    if(got_ns == 0u)
    {
        ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_res_get: zero resolution");
        ::fast_io::fast_terminate();
    }
    if(got_ns >= 1'000'000'000ull)
    {
        ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_res_get: resolution >= 1s");
        ::fast_io::fast_terminate();
    }
}

