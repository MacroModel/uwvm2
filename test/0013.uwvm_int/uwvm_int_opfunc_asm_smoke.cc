#include <uwvm2/runtime/compiler/uwvm_int/optable/conbine.h>
#include <uwvm2/runtime/compiler/uwvm_int/optable/numeric.h>
#include <uwvm2/runtime/compiler/uwvm_int/optable/stack.h>
#include <uwvm2/runtime/compiler/uwvm_int/optable/wasm1p1.h>

#include <cstddef>

namespace optable = ::uwvm2::runtime::compiler::uwvm_int::optable;

using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
using wasm_v128 = ::uwvm2::parser::wasm::standard::wasm1p1::type::wasm_v128;

inline constexpr optable::uwvm_interpreter_translate_option_t opt_fv2{.is_tail_call = true,
                                                                       .f32_stack_top_begin_pos = 0uz,
                                                                       .f32_stack_top_end_pos = 2uz,
                                                                       .f64_stack_top_begin_pos = 0uz,
                                                                       .f64_stack_top_end_pos = 2uz,
                                                                       .v128_stack_top_begin_pos = 0uz,
                                                                       .v128_stack_top_end_pos = 2uz};

static_assert(opt_fv2.i32_stack_top_begin_pos == SIZE_MAX && opt_fv2.i32_stack_top_end_pos == SIZE_MAX);
static_assert(opt_fv2.i64_stack_top_begin_pos == SIZE_MAX && opt_fv2.i64_stack_top_end_pos == SIZE_MAX);
static_assert(optable::details::uwvm_interpreter_uses_logical_fv_stacktop<opt_fv2>());

using fv2_opfunc_t = optable::uwvm_interpreter_opfunc_t<::std::byte const*, ::std::byte*, wasm_v128, wasm_v128>;

#define UWVM_KEEP_I32_ANON(NAME, OP)                                                                                              \
    extern "C" [[gnu::used]] fv2_opfunc_t keep_i32_##NAME##_anon0 =                                                              \
        optable::uwvmint_i32_binop_anonymous_stacktop<opt_fv2,                                                                    \
                                                      optable::numeric_details::int_binop::OP,                                    \
                                                      0uz,                                                                        \
                                                      ::std::byte const*,                                                         \
                                                      ::std::byte*,                                                               \
                                                      wasm_v128,                                                                  \
                                                      wasm_v128>

#define UWVM_KEEP_I64_ANON(NAME, OP)                                                                                              \
    extern "C" [[gnu::used]] fv2_opfunc_t keep_i64_##NAME##_anon0 =                                                              \
        optable::uwvmint_i64_binop_anonymous_stacktop<opt_fv2,                                                                    \
                                                      optable::numeric_details::int_binop::OP,                                    \
                                                      0uz,                                                                        \
                                                      ::std::byte const*,                                                         \
                                                      ::std::byte*,                                                               \
                                                      wasm_v128,                                                                  \
                                                      wasm_v128>

#define UWVM_KEEP_I32_2LOCALGET(NAME, OP)                                                                                         \
    extern "C" [[gnu::used]] fv2_opfunc_t keep_i32_##NAME##_2localget_fv2 =                                                      \
        optable::uwvmint_i32_binop_2localget<opt_fv2,                                                                             \
                                             optable::numeric_details::int_binop::OP,                                             \
                                             0uz,                                                                                 \
                                             ::std::byte const*,                                                                  \
                                             ::std::byte*,                                                                        \
                                             wasm_v128,                                                                           \
                                             wasm_v128>

#define UWVM_KEEP_I64_2LOCALGET(NAME, OP)                                                                                         \
    extern "C" [[gnu::used]] fv2_opfunc_t keep_i64_##NAME##_2localget_fv2 =                                                      \
        optable::uwvmint_i64_binop_2localget<opt_fv2,                                                                             \
                                             optable::numeric_details::int_binop::OP,                                             \
                                             0uz,                                                                                 \
                                             ::std::byte const*,                                                                  \
                                             ::std::byte*,                                                                        \
                                             wasm_v128,                                                                           \
                                             wasm_v128>

UWVM_KEEP_I32_ANON(add, add);
UWVM_KEEP_I32_ANON(sub, sub);
UWVM_KEEP_I32_ANON(mul, mul);
UWVM_KEEP_I32_ANON(div_s, div_s);
UWVM_KEEP_I32_ANON(div_u, div_u);
UWVM_KEEP_I32_ANON(rem_s, rem_s);
UWVM_KEEP_I32_ANON(rem_u, rem_u);
UWVM_KEEP_I32_ANON(band, and_);
UWVM_KEEP_I32_ANON(bor, or_);
UWVM_KEEP_I32_ANON(bxor, xor_);
UWVM_KEEP_I32_ANON(shl, shl);
UWVM_KEEP_I32_ANON(shr_s, shr_s);
UWVM_KEEP_I32_ANON(shr_u, shr_u);
UWVM_KEEP_I32_ANON(rotl, rotl);
UWVM_KEEP_I32_ANON(rotr, rotr);

UWVM_KEEP_I64_ANON(add, add);
UWVM_KEEP_I64_ANON(sub, sub);
UWVM_KEEP_I64_ANON(mul, mul);
UWVM_KEEP_I64_ANON(div_s, div_s);
UWVM_KEEP_I64_ANON(div_u, div_u);
UWVM_KEEP_I64_ANON(rem_s, rem_s);
UWVM_KEEP_I64_ANON(rem_u, rem_u);
UWVM_KEEP_I64_ANON(band, and_);
UWVM_KEEP_I64_ANON(bor, or_);
UWVM_KEEP_I64_ANON(bxor, xor_);
UWVM_KEEP_I64_ANON(shl, shl);
UWVM_KEEP_I64_ANON(shr_s, shr_s);
UWVM_KEEP_I64_ANON(shr_u, shr_u);
UWVM_KEEP_I64_ANON(rotl, rotl);
UWVM_KEEP_I64_ANON(rotr, rotr);

extern "C" [[gnu::used]] fv2_opfunc_t keep_f64_add_fv2 =
    optable::uwvmint_f64_binop<opt_fv2,
                               optable::numeric_details::float_binop::add,
                               0uz,
                               ::std::byte const*,
                               ::std::byte*,
                               wasm_v128,
                               wasm_v128>;

extern "C" [[gnu::used]] fv2_opfunc_t keep_f32_add_fv2 =
    optable::uwvmint_f32_binop<opt_fv2,
                               optable::numeric_details::float_binop::add,
                               0uz,
                               ::std::byte const*,
                               ::std::byte*,
                               wasm_v128,
                               wasm_v128>;

extern "C" [[gnu::used]] fv2_opfunc_t keep_v128_xor_fv2 =
    optable::uwvmint_simd_v128_binop<opt_fv2,
                                     optable::wasm1p1_simd_details::v128_binop::xor_,
                                     0uz,
                                     ::std::byte const*,
                                     ::std::byte*,
                                     wasm_v128,
                                     wasm_v128>;

UWVM_KEEP_I32_2LOCALGET(add, add);
UWVM_KEEP_I32_2LOCALGET(sub, sub);
UWVM_KEEP_I32_2LOCALGET(mul, mul);
UWVM_KEEP_I32_2LOCALGET(div_s, div_s);
UWVM_KEEP_I32_2LOCALGET(div_u, div_u);
UWVM_KEEP_I32_2LOCALGET(rem_s, rem_s);
UWVM_KEEP_I32_2LOCALGET(rem_u, rem_u);
UWVM_KEEP_I32_2LOCALGET(band, and_);
UWVM_KEEP_I32_2LOCALGET(bor, or_);
UWVM_KEEP_I32_2LOCALGET(bxor, xor_);
UWVM_KEEP_I32_2LOCALGET(shl, shl);
UWVM_KEEP_I32_2LOCALGET(shr_s, shr_s);
UWVM_KEEP_I32_2LOCALGET(shr_u, shr_u);
UWVM_KEEP_I32_2LOCALGET(rotl, rotl);
UWVM_KEEP_I32_2LOCALGET(rotr, rotr);

UWVM_KEEP_I64_2LOCALGET(add, add);
UWVM_KEEP_I64_2LOCALGET(sub, sub);
UWVM_KEEP_I64_2LOCALGET(mul, mul);
UWVM_KEEP_I64_2LOCALGET(div_s, div_s);
UWVM_KEEP_I64_2LOCALGET(div_u, div_u);
UWVM_KEEP_I64_2LOCALGET(rem_s, rem_s);
UWVM_KEEP_I64_2LOCALGET(rem_u, rem_u);
UWVM_KEEP_I64_2LOCALGET(band, and_);
UWVM_KEEP_I64_2LOCALGET(bor, or_);
UWVM_KEEP_I64_2LOCALGET(bxor, xor_);
UWVM_KEEP_I64_2LOCALGET(shl, shl);
UWVM_KEEP_I64_2LOCALGET(shr_s, shr_s);
UWVM_KEEP_I64_2LOCALGET(shr_u, shr_u);
UWVM_KEEP_I64_2LOCALGET(rotl, rotl);
UWVM_KEEP_I64_2LOCALGET(rotr, rotr);

#undef UWVM_KEEP_I32_ANON
#undef UWVM_KEEP_I64_ANON
#undef UWVM_KEEP_I32_2LOCALGET
#undef UWVM_KEEP_I64_2LOCALGET

int main()
{
    return keep_i32_add_anon0 != nullptr && keep_i64_add_anon0 != nullptr && keep_f64_add_fv2 != nullptr &&
                   keep_f32_add_fv2 != nullptr && keep_v128_xor_fv2 != nullptr && keep_i32_add_2localget_fv2 != nullptr &&
                   keep_i64_add_2localget_fv2 != nullptr
               ? 0
               : 1;
}
