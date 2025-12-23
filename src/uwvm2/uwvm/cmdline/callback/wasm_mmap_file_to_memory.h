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
# include <memory>
// macro
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/uwvm/utils/ansies/uwvm_color_push_macro.h>
// import
# include <fast_io.h>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/utils/ansies/impl.h>
# include <uwvm2/utils/cmdline/impl.h>
# include <uwvm2/uwvm/io/impl.h>
# include <uwvm2/uwvm/utils/ansies/impl.h>
# include <uwvm2/uwvm/cmdline/impl.h>
# include <uwvm2/uwvm/cmdline/params/impl.h>
# include <uwvm2/uwvm/wasm/base/impl.h>
# include <uwvm2/uwvm/wasm/storage/impl.h>
# include <uwvm2/uwvm/wasm/loader/impl.h>
# include <uwvm2/uwvm/run/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::uwvm::cmdline::params::details
{
#if defined(UWVM_MODULE)
    extern "C++" UWVM_GNU_COLD
#else
    UWVM_GNU_COLD inline constexpr
#endif
        ::uwvm2::utils::cmdline::parameter_return_type wasm_mmap_file_to_memory_callback([[maybe_unused]] ::uwvm2::utils::cmdline::parameter_parsing_results *
                                                                                             para_begin,
                                                                                         ::uwvm2::utils::cmdline::parameter_parsing_results * para_curr,
                                                                                         ::uwvm2::utils::cmdline::parameter_parsing_results * para_end) noexcept
    {

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //      ^^ para_curr

        auto currp1{para_curr + 1u};

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //            ^^ currp1

        // Check for out-of-bounds and not-argument
        if(currp1 == para_end || currp1->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg) [[unlikely]]
        {
            // (currp1 == para_end):
            // [... curr] (end) ...
            // [  safe  ] unsafe (could be the module_end)
            //            ^^ currp1

            // (currp1->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg):
            // [... curr para] ...
            // [     safe    ] unsafe (could be the module_end)
            //           ^^ currp1

            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Usage: ",
                                ::uwvm2::utils::cmdline::print_usage(::uwvm2::uwvm::cmdline::params::wasm_mmap_file_to_memory),
                                // print_usage comes with UWVM_COLOR_U8_RST_ALL
                                u8"\n\n");

            return ::uwvm2::utils::cmdline::parameter_return_type::return_m1_imme;
        }

        currp1->type = ::uwvm2::utils::cmdline::parameter_parsing_results_type::occupied_arg;

        ::uwvm2::utils::container::u8cstring_view const file_path{currp1->str};

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //      ^^ para_curr

        auto currp2{para_curr + 2u};

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //            ^^ currp2

        // Check for out-of-bounds and not-argument
        if(currp2 == para_end || currp2->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg) [[unlikely]]
        {
            // (currp2 == para_end):
            // [... curr] (end) ...
            // [  safe  ] unsafe (could be the module_end)
            //            ^^ currp2

            // (currp2->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg):
            // [... curr para] ...
            // [     safe    ] unsafe (could be the module_end)
            //           ^^ currp2

            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Usage: ",
                                ::uwvm2::utils::cmdline::print_usage(::uwvm2::uwvm::cmdline::params::wasm_mmap_file_to_memory),
                                // print_usage comes with UWVM_COLOR_U8_RST_ALL
                                u8"\n\n");

            return ::uwvm2::utils::cmdline::parameter_return_type::return_m1_imme;
        }

        currp2->type = ::uwvm2::utils::cmdline::parameter_parsing_results_type::occupied_arg;

        ::uwvm2::utils::container::u8string_view const module_name{currp2->str};

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //      ^^ para_curr

        auto currp3{para_curr + 3u};

        // [... curr] ...
        // [  safe  ] unsafe (could be the module_end)
        //            ^^ currp3

        // Check for out-of-bounds and not-argument
        if(currp3 == para_end || currp3->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg) [[unlikely]]
        {
            // (currp3 == para_end):
            // [... curr] (end) ...
            // [  safe  ] unsafe (could be the module_end)
            //            ^^ currp3

            // (currp3->type != ::uwvm2::utils::cmdline::parameter_parsing_results_type::arg):
            // [... curr para] ...
            // [     safe    ] unsafe (could be the module_end)
            //           ^^ currp3

            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Usage: ",
                                ::uwvm2::utils::cmdline::print_usage(::uwvm2::uwvm::cmdline::params::wasm_mmap_file_to_memory),
                                // print_usage comes with UWVM_COLOR_U8_RST_ALL
                                u8"\n\n");

            return ::uwvm2::utils::cmdline::parameter_return_type::return_m1_imme;
        }

        currp3->type = ::uwvm2::utils::cmdline::parameter_parsing_results_type::occupied_arg;

        ::uwvm2::utils::container::u8string_view const export_name{currp3->str};

        // implace module
        [[maybe_unused]] auto [mmap_it, mmap_inserted]{::uwvm2::uwvm::wasm::storage::wasm_mmap_memory_modules.try_emplace(module_name)};

        auto& mmap_exports{mmap_it->second};
        auto const [export_it, export_inserted]{mmap_exports.emplace(::uwvm2::uwvm::wasm::storage::mmap_file_results{export_name, file_path})};

        // Since mmap_file_results compares only export_name, treat duplicates as overwrite (last wins).
        if(!export_inserted) [[unlikely]]
        {
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RED),
                                u8"[error] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"Module \"",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                module_name,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"\" already has an export \"",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                export_name,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"\".\n\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));

            return ::uwvm2::utils::cmdline::parameter_return_type::return_m1_imme;
        }

        return ::uwvm2::utils::cmdline::parameter_return_type::def;
    }
}  // namespace uwvm2::uwvm::cmdline::params::details

#ifndef UWVM_MODULE
# include <uwvm2/uwvm/utils/ansies/uwvm_color_pop_macro.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif
