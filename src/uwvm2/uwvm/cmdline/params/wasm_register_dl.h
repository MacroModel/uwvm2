﻿/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-31
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
# include <uwvm2/utils/cmdline/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

#if defined(UWVM_SUPPORT_PRELOAD_DL)
UWVM_MODULE_EXPORT namespace uwvm2::uwvm::cmdline::params
{

    namespace details
    {
        inline constexpr ::uwvm2::utils::container::u8string_view wasm_register_dl_alias{u8"-Wdl"};
        extern "C++" ::uwvm2::utils::cmdline::parameter_return_type wasm_register_dl_callback(::uwvm2::utils::cmdline::parameter_parsing_results*,
                                                                                              ::uwvm2::utils::cmdline::parameter_parsing_results*,
                                                                                              ::uwvm2::utils::cmdline::parameter_parsing_results*) noexcept;

    }  // namespace details

# if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wbraced-scalar-init"
# endif
    inline constexpr ::uwvm2::utils::cmdline::parameter wasm_register_dl{
        .name{u8"--wasm-register-dl"},
        .describe{u8"Loads a dynamic library and registers its exported functions as a WebAssembly module, module name can be renamed via argument."},
        .usage{u8"<dl:str> (<rename:str>)"},
        .alias{::uwvm2::utils::cmdline::kns_u8_str_scatter_t{::std::addressof(details::wasm_register_dl_alias), 1uz}},
        .handle{::std::addressof(details::wasm_register_dl_callback)},
        .cate{::uwvm2::utils::cmdline::categorization::wasm}};
# if defined(__clang__)
#  pragma clang diagnostic pop
# endif
}  // namespace uwvm2::uwvm::cmdline::params
#endif

#ifndef UWVM_MODULE
// macro
# include <uwvm2/uwvm/utils/ansies/uwvm_color_pop_macro.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif
