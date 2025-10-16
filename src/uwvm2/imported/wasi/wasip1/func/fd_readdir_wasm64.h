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
# include <atomic>
// macro
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
# include <uwvm2/utils/macro/push_macros.h>
// platform
# if !defined(_WIN32)
#  include <errno.h>
#  include <unistd.h>
#  include <sys/stat.h>
# endif
// import
# include <fast_io.h>
# include <uwvm2/uwvm_predefine/utils/ansies/impl.h>
# include <uwvm2/uwvm_predefine/io/impl.h>
# include <uwvm2/utils/mutex/impl.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/utils/container/impl.h>
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
    struct alignas(8uz) wasi_dirent_wasm64_t
    {
        ::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t d_next;
        ::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t d_ino;
        ::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t d_namlen;
        ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t d_type;
    };

    inline constexpr ::std::size_t size_of_wasi_dirent_wasm64_t{24uz};

    inline consteval bool is_default_wasi_dirent_wasm64_data_layout() noexcept
    {
        // static layout identical to wasm32 dirent, but using wasm64 ABI typedefs
        return __builtin_offsetof(wasi_dirent_wasm64_t, d_next) == 0uz && __builtin_offsetof(wasi_dirent_wasm64_t, d_ino) == 8uz &&
               __builtin_offsetof(wasi_dirent_wasm64_t, d_namlen) == 16uz && __builtin_offsetof(wasi_dirent_wasm64_t, d_type) == 20uz &&
               sizeof(wasi_dirent_wasm64_t) == size_of_wasi_dirent_wasm64_t && alignof(wasi_dirent_wasm64_t) == 8uz &&
               ::std::endian::native == ::std::endian::little;
    }

    /// @brief     WasiPreview1.fd_readdir (WASM64 ABI)
    /// @details   __wasi_errno_t __wasi_fd_readdir(__wasi_fd_t fd, void *buf, __wasi_size_t buf_len, __wasi_dircookie cookie, __wasi_size_t *buf_used);
    ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t fd_readdir_wasm64(
        ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment<::uwvm2::object::memory::linear::native_memory_t> & env,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t fd,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t buf_ptrsz,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t buf_len,
        ::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t cookie,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t buf_used_ptrsz) noexcept
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
                                u8"fd_readdir_wasm64",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"(",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                fd,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                ::fast_io::mnp::addrvw(buf_ptrsz),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                buf_len,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(cookie)>>>(cookie),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                ::fast_io::mnp::addrvw(buf_used_ptrsz),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8") ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_ORANGE),
                                u8"(wasi-trace)\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
#else
            ::fast_io::io::perr(::fast_io::u8err(),
                                u8"uwvm: [info]  wasip1: fd_readdir_wasm64(",
                                fd,
                                u8", ",
                                ::fast_io::mnp::addrvw(buf_ptrsz),
                                u8", ",
                                buf_len,
                                u8", ",
                                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(cookie)>>>(cookie),
                                u8", ",
                                ::fast_io::mnp::addrvw(buf_used_ptrsz),
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

            // Lock before releasing fds_rwlock
            curr_fd_release_guard.device_p = ::std::addressof(curr_wasi_fd_t_p->fd_mutex);
            curr_fd_release_guard.lock();

            // After unlocking fds_lock, members within `wasm_fd_storage_t` can no longer be accessed or modified.
        }

        // curr_fd_uniptr is not null.
        auto& curr_fd{*curr_wasi_fd_t_p};

        // If obtained from the renumber map, it will always be the correct value. If obtained from the open vec, it requires checking whether it is closed.
        if(curr_fd.close_pos != SIZE_MAX) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::ebadf; }

        // Rights check: readdir uses readdir permission
        if((curr_fd.rights_base & ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t::right_fd_readdir) !=
           ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t::right_fd_readdir) [[unlikely]]
        {
            return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::enotcapable;
        }

        using underlying_dircookie_t = ::std::make_unsigned_t<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>;
        auto const underlying_dircookie{static_cast<underlying_dircookie_t>(cookie)};
        underlying_dircookie_t dircookie_counter{};

        ::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t byte_write_all_size{};
        ::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t buf_remaining_size{buf_len};

#if defined(_WIN32) && !defined(__CYGWIN__)
        // win32
        switch(curr_fd.file_type)
        {
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::socket:
            {
                return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::enotdir;
            }
# if defined(_WIN32_WINDOWS)
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::dir:
            {
#  ifdef UWVM_CPP_EXCEPTIONS
                try
#  endif
                {
                    for(auto const& ent: current(at(curr_fd.dir_fd)))
                    {
                        ::uwvm2::utils::container::u8cstring_view tmp_filename{u8filename(ent)};

                        // wasi prohibits opening parent paths in the preload directory
                        if(!curr_fd.preloaded_dir.empty() && tmp_filename == u8"..") [[unlikely]] { continue; }

                        auto const u8res{::uwvm2::utils::utf::check_legal_utf8<::uwvm2::utils::utf::utf8_specification::utf8_rfc3629>(tmp_filename.cbegin(),
                                                                                                                                      tmp_filename.cend())};
                        if(u8res.err != ::uwvm2::utils::utf::utf_error_code::success) [[unlikely]]
                        {
                            // File names are expected to be valid UTF-8. However, the host filesystem might contain entries that are not valid UTF-8.
                            // Implementations MAY replace invalid sequences with the Unicode replacement character (U+FFFD), or MAY omit such entries.
                            continue;
                        }

                        if(dircookie_counter++ < underlying_dircookie) [[likely]] { continue; }

                        auto const d_next{static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(dircookie_counter)};
                        auto const d_ino{static_cast<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>(inode_ul64(ent))};
                        auto const d_filename{tmp_filename};
                        auto const d_namlen{static_cast<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>(d_filename.size())};
                        ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t d_type;  // no initialize

                        switch(type(ent))
                        {
                            case ::fast_io::file_type::none:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::not_found:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::regular:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_regular_file;
                                break;
                            }
                            case ::fast_io::file_type::directory:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                                break;
                            }
                            case ::fast_io::file_type::symlink:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_symbolic_link;
                                break;
                            }
                            case ::fast_io::file_type::block:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_block_device;
                                break;
                            }
                            case ::fast_io::file_type::character:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_character_device;
                                break;
                            }
                            case ::fast_io::file_type::fifo:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::socket:
                            {
                                // You won't encounter sockets in the directory.
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::unknown:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::remote:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            [[unlikely]] default:
                            {
#  if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#  endif

                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                        }

                        auto const curr_write_size{static_cast<::std::size_t>(
                            size_of_wasi_dirent_wasm64_t +
                            static_cast<::std::size_t>(
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen)))};

                        if(curr_write_size < size_of_wasi_dirent_wasm64_t) [[unlikely]] { break; }

                        if constexpr(::std::numeric_limits<::std::size_t>::max() >
                                     ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max())
                        {
                            if(curr_write_size > ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max()) [[unlikely]] { break; }
                        }

                        auto const curr_write_size_wasi{static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(curr_write_size)};

                        auto const new_byte_write_all_size{
                            static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(byte_write_all_size + curr_write_size_wasi)};
                        if(new_byte_write_all_size < byte_write_all_size) [[unlikely]] { break; }

                        if(buf_remaining_size < curr_write_size_wasi) { break; }

                        // write to memory (Check all boundaries)
                        ::uwvm2::imported::wasi::wasip1::memory::check_memory_bounds_wasm64(memory, buf_ptrsz, new_byte_write_all_size);

                        if constexpr(is_default_wasi_dirent_wasm64_data_layout())
                        {
                            wasi_dirent_wasm64_t tmp_wasi_dirent;  // no initialize
                            tmp_wasi_dirent.d_next = d_next;
                            tmp_wasi_dirent.d_ino = d_ino;
                            tmp_wasi_dirent.d_namlen = d_namlen;
                            tmp_wasi_dirent.d_type = d_type;

                            ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size,
                                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)),
                                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)) + sizeof(tmp_wasi_dirent));
                        }
                        else
                        {
                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>>(d_next));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 8u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>>(d_ino));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 16u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 20u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t>>(d_type));
                        }

                        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(memory,
                                                                                                      buf_ptrsz + byte_write_all_size +
                                                                                                          size_of_wasi_dirent_wasm64_t,
                                                                                                      reinterpret_cast<::std::byte const*>(d_filename.cbegin()),
                                                                                                      reinterpret_cast<::std::byte const*>(d_filename.cend()));

                        buf_remaining_size -= curr_write_size_wasi;
                        byte_write_all_size = new_byte_write_all_size;
                    }
                }
#  ifdef UWVM_CPP_EXCEPTIONS
                catch(::fast_io::error)
                {
                }
#  endif

                break;
            }
# endif
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::file:
            {
# if defined(_WIN32_WINDOWS)
                return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::enotdir;
# else
                ::fast_io::win32::nt::file_basic_information fbi;
                ::fast_io::win32::nt::io_status_block isb;

                constexpr bool zw{false};
                auto const status{::fast_io::win32::nt::nt_query_information_file<zw>(curr_fd.file_fd.native_handle(),
                                                                                      ::std::addressof(isb),
                                                                                      ::std::addressof(fbi),
                                                                                      static_cast<::std::uint_least32_t>(sizeof(fbi)),
                                                                                      ::fast_io::win32::nt::file_information_class::FileBasicInformation)};

                if(status) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio; }

                if((fbi.FileAttributes & 0x00000010 /*FILE_ATTRIBUTE_DIRECTORY*/) != 0x00000010) [[unlikely]]
                {
                    return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::enotdir;
                }

#  ifdef UWVM_CPP_EXCEPTIONS
                try
#  endif
                {
                    for(auto const& ent: current(at(curr_fd.file_fd)))
                    {
                        ::uwvm2::utils::container::u8cstring_view tmp_filename{u8filename(ent)};

                        // wasi prohibits opening parent paths in the preload directory
                        if(!curr_fd.preloaded_dir.empty() && tmp_filename == u8"..") [[unlikely]] { continue; }

                        auto const u8res{::uwvm2::utils::utf::check_legal_utf8<::uwvm2::utils::utf::utf8_specification::utf8_rfc3629>(tmp_filename.cbegin(),
                                                                                                                                      tmp_filename.cend())};
                        if(u8res.err != ::uwvm2::utils::utf::utf_error_code::success) [[unlikely]] { continue; }

                        if(dircookie_counter++ < underlying_dircookie) [[likely]] { continue; }

                        auto const d_next{static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(dircookie_counter)};
                        auto const d_ino{static_cast<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>(inode_ul64(ent))};
                        auto const d_filename{tmp_filename};
                        auto const d_namlen{static_cast<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>(d_filename.size())};
                        ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t d_type;  // no initialize

                        switch(type(ent))
                        {
                            case ::fast_io::file_type::none:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::not_found:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::regular:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_regular_file;
                                break;
                            }
                            case ::fast_io::file_type::directory:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                                break;
                            }
                            case ::fast_io::file_type::symlink:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_symbolic_link;
                                break;
                            }
                            case ::fast_io::file_type::block:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_block_device;
                                break;
                            }
                            case ::fast_io::file_type::character:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_character_device;
                                break;
                            }
                            case ::fast_io::file_type::fifo:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::socket:
                            {
                                // You won't encounter sockets in the directory.
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::unknown:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            case ::fast_io::file_type::remote:
                            {
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                            [[unlikely]] default:
                            {
#  if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#  endif
                                d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                                break;
                            }
                        }

                        auto const curr_write_size{static_cast<::std::size_t>(
                            size_of_wasi_dirent_wasm64_t +
                            static_cast<::std::size_t>(
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen)))};

                        if(curr_write_size < size_of_wasi_dirent_wasm64_t) [[unlikely]] { break; }

                        if constexpr(::std::numeric_limits<::std::size_t>::max() >
                                     ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max())
                        {
                            if(curr_write_size > ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max()) [[unlikely]] { break; }
                        }

                        auto const curr_write_size_wasi{static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(curr_write_size)};

                        auto const new_byte_write_all_size{
                            static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(byte_write_all_size + curr_write_size_wasi)};
                        if(new_byte_write_all_size < byte_write_all_size) [[unlikely]] { break; }

                        if(buf_remaining_size < curr_write_size_wasi) { break; }

                        ::uwvm2::imported::wasi::wasip1::memory::check_memory_bounds_wasm64(memory, buf_ptrsz, new_byte_write_all_size);

                        if constexpr(is_default_wasi_dirent_wasm64_data_layout())
                        {
                            wasi_dirent_wasm64_t tmp_wasi_dirent;  // no initialize
                            tmp_wasi_dirent.d_next = d_next;
                            tmp_wasi_dirent.d_ino = d_ino;
                            tmp_wasi_dirent.d_namlen = d_namlen;
                            tmp_wasi_dirent.d_type = d_type;

                            ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size,
                                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)),
                                reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)) + sizeof(tmp_wasi_dirent));
                        }
                        else
                        {
                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>>(d_next));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 8u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>>(d_ino));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 16u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen));

                            ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                                memory,
                                buf_ptrsz + byte_write_all_size + 20u,
                                static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t>>(d_type));
                        }

                        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(memory,
                                                                                                      buf_ptrsz + byte_write_all_size +
                                                                                                          size_of_wasi_dirent_wasm64_t,
                                                                                                      reinterpret_cast<::std::byte const*>(d_filename.cbegin()),
                                                                                                      reinterpret_cast<::std::byte const*>(d_filename.cend()));

                        buf_remaining_size -= curr_write_size_wasi;
                        byte_write_all_size = new_byte_write_all_size;
                    }
                }
#  ifdef UWVM_CPP_EXCEPTIONS
                catch(::fast_io::error)
                {
                }
#  endif

                break;
# endif
            }
            [[unlikely]] default:
            {
# if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
# endif
                return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
            }
        }
#else
        // posix

        struct ::stat stbuf;  // no initialize
        if(::uwvm2::imported::wasi::wasip1::func::posix::fstat(curr_fd.file_fd.native_handle(), ::std::addressof(stbuf)) != 0) [[unlikely]]
        {
            return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::eio;
        }

        if(!S_ISDIR(stbuf.st_mode)) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::enotdir; }

# ifdef UWVM_CPP_EXCEPTIONS
        try
# endif
        {
            for(auto const& ent: current(at(curr_fd.file_fd)))
            {
                ::uwvm2::utils::container::u8cstring_view tmp_filename{u8filename(ent)};

                // wasi prohibits opening parent paths in the preload directory
                if(!curr_fd.preloaded_dir.empty() && tmp_filename == u8"..") [[unlikely]] { continue; }

                auto const u8res{
                    ::uwvm2::utils::utf::check_legal_utf8<::uwvm2::utils::utf::utf8_specification::utf8_rfc3629>(tmp_filename.cbegin(), tmp_filename.cend())};
                if(u8res.err != ::uwvm2::utils::utf::utf_error_code::success) [[unlikely]] { continue; }

                if(dircookie_counter++ < underlying_dircookie) [[likely]] { continue; }

                auto const d_next{static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(dircookie_counter)};
                auto const d_ino{static_cast<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>(inode_ul64(ent))};
                auto const d_filename{tmp_filename};
                auto const d_namlen{static_cast<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>(d_filename.size())};
                ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t d_type;  // no initialize

                switch(type(ent))
                {
                    case ::fast_io::file_type::none:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::not_found:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::regular:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_regular_file;
                        break;
                    }
                    case ::fast_io::file_type::directory:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_directory;
                        break;
                    }
                    case ::fast_io::file_type::symlink:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_symbolic_link;
                        break;
                    }
                    case ::fast_io::file_type::block:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_block_device;
                        break;
                    }
                    case ::fast_io::file_type::character:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_character_device;
                        break;
                    }
                    case ::fast_io::file_type::fifo:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::socket:
                    {
                        // You won't encounter sockets in the directory.
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::unknown:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    case ::fast_io::file_type::remote:
                    {
                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                    [[unlikely]] default:
                    {
# if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                        ::uwvm2::utils::debug::trap_and_inform_bug_pos();
# endif

                        d_type = ::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t::filetype_unknown;
                        break;
                    }
                }

                auto const curr_write_size{static_cast<::std::size_t>(
                    size_of_wasi_dirent_wasm64_t +
                    static_cast<::std::size_t>(static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen)))};

                if(curr_write_size < size_of_wasi_dirent_wasm64_t) [[unlikely]] { break; }

                if constexpr(::std::numeric_limits<::std::size_t>::max() >
                             ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max())
                {
                    if(curr_write_size > ::std::numeric_limits<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>::max()) [[unlikely]] { break; }
                }

                auto const curr_write_size_wasi{static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(curr_write_size)};

                auto const new_byte_write_all_size{
                    static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(byte_write_all_size + curr_write_size_wasi)};
                if(new_byte_write_all_size < byte_write_all_size) [[unlikely]] { break; }

                if(buf_remaining_size < curr_write_size_wasi) { break; }

                // write to memory (Check all boundaries)
                ::uwvm2::imported::wasi::wasip1::memory::check_memory_bounds_wasm64(memory, buf_ptrsz, new_byte_write_all_size);

                if constexpr(is_default_wasi_dirent_wasm64_data_layout())
                {
                    wasi_dirent_wasm64_t tmp_wasi_dirent;  // no initialize
                    tmp_wasi_dirent.d_next = d_next;
                    tmp_wasi_dirent.d_ino = d_ino;
                    tmp_wasi_dirent.d_namlen = d_namlen;
                    tmp_wasi_dirent.d_type = d_type;

                    ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(
                        memory,
                        buf_ptrsz + byte_write_all_size,
                        reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)),
                        reinterpret_cast<::std::byte const*>(::std::addressof(tmp_wasi_dirent)) + sizeof(tmp_wasi_dirent));
                }
                else
                {
                    ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                        memory,
                        buf_ptrsz + byte_write_all_size,
                        static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>>(d_next));

                    ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                        memory,
                        buf_ptrsz + byte_write_all_size + 8u,
                        static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::inode_wasm64_t>>(d_ino));

                    ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                        memory,
                        buf_ptrsz + byte_write_all_size + 16u,
                        static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::dirnamlen_wasm64_t>>(d_namlen));

                    ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64_unchecked(
                        memory,
                        buf_ptrsz + byte_write_all_size + 20u,
                        static_cast<::std::underlying_type_t<::uwvm2::imported::wasi::wasip1::abi::filetype_wasm64_t>>(d_type));
                }

                ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm64_unchecked(memory,
                                                                                              buf_ptrsz + byte_write_all_size + size_of_wasi_dirent_wasm64_t,
                                                                                              reinterpret_cast<::std::byte const*>(d_filename.cbegin()),
                                                                                              reinterpret_cast<::std::byte const*>(d_filename.cend()));

                buf_remaining_size -= curr_write_size_wasi;
                byte_write_all_size = new_byte_write_all_size;
            }
        }
# ifdef UWVM_CPP_EXCEPTIONS
        catch(::fast_io::error)
        {
        }
# endif
#endif

        ::uwvm2::imported::wasi::wasip1::memory::store_basic_wasm_type_to_memory_wasm64(
            memory,
            buf_used_ptrsz,
            static_cast<::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t>(byte_write_all_size));

        return ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t::esuccess;
    }
}  // namespace uwvm2::imported::wasi::wasip1::func

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_pop_macro.h>
#endif

