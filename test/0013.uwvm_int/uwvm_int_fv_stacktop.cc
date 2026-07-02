#include <uwvm2/runtime/compiler/uwvm_int/compile_all_from_uwvm/translate.h>
#include <uwvm2/runtime/compiler/uwvm_int/optable/stacktop_window.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

namespace optable = ::uwvm2::runtime::compiler::uwvm_int::optable;
namespace compiler = ::uwvm2::runtime::compiler::uwvm_int::compile_all_from_uwvm;

using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;
using wasm_v128 = ::uwvm2::parser::wasm::standard::wasm1p1::type::wasm_v128;

inline constexpr optable::uwvm_interpreter_translate_option_t opt_fv8{.is_tail_call = true,
                                                                       .f32_stack_top_begin_pos = 0uz,
                                                                       .f32_stack_top_end_pos = 8uz,
                                                                       .f64_stack_top_begin_pos = 0uz,
                                                                       .f64_stack_top_end_pos = 8uz,
                                                                       .v128_stack_top_begin_pos = 0uz,
                                                                       .v128_stack_top_end_pos = 8uz};

static_assert(opt_fv8.i32_stack_top_begin_pos == SIZE_MAX && opt_fv8.i32_stack_top_end_pos == SIZE_MAX);
static_assert(opt_fv8.i64_stack_top_begin_pos == SIZE_MAX && opt_fv8.i64_stack_top_end_pos == SIZE_MAX);
static_assert(optable::details::uwvm_interpreter_uses_logical_fv_stacktop<opt_fv8>());
static_assert(optable::details::uwvm_interpreter_stacktop_arg_pos<opt_fv8, wasm_f32, 0uz>() == 2uz);
static_assert(optable::details::uwvm_interpreter_stacktop_arg_pos<opt_fv8, wasm_f64, 7uz>() == 9uz);
static_assert(optable::details::uwvm_interpreter_stacktop_arg_pos<opt_fv8, wasm_v128, 7uz>() == 9uz);

static_assert(compiler::details::interpreter_tuple_size<opt_fv8>() == 10uz);
static_assert(compiler::details::interpreter_tuple_has_no_holes<opt_fv8>());
static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<0uz, opt_fv8>, ::std::byte const*>);
static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<1uz, opt_fv8>, ::std::byte*>);
static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<2uz, opt_fv8>, wasm_v128>);
static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<9uz, opt_fv8>, wasm_v128>);

using expected_opfunc_t =
    optable::uwvm_interpreter_opfunc_t<::std::byte const*,
                                       ::std::byte*,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128,
                                       wasm_v128>;
static_assert(::std::same_as<compiler::details::interpreter_expected_opfunc_ptr_t<opt_fv8>, expected_opfunc_t>);

[[nodiscard]] static wasm_v128 make_v128(::std::uint_least8_t seed) noexcept
{
    wasm_v128 out{};
    auto* const p{reinterpret_cast<::std::byte*>(::std::addressof(out))};
    for(::std::size_t i{}; i != sizeof(out); ++i) { p[i] = static_cast<::std::byte>(seed + i); }
    return out;
}

[[nodiscard]] static bool same_v128(wasm_v128 a, wasm_v128 b) noexcept
{ return ::std::memcmp(::std::addressof(a), ::std::addressof(b), sizeof(wasm_v128)) == 0; }

template <typename T>
[[nodiscard]] static bool same_bits(T a, T b) noexcept
{
    using uint_t = ::std::conditional_t<sizeof(T) == 4uz, ::std::uint_least32_t, ::std::uint_least64_t>;
    return ::std::bit_cast<uint_t>(a) == ::std::bit_cast<uint_t>(b);
}

int main()
{
    ::std::byte instr{};
    ::std::byte operand_mem[64]{};
    ::std::byte local_mem[64]{};
    ::std::byte const* ip{::std::addressof(instr)};
    ::std::byte* sp{operand_mem};
    ::std::byte* local_base{local_mem};
    optable::uwvm_interpreter_frame_slot_t frame_slot{.operand_stack_top = sp, .local_base = local_base, .operand_base = operand_mem};
    ::std::byte* slot_base{reinterpret_cast<::std::byte*>(::std::addressof(frame_slot))};

    wasm_v128 fv0{};
    wasm_v128 fv1{};
    wasm_v128 fv2{};
    wasm_v128 fv3{};
    wasm_v128 fv4{};
    wasm_v128 fv5{};
    wasm_v128 fv6{};
    wasm_v128 fv7{};

    optable::details::set_curr_val_to_stacktop_cache<opt_fv8, wasm_f32, 0uz>(wasm_f32{1.25f},
                                                                             ip,
                                                                             slot_base,
                                                                             fv0,
                                                                             fv1,
                                                                             fv2,
                                                                             fv3,
                                                                             fv4,
                                                                             fv5,
                                                                             fv6,
                                                                             fv7);
    wasm_f32 const f32_read{optable::get_curr_val_from_operand_stack_top<opt_fv8, wasm_f32, 0uz>(ip,
                                                                                                  slot_base,
                                                                                                  fv0,
                                                                                                  fv1,
                                                                                                  fv2,
                                                                                                  fv3,
                                                                                                  fv4,
                                                                                                  fv5,
                                                                                                  fv6,
                                                                                                  fv7)};
    if(!same_bits(f32_read, wasm_f32{1.25f})) { return 1; }
    if(ip != ::std::addressof(instr) || frame_slot.operand_stack_top != operand_mem || frame_slot.local_base != local_mem) { return 2; }

    optable::details::set_curr_val_to_stacktop_cache<opt_fv8, wasm_f64, 7uz>(wasm_f64{9.5},
                                                                             ip,
                                                                             slot_base,
                                                                             fv0,
                                                                             fv1,
                                                                             fv2,
                                                                             fv3,
                                                                             fv4,
                                                                             fv5,
                                                                             fv6,
                                                                             fv7);
    wasm_f64 const f64_read{optable::get_curr_val_from_operand_stack_top<opt_fv8, wasm_f64, 7uz>(ip,
                                                                                                  slot_base,
                                                                                                  fv0,
                                                                                                  fv1,
                                                                                                  fv2,
                                                                                                  fv3,
                                                                                                  fv4,
                                                                                                  fv5,
                                                                                                  fv6,
                                                                                                  fv7)};
    if(!same_bits(f64_read, wasm_f64{9.5})) { return 3; }

    wasm_v128 const vec{make_v128(0x40u)};
    optable::details::set_curr_val_to_stacktop_cache<opt_fv8, wasm_v128, 2uz>(vec,
                                                                              ip,
                                                                              slot_base,
                                                                              fv0,
                                                                              fv1,
                                                                              fv2,
                                                                              fv3,
                                                                              fv4,
                                                                              fv5,
                                                                              fv6,
                                                                              fv7);
    wasm_v128 const vec_read{optable::get_curr_val_from_operand_stack_top<opt_fv8, wasm_v128, 2uz>(ip,
                                                                                                    slot_base,
                                                                                                    fv0,
                                                                                                    fv1,
                                                                                                    fv2,
                                                                                                    fv3,
                                                                                                    fv4,
                                                                                                    fv5,
                                                                                                    fv6,
                                                                                                    fv7)};
    if(!same_v128(vec, vec_read)) { return 4; }

    wasm_v128 const fv_a{make_v128(0x10u)};
    wasm_v128 const fv_b{make_v128(0x20u)};
    wasm_v128 const fv_c{make_v128(0x30u)};
    wasm_v128 const fv_d{make_v128(0x50u)};
    fv0 = fv_a;
    fv1 = fv_b;
    fv2 = fv_c;
    fv3 = fv_d;

    optable::details::rotate_fv_stacktop_range_next<opt_fv8, 0uz, 4uz, 1uz>(ip, slot_base, fv0, fv1, fv2, fv3, fv4, fv5, fv6, fv7);
    if(!same_v128(fv0, fv_d) || !same_v128(fv1, fv_a) || !same_v128(fv2, fv_b) || !same_v128(fv3, fv_c)) { return 5; }
    if(ip != ::std::addressof(instr) || frame_slot.operand_stack_top != operand_mem || frame_slot.local_base != local_mem) { return 6; }

    return 0;
}
