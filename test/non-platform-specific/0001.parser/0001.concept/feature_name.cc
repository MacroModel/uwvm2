/********************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)     *
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

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>

import fast_io;
import parser.wasm.concepts;

struct feature1
{
    inline static constexpr ::fast_io::u8string_view feature_name{u8"<name>"};
};

static_assert(::parser::wasm::concepts::has_feature_name<feature1>);

struct feature2
{
    ::fast_io::u8string_view feature_name{u8"<name>"};
};

static_assert(!::parser::wasm::concepts::has_feature_name<feature2>);

int main() {}
