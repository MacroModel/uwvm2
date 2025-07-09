﻿/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the ASHP-1.0 License (see LICENSE file).   *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-27
 * @copyright   ASHP-1.0 License
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

#ifdef UWVM_MODULE
import fast_io;
import uwvm2.uwvm.io;
import uwvm2.utils.ansies;
# if defined(UWVM_TIMER) || ((defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK))
import uwvm2.utils.debug;
# endif
import uwvm2.utils.madvise;
import uwvm2.parser.wasm.base;
import uwvm2.parser.wasm.concepts;
import uwvm2.parser.wasm.standard;
import uwvm2.parser.wasm.binfmt.base;
import uwvm2.uwvm.utils.ansies;
import uwvm2.uwvm.utils.memory;
import uwvm2.uwvm.cmdline;
import uwvm2.uwvm.wasm.base;
import uwvm2.uwvm.wasm.storage;
import uwvm2.uwvm.wasm.feature;
import uwvm2.uwvm.wasm.custom;
import :retval;
#else
// std
# include <cstddef>
# include <cstdint>
# include <type_traits>
// macro
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/uwvm/utils/ansies/uwvm_color_push_macro.h>
// import
# include <fast_io.h>
# include <uwvm2/uwvm/io/impl.h>
# include <uwvm2/utils/ansies/impl.h>
# if defined(UWVM_TIMER) || ((defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK))
#  include <uwvm2/utils/debug/impl.h>
# endif
# include <uwvm2/utils/madvise/impl.h>
# include <uwvm2/parser/wasm/base/impl.h>
# include <uwvm2/parser/wasm/concepts/impl.h>
# include <uwvm2/parser/wasm/standard/impl.h>
# include <uwvm2/parser/wasm/binfmt/base/impl.h>
# include <uwvm2/uwvm/utils/ansies/impl.h>
# include <uwvm2/uwvm/utils/memory/impl.h>
# include <uwvm2/uwvm/cmdline/impl.h>
# include <uwvm2/uwvm/wasm/base/impl.h>
# include <uwvm2/uwvm/wasm/storage/impl.h>
# include <uwvm2/uwvm/wasm/feature/impl.h>
# include <uwvm2/uwvm/wasm/custom/impl.h>
# include "retval.h"
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif
UWVM_MODULE_EXPORT namespace uwvm2::uwvm::run
{
    inline int run() noexcept
    {
        if(!::uwvm2::uwvm::cmdline::wasm_file_ppos) [[unlikely]]
        {
            ::fast_io::io::perr(::uwvm2::uwvm::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                // 1
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"No input file.\n"
                                // 2
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                u8"[info]  ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Try \"",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                u8"uwvm ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_GREEN),
                                u8"--help",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"\" for more information.\n\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
            return static_cast<int>(::uwvm2::uwvm::run::retval::load_error);
        }

        // get main module path
        auto const module_file_name{::uwvm2::uwvm::cmdline::wasm_file_ppos->str};

#ifdef __cpp_exceptions
        try
#endif
        {
#ifdef UWVM_TIMER
            ::uwvm2::utils::debug::timer parsing_timer{u8"file loader"};
#endif
            // On platforms where CHAR_BIT is greater than 8, there is no need to clear the utf-8 non-low 8 bits here
            ::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file = ::fast_io::native_file_loader{module_file_name};
        }
#ifdef __cpp_exceptions
        catch(::fast_io::error e)
        {
            ::fast_io::io::perr(::uwvm2::uwvm::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Unable to open WASM file \"",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_CYAN),
                                module_file_name,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"\": ",
                                e,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL),
                                u8"\n"
# ifndef _WIN32  // Win32 automatically adds a newline (winnt and win9x)
                                u8"\n"
# endif
            );

            return static_cast<int>(::uwvm2::uwvm::run::retval::load_error);
        }
#endif

        // If not specified by the user, detect it
        if(::uwvm2::uwvm::wasm::storage::execute_wasm_binfmt_ver == static_cast<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(0u))
        {
            ::uwvm2::uwvm::wasm::storage::execute_wasm_binfmt_ver = ::uwvm2::parser::wasm::binfmt::detect_wasm_binfmt_version(
                reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cend()));
        }

        // After detect
        // Instructs to read the file all the way into memory
        ::uwvm2::utils::madvise::my_madvise(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin(),
                                            ::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.size(),
                                            ::uwvm2::utils::madvise::madvise_flag::willneed);

        switch(::uwvm2::uwvm::wasm::storage::execute_wasm_binfmt_ver)
        {
            case static_cast<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(0u):
            {
#ifndef UWVM_DISABLE_OUTPUT_WHEN_PARSE
                ::fast_io::io::perr(::uwvm2::uwvm::u8log_output,
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                    u8"uwvm: ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                    u8"[error] ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"(offset=",
                                    ::fast_io::mnp::addrvw(nullptr),
                                    u8") Illegal WebAssembly file format.",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"\nuwvm: ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                    u8"[info]  ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"Parser Memory Indication: ",
                                    ::uwvm2::uwvm::utils::memory::print_memory{
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cend())},
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL),
                                    u8"\n\n");
#endif
                return static_cast<int>(::uwvm2::uwvm::run::retval::wasm_parser_error);  // wasm parsing error
            }
            case static_cast<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(1u):
            {
                // handle exec (main) module
                {
                    // storage wasm err
                    ::uwvm2::parser::wasm::base::error_impl execute_wasm_binfmt_ver1_storage_wasm_err{};

                    // parse wasm 1
#if defined(__cpp_exceptions) && !defined(UWVM_TERMINATE_IMME_WHEN_PARSE)
                    try
#endif
                    {
#ifdef UWVM_TIMER
                        ::uwvm2::utils::debug::timer parsing_timer{u8"parse binfmt ver1"};
#endif
                        ::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_binfmt_ver1_storage = ::uwvm2::uwvm::wasm::feature::binfmt_ver1_handler(
                            reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                            reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cend()),
                            execute_wasm_binfmt_ver1_storage_wasm_err,
                            ::uwvm2::uwvm::wasm::storage::global_wasm_binfmt_ver1_parameters);
                    }
#if defined(__cpp_exceptions) && !defined(UWVM_TERMINATE_IMME_WHEN_PARSE)
                    catch(::fast_io::error)
                    {
# ifndef UWVM_DISABLE_OUTPUT_WHEN_PARSE

                        // catch fast_io::error (wasm parser error)
#  if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                        if(execute_wasm_binfmt_ver1_storage_wasm_err.err_code == ::uwvm2::parser::wasm::base::wasm_parse_error_code::ok) [[unlikely]]
                        {
                            // The `ok` exception was thrown. It's a bug in the program.
                            ::uwvm2::utils::debug::trap_and_inform_bug_pos();
                        }
#  endif

                        // print error
                        auto u8log_output_osr{::fast_io::operations::output_stream_ref(::uwvm2::uwvm::u8log_output)};
                        // Add raii locks while unlocking operations
                        ::fast_io::operations::decay::stream_ref_decay_lock_guard u8log_output_lg{
                            ::fast_io::operations::decay::output_stream_mutex_ref_decay(u8log_output_osr)};
                        // No copies will be made here.
                        auto u8log_output_ul{::fast_io::operations::decay::output_stream_unlocked_ref_decay(u8log_output_osr)};

                        // default print_memory
                        ::uwvm2::uwvm::utils::memory::print_memory memory_printer{
                            reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                            execute_wasm_binfmt_ver1_storage_wasm_err.err_curr,
                            reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cend())};

                        // set errout
                        ::uwvm2::parser::wasm::base::error_output_t errout;
                        errout.module_begin = reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin());
                        errout.err = execute_wasm_binfmt_ver1_storage_wasm_err;
                        errout.flag.enable_ansi = static_cast<::std::uint_least8_t>(::uwvm2::uwvm::utils::ansies::put_color);
#  if defined(_WIN32) && (_WIN32_WINNT < 0x0A00 || defined(_WIN32_WINDOWS))
                        errout.flag.win32_use_text_attr = static_cast<::std::uint_least8_t>(!::uwvm2::uwvm::utils::ansies::log_win32_use_ansi_b);
#  endif

                        // Output the main information and memory indication
                        ::fast_io::io::perr(u8log_output_ul,
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                            u8"uwvm: ",
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                            u8"[error] ",
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                            u8"Parsing error in WebAssembly File \"",
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                            module_file_name,
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                            u8"\" to be executed.\n",
                                            errout,
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                            u8"\n" u8"uwvm: ",
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                            u8"[info]  ",
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                            u8"Parser Memory Indication: ",
                                            memory_printer,
                                            ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL),
                                            u8"\n");

                        // Extra line breaks
                        ::fast_io::perrln(u8log_output_ul);

# endif

                        return static_cast<int>(::uwvm2::uwvm::run::retval::wasm_parser_error);
                    }
#endif
                    // handle custom section
                    ::uwvm2::uwvm::wasm::custom::handle_binfmtver1_custom_section(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_binfmt_ver1_storage,
                                                                                  ::uwvm2::uwvm::wasm::custom::custom_handle_funcs);

                    // set exec module name
                    // 1st: para --wasm-set-main-module-name
                    // 2st: custom section "name": module name
                    // 3st: file path

                    /// @todo
                }

                /// @todo handle import modules

                /// @todo handle import dll

                /// @todo handle cli module

                break;
            }
            [[unlikely]] default:
            {
#ifndef UWVM_DISABLE_OUTPUT_WHEN_PARSE
                ::fast_io::io::perr(::uwvm2::uwvm::u8log_output,
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                    u8"uwvm: ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                    u8"[error] ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"(offset=",
                                    ::fast_io::mnp::addrvw(4uz),
                                    u8") Unknown Binary Format Version of WebAssembly: \"",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_CYAN),
                                    ::uwvm2::uwvm::wasm::storage::execute_wasm_binfmt_ver,
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"\".",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"\nuwvm: ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                    u8"[info]  ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"Parser Memory Indication: ",
                                    ::uwvm2::uwvm::utils::memory::print_memory{
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()),
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cbegin()) + 4uz,
                                        reinterpret_cast<::std::byte const*>(::uwvm2::uwvm::wasm::storage::execute_wasm.wasm_file.cend())},
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL),
                                    u8"\n\n");
#endif

                return static_cast<int>(::uwvm2::uwvm::run::retval::wasm_parser_error);
            }
        }

        // run vm
        switch(::uwvm2::uwvm::wasm::storage::execute_wasm_mode)
        {
            case ::uwvm2::uwvm::wasm::base::mode::objdump:
            {
                /// @todo objdump
                break;
            }
            [[unlikely]] default:
            {
                ::fast_io::unreachable();
            }
        }

        return static_cast<int>(::uwvm2::uwvm::run::retval::ok);
    }
}  // namespace uwvm2::uwvm::run

#ifndef UWVM_MODULE
// macro
# include <uwvm2/uwvm/utils/ansies/uwvm_color_pop_macro.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif
