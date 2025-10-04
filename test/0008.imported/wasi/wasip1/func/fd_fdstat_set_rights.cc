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

#include <uwvm2/imported/wasi/wasip1/func/fd_fdstat_set_rights.h>
#include <uwvm2/imported/wasi/wasip1/func/fd_fdstat_get.h>
#include <uwvm2/imported/wasi/wasip1/func/fd_close.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory), .argv = {}, .envs = {}, .fd_storage = {}, .mount_root={}, .trace_wasip1_call = false};

    // Prepare fd table
    env.fd_storage.opens.resize(8uz);

    // Case 0: negative fd => ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env,
                                                                                    static_cast<wasi_posix_fd_t>(-1),
                                                                                    static_cast<rights_t>(0),
                                                                                    static_cast<rights_t>(0));
        if(ret != errno_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: expected ebadf for negative fd");
            ::fast_io::fast_terminate();
        }
    }

    // Setup fd=4 with valid file and all rights
    auto& fd4 = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
    fd4.file_fd = ::fast_io::native_file{u8"test_fd_fdstat_set_rights.tmp",
                                         ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};
    fd4.rights_base = static_cast<rights_t>(-1);
    fd4.rights_inherit = static_cast<rights_t>(-1);

    // Read back helper via fd_fdstat_get
    auto read_rights = [&] (wasi_posix_fd_t fd, rights_t& out_base, rights_t& out_inh) {
        constexpr wasi_void_ptr_t stat_ptr{4096u};
        auto const gr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_get(env, fd, stat_ptr);
        if(gr != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: fd_fdstat_get expected esuccess");
            ::fast_io::fast_terminate();
        }
        using u_rights = ::std::underlying_type_t<rights_t>;
        u_rights const base_u = ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm32<u_rights>(memory, static_cast<wasi_void_ptr_t>(stat_ptr + 8u));
        u_rights const inh_u = ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm32<u_rights>(memory, static_cast<wasi_void_ptr_t>(stat_ptr + 16u));
        out_base = static_cast<rights_t>(base_u);
        out_inh = static_cast<rights_t>(inh_u);
    };

    // Case 1: shrink base only (success)
    {
        rights_t const new_base = rights_t::right_fd_read;
        rights_t const new_inh = static_cast<rights_t>(-1); // keep inheriting unchanged
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env, static_cast<wasi_posix_fd_t>(4), new_base, new_inh);
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: shrink base expected esuccess");
            ::fast_io::fast_terminate();
        }

        rights_t rb{}, ri{};
        read_rights(static_cast<wasi_posix_fd_t>(4), rb, ri);
        if(rb != new_base || ri != new_inh)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: shrink base verification failed");
            ::fast_io::fast_terminate();
        }
    }

    // Case 2: shrink inheriting only (success), base unchanged
    {
        rights_t curr_base{}, curr_inh{};
        read_rights(static_cast<wasi_posix_fd_t>(4), curr_base, curr_inh);

        rights_t const new_base = curr_base;                 // unchanged
        rights_t const new_inh = static_cast<rights_t>(0);   // shrink to zero
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env, static_cast<wasi_posix_fd_t>(4), new_base, new_inh);
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: shrink inheriting expected esuccess");
            ::fast_io::fast_terminate();
        }

        rights_t rb{}, ri{};
        read_rights(static_cast<wasi_posix_fd_t>(4), rb, ri);
        if(rb != new_base || ri != new_inh)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: shrink inheriting verification failed");
            ::fast_io::fast_terminate();
        }
    }

    // Case 3: attempt to increase base (should be enotcapable)
    {
        rights_t curr_base{}, curr_inh{};
        read_rights(static_cast<wasi_posix_fd_t>(4), curr_base, curr_inh);

        rights_t const new_base = static_cast<rights_t>(static_cast<::std::underlying_type_t<rights_t>>(curr_base) |
                                                        static_cast<::std::underlying_type_t<rights_t>>(rights_t::right_fd_write));
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env,
                                                                                    static_cast<wasi_posix_fd_t>(4),
                                                                                    new_base,
                                                                                    curr_inh);
        if(ret != errno_t::enotcapable)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: increase base expected enotcapable");
            ::fast_io::fast_terminate();
        }
    }

    // Case 4: attempt to increase inheriting (should be enotcapable)
    {
        rights_t curr_base{}, curr_inh{};
        read_rights(static_cast<wasi_posix_fd_t>(4), curr_base, curr_inh);

        rights_t const inc_inh = rights_t::right_fd_write;
        rights_t const new_inh = static_cast<rights_t>(static_cast<::std::underlying_type_t<rights_t>>(curr_inh) |
                                                       static_cast<::std::underlying_type_t<rights_t>>(inc_inh));
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env,
                                                                                    static_cast<wasi_posix_fd_t>(4),
                                                                                    curr_base,
                                                                                    new_inh);
        if(ret != errno_t::enotcapable)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: increase inheriting expected enotcapable");
            ::fast_io::fast_terminate();
        }
    }

    // Case 5: inheriting may contain bits that base does not (allowed by spec)
    {
        auto& fd5 = *env.fd_storage.opens.index_unchecked(5uz).fd_p;
        fd5.file_fd = ::fast_io::native_file{u8"test_fd_fdstat_set_rights_case5.tmp",
                                             ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};
        fd5.rights_base = static_cast<rights_t>(0);
        fd5.rights_inherit = rights_t::right_fd_write; // inheriting not subset of base

        rights_t const new_base = fd5.rights_base;     // unchanged
        rights_t const new_inh = fd5.rights_inherit;   // unchanged

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env, static_cast<wasi_posix_fd_t>(5), new_base, new_inh);
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: inheriting-not-subset-of-base scenario should succeed");
            ::fast_io::fast_terminate();
        }

        rights_t rb{}, ri{};
        read_rights(static_cast<wasi_posix_fd_t>(5), rb, ri);
        if(rb != new_base || ri != new_inh)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: case5 verification failed");
            ::fast_io::fast_terminate();
        }
    }

    // Case 6: ebadf after fd_close
    {
        auto& fde = *env.fd_storage.opens.index_unchecked(2uz).fd_p;
        fde.file_fd = ::fast_io::native_file{u8"test_fd_fdstat_set_rights_close.tmp",
                                             ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};
        fde.rights_base = static_cast<rights_t>(-1);
        fde.rights_inherit = static_cast<rights_t>(-1);

        auto const closed = ::uwvm2::imported::wasi::wasip1::func::fd_close(env, static_cast<wasi_posix_fd_t>(2));
        if(closed != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: close expected esuccess");
            ::fast_io::fast_terminate();
        }

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_rights(env,
                                                                                    static_cast<wasi_posix_fd_t>(2),
                                                                                    static_cast<rights_t>(0),
                                                                                    static_cast<rights_t>(0));
        if(ret != errno_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_rights: expected ebadf after fd_close");
            ::fast_io::fast_terminate();
        }
    }
}


