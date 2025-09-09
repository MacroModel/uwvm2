/*************************************************************
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

module;

// std
#include <cstddef>
#include <cstdint>
#include <climits>
#include <cstring>
#include <limits>
#include <concepts>
#include <bit>
#include <memory>
// macro
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
#include <uwvm2/utils/macro/push_macros.h>

export module uwvm2.imported.wasi.wasip1.func:args_sizes_get_wasm64;

import fast_io;
import uwvm2.uwvm_predefine.utils.ansies;
import uwvm2.uwvm_predefine.io;
import uwvm2.object.memory.linear;
import uwvm2.imported.wasi.wasip1.abi;
import uwvm2.imported.wasi.wasip1.fd_manager;
import uwvm2.imported.wasi.wasip1.memory;
import uwvm2.imported.wasi.wasip1.environment;

#ifndef UWVM_MODULE
# define UWVM_MODULE
#endif
#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT export
#endif

#include "args_sizes_get_wasm64.h"

