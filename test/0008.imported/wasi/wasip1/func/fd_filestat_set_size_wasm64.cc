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

#include <fast_io.h>

#include <uwvm2/imported/wasi/wasip1/func/fd_filestat_set_size_wasm64.h>
#include <uwvm2/imported/wasi/wasip1/func/fd_close_wasm64.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::filesize_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory), .argv = {}, .envs = {}, .fd_storage = {}, .mount_root={}, .trace_wasip1_call = false};

    env.fd_storage.opens.resize(6uz);

    // Case 0: negative fd → ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_filestat_set_size_wasm64(env,
                                                                                            static_cast<wasi_posix_fd_wasm64_t>(-1),
                                                                                            static_cast<filesize_wasm64_t>(0));
        if(ret != errno_wasm64_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: expected ebadf for negative fd");
            ::fast_io::fast_terminate();
        }
    }

    // Prepare a regular file at fd=4
    env.fd_storage.opens.index_unchecked(4uz).fd_p->rights_base = static_cast<rights_wasm64_t>(-1);
    env.fd_storage.opens.index_unchecked(4uz).fd_p->rights_inherit = static_cast<rights_wasm64_t>(-1);
    env.fd_storage.opens.index_unchecked(4uz).fd_p->file_fd =
        ::fast_io::native_file{u8"test_fd_filestat_set_size_wasm64_regular.tmp",
                               ::fast_io::open_mode::out | ::fast_io::open_mode::trunc | ::fast_io::open_mode::creat};

    // Case 1: success to extend file to 8192
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_filestat_set_size_wasm64(env,
                                                                                            static_cast<wasi_posix_fd_wasm64_t>(4),
                                                                                            static_cast<filesize_wasm64_t>(8192u));
        if(ret != errno_wasm64_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: expected esuccess for regular file");
            ::fast_io::fast_terminate();
        }

        auto const &nf = env.fd_storage.opens.index_unchecked(4uz).fd_p->file_fd;
        auto const st = status(nf);
        if(st.size != 8192u)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: file size should be 8192");
            ::fast_io::fast_terminate();
        }
    }

    // Case 2: enotcapable when rights do not include right_fd_filestat_set_size
    {
        env.fd_storage.opens.index_unchecked(5uz).fd_p->rights_base = static_cast<rights_wasm64_t>(0);
        env.fd_storage.opens.index_unchecked(5uz).fd_p->rights_inherit = static_cast<rights_wasm64_t>(0);
        env.fd_storage.opens.index_unchecked(5uz).fd_p->file_fd =
            ::fast_io::native_file{u8"test_fd_filestat_set_size_wasm64_rights.tmp",
                                   ::fast_io::open_mode::out | ::fast_io::open_mode::trunc | ::fast_io::open_mode::creat};

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_filestat_set_size_wasm64(env,
                                                                                            static_cast<wasi_posix_fd_wasm64_t>(5),
                                                                                            static_cast<filesize_wasm64_t>(2048u));
        if(ret != errno_wasm64_t::enotcapable)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: expected enotcapable when rights missing");
            ::fast_io::fast_terminate();
        }
    }

    // Case 3: ebadf after fd_close
    {
        auto& fde = *env.fd_storage.opens.index_unchecked(2uz).fd_p;
        fde.rights_base = static_cast<rights_wasm64_t>(-1);
        fde.rights_inherit = static_cast<rights_wasm64_t>(-1);
        fde.file_fd = ::fast_io::native_file{u8"test_fd_filestat_set_size_wasm64_close.tmp",
                                             ::fast_io::open_mode::out | ::fast_io::open_mode::trunc | ::fast_io::open_mode::creat};

        auto const closed = ::uwvm2::imported::wasi::wasip1::func::fd_close_wasm64(env, static_cast<wasi_posix_fd_wasm64_t>(2));
        if(closed != errno_wasm64_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: close expected esuccess");
            ::fast_io::fast_terminate();
        }

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_filestat_set_size_wasm64(env,
                                                                                            static_cast<wasi_posix_fd_wasm64_t>(2),
                                                                                            static_cast<filesize_wasm64_t>(1024u));
        if(ret != errno_wasm64_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_filestat_set_size_wasm64: expected ebadf after fd_close");
            ::fast_io::fast_terminate();
        }
    }
}



