/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-21
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

module;

/// @brief      uwvm.crtmain:global module declaration
export module uwvm.crtmain:global;

/// @brief      import utils.global
import utils.global;

/// @brief      For msvc, set the initialization order of this global value to library
#if !__has_cpp_attribute(__gnu__::__init_priority__)
# pragma init_seg(library)
#endif

export namespace uwvm::global
{
    /// @brief      Control program initialization and termination functions through RTTI.
    ///             Set __init_priority__ to 400
    ///             Ensure initialization occurs only after the lowest-level module has been initialized.

    /// @brief      Automatically setup windows consolecp with ansi
#if (defined(_WIN32) && !defined(__CYGWIN__)) && !defined(_WIN32_WINDOWS)
    /// @brief      set win32 console output and windows cp to utf8
# if __has_cpp_attribute(__gnu__::__init_priority__)
    [[__gnu__::__init_priority__(400)]]
# endif
    inline ::utils::global::set_win32_console_io_cp_to_utf8 set_native_console_io_cp_to_utf8_ele{};

    /// @brief      set win32 console ansi escape
# if __has_cpp_attribute(__gnu__::__init_priority__)
    [[__gnu__::__init_priority__(400)]]
# endif
    inline ::utils::global::enable_win32_ansi enable_native_ansi_ele{};
#endif

/// @brief      Automatically get the correct timezone data
#if __has_cpp_attribute(__gnu__::__init_priority__)
    [[__gnu__::__init_priority__(400)]]
#endif
    inline ::utils::global::tz_set_s tz_set_ele{};
}  // namespace uwvm::global
