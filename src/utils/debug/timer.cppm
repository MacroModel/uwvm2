/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-04-01
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

#include <utils/macro/push_macros.h>
#include <utils/ansies/ansi_push_macro.h>

export module utils.debug:timer;

import fast_io;
import utils.io;

export namespace utils::debug
{
    struct timer
    {
        ::fast_io::u8string_view s;
        ::fast_io::unix_timestamp t0;

        // posix_clock_gettime may throw
        UWVM_GNU_COLD inline explicit timer(::fast_io::u8string_view strvw) noexcept : s{strvw}
        {
#ifdef __cpp_exceptions
            try
#endif
            {
                t0 = ::fast_io::posix_clock_gettime(::fast_io::posix_clock_id::monotonic_raw);
            }
#ifdef __cpp_exceptions
            catch(::fast_io::error e)
            {
                // do nothing
            }
#endif
        }

        inline timer(timer const&) = delete;
        inline timer& operator= (timer const&) = delete;
        inline timer(timer&&) = delete;
        inline timer& operator= (timer&&) = delete;

        UWVM_GNU_COLD inline ~timer()
        {
            ::fast_io::unix_timestamp t1{};

#ifdef __cpp_exceptions
            try
#endif
            {
                t1 = ::fast_io::posix_clock_gettime(::fast_io::posix_clock_id::monotonic_raw);
            }
#ifdef __cpp_exceptions
            catch(::fast_io::error e)
            {
                // do nothing and return imme
                return;
            }
#endif
            ::fast_io::io::perr(
                ::utils::u8err,
                UWVM_AES_U8_RST_ALL UWVM_AES_U8_WHITE u8"uwvm: " UWVM_AES_U8_LT_CYAN u8"[debug] " UWVM_AES_U8_WHITE u8"timer \"" UWVM_AES_U8_YELLOW,
                s,
                UWVM_AES_U8_WHITE u8"\": ",
                t1 - t0,
                u8"s\n" UWVM_AES_U8_RST_ALL);
        }
    };

}  // namespace utils::debug
