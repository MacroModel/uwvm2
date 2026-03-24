/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2026-03-16
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

import fast_io;
import uwvm2.runtime;
import uwvm2.runtime.llvm_jit;
import uwvm2.runtime.compiler.uwvm_int.optable;
import uwvm2.utils.container;
import uwvm2.utils.debug;
import uwvm2.utils.mutex;
import uwvm2.uwvm.io;
import uwvm2.uwvm.runtime.storage;
import uwvm2.uwvm.utils.ansies;

#include "llvm_jit_runtime.default.cpp"
