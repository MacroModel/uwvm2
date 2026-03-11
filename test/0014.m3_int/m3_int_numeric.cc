#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

template <typename T>
inline void store(::std::byte* base, ::std::ptrdiff_t offset, T value) noexcept
{
    ::std::memcpy(base + offset, ::std::addressof(value), sizeof(T));
}

template <typename T>
inline T load(::std::byte const* base, ::std::ptrdiff_t offset = 0) noexcept
{
    T out{};
    ::std::memcpy(::std::addressof(out), base + offset, sizeof(T));
    return out;
}

inline void emit_offset(::std::byte*& code, optable::m3_slot_offset_t offset) noexcept
{
    ::std::memcpy(code, ::std::addressof(offset), sizeof(offset));
    code += sizeof(offset);
}

template <typename T>
inline bool memeq(T const& lhs, T const& rhs) noexcept
{
    return ::std::memcmp(::std::addressof(lhs), ::std::addressof(rhs), sizeof(T)) == 0;
}

static_assert(optable::translate::get_i32_Add_fptr<optable::binary_shape_t::sr>() == optable::translate::get_i32_Add_fptr<optable::binary_shape_t::rs>());
static_assert(optable::translate::get_i32_Subtract_fptr<optable::binary_shape_t::sr>() !=
              optable::translate::get_i32_Subtract_fptr<optable::binary_shape_t::rs>());
static_assert(optable::translate::get_f64_Convert_i32_fptr<optable::register_slot_shape_t::rr>() != nullptr);
static_assert(optable::translate::get_i32_Reinterpret_f32_fptr<optable::register_slot_shape_t::ss>() != nullptr);
static_assert(optable::translate::get_i32_Wrap_i64_fptr<optable::unary_shape_t::r>() != nullptr);
static_assert(optable::translate::get_i32_NotEqual_fptr<optable::binary_shape_t::rs>() != nullptr);
static_assert(optable::translate::get_i32_Divide_fptr<optable::binary_shape_t::sr>() != nullptr);
static_assert(optable::translate::get_u32_Remainder_fptr<optable::binary_shape_t::ss>() != nullptr);
static_assert(optable::translate::get_u32_Rotl_fptr<optable::binary_shape_t::ss>() != nullptr);
static_assert(optable::translate::get_u32_Clz_fptr<optable::unary_shape_t::s>() != nullptr);
static_assert(optable::translate::get_f64_Sqrt_fptr<optable::unary_shape_t::s>() != nullptr);
static_assert(optable::translate::get_f32_Min_fptr<optable::binary_shape_t::ss>() != nullptr);
static_assert(optable::translate::get_f64_CopySign_fptr<optable::binary_shape_t::rs>() != nullptr);
static_assert(optable::translate::get_f64_Convert_u64_fptr<optable::register_slot_shape_t::rr>() != nullptr);
static_assert(optable::translate::get_i32_Trunc_f64_fptr<optable::register_slot_shape_t::rr>() != nullptr);
static_assert(optable::translate::get_u32_TruncSat_f32_fptr<optable::register_slot_shape_t::rr>() != nullptr);
static_assert(optable::translate::get_i64_Extend_u32_fptr<optable::unary_shape_t::s>() != nullptr);

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_u32 = optable::wasm_u32;
    using wasm_i64 = optable::wasm_i64;
    using wasm_u64 = optable::wasm_u64;
    using wasm_f32 = optable::wasm_f32;
    using wasm_f64 = optable::wasm_f64;

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 8, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{5};
        optable::m3_fp_register_t fp0{};
        optable::i32_Add<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{12}) { return 1; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t)) { return 2; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 12);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 12, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{5};
        optable::m3_fp_register_t fp0{};
        optable::i32_Add<optable::binary_shape_t::rs>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{12}) { return 3; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t)) { return 4; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 12);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 4, wasm_i32{5});
        store(stack, 12, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i32_Add<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{12}) { return 5; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t) * 2) { return 6; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 20);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 20, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{15};
        optable::m3_fp_register_t fp0{};
        optable::i32_Subtract<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{8}) { return 7; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 24);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 24, wasm_i32{9});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{0};
        optable::m3_fp_register_t fp0{};
        optable::i32_EqualToZero<optable::unary_shape_t::r>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 8; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{0});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i32_EqualToZero<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 9; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        auto const rhs{static_cast<wasm_f32>(2.25f)};
        store(stack, 8, rhs);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(1.5)};
        optable::f32_Add<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        auto const expected{static_cast<wasm_f32>(3.75f)};
        auto const actual{static_cast<wasm_f32>(fp0)};
        if(!memeq(actual, expected)) { return 10; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 12);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 0, static_cast<wasm_f64>(6.0));
        store(stack, 12, static_cast<wasm_f64>(7.0));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Multiply<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(42.0))) { return 11; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, static_cast<wasm_f64>(5.0));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Negate<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(-5.0))) { return 12; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{-7};
        optable::m3_fp_register_t fp0{};
        optable::f64_Convert_i32<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(-7.0))) { return 13; }
        if(ip != code_buf) { return 14; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 8, wasm_i64{42});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f32_Convert_i64<optable::register_slot_shape_t::rs>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(42.0f))) { return 15; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t)) { return 16; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{9};
        optable::m3_fp_register_t fp0{};
        optable::f32_Convert_i32<optable::register_slot_shape_t::sr>(ip, stack, r0, fp0);

        if(!memeq(load<wasm_f32>(stack, 4), static_cast<wasm_f32>(9.0f))) { return 17; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t)) { return 18; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 16);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 16, wasm_i64{-11});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Convert_i64<optable::register_slot_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(load<wasm_f64>(stack, 0), static_cast<wasm_f64>(-11.0))) { return 19; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t) * 2) { return 20; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        auto const source{static_cast<wasm_f32>(1.0f)};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(source)};
        optable::i32_Reinterpret_f32<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        auto const expected{::std::bit_cast<wasm_i32>(source)};
        if(!memeq(static_cast<wasm_i32>(r0), expected)) { return 21; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        auto const expected{static_cast<wasm_f32>(2.5f)};
        store(stack, 8, ::std::bit_cast<wasm_i32>(expected));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f32_Reinterpret_i32<optable::register_slot_shape_t::rs>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), expected)) { return 22; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        auto const expected{static_cast<wasm_f64>(-0.0)};
        auto const bits{::std::bit_cast<wasm_i64>(expected)};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{bits};
        optable::m3_fp_register_t fp0{};
        optable::f64_Reinterpret_i64<optable::register_slot_shape_t::sr>(ip, stack, r0, fp0);

        if(!memeq(load<wasm_f64>(stack, 0), expected)) { return 23; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[24]{};
        auto const source{static_cast<wasm_f64>(3.25)};
        store(stack, 8, source);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i64_Reinterpret_f64<optable::register_slot_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(load<wasm_i64>(stack, 0), ::std::bit_cast<wasm_i64>(source))) { return 24; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{static_cast<wasm_i64>(0x100000001ull)};
        optable::m3_fp_register_t fp0{};
        optable::i32_Wrap_i64<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 25; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_i32{-7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i64_Extend_i32<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i64>(r0) != wasm_i64{-7}) { return 26; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, static_cast<wasm_f32>(3.25f));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Promote_f32<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(3.25))) { return 27; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(6.5)};
        optable::f32_Demote_f64<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(6.5f))) { return 28; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{4});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{5};
        optable::m3_fp_register_t fp0{};
        optable::i32_NotEqual<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 29; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0xffffffffu});
        store(stack, 8, wasm_u32{1u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u32_LessThan<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{0}) { return 30; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_u64{1u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{::std::bit_cast<wasm_i64>(wasm_u64{0x8000000000000000ull})};
        optable::m3_fp_register_t fp0{};
        optable::u64_GreaterThan<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 31; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0x80000000u});
        store(stack, 8, wasm_u32{0u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u32_Xor<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_i32>(r0), ::std::bit_cast<wasm_i32>(wasm_u32{0x80000000u}))) { return 32; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_i32{2});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{-8};
        optable::m3_fp_register_t fp0{};
        optable::i32_ShiftRight<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{-2}) { return 33; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_u32{1u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{::std::bit_cast<wasm_i32>(wasm_u32{0x80000000u})};
        optable::m3_fp_register_t fp0{};
        optable::u32_ShiftRight<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_i32>(r0), ::std::bit_cast<wasm_i32>(wasm_u32{0x40000000u}))) { return 34; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0x80000001u});
        store(stack, 8, wasm_u32{1u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u32_Rotl<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_i32>(r0), ::std::bit_cast<wasm_i32>(wasm_u32{0x00000003u}))) { return 35; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[24]{};
        store(stack, 0, wasm_u64{0x8000000000000001ull});
        store(stack, 8, wasm_u64{1u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u64_Rotr<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_i64>(r0), ::std::bit_cast<wasm_i64>(wasm_u64{0xc000000000000000ull}))) { return 36; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_u32{0x00100000u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u32_Clz<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{11}) { return 37; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_u64{0xf0f0f0f0f0f0f0f0ull});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u64_Popcnt<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i64>(r0) != wasm_i64{32}) { return 38; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{0x80}};
        optable::m3_fp_register_t fp0{};
        optable::i32_Extend8_s<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{-128}) { return 39; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{static_cast<wasm_i64>(0x80000000ull)};
        optable::m3_fp_register_t fp0{};
        optable::i64_Extend32_s<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i64>(r0) != static_cast<wasm_i64>(static_cast<::std::int_least32_t>(0x80000000u))) { return 40; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_i32{3});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{15};
        optable::m3_fp_register_t fp0{};
        optable::i32_Divide<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{5}) { return 41; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{17u});
        store(stack, 8, wasm_u32{5u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::u32_Remainder<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{2}) { return 42; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_i64{-1});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{::std::numeric_limits<wasm_i64>::min()};
        optable::m3_fp_register_t fp0{};
        optable::i64_Remainder<optable::binary_shape_t::sr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i64>(r0) != wasm_i64{0}) { return 43; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(-1.5f)};
        optable::f32_Abs<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(1.5f))) { return 44; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, static_cast<wasm_f64>(81.0));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Sqrt<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(9.0))) { return 45; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, static_cast<wasm_f32>(2.75f));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f32_Nearest<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(3.0f))) { return 46; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{::std::bit_cast<wasm_i64>(wasm_u64{42u})};
        optable::m3_fp_register_t fp0{};
        optable::f64_Convert_u64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(!memeq(static_cast<wasm_f64>(fp0), static_cast<wasm_f64>(42.0))) { return 47; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0xffffffffu});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i64_Extend_u32<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i64>(r0) != static_cast<wasm_i64>(wasm_u32{0xffffffffu})) { return 48; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(7.9)};
        optable::i32_Trunc_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{7}) { return 49; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(42.99)};
        optable::u64_Trunc_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i64>(r0) != wasm_i64{42}) { return 50; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(-3.0)};
        optable::u32_TruncSat_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{0}) { return 51; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(1.0e300)};
        optable::i64_TruncSat_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i64>(r0) != ::std::numeric_limits<wasm_i64>::max()) { return 52; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(::std::numeric_limits<wasm_f32>::quiet_NaN())};
        optable::i32_TruncSat_f32<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{0}) { return 53; }
    }

    {
        optable::m3_interpreter_opfunc_t f{optable::translate::get_i32_EqualToZero_fptr<optable::unary_shape_t::s>()};
        if(f == nullptr) { return 54; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, -0.0f);
        store(stack, 4, 0.0f);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f32_Min<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        auto const result{static_cast<wasm_f32>(fp0)};
        if(!::std::signbit(result) || result != 0.0f) { return 55; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, 0.0);
        store(stack, 8, -0.0);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Max<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        auto const result{static_cast<wasm_f64>(fp0)};
        if(::std::signbit(result) || result != 0.0) { return 56; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, 3.25f);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{-1.0};
        optable::f32_CopySign<optable::binary_shape_t::rs>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(-3.25f))) { return 57; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, ::std::numeric_limits<wasm_f64>::quiet_NaN());
        store(stack, 8, static_cast<wasm_f64>(1.0));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::f64_Min<optable::binary_shape_t::ss>(ip, stack, r0, fp0);

        if(!::std::isnan(static_cast<wasm_f64>(fp0))) { return 58; }
    }

    (void)load<wasm_i64>;
    return 0;
}
