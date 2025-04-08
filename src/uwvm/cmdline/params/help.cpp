/********************************************************
 * Ultimate WebAssembly Virtual Machine (help 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-30
 * @copyright   APL-2 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

#include <memory>

#include <utils/macro/push_macros.h>
#include <utils/ansies/ansi_push_macro.h>

import fast_io;
import utils.io;
import utils.cmdline;
import uwvm.cmdline;

namespace uwvm::cmdline::paras::details
{
    template <::std::size_t N>
    inline constexpr ::fast_io::array<char8_t, N + 1> u8nspace() noexcept
    {
        ::fast_io::array<char8_t, N + 1> res{};

        for(::std::size_t i{}; i != N; ++i) { res[i] = u8' '; }

        res[N] = u8'\0';

        return res;
    }

    // max parameter + left "  " + right "  "
    inline constexpr auto parameter_max_principal_name_size_u8nspace{u8nspace<::uwvm::cmdline::parameter_max_principal_name_size + 4>()};

    inline void help_output_singal_cate(::utils::cmdline::categorization cate) noexcept
    {
        for(auto const p: ::uwvm::cmdline::parameters)
        {
            if(p->cate != cate) { continue; }
            ::fast_io::io::perr(::utils::u8err, u8"  " UWVM_AES_U8_GREEN, ::fast_io::mnp::left(p->name, ::uwvm::cmdline::parameter_max_principal_name_size));
            ::fast_io::io::perrln(::utils::u8err, UWVM_AES_U8_YELLOW u8"  -----  " UWVM_AES_U8_WHITE, p->describe);
            ::fast_io::io::perr(::utils::u8err,
                                parameter_max_principal_name_size_u8nspace.element,
                                UWVM_AES_U8_LT_PURPLE u8"Usage: " UWVM_AES_U8_WHITE u8"[" UWVM_AES_U8_GREEN,
                                p->name);
            for(auto curr_base{p->alias.base}; curr_base != p->alias.base + p->alias.len; ++curr_base)
            {
                ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_WHITE u8"|" UWVM_AES_U8_GREEN, *curr_base);
            }
            ::fast_io::io::perrln(::utils::u8err, UWVM_AES_U8_WHITE u8"] " UWVM_AES_U8_YELLOW, p->usage, UWVM_AES_U8_RST_ALL);
        }
    }

    inline void help_output_all() noexcept
    {
        for(auto const p: ::uwvm::cmdline::parameters)
        {
            ::fast_io::io::perr(::utils::u8err, u8"  " UWVM_AES_U8_GREEN, ::fast_io::mnp::left(p->name, ::uwvm::cmdline::parameter_max_principal_name_size));
            ::fast_io::io::perrln(::utils::u8err, UWVM_AES_U8_YELLOW u8"  -----  " UWVM_AES_U8_WHITE, p->describe);
            ::fast_io::io::perr(::utils::u8err,
                                parameter_max_principal_name_size_u8nspace.element,
                                UWVM_AES_U8_LT_PURPLE u8"Usage: " UWVM_AES_U8_WHITE u8"[" UWVM_AES_U8_GREEN,
                                p->name);
            for(auto curr_base{p->alias.base}; curr_base != p->alias.base + p->alias.len; ++curr_base)
            {
                ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_WHITE u8"|" UWVM_AES_U8_GREEN, *curr_base);
            }
            ::fast_io::io::perrln(::utils::u8err, UWVM_AES_U8_WHITE u8"] " UWVM_AES_U8_YELLOW, p->usage, UWVM_AES_U8_RST_ALL);
        }
    }

    UWVM_GNU_COLD extern ::utils::cmdline::parameter_return_type help_callback(::utils::cmdline::parameter_parsing_results* para_begin,
                                                                               ::utils::cmdline::parameter_parsing_results* para_curr,
                                                                               ::utils::cmdline::parameter_parsing_results* para_end) noexcept
    {
        auto currp1{para_curr + 1};

        // Check for out-of-bounds and not-argument
        if(currp1 == para_end || currp1->type != ::utils::cmdline::parameter_parsing_results_type::arg) [[unlikely]]
        {
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"Arguments:\n");
            help_output_singal_cate(::utils::cmdline::categorization::global);

            // display other parameter
            ::fast_io::io::perr(::utils::u8err, 
                // wasm
                UWVM_AES_U8_LT_CYAN u8"  " , ::fast_io::mnp::left(u8"<wasm>", ::uwvm::cmdline::parameter_max_principal_name_size), UWVM_AES_U8_YELLOW u8"  -----  " UWVM_AES_U8_WHITE u8"Use \"" UWVM_AES_U8_YELLOW u8"--help wasm" UWVM_AES_U8_WHITE u8"\" to display the wasm arguments." 
                // endl
                u8"\n\n");

            return ::utils::cmdline::parameter_return_type::return_imme;
        }

        currp1->type = ::utils::cmdline::parameter_parsing_results_type::occupied_arg;

        if(auto currp1_str{currp1->str}; currp1_str == u8"all")
        {
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"Arguments:\n");
            help_output_all();
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL u8"\n");
        }
        else if(currp1_str == u8"global")
        {
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"Arguments:\n" UWVM_AES_U8_LT_CYAN u8"  <global>" UWVM_AES_U8_WHITE u8":\n");
            help_output_singal_cate(::utils::cmdline::categorization::global);
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL u8"\n");
        }
        else if(currp1_str == u8"wasm")
        {
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"Arguments:\n" UWVM_AES_U8_LT_CYAN u8"  <wasm>" UWVM_AES_U8_WHITE u8":\n");
            help_output_singal_cate(::utils::cmdline::categorization::wasm);
            ::fast_io::io::perr(::utils::u8err, UWVM_AES_U8_RST_ALL u8"\n");
        }
        else
        {
            ::fast_io::io::perr(
                ::utils::u8err,
                UWVM_AES_U8_RST_ALL
                    UWVM_AES_U8_WHITE u8"uwvm: " UWVM_AES_U8_RED u8"[error] " UWVM_AES_U8_WHITE u8"Invalid Extra Help Name \"" UWVM_AES_U8_CYAN,
                currp1_str,
                UWVM_AES_U8_WHITE u8"\". Usage: "  u8"[" UWVM_AES_U8_GREEN u8"--help" UWVM_AES_U8_WHITE u8"|" UWVM_AES_U8_GREEN u8"-h" UWVM_AES_U8_WHITE u8"] "
                UWVM_AES_U8_YELLOW u8"[<null>|all|global|wasm]" UWVM_AES_U8_RST_ALL u8"\n\n");

            return ::utils::cmdline::parameter_return_type::return_m1_imme;
        }

        return ::utils::cmdline::parameter_return_type::return_imme;
    }

}  // namespace uwvm::cmdline::paras::details
