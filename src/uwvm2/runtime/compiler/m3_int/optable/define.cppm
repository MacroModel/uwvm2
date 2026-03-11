module;

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <uwvm2/utils/macro/push_macros.h>
#include <uwvm2/runtime/compiler/m3_int/macro/push_macros.h>
#include <uwvm2/parser/wasm/standard/wasm1/type/value_type.h>

export module uwvm2.runtime.compiler.m3_int.optable:define;

#ifndef UWVM_MODULE
# define UWVM_MODULE
#endif
#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT export
#endif

#include "define.h"
