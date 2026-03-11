#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
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

template <typename T>
inline void emit_immediate(::std::byte*& code, T const& value) noexcept
{
    ::std::memcpy(code, ::std::addressof(value), sizeof(T));
    code += sizeof(T);
}

template <typename T>
inline bool memeq(T const& lhs, T const& rhs) noexcept
{
    return ::std::memcmp(::std::addressof(lhs), ::std::addressof(rhs), sizeof(T)) == 0;
}

static_assert(optable::translate::get_SetRegister_fptr<optable::wasm_i32>() != nullptr);
static_assert(optable::translate::get_CopySlot_fptr<optable::wasm_u64>() != nullptr);
static_assert(optable::translate::get_Branch_fptr() != nullptr);
static_assert(optable::translate::get_If_fptr<optable::unary_shape_t::s>() != nullptr);
static_assert(optable::translate::get_BranchTable_fptr() != nullptr);

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
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{11});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::SetRegister<wasm_i32>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{11}) { return 1; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[16]{};

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i64{99}};
        optable::m3_fp_register_t fp0{};
        optable::SetSlot<wasm_i64>(ip, stack, r0, fp0);

        if(load<wasm_i64>(stack, 8) != wasm_i64{99}) { return 2; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_offset(code_it, 12);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 4, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{22}};
        optable::m3_fp_register_t fp0{};
        optable::PreserveSetSlot<wasm_i32>(ip, stack, r0, fp0);

        if(load<wasm_i32>(stack, 4) != wasm_i32{22}) { return 3; }
        if(load<wasm_i32>(stack, 12) != wasm_i32{7}) { return 4; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 12);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 4, wasm_u32{0x12345678u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::CopySlot<wasm_u32>(ip, stack, r0, fp0);

        if(load<wasm_u32>(stack, 12) != wasm_u32{0x12345678u}) { return 5; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 16);
        emit_offset(code_it, 0);
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 16, wasm_u64{0xaaaaull});
        store(stack, 0, wasm_u64{0xbbbbull});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::PreserveCopySlot<wasm_u64>(ip, stack, r0, fp0);

        if(load<wasm_u64>(stack, 16) != wasm_u64{0xbbbbull}) { return 6; }
        if(load<wasm_u64>(stack, 8) != wasm_u64{0xaaaaull}) { return 7; }
    }

    {
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, wasm_u32{77u});
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Const32(ip, stack, r0, fp0);

        if(load<wasm_u32>(stack, 4) != wasm_u32{77u}) { return 8; }
    }

    {
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, wasm_u64{0x1122334455667788ull});
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[24]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Const64(ip, stack, r0, fp0);

        if(load<wasm_u64>(stack, 8) != wasm_u64{0x1122334455667788ull}) { return 9; }
    }

    {
        wasm_u32 global{123u};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, &global);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::GetGlobal_s32(ip, stack, r0, fp0);

        if(load<wasm_u32>(stack, 4) != wasm_u32{123u}) { return 10; }
    }

    {
        wasm_u64 global{0};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, &global);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i64{456}};
        optable::m3_fp_register_t fp0{};
        optable::SetGlobal_i64(ip, nullptr, r0, fp0);

        if(global != wasm_u64{456u}) { return 11; }
    }

    {
        wasm_u32 global{0};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, &global);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_u32{789u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::SetGlobal_s32(ip, stack, r0, fp0);

        if(global != wasm_u32{789u}) { return 12; }
    }

    {
        wasm_f64 global{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, &global);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(3.5)};
        optable::SetGlobal_f64(ip, nullptr, r0, fp0);

        if(!memeq(global, static_cast<wasm_f64>(3.5))) { return 13; }
    }

    {
        alignas(16) ::std::byte target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<optable::m3_code_t>(target));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Branch(ip, nullptr, r0, fp0);

        if(ip != target) { return 14; }
    }

    {
        alignas(16) ::std::byte else_target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<optable::m3_code_t>(else_target));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{0}};
        optable::m3_fp_register_t fp0{};
        optable::If<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(ip != else_target) { return 15; }
    }

    {
        alignas(16) ::std::byte else_target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_immediate(code_it, static_cast<optable::m3_code_t>(else_target));

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{1});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::If<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(ip != code_buf + sizeof(optable::m3_slot_offset_t) + sizeof(optable::m3_code_t)) { return 16; }
    }

    {
        alignas(16) ::std::byte branch_target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<optable::m3_code_t>(branch_target));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{1}};
        optable::m3_fp_register_t fp0{};
        optable::BranchIf<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);

        if(ip != branch_target) { return 17; }
    }

    {
        alignas(16) ::std::byte branch_target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_immediate(code_it, static_cast<optable::m3_code_t>(branch_target));

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{0});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::BranchIfPrologue<optable::unary_shape_t::s>(ip, stack, r0, fp0);

        if(ip != branch_target) { return 18; }
    }

    {
        alignas(16) ::std::byte t0[4]{};
        alignas(16) ::std::byte t1[4]{};
        alignas(16) ::std::byte td[4]{};
        alignas(16) ::std::byte code_buf[64]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_immediate(code_it, wasm_u32{2u});
        emit_immediate(code_it, static_cast<optable::m3_code_t>(t0));
        emit_immediate(code_it, static_cast<optable::m3_code_t>(t1));
        emit_immediate(code_it, static_cast<optable::m3_code_t>(td));

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_u32{5u});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::BranchTable(ip, stack, r0, fp0);

        if(ip != td) { return 19; }
    }

    return 0;
}
