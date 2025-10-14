/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-10-01
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

UWVM_MODULE_EXPORT namespace uwvm2::uwvm::cmdline::params
{
    namespace details
    {
        inline constexpr ::uwvm2::utils::container::u8string_view wasi_mount_dir_alias{u8"-Idir"};
        extern "C++" ::uwvm2::utils::cmdline::parameter_return_type wasi_mount_dir_callback(::uwvm2::utils::cmdline::parameter_parsing_results*,
                                                                                            ::uwvm2::utils::cmdline::parameter_parsing_results*,
                                                                                            ::uwvm2::utils::cmdline::parameter_parsing_results*) noexcept;
    }  // namespace details

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wbraced-scalar-init"
#endif
    inline constexpr ::uwvm2::utils::cmdline::parameter wasi_mount_dir{
        .name{u8"--wasi-mount-dir"},
        .describe{
            u8"Mount a host directory to the WASI sandbox at a fixed WASI mount point. Usage requires two arguments: <wasi dir> <system dir>. The <wasi dir> may be an absolute POSIX-style path (e.g. /a/x/d) or a relative path. In both modes, '//' and any path segment of '.' or '..' are forbidden; '.' is only allowed when used alone as the entire mount point."},
        .usage{u8"<wasi dir:str> <system dir:path>"},
        .alias{::uwvm2::utils::cmdline::kns_u8_str_scatter_t{::std::addressof(details::wasi_mount_dir_alias), 1uz}},
        .handle{::std::addressof(details::wasi_mount_dir_callback)},
        .pretreatment{nullptr},
        .cate{::uwvm2::utils::cmdline::categorization::wasi}};
#if defined(__clang__)
# pragma clang diagnostic pop
#endif
}

#ifndef UWVM_MODULE
// macro
# include <uwvm2/uwvm/utils/ansies/uwvm_color_pop_macro.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif

