﻿/*************************************************************
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
// import
# include <fast_io.h>
# include <fast_io_device.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

#ifdef UWVM
UWVM_MODULE_EXPORT namespace uwvm2::uwvm::io
{
    /// @brief warning control
    inline bool show_vm_warning{true};            // [global]
    inline bool show_parser_warning{true};        // [global]
    inline bool show_untrusted_dl_warning{true};  // [global]
    inline bool show_dl_warning{true};            // [global]
    inline bool show_depend_warning{true};        // [global]
# if defined(_WIN32) && !defined(_WIN32_WINDOWS)
    inline bool show_nt_path_warning{true};  // [global]
# endif

    /// @brief wtrap control
    inline bool vm_warning_fatal{};            // [global]
    inline bool parser_warning_fatal{};        // [global]
    inline bool untrusted_dl_warning_fatal{};  // [global]
    inline bool dl_warning_fatal{};            // [global]
    inline bool depend_warning_fatal{};        // [global]
# if defined(_WIN32) && !defined(_WIN32_WINDOWS)
    inline bool nt_path_warning_fatal{};  // [global]
# endif

}  // namespace uwvm2::uwvm::io
#endif
