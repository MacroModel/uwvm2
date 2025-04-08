/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @brief       The wasm file that will be run
 * @details     "--run" or "-r"
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-03-28
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

export module uwvm.wasm.storage:execute_wasm;

import fast_io;
import parser.wasm.base;

export namespace uwvm::wasm::storage
{
    inline ::fast_io::native_file_loader execute_wasm_file{};
    inline ::parser::wasm::base::mode execute_wasm_mode{::parser::wasm::base::mode::objdump};
    inline ::parser::wasm::base::abi execute_wasm_abi{::parser::wasm::base::abi::detect};
    // TODO: execute_wasm_module // The result of parsing the exewasm is used to add the import module directly.
}  // namespace uwvm::wasm::storage
