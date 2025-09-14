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

export module uwvm2.imported.wasi.wasip1.func;

export import :base;
export import :posix;
export import :args_get_wasm64;
export import :args_get;
export import :args_sizes_get_wasm64;
export import :args_sizes_get;
export import :clock_res_get_wasm64;
export import :clock_res_get;
export import :clock_time_get_wasm64;
export import :clock_time_get;
export import :environ_get_wasm64;
export import :environ_get;
export import :environ_sizes_get_wasm64;
export import :environ_sizes_get;
export import :fd_advise_wasm64;
export import :fd_advise;
export import :fd_allocate_wasm64;
export import :fd_allocate;

#ifndef UWVM_MODULE
# define UWVM_MODULE
#endif
#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT export
#endif

#include "impl.h"
