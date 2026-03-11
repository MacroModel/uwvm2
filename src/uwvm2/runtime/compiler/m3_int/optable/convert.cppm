module;

#include <bit>
#include <concepts>
#include <type_traits>
#include <uwvm2/utils/macro/push_macros.h>
#include <uwvm2/runtime/compiler/m3_int/macro/push_macros.h>

export module uwvm2.runtime.compiler.m3_int.optable:convert;

import :define;

#ifndef UWVM_MODULE
# define UWVM_MODULE
#endif
#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT export
#endif

#include "convert.h"
