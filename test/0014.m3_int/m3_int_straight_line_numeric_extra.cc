#include <uwvm2/runtime/compiler/m3_int/straight_line.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace sl = ::uwvm2::runtime::compiler::m3_int::straight_line;
namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

namespace
{
    using wasm_byte = sl::wasm_byte;
    using wasm_i32 = sl::wasm_i32;
    using wasm_u32 = sl::wasm_u32;
    using wasm_i64 = sl::wasm_i64;
    using wasm_u64 = sl::wasm_u64;
    using wasm_f32 = sl::wasm_f32;
    using wasm_f64 = sl::wasm_f64;
    using value_type = sl::value_type;
    using op_basic = sl::op_basic;

    inline void emit_opcode(::std::vector<wasm_byte>& out, op_basic opcode)
    {
        out.push_back(static_cast<wasm_byte>(opcode));
    }

    inline void emit_uleb(::std::vector<wasm_byte>& out, wasm_u32 value)
    {
        do
        {
            auto byte{static_cast<wasm_byte>(value & 0x7Fu)};
            value >>= 7u;
            if(value != 0u)
            {
                byte = static_cast<wasm_byte>(byte | 0x80u);
            }
            out.push_back(byte);
        } while(value != 0u);
    }
}

int main()
{
    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_lt_s);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32, value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 1; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{static_cast<wasm_u64>(static_cast<wasm_i64>(-1)), 1u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 2; }
        if(sl::get_result<wasm_i32>(exec) != 1) { return 3; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_lt_u);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32, value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 4; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{static_cast<wasm_u64>(0xffffffffu), 1u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 5; }
        if(sl::get_result<wasm_i32>(exec) != 0) { return 6; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_div_u);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32, value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 7; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{0xfffffffEu, 2u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 8; }
        if(sl::get_result<wasm_u32>(exec) != 0x7fffffffu) { return 9; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_rotl);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32, value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 10; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{1u, 31u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 11; }
        if(sl::get_result<wasm_u32>(exec) != 0x80000000u) { return 12; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::i64_popcnt);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i64};
        constexpr value_type results[]{value_type::i64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 13; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0xf0f0u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 14; }
        if(sl::get_result<wasm_u64>(exec) != 8u) { return 15; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::f32_ceil);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f32};
        constexpr value_type results[]{value_type::f32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 16; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{static_cast<wasm_u64>(::std::bit_cast<wasm_u32>(wasm_f32{1.25f}))};
        if(!sl::execute_function(compiled.function, args, exec)) { return 17; }
        if(sl::get_result<wasm_f32>(exec) != wasm_f32{2.0f}) { return 18; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::f32_min);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f32, value_type::f32};
        constexpr value_type results[]{value_type::f32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 19; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{
            static_cast<wasm_u64>(::std::bit_cast<wasm_u32>(wasm_f32{0.0f})),
            static_cast<wasm_u64>(::std::bit_cast<wasm_u32>(-wasm_f32{0.0f}))};
        if(!sl::execute_function(compiled.function, args, exec)) { return 20; }
        if(::std::bit_cast<wasm_u32>(sl::get_result<wasm_f32>(exec)) != ::std::bit_cast<wasm_u32>(-wasm_f32{0.0f})) { return 21; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::f64_copysign);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f64, value_type::f64};
        constexpr value_type results[]{value_type::f64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 22; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{
            ::std::bit_cast<wasm_u64>(wasm_f64{3.5}),
            ::std::bit_cast<wasm_u64>(-wasm_f64{0.0})};
        if(!sl::execute_function(compiled.function, args, exec)) { return 23; }
        if(::std::bit_cast<wasm_u64>(sl::get_result<wasm_f64>(exec)) != ::std::bit_cast<wasm_u64>(wasm_f64{-3.5})) { return 24; }
    }

    return 0;
}
