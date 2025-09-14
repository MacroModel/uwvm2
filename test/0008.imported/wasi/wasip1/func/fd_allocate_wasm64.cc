/*************************************************************
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

#include <uwvm2/imported/wasi/wasip1/func/fd_allocate_wasm64.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::filesize_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = memory, .argv = {}, .envs = {}, .fd_storage = {}, .trace_wasip1_call = false};

    // Prepare fd table: ensure indices [0..4] exist with valid entries
    env.fd_storage.opens.resize(5uz);

    // Case 1: esuccess when len == 0 and rights ok
    {
        env.fd_storage.opens.index_unchecked(3uz).fd_p->file_fd = ::fast_io::posix_file{u8"test_fd_allocate.log", ::fast_io::open_mode::out};
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_allocate_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(3),
                                                                                   static_cast<filesize_wasm64_t>(0),
                                                                                   static_cast<filesize_wasm64_t>(0));
        if(ret != errno_wasm64_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_allocate_wasm64: expected esuccess when len==0");
            ::fast_io::fast_terminate();
        }
    }

    // Case 2: enotcapable when rights do not include right_fd_allocate
    {
        env.fd_storage.opens.index_unchecked(4uz).fd_p->rights_base = static_cast<rights_t>(0);
        env.fd_storage.opens.index_unchecked(4uz).fd_p->file_fd = ::fast_io::posix_file{u8"test_fd_allocate.log", ::fast_io::open_mode::out};

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_allocate_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(4),
                                                                                   static_cast<filesize_wasm64_t>(0),
                                                                                   static_cast<filesize_wasm64_t>(0));
        if(ret != errno_wasm64_t::enotcapable)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_allocate_wasm64: expected enotcapable when rights missing");
            ::fast_io::fast_terminate();
        }
    }

    // Case 3: ebadf for negative fd
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_allocate_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(-1),
                                                                                   static_cast<filesize_wasm64_t>(0),
                                                                                   static_cast<filesize_wasm64_t>(0));
        if(ret != errno_wasm64_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_allocate_wasm64: expected ebadf for negative fd");
            ::fast_io::fast_terminate();
        }
    }
}


