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
# include <climits>
# include <cstring>
# include <limits>
# include <concepts>
# include <bit>
# include <memory>
# include <type_traits>
// macro
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
# include <uwvm2/utils/macro/push_macros.h>
// platform
# if (!defined(__NEWLIB__) || defined(__CYGWIN__)) && !defined(_WIN32) && __has_include(<dirent.h>) && !defined(_PICOLIBC__)
#  include <unistd.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/stat.h>
#  if !(defined(__MSDOS__) || defined(__DJGPP__))
#   include <sys/socket.h>
#  endif
# endif
// import
# include <fast_io.h>
# include <uwvm2/uwvm_predefine/utils/ansies/impl.h>
# include <uwvm2/uwvm_predefine/io/impl.h>
# include <uwvm2/utils/mutex/impl.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/object/memory/linear/impl.h>
# include <uwvm2/imported/wasi/wasip1/abi/impl.h>
# include <uwvm2/imported/wasi/wasip1/fd_manager/impl.h>
# include <uwvm2/imported/wasi/wasip1/memory/impl.h>
# include <uwvm2/imported/wasi/wasip1/environment/impl.h>
# include "base.h"
# include "posix.h"
#endif

#ifndef UWVM_CPP_EXCEPTIONS
# warning "Without enabling C++ exceptions, using this WASI function may cause termination."
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::imported::wasi::wasip1::func
{

    struct alignas(8uz) wasi_fdstat_wasm64_t
    {
        ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t fs_filetype;
        ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t fs_flags;
        ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t fs_rights_base;
        ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t fs_rights_inheriting;
    };

    inline constexpr ::std::size_t size_of_wasi_fdstat_wasm64_t{24uz};

    inline consteval bool is_default_wasi_fdstat_wasm64_data_layout() noexcept
    {
        // In standard layout mode, data can be transferred in a single memcpy operation (static length), improving read efficiency.
        return __builtin_offsetof(wasi_fdstat_wasm64_t, fs_filetype) == 0uz && __builtin_offsetof(wasi_fdstat_wasm64_t, fs_flags) == 2uz &&
               __builtin_offsetof(wasi_fdstat_wasm64_t, fs_rights_base) == 8uz && __builtin_offsetof(wasi_fdstat_wasm64_t, fs_rights_inheriting) == 16uz &&
               sizeof(wasi_fdstat_wasm64_t) == size_of_wasi_fdstat_wasm64_t && alignof(wasi_fdstat_wasm64_t) == 8uz &&
               ::std::endian::native == ::std::endian::little;
    }

    /// @brief     WasiPreview1.fd_fdstat_get
    /// @details   __wasi_errno_t fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t *stat);
    ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t fd_fdstat_get_wasm64(
        ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment<::uwvm2::object::memory::linear::native_memory_t> & env,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t fd,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t stat_ptrsz) noexcept
    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
        if(env.wasip1_memory == nullptr) [[unlikely]]
        {
            // Security issues inherent to virtual machines
            ::uwvm2::utils::debug::trap_and_inform_bug_pos();
        }
#endif
        auto& memory{*env.wasip1_memory};

        auto const trace_wasip1_call{env.trace_wasip1_call};

        if(trace_wasip1_call) [[unlikely]]
        {
#ifdef UWVM
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                u8"[info]  ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"wasip1: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                u8"fd_fdstat_get_wasm64",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"(",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                fd,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                ::fast_io::mnp::addrvw(stat_ptrsz),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8") ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_ORANGE),
                                u8"(wasi-trace)\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
#else
            ::fast_io::io::perr(::fast_io::u8err(),
                                u8"uwvm: [info]  wasip1: fd_fdstat_get_wasm64(",
                                fd,
                                u8", ",
                                ::fast_io::mnp::addrvw(stat_ptrsz),
                                u8") (wasi-trace)\n");
#endif
        }

        // The negative value fd is invalid, and this check prevents subsequent undefined behavior.
        if(fd < 0) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::ebadf; }

        auto& wasm_fd_storage{env.fd_storage};

        // The pointer to `wasm_fd` is fixed and remains unchanged even when the vector within `fd_manager` is resized.
        ::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_t* curr_wasi_fd_t_p;  // no initialize

        // Subsequent operations involving the file descriptor require locking. curr_fd_release_guard release when return.
        ::uwvm2::utils::mutex::mutex_merely_release_guard_t curr_fd_release_guard{};

        {
            // Prevent operations to obtain the size or perform resizing at this time.
            // Only a lock is required when acquiring the unique pointer for the file descriptor. The lock can be released once the acquisition is complete.
            // Since the file descriptor's location is fixed and accessed via the unique pointer,

            // Simply acquiring data using a shared_lock
            ::uwvm2::utils::mutex::rw_shared_guard_t fds_lock{wasm_fd_storage.fds_rwlock};

            // Negative states have been excluded, so the conversion result will only be positive numbers.
            using unsigned_fd_t = ::std::make_unsigned_t<::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t>;
            auto const unsigned_fd{static_cast<unsigned_fd_t>(fd)};

            // On platforms where `size_t` is smaller than the `fd` type, this check must be added.
            constexpr auto size_t_max{::std::numeric_limits<::std::size_t>::max()};
            if constexpr(::std::numeric_limits<unsigned_fd_t>::max() > size_t_max)
            {
                if(unsigned_fd > size_t_max) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::ebadf; }
            }

            auto const fd_opens_pos{static_cast<::std::size_t>(unsigned_fd)};

            // The minimum value in rename_map is greater than opensize.
            if(wasm_fd_storage.opens.size() <= fd_opens_pos)
            {
                // Possibly within the tree being renumbered
                if(auto const renumber_map_iter{wasm_fd_storage.renumber_map.find(fd)}; renumber_map_iter != wasm_fd_storage.renumber_map.end())
                {
                    curr_wasi_fd_t_p = renumber_map_iter->second.fd_p;
                }
                else [[unlikely]]
                {
                    return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::ebadf;
                }
            }
            else
            {
                // The addition here is safe.
                curr_wasi_fd_t_p = wasm_fd_storage.opens.index_unchecked(fd_opens_pos).fd_p;
            }

            // curr_wasi_fd_t_p never nullptr
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
            if(curr_wasi_fd_t_p == nullptr) [[unlikely]]
            {
                // Security issues inherent to virtual machines
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
            }
#endif

            // Other threads will definitely lock fds_rwlock when performing close operations (since they need to access the fd vector). If the current thread
            // is performing fdstat_get, no other thread can be executing any close operations simultaneously, eliminating any destruction issues. Therefore,
            // acquiring the lock at this point is safe. However, the problem arises when, immediately after acquiring the lock and before releasing the manager
            // lock and beginning fd operations, another thread executes a deletion that removes this fd. Subsequent operations by the current thread would then
            // encounter issues. Thus, locking must occur before releasing fds_rwlock.
            curr_fd_release_guard.device_p = ::std::addressof(curr_wasi_fd_t_p->fd_mutex);
            curr_fd_release_guard.lock();

            // After unlocking fds_lock, members within `wasm_fd_storage_t` can no longer be accessed or modified.
        }

        // curr_fd_uniptr is not null.
        auto& curr_fd{*curr_wasi_fd_t_p};

        // If obtained from the renumber map, it will always be the correct value. If obtained from the open vec, it requires checking whether it is closed.
        // Therefore, a unified check is implemented.
        if(curr_fd.close_pos != SIZE_MAX) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::ebadf; }

        // The purpose of `fd_fdstat_get` is solely to read the metadata of an FD (type, flags, rights). It does not access the file contents nor alter any
        // state.

        ::uwvm2::imported::wasi::wasip1::memory::check_memory_bounds_wasm64(memory, stat_ptrsz, size_of_wasi_fdstat_wasm64_t);

        auto const& curr_fd_native_file{curr_fd.file_fd};
        [[maybe_unused]] auto const native_fd{curr_fd_native_file.native_handle()};

        // Query native fd flags via fcntl(F_GETFL) for WASI fdflags mapping
        // All require initialization to prevent subsequent unconfigured settings from causing undefined behavior.
        ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t fs_filetype{};                               // 0
        ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t fs_flags{};                                   // 2
        ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t fs_rights_base{curr_fd.rights_base};           // 8
        ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t fs_rights_inheriting{curr_fd.rights_inherit};  // 16

#if (!defined(__NEWLIB__) || defined(__CYGWIN__)) && !defined(_WIN32) && __has_include(<dirent.h>) && !defined(_PICOLIBC__)
# if defined(__linux__) && defined(__NR_fcntl)
        int const oflags{::fast_io::system_call<__NR_fcntl, int>(native_fd, F_GETFL)};

        if(::fast_io::linux_system_call_fails(oflags)) [[unlikely]]
        {
            int const err{static_cast<int>(-oflags)};
            switch(err)
            {
                // If “ebadf” appears here, it is caused by a WASI implementation issue. This differs from WASI's ‘ebadf’; here, “eio” is used instead.
                case EBADF: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
                case EINVAL: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::einval;
                default: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
            }
        }
# else
        int const oflags{::uwvm2::imported::wasi::wasip1::func::posix::fcntl(native_fd, F_GETFL)};

        if(oflags == -1) [[unlikely]]
        {
            switch(errno)
            {
                // If “ebadf” appears here, it is caused by a WASI implementation issue. This differs from WASI's ‘ebadf’; here, “eio” is used instead.
                case EBADF: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
                case EINVAL: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::einval;
                default: return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
            }
        }
# endif

# ifdef O_APPEND
        if(oflags & O_APPEND) { fs_flags |= ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t::fdflag_append; }
# endif
# ifdef O_DSYNC
        if(oflags & O_DSYNC) { fs_flags |= ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t::fdflag_dsync; }
# endif
# ifdef O_NONBLOCK
        if(oflags & O_NONBLOCK) { fs_flags |= ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t::fdflag_nonblock; }
# endif
# ifdef O_RSYNC
        if(oflags & O_RSYNC) { fs_flags |= ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t::fdflag_rsync; }
# endif
# ifdef O_SYNC
        if(oflags & O_SYNC) { fs_flags |= ::uwvm2::imported::wasi::wasip1::abi::fdflags_wasm64_t::fdflag_sync; }
# endif
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
        switch(curr_fd.file_type)
        {
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::socket:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_socket_stream;

                char so_type{};
                int optlen{static_cast<int>(sizeof(so_type))};
                if(::fast_io::win32::getsockopt(curr_fd.socket_fd.native_handle(),
                                                0xFFFF /*SOL_SOCKET*/,
                                                0x1008 /*SO_TYPE*/,
                                                ::std::addressof(so_type),
                                                ::std::addressof(optlen)) == 0 &&
                   optlen == sizeof(so_type))
                {
                    if(so_type == 2 /*SOCK_DGRAM*/) { fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_socket_dgram; }
                }

                break;
            }
# if defined(_WIN32_WINDOWS)
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::dir:
            {
                // There is no need to use status, as status can only retrieve the creation time.
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                break;
            }
# endif
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::file:
            {
                ::fast_io::posix_file_status curr_fd_status;  // no initialize

# ifdef UWVM_CPP_EXCEPTIONS
                try
# endif
                {
                    curr_fd_status = status(curr_fd_native_file);
                }
# ifdef UWVM_CPP_EXCEPTIONS
                catch(::fast_io::error)
                {
                    return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
                }
# endif

                switch(curr_fd_status.type)
                {
                    case ::fast_io::file_type::none:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::not_found:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::regular:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_regular_file;
                        break;
                    }
                    case ::fast_io::file_type::directory:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                        break;
                    }
                    case ::fast_io::file_type::symlink:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_symbolic_link;
                        break;
                    }
                    case ::fast_io::file_type::block:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_block_device;
                        break;
                    }
                    case ::fast_io::file_type::character:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_character_device;
                        break;
                    }
                    case ::fast_io::file_type::fifo:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::socket:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_socket_stream;
                        break;
                    }
                    case ::fast_io::file_type::unknown:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::remote:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    [[unlikely]] default:
                    {
                        fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                }

                break;
            }
            [[unlikely]] default:
            {
# if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                // Security issues inherent to virtual machines
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
# endif
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
        }
#else
        ::fast_io::posix_file_status curr_fd_status;  // no initialize

# ifdef UWVM_CPP_EXCEPTIONS
        try
# endif
        {
            curr_fd_status = status(curr_fd_native_file);
        }
# ifdef UWVM_CPP_EXCEPTIONS
        catch(::fast_io::error)
        {
            return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
        }
# endif

        switch(curr_fd_status.type)
        {
            case ::fast_io::file_type::none:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
            case ::fast_io::file_type::not_found:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
            case ::fast_io::file_type::regular:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_regular_file;
                break;
            }
            case ::fast_io::file_type::directory:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                break;
            }
            case ::fast_io::file_type::symlink:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_symbolic_link;
                break;
            }
            case ::fast_io::file_type::block:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_block_device;
                break;
            }
            case ::fast_io::file_type::character:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_character_device;
                break;
            }
            case ::fast_io::file_type::fifo:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
            case ::fast_io::file_type::socket:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_socket_stream;

# if (!defined(__NEWLIB__) || defined(__CYGWIN__)) && !defined(_WIN32) &&                                                                                      \
     __has_include(<dirent.h>) && !defined(_PICOLIBC__) && !(defined(__MSDOS__) || defined(__DJGPP__))
                int so_type{};
                auto optlen{static_cast<::socklen_t>(sizeof(so_type))};
                if(::uwvm2::imported::wasi::wasip1::func::posix::getsockopt(native_fd,
                                                                            SOL_SOCKET,
                                                                            SO_TYPE,
                                                                            ::std::addressof(so_type),
                                                                            ::std::addressof(optlen)) == 0 &&
                   optlen == sizeof(so_type))
                {
                    if(so_type == SOCK_DGRAM) { fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_socket_dgram; }
                }
# endif
                break;
            }
            case ::fast_io::file_type::unknown:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
            case ::fast_io::file_type::remote:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
            [[unlikely]] default:
            {
                fs_filetype = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                break;
            }
        }
#endif

        if constexpr(is_default_wasi_fdstat_wasm64_data_layout())
        {
            // If the memory is identical, it is copied directly, which is the most efficient approach.
            wasi_fdstat_wasm64_t const tmp_wasi_fdstat{fs_filetype, fs_flags, fs_rights_base, fs_rights_inheriting};

            ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(
                memory,
                stat_ptrsz,
                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_fdstat)),
                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_fdstat)) + sizeof(tmp_wasi_fdstat));
        }
        else
        {
            // Ensure the structure meets the requirements for wasi memory.
            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                memory,
                stat_ptrsz,
                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(fs_filetype)>>>(fs_filetype));

            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                memory,
                stat_ptrsz + 2u,
                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(fs_flags)>>>(fs_flags));

            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                memory,
                stat_ptrsz + 8u,
                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(fs_rights_base)>>>(fs_rights_base));

            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                memory,
                stat_ptrsz + 16u,
                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(fs_rights_inheriting)>>>(fs_rights_inheriting));
        }

        return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::esuccess;
    }
}  // namespace uwvm2::imported::wasi::wasip1::func

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_pop_macro.h>
#endif

