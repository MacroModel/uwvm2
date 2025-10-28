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

#include <uwvm2/imported/wasi/wasip1/func/fd_fdstat_set_flags.h>
#include <uwvm2/imported/wasi/wasip1/func/fd_fdstat_get.h>
#include <uwvm2/imported/wasi/wasip1/func/fd_close.h>
#include <uwvm2/imported/wasi/wasip1/func/posix.h>

#if (!defined(__NEWLIB__) || defined(__CYGWIN__)) && !defined(_WIN32) && __has_include(<dirent.h>) && !defined(_PICOLIBC__) && \
    !(defined(__MSDOS__) || defined(__DJGPP__))

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_t;
    using ::uwvm2::imported::wasi::wasip1::abi::fdflags_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory),
                                            .argv = {},
                                            .envs = {},
                                            .fd_storage = {},
                                            .mount_dir_roots = {},
                                            .trace_wasip1_call = false};

    // Prepare fd table
    env.fd_storage.opens.resize(6uz);

    // Negative fd => ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(-1), static_cast<fdflags_t>(0));
        if(ret != errno_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: expected ebadf for negative fd");
            ::fast_io::fast_terminate();
        }
    }

    // Setup fd=4 with valid file and all rights
    auto& fd4 = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
    fd4.rights_base = static_cast<rights_t>(-1);
    fd4.rights_inherit = static_cast<rights_t>(-1);
    fd4.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::file);
    fd4.wasi_fd.ptr->wasi_fd_storage.storage.file_fd =
        ::fast_io::native_file{u8"test_fd_fdstat_set_flags.tmp", ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};

    int const native_fd = fd4.wasi_fd.ptr->wasi_fd_storage.storage.file_fd.native_handle();

    // Baseline: ensure we can read current flags
    int const curr0 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
    if(curr0 == -1)
    {
        ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL failed");
        ::fast_io::fast_terminate();
    }

    // Case 1: set only APPEND, verify APPEND is set and other managed bits cleared
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_append);
        if(ret != errno_t::esuccess)
        {
            if(ret == errno_t::enotsup)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: append not supported on this platform, skip");
                goto case1b;
            }
            else
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set append expected esuccess");
                ::fast_io::fast_terminate();
            }
        }

        int const flags1 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(flags1 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after set failed");
            ::fast_io::fast_terminate();
        }

# if defined(O_APPEND) && O_APPEND != 0
        if(ret == errno_t::esuccess && (flags1 & O_APPEND) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND not set after call");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if(ret == errno_t::esuccess && (flags1 & O_NONBLOCK) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::esuccess && (flags1 & O_DSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::esuccess && (flags1 & O_RSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::esuccess && (flags1 & O_SYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
    }
[[maybe_unused]] case1b:
    // Case 1b: set only NONBLOCK, verify NONBLOCK is set and others cleared; ENOTSUP -> skip
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_nonblock);
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: nonblock not supported on this platform, skip");
            goto case1c;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set nonblock expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after set nonblock failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if(ret == errno_t::esuccess && (f & O_NONBLOCK) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK not set");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_APPEND) && O_APPEND != 0
        if(ret == errno_t::esuccess && (f & O_APPEND) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::esuccess && (f & O_DSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::esuccess && (f & O_RSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::esuccess && (f & O_SYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif

        // Clear all flags
        if(ret == errno_t::esuccess)
        {
            auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
            if(clr != errno_t::esuccess)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear nonblock expected esuccess");
                ::fast_io::fast_terminate();
            }
        }
    }

[[maybe_unused]] case1c:
    // Case 1c: set only DSYNC, verify DSYNC is set and others cleared; ENOTSUP -> skip
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_dsync);
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: dsync not supported on this platform, skip");
            goto case1d;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set dsync expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after set dsync failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::esuccess && (f & O_DSYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC not set");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_APPEND) && O_APPEND != 0
        if(ret == errno_t::esuccess && (f & O_APPEND) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if(ret == errno_t::esuccess && (f & O_NONBLOCK) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::esuccess && (f & O_RSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::esuccess && (f & O_SYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif

        // Clear all flags
        if(ret == errno_t::esuccess)
        {
            auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
            if(clr != errno_t::esuccess)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear dsync expected esuccess");
                ::fast_io::fast_terminate();
            }
        }
    }

[[maybe_unused]] case1d:
    // Case 1d: set only RSYNC, verify RSYNC is set and others cleared; ENOTSUP -> skip
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_rsync);
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: rsync not supported on this platform, skip");
            goto case1e;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set rsync expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after set rsync failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::esuccess && (f & O_RSYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC not set");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_APPEND) && O_APPEND != 0
        if(ret == errno_t::esuccess && (f & O_APPEND) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if(ret == errno_t::esuccess && (f & O_NONBLOCK) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::esuccess && (f & O_DSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::esuccess && (f & O_SYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif

        // Clear all flags
        if(ret == errno_t::esuccess)
        {
            auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
            if(clr != errno_t::esuccess)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear rsync expected esuccess");
                ::fast_io::fast_terminate();
            }
        }
    }

[[maybe_unused]] case1e:
    // Case 1e: set only SYNC, verify SYNC is set and others cleared; ENOTSUP -> skip
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_sync);
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: sync not supported on this platform, skip");
            goto case2;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set sync expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after set sync failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::esuccess && (f & O_SYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC not set");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_APPEND) && O_APPEND != 0
        if(ret == errno_t::esuccess && (f & O_APPEND) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if(ret == errno_t::esuccess && (f & O_NONBLOCK) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::esuccess && (f & O_DSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::esuccess && (f & O_RSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC should be cleared when not requested");
            ::fast_io::fast_terminate();
        }
# endif

        // Clear all flags
        if(ret == errno_t::esuccess)
        {
            auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
            if(clr != errno_t::esuccess)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear sync expected esuccess");
                ::fast_io::fast_terminate();
            }
        }
    }

[[maybe_unused]] case2:
    // Case 2: clear APPEND (pass 0), verify APPEND cleared
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
        if(ret != errno_t::esuccess && ret != errno_t::enosys)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear append expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const flags2 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(flags2 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after clear failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_APPEND) && O_APPEND != 0
        if((flags2 & O_APPEND) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_APPEND should be cleared");
            ::fast_io::fast_terminate();
        }
# endif
    }

    // Case 3: set NONBLOCK through WASI and verify
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_nonblock);
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: nonblock not supported on this platform, skip");
            goto case3b;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set nonblock expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const flags3 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(flags3 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after nonblock failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if((flags3 & O_NONBLOCK) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK not set");
            ::fast_io::fast_terminate();
        }
# endif
    }

[[maybe_unused]] case3b:
    // Case 3b: clear NONBLOCK by passing 0
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear nonblock expected esuccess");
            ::fast_io::fast_terminate();
        }

        int const flags4 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(flags4 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after clear nonblock failed");
            ::fast_io::fast_terminate();
        }
# if defined(O_NONBLOCK) && O_NONBLOCK != 0
        if((flags4 & O_NONBLOCK) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_NONBLOCK should be cleared");
            ::fast_io::fast_terminate();
        }
# endif
    }

    // Case 6: DSYNC set/clear with platform variance
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_dsync);
# if defined(O_DSYNC) && O_DSYNC != 0
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: dsync not supported on this platform, skip");
            goto case7;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set dsync expected esuccess");
            ::fast_io::fast_terminate();
        }
        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after dsync failed");
            ::fast_io::fast_terminate();
        }
        if((f & O_DSYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC not set");
            ::fast_io::fast_terminate();
        }
# else
        if(ret != errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set dsync expected enotsup on this platform");
            ::fast_io::fast_terminate();
        }
# endif

        // Clear all managed bits and ensure DSYNC cleared when available
        auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
        if(clr != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear dsync expected esuccess");
            ::fast_io::fast_terminate();
        }
# if defined(O_DSYNC) && O_DSYNC != 0
        int const f2 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f2 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after clear dsync failed");
            ::fast_io::fast_terminate();
        }
        if((f2 & O_DSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_DSYNC should be cleared");
            ::fast_io::fast_terminate();
        }
# endif
    }

[[maybe_unused]] case7:
    // Case 7: SYNC set/clear with platform variance
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_sync);
# if defined(O_SYNC) && O_SYNC != 0
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: sync not supported on this platform, skip");
            goto case8;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set sync expected esuccess");
            ::fast_io::fast_terminate();
        }
        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after sync failed");
            ::fast_io::fast_terminate();
        }
        if((f & O_SYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC not set");
            ::fast_io::fast_terminate();
        }
# else
        if(ret != errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set sync expected enotsup on this platform");
            ::fast_io::fast_terminate();
        }
# endif

        auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
        if(clr != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear sync expected esuccess");
            ::fast_io::fast_terminate();
        }
# if defined(O_SYNC) && O_SYNC != 0
        int const f2 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f2 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after clear sync failed");
            ::fast_io::fast_terminate();
        }
        if((f2 & O_SYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_SYNC should be cleared");
            ::fast_io::fast_terminate();
        }
# endif
    }

[[maybe_unused]] case8:
    // Case 8: RSYNC set/clear with platform variance
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_rsync);
# if defined(O_RSYNC) && O_RSYNC != 0
        if(ret == errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: rsync not supported on this platform, skip");
            goto after_case8;
        }
        else if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set rsync expected esuccess");
            ::fast_io::fast_terminate();
        }
        int const f = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after rsync failed");
            ::fast_io::fast_terminate();
        }
        if((f & O_RSYNC) == 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC not set");
            ::fast_io::fast_terminate();
        }
# else
        if(ret != errno_t::enotsup)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: set rsync expected enotsup on this platform");
            ::fast_io::fast_terminate();
        }
# endif

        auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
        if(clr != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: clear rsync expected esuccess");
            ::fast_io::fast_terminate();
        }
# if defined(O_RSYNC) && O_RSYNC != 0
        int const f2 = ::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL);
        if(f2 == -1)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: F_GETFL after clear rsync failed");
            ::fast_io::fast_terminate();
        }
        if((f2 & O_RSYNC) != 0)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: O_RSYNC should be cleared");
            ::fast_io::fast_terminate();
        }
# endif
    }

[[maybe_unused]] after_case8:
    // Case 4: enotcapable when rights missing
    {
        auto& fde = *env.fd_storage.opens.index_unchecked(3uz).fd_p;
        fde.rights_base = static_cast<rights_t>(0);
        fde.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::file);
        fde.wasi_fd.ptr->wasi_fd_storage.storage.file_fd =
            ::fast_io::native_file{u8"test_fd_fdstat_set_flags_rights.tmp",
                                   ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(3), fdflags_t::fdflag_append);
        if(ret != errno_t::enotcapable)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: expected enotcapable when rights missing");
            ::fast_io::fast_terminate();
        }
    }

    // Case 5: ebadf after fd_close
    {
        auto& fde = *env.fd_storage.opens.index_unchecked(2uz).fd_p;
        fde.rights_base = static_cast<rights_t>(-1);
        fde.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::file);
        fde.wasi_fd.ptr->wasi_fd_storage.storage.file_fd =
            ::fast_io::native_file{u8"test_fd_fdstat_set_flags_close.tmp",
                                   ::fast_io::open_mode::out | ::fast_io::open_mode::creat | ::fast_io::open_mode::trunc};

        auto const closed = ::uwvm2::imported::wasi::wasip1::func::fd_close(env, static_cast<wasi_posix_fd_t>(2));
        if(closed != errno_t::esuccess)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: close expected esuccess");
            ::fast_io::fast_terminate();
        }

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(2), fdflags_t::fdflag_append);
        if(ret != errno_t::ebadf)
        {
            ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags: expected ebadf after fd_close");
            ::fast_io::fast_terminate();
        }
    }

# if defined(_WIN32) && !defined(__CYGWIN__)
    {
        using ::uwvm2::imported::wasi::wasip1::abi::errno_t;
        using ::uwvm2::imported::wasi::wasip1::abi::fdflags_t;
        using ::uwvm2::imported::wasi::wasip1::abi::rights_t;
        using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t;
        using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
        using ::uwvm2::object::memory::linear::native_memory_t;

        native_memory_t memory{};
        memory.init_by_page_count(1uz);

        wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory),
                                                .argv = {},
                                                .envs = {},
                                                .fd_storage = {},
                                                .mount_dir_roots = {},
                                                .trace_wasip1_call = false};

        // Prepare fd table
        env.fd_storage.opens.resize(6uz);

        // Setup fd=4 as a Win32 socket with all rights
        {
            auto& fd4 = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
            fd4.rights_base = static_cast<rights_t>(-1);
            fd4.rights_inherit = static_cast<rights_t>(-1);
            fd4.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::socket);
            fd4.wasi_fd.ptr->wasi_fd_storage.storage.socket_fd =
                ::fast_io::win32_socket_file{::fast_io::socket_family::ipv4, ::fast_io::socket_type::stream, ::fast_io::socket_protocol::tcp};

            // nonblock should be supported on sockets
            {
                auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_nonblock);
                if(ret != errno_t::esuccess)
                {
                    ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): set nonblock expected esuccess");
                    ::fast_io::fast_terminate();
                }

                auto const clr = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), static_cast<fdflags_t>(0));
                if(clr != errno_t::esuccess)
                {
                    ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): clear nonblock expected esuccess");
                    ::fast_io::fast_terminate();
                }
            }

            // Other flags should be ENOTSUP for sockets on Win32
            {
                auto const r1 = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_append);
                auto const r2 = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_dsync);
                auto const r3 = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_rsync);
                auto const r4 = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(4), fdflags_t::fdflag_sync);
                if(!(r1 == errno_t::enotsup && r2 == errno_t::enotsup && r3 == errno_t::enotsup && r4 == errno_t::enotsup))
                {
                    ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): unsupported flags should return enotsup");
                    ::fast_io::fast_terminate();
                }
            }
        }

        // Rights missing => enotcapable
        {
            auto& fde = *env.fd_storage.opens.index_unchecked(3uz).fd_p;
            fde.rights_base = static_cast<rights_t>(0);
            fde.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::socket);
            fde.wasi_fd.ptr->wasi_fd_storage.storage.socket_fd =
                ::fast_io::win32_socket_file{::fast_io::socket_family::ipv4, ::fast_io::socket_type::stream, ::fast_io::socket_protocol::tcp};

            auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(3), fdflags_t::fdflag_nonblock);
            if(ret != errno_t::enotcapable)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): expected enotcapable when rights missing");
                ::fast_io::fast_terminate();
            }
        }

        // After close => ebadf
        {
            auto& fde = *env.fd_storage.opens.index_unchecked(2uz).fd_p;
            fde.rights_base = static_cast<rights_t>(-1);
            fde.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::socket);
            fde.wasi_fd.ptr->wasi_fd_storage.storage.socket_fd =
                ::fast_io::win32_socket_file{::fast_io::socket_family::ipv4, ::fast_io::socket_type::stream, ::fast_io::socket_protocol::tcp};

            auto const closed = ::uwvm2::imported::wasi::wasip1::func::fd_close(env, static_cast<wasi_posix_fd_t>(2));
            if(closed != errno_t::esuccess)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): close expected esuccess");
                ::fast_io::fast_terminate();
            }

            auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_fdstat_set_flags(env, static_cast<wasi_posix_fd_t>(2), fdflags_t::fdflag_nonblock);
            if(ret != errno_t::ebadf)
            {
                ::fast_io::io::perrln(::fast_io::u8err(), u8"fd_fdstat_set_flags(win32-socket): expected ebadf after fd_close");
                ::fast_io::fast_terminate();
            }
        }
    }
# endif
}

#else

int main() {}

#endif

