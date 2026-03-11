#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

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

inline void emit_memory_offset(::std::byte*& code, optable::wasm_u32 offset) noexcept
{
    ::std::memcpy(code, ::std::addressof(offset), sizeof(offset));
    code += sizeof(offset);
}

template <typename T>
inline bool memeq(T const& lhs, T const& rhs) noexcept
{
    return ::std::memcmp(::std::addressof(lhs), ::std::addressof(rhs), sizeof(T)) == 0;
}

static_assert(optable::translate::get_i32_Load_i8_fptr<optable::unary_shape_t::r>() != nullptr);
static_assert(optable::translate::get_i64_Load_u32_fptr<optable::unary_shape_t::s>() != nullptr);
static_assert(optable::translate::get_f64_Store_f64_fptr<optable::register_slot_shape_t::rr>() != nullptr);
static_assert(optable::translate::get_i64_Store_i64_fptr<optable::register_slot_shape_t::ss>() != nullptr);

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_u32 = optable::wasm_u32;
    using wasm_i64 = optable::wasm_i64;
    using wasm_u64 = optable::wasm_u64;
    using wasm_f32 = optable::wasm_f32;
    using wasm_f64 = optable::wasm_f64;

    alignas(16) ::std::byte memory[64]{};
    optable::memory_view.data = memory;
    optable::memory_view.size = sizeof(memory);

    {
        alignas(16) ::std::byte code_buf[8]{};
        ::std::byte* code_it{code_buf};
        emit_memory_offset(code_it, 4);

        store(memory, 7, static_cast<::std::int_least8_t>(-5));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{3}};
        optable::m3_fp_register_t fp0{};
        optable::i32_Load_i8<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{-5}) { return 1; }
        if(ip != code_buf + sizeof(wasm_u32)) { return 2; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);
        emit_memory_offset(code_it, 12);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 8, wasm_u32{4});
        store(memory, 16, wasm_u32{0xffffffffu});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i64_Load_u32<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(static_cast<wasm_i64>(r0) != static_cast<wasm_i64>(wasm_u32{0xffffffffu})) { return 3; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t) + sizeof(wasm_u32)) { return 4; }
    }

    {
        alignas(16) ::std::byte code_buf[8]{};
        ::std::byte* code_it{code_buf};
        emit_memory_offset(code_it, 0);

        store(memory, 24, static_cast<wasm_f32>(1.5f));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{24}};
        optable::m3_fp_register_t fp0{};
        optable::f32_Load_f32<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), static_cast<wasm_f32>(1.5f))) { return 5; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_memory_offset(code_it, 3);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_u32{5});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{0x123}};
        optable::m3_fp_register_t fp0{};
        optable::i32_Store_u8<optable::register_slot_shape_t::rs>(ip, stack, r0, fp0);

        if(load<::std::uint_least8_t>(memory, 8) != ::std::uint_least8_t{0x23}) { return 6; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_memory_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{-2});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{12}};
        optable::m3_fp_register_t fp0{};
        optable::i64_Store_i32<optable::register_slot_shape_t::sr>(ip, stack, r0, fp0);

        if(load<wasm_i32>(memory, 16) != wasm_i32{-2}) { return 7; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 8);
        emit_memory_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_i64{0x1122334455667788ll});
        store(stack, 8, wasm_u32{20});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::i64_Store_i64<optable::register_slot_shape_t::ss>(ip, stack, r0, fp0);

        if(load<wasm_i64>(memory, 24) != wasm_i64{0x1122334455667788ll}) { return 8; }
    }

    {
        alignas(16) ::std::byte code_buf[8]{};
        ::std::byte* code_it{code_buf};
        emit_memory_offset(code_it, 0);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{32}};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(6.25)};
        optable::f64_Store_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);

        if(!memeq(load<wasm_f64>(memory, 32), static_cast<wasm_f64>(6.25))) { return 9; }
    }

    optable::memory_view = {};
    return 0;
}
