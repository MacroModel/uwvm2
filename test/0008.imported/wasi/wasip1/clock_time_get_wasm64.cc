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

#include <uwvm2/imported/wasi/wasip1/func/clock_time_get_wasm64.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::clockid_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::timestamp_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = memory, .argv = {}, .envs = {}, .trace_wasip1_call = false};

    constexpr wasi_void_ptr_wasm64_t time_ptr{4096u};

    // Test 1: Basic monotonic clock with zero precision
    {
        auto const ret{::uwvm2::imported::wasi::wasip1::func::clock_time_get_wasm64(env, clockid_wasm64_t::clock_monotonic, timestamp_wasm64_t{}, time_ptr)};
        if(ret != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        auto const time1{::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm64<std::underlying_type_t<timestamp_wasm64_t>>(memory, time_ptr)};

        // Sanity: time should be > 0
        if(time1 == 0u)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_time_get_wasm64: zero time");
            ::fast_io::fast_terminate();
        }

        // Get time again to verify monotonicity
        auto const ret2{::uwvm2::imported::wasi::wasip1::func::clock_time_get_wasm64(env, clockid_wasm64_t::clock_monotonic, timestamp_wasm64_t{}, time_ptr)};
        if(ret2 != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        auto const time2{::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm64<std::underlying_type_t<timestamp_wasm64_t>>(memory, time_ptr)};

        if(time2 < time1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_time_get_wasm64: time went backwards");
            ::fast_io::fast_terminate();
        }
    }

    // Test 2: Different precision hints should not affect returned time
    {
        auto const ret1{::uwvm2::imported::wasi::wasip1::func::clock_time_get_wasm64(env, clockid_wasm64_t::clock_monotonic, static_cast<timestamp_wasm64_t>(1'000'000u), time_ptr)};
        if(ret1 != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        auto const time1{::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm64<std::underlying_type_t<timestamp_wasm64_t>>(memory, time_ptr)};

        auto const ret2{::uwvm2::imported::wasi::wasip1::func::clock_time_get_wasm64(env, clockid_wasm64_t::clock_monotonic, static_cast<timestamp_wasm64_t>(1'000'000'000u), time_ptr)};
        if(ret2 != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        auto const time2{::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm64<std::underlying_type_t<timestamp_wasm64_t>>(memory, time_ptr)};

        // Times should be monotonic regardless of precision hint
        if(time2 < time1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_time_get_wasm64: time went backwards with different precisions");
            ::fast_io::fast_terminate();
        }
    }

    // Test 3: Invalid clock ID should return einval
    {
        auto const ret{::uwvm2::imported::wasi::wasip1::func::clock_time_get_wasm64(env, static_cast<clockid_wasm64_t>(99), timestamp_wasm64_t{}, time_ptr)};
        if(ret != errno_wasm64_t::einval)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"clock_time_get_wasm64: invalid clockid did not return einval");
            ::fast_io::fast_terminate();
        }
    }
}
