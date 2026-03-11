module;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <uwvm2/utils/macro/push_macros.h>
#include <uwvm2/runtime/compiler/m3_int/macro/push_macros.h>
#include <uwvm2/utils/container/impl.h>
#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>
#include <uwvm2/parser/wasm/standard/wasm1/opcode/mvp.h>
#include <uwvm2/parser/wasm/standard/wasm1/type/value_binfmt.h>

export module uwvm2.runtime.compiler.m3_int.straight_line;

#ifndef UWVM_MODULE
# define UWVM_MODULE
#endif
#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT export
#endif

#include "straight_line.h"
