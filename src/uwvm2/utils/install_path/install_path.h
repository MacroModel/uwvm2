﻿/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-29
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
// macro
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
// platform
# ifdef UWVM_SUPPORT_INSTALL_PATH
#  include <fast_io_driver/install_path.h>
# endif
// import
# include <fast_io.h>
# include <uwvm2/uwvm_predefine/io/impl.h>
# include <uwvm2/uwvm_predefine/utils/ansies/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

#ifdef UWVM_SUPPORT_INSTALL_PATH
UWVM_MODULE_EXPORT namespace uwvm2::utils::install_path
{
    inline ::fast_io::install_path get_module_install_path_noexcept() noexcept
    {
        ::fast_io::install_path ret{};
# ifdef __cpp_exceptions
        try
# endif
        {
            ret = ::fast_io::get_module_install_path();
        }
# ifdef __cpp_exceptions
        catch(::fast_io::error)
        {
#  ifdef UWVM
            if(::uwvm2::uwvm::io::show_vm_warning)
            {
                ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                    u8"uwvm: ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                    u8"[warn]  ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                    u8"Cannot get install path. ",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_ORANGE),
                                    u8"(vm)\n",
                                    ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
            }
#  else
            ::fast_io::io::perr(::fast_io::u8err(), u8"uwvm: [warn]  Cannot get install path.\n");
#  endif
        }
# endif
        return ret;
    }

}  // namespace uwvm2::utils::install_path
#endif

#ifndef UWVM_MODULE
// macro-controlled macro
# ifdef UWVM
#  include <uwvm2/uwvm/utils/ansies/uwvm_color_pop_macro.h>
# endif
// macro
# include <uwvm2/utils/macro/pop_macros.h>
#endif
