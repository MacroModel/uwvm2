#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
#include <cstring>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

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

template <typename T>
inline bool memeq(T const& lhs, T const& rhs) noexcept
{
    return ::std::memcmp(::std::addressof(lhs), ::std::addressof(rhs), sizeof(T)) == 0;
}

static_assert(optable::valid_select_shape_v<optable::wasm_i32, optable::select_shape_t::rss>);
static_assert(!optable::valid_select_shape_v<optable::wasm_i32, optable::select_shape_t::rrs>);
static_assert(optable::valid_select_shape_v<optable::wasm_f32, optable::select_shape_t::rrs>);
static_assert(requires { optable::translate::get_Select_i32_fptr<optable::select_shape_t::sss>(); });

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_f32 = optable::wasm_f32;

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_offset(code_it, 12);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 4, wasm_i32{2});
        store(stack, 12, wasm_i32{7});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{1};
        optable::m3_fp_register_t fp0{};
        optable::Select_i32<optable::select_shape_t::rss>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{7}) { return 1; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 12);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 0, wasm_i32{0});
        store(stack, 12, wasm_i32{9});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{5};
        optable::m3_fp_register_t fp0{};
        optable::Select_i32<optable::select_shape_t::srs>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{5}) { return 2; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 12);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 0, wasm_i32{1});
        store(stack, 12, wasm_i32{4});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{8};
        optable::m3_fp_register_t fp0{};
        optable::Select_i32<optable::select_shape_t::ssr>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{8}) { return 3; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 8);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 0, wasm_i32{0});
        store(stack, 4, wasm_i32{11});
        store(stack, 8, wasm_i32{3});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Select_i32<optable::select_shape_t::sss>(ip, stack, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{3}) { return 4; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[32]{};
        auto const lhs{static_cast<wasm_f32>(7.5f)};
        store(stack, 4, lhs);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{1};
        optable::m3_fp_register_t fp0{static_cast<optable::wasm_f64>(2.0)};
        optable::Select_f32<optable::select_shape_t::rrs>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), lhs)) { return 5; }
    }

    {
        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        auto const rhs{static_cast<wasm_f32>(1.25f)};
        store(stack, 8, rhs);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{0};
        optable::m3_fp_register_t fp0{static_cast<optable::wasm_f64>(9.0)};
        optable::Select_f32<optable::select_shape_t::rsr>(ip, stack, r0, fp0);

        if(!memeq(static_cast<wasm_f32>(fp0), rhs)) { return 6; }
    }

    {
        optable::m3_interpreter_opfunc_t f{optable::translate::get_Select_f32_fptr<optable::select_shape_t::rrs>()};
        if(f == nullptr) { return 7; }
    }

    return 0;
}
