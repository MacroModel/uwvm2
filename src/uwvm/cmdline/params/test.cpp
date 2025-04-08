/********************************************************
 * Ultimate WebAssembly Virtual Machine (help 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-04-07
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

#ifdef _DEBUG

# include <memory>

# include <utils/macro/push_macros.h>
# include <utils/ansies/ansi_push_macro.h>

import fast_io;
import utils.io;
import utils.cmdline;
import uwvm.cmdline;
import parser.wasm.base;
import uwvm.wasm.storage;

namespace uwvm::cmdline::paras::details
{
    UWVM_GNU_COLD extern ::utils::cmdline::parameter_return_type test_callback([[maybe_unused]] ::utils::cmdline::parameter_parsing_results* para_begin,
                                                                               [[maybe_unused]] ::utils::cmdline::parameter_parsing_results* para_curr,
                                                                               [[maybe_unused]] ::utils::cmdline::parameter_parsing_results* para_end) noexcept
    {


        return ::utils::cmdline::parameter_return_type::return_soon;
    }

}  // namespace uwvm::cmdline::paras::details

#endif
