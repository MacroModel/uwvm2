#include <uwvm2/runtime/compiler/m3_int/straight_line.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sl = ::uwvm2::runtime::compiler::m3_int::straight_line;

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
}

int main()
{
    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::i32_wrap_i64);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i64};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 1; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0x100000002ULL};
        if(!sl::execute_function(compiled.function, args, exec)) { return 2; }
        if(sl::get_result<wasm_i32>(exec) != 2) { return 3; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::i64_extend_i32_u);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::i64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 4; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0xffffffffu};
        if(!sl::execute_function(compiled.function, args, exec)) { return 5; }
        if(sl::get_result<wasm_u64>(exec) != 0xffffffffULL) { return 6; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::f32_convert_i32_u);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::f32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 7; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{17u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 8; }
        if(sl::get_result<wasm_f32>(exec) != wasm_f32{17.0f}) { return 9; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::i32_trunc_f64_s);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f64};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 10; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{::std::bit_cast<wasm_u64>(wasm_f64{42.875})};
        if(!sl::execute_function(compiled.function, args, exec)) { return 11; }
        if(sl::get_result<wasm_i32>(exec) != 42) { return 12; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::f64_promote_f32);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f32};
        constexpr value_type results[]{value_type::f64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 13; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{static_cast<wasm_u64>(::std::bit_cast<wasm_u32>(wasm_f32{1.5f}))};
        if(!sl::execute_function(compiled.function, args, exec)) { return 14; }
        if(sl::get_result<wasm_f64>(exec) != wasm_f64{1.5}) { return 15; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::i32_reinterpret_f32);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 16; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{static_cast<wasm_u64>(::std::bit_cast<wasm_u32>(-wasm_f32{0.0f}))};
        if(!sl::execute_function(compiled.function, args, exec)) { return 17; }
        if(sl::get_result<wasm_u32>(exec) != ::std::bit_cast<wasm_u32>(-wasm_f32{0.0f})) { return 18; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::f32_reinterpret_i32);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::f32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 19; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0x3f800000u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 20; }
        if(::std::bit_cast<wasm_u32>(sl::get_result<wasm_f32>(exec)) != 0x3f800000u) { return 21; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_opcode(expr, static_cast<op_basic>(0));
        emit_opcode(expr, op_basic::i64_reinterpret_f64);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f64};
        constexpr value_type results[]{value_type::i64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 22; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{::std::bit_cast<wasm_u64>(wasm_f64{1.0})};
        if(!sl::execute_function(compiled.function, args, exec)) { return 23; }
        if(sl::get_result<wasm_u64>(exec) != ::std::bit_cast<wasm_u64>(wasm_f64{1.0})) { return 24; }
    }

    return 0;
}
