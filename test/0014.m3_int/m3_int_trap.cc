#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <csetjmp>
#include <cstddef>
#include <cstring>
#include <limits>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

namespace
{
    ::std::jmp_buf jump_env;
    int trap_tag{};

    [[noreturn]] void jump_with_tag(int tag) noexcept
    {
        trap_tag = tag;
        std::longjmp(jump_env, 1);
    }

    void unsupported_hook() noexcept { jump_with_tag(1); }
    void unreachable_hook() noexcept { jump_with_tag(2); }
    void divide_by_zero_hook() noexcept { jump_with_tag(3); }
    void overflow_hook() noexcept { jump_with_tag(4); }
    void invalid_conversion_hook() noexcept { jump_with_tag(5); }
    void out_of_bounds_hook() noexcept { jump_with_tag(6); }
}

template <typename T>
inline void store(::std::byte* base, ::std::ptrdiff_t offset, T value) noexcept
{
    ::std::memcpy(base + offset, ::std::addressof(value), sizeof(T));
}

inline void emit_offset(::std::byte*& code, optable::m3_slot_offset_t offset) noexcept
{
    ::std::memcpy(code, ::std::addressof(offset), sizeof(offset));
    code += sizeof(offset);
}

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_i64 = optable::wasm_i64;
    using wasm_f64 = optable::wasm_f64;

    static_assert(optable::translate::get_Unsupported_fptr() != nullptr);
    static_assert(optable::translate::get_Unreachable_fptr() != nullptr);

    auto const reset_hooks = []() noexcept {
        optable::unsupported_func = nullptr;
        optable::unreachable_func = nullptr;
        optable::trap_integer_divide_by_zero_func = nullptr;
        optable::trap_integer_overflow_func = nullptr;
        optable::trap_invalid_conversion_to_integer_func = nullptr;
        optable::trap_out_of_bounds_memory_access_func = nullptr;
        optable::memory_grow_func = nullptr;
        optable::memory_view = {};
    };

    {
        reset_hooks();
        optable::unsupported_func = unsupported_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[4]{};
            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{};
            optable::m3_fp_register_t fp0{};
            optable::Unsupported(ip, nullptr, r0, fp0);
            return 1;
        }
        if(trap_tag != 1) { return 2; }
    }

    {
        reset_hooks();
        optable::unreachable_func = unreachable_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[4]{};
            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{};
            optable::m3_fp_register_t fp0{};
            optable::Unreachable(ip, nullptr, r0, fp0);
            return 3;
        }
        if(trap_tag != 2) { return 4; }
    }

    {
        reset_hooks();
        optable::trap_integer_divide_by_zero_func = divide_by_zero_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[16]{};
            ::std::byte* code_it{code_buf};
            emit_offset(code_it, 8);

            alignas(16) ::std::byte stack[16]{};
            store(stack, 8, wasm_i32{0});

            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{wasm_i32{7}};
            optable::m3_fp_register_t fp0{};
            optable::i32_Divide<optable::binary_shape_t::sr>(ip, stack, r0, fp0);
            return 5;
        }
        if(trap_tag != 3) { return 6; }
    }

    {
        reset_hooks();
        optable::trap_integer_overflow_func = overflow_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[16]{};
            ::std::byte* code_it{code_buf};
            emit_offset(code_it, 8);

            alignas(16) ::std::byte stack[16]{};
            store(stack, 8, wasm_i64{-1});

            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{::std::numeric_limits<wasm_i64>::min()};
            optable::m3_fp_register_t fp0{};
            optable::i64_Divide<optable::binary_shape_t::sr>(ip, stack, r0, fp0);
            return 7;
        }
        if(trap_tag != 4) { return 8; }
    }

    {
        reset_hooks();
        optable::trap_invalid_conversion_to_integer_func = invalid_conversion_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[4]{};
            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{};
            optable::m3_fp_register_t fp0{static_cast<wasm_f64>(std::numeric_limits<double>::quiet_NaN())};
            optable::i32_Trunc_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);
            return 9;
        }
        if(trap_tag != 5) { return 10; }
    }

    {
        reset_hooks();
        optable::trap_integer_overflow_func = overflow_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte code_buf[4]{};
            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{};
            optable::m3_fp_register_t fp0{static_cast<wasm_f64>(1.0e300)};
            optable::i32_Trunc_f64<optable::register_slot_shape_t::rr>(ip, nullptr, r0, fp0);
            return 11;
        }
        if(trap_tag != 4) { return 12; }
    }

    {
        reset_hooks();
        optable::trap_out_of_bounds_memory_access_func = out_of_bounds_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte memory[8]{};
            optable::memory_view.data = memory;
            optable::memory_view.size = sizeof(memory);

            alignas(16) ::std::byte code_buf[4]{};
            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{wasm_i32{6}};
            optable::m3_fp_register_t fp0{};
            optable::i32_Load_i32<optable::unary_shape_t::r>(ip, nullptr, r0, fp0);
            return 13;
        }
        if(trap_tag != 6) { return 14; }
    }

    {
        reset_hooks();
        optable::trap_out_of_bounds_memory_access_func = out_of_bounds_hook;
        trap_tag = 0;
        if(setjmp(jump_env) == 0)
        {
            alignas(16) ::std::byte memory[8]{};
            optable::memory_view.data = memory;
            optable::memory_view.size = sizeof(memory);
            optable::memory_view.page_size = 8;
            optable::memory_view.page_count = 1;

            alignas(16) ::std::byte code_buf[16]{};
            ::std::byte* code_it{code_buf};
            emit_offset(code_it, 0);
            emit_offset(code_it, 4);

            alignas(16) ::std::byte stack[8]{};
            store(stack, 0, wasm_i32{0x7f});
            store(stack, 4, wasm_i32{6});

            optable::m3_code_t ip{code_buf};
            optable::m3_int_register_t r0{wasm_i32{4}};
            optable::m3_fp_register_t fp0{};
            optable::MemFill(ip, stack, r0, fp0);
            return 15;
        }
        if(trap_tag != 6) { return 16; }
    }

    reset_hooks();
    return 0;
}
