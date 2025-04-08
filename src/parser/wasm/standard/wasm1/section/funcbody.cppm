/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
 * Copyright (c) 2025 MacroModel. All rights reserved.  *
 * Licensed under the APL-2 License (see LICENSE file). *
 ********************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2025-04-05
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

export module parser.wasm.standard.wasm1.section:funcbody;

import fast_io;
import parser.wasm.standard.wasm1.type;

export namespace parser::wasm::standard::wasm1::section
{
    /// @brief function bodys, use to storage
    struct code_func_body UWVM_TRIVIALLY_RELOCATABLE_IF_ELIGIBLE
    {
        ::fast_io::vector<::parser::wasm::standard::wasm1::type::local_entry> locals{};
        ::parser::wasm::standard::wasm1::type::vec_byte body{};
    };
}  // namespace parser::wasm::standard::wasm1::section

export namespace fast_io::freestanding
{
    template <>
    struct is_trivially_copyable_or_relocatable<parser::wasm::standard::wasm1::section::code_func_body>
    {
        inline static constexpr bool value = true;
    };

    template <>
    struct is_zero_default_constructible<parser::wasm::standard::wasm1::section::code_func_body>
    {
        inline static constexpr bool value = true;
    };
}  // namespace fast_io::freestanding
