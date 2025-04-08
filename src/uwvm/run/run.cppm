/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-27
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

#include <utils/ansies/ansi_push_macro.h>

export module uwvm.run:run;

import fast_io;
import utils.io;
import uwvm.cmdline;
import uwvm.wasm.storage;

export namespace uwvm::run
{
    inline int run() noexcept
    {

        if(!::uwvm::cmdline::wasm_file_ppos) [[unlikely]]
        {
            ::fast_io::io::perr(
                ::utils::u8err,
                UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"uwvm: " UWVM_AES_U8_RED u8"[error] " UWVM_AES_U8_WHITE u8"No input file.\n\n" UWVM_AES_U8_RST_ALL);
            return -1;
        }

        auto module_name{::uwvm::cmdline::wasm_file_ppos->str};
#ifdef __cpp_exceptions
        try
#endif
        {
            ::uwvm::wasm::storage::execute_wasm_file = ::fast_io::native_file_loader{module_name};
        }
#ifdef __cpp_exceptions
        catch(::fast_io::error e)
        {
            ::fast_io::io::perr(::utils::u8err,
                                UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"uwvm: " UWVM_AES_U8_RED u8"[error] " UWVM_AES_U8_WHITE u8"Unable to open WASM file \"" UWVM_AES_U8_CYAN,
                                module_name,
                                UWVM_AES_U8_WHITE u8"\": ",
                                e,
                                UWVM_AES_U8_RST_ALL u8"\n"
# ifndef _WIN32  // Win32 automatically adds a newline
                                u8"\n"
# endif
                            );
            return -1;
        }
#endif

        return 0;
    }
}  // namespace uwvm::run
