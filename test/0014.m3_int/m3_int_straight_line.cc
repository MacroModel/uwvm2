#include <uwvm2/runtime/compiler/m3_int/straight_line.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
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

    template <typename Signed>
    inline void emit_sleb(::std::vector<wasm_byte>& out, Signed value)
    {
        static_assert(::std::is_signed_v<Signed>);
        bool more{true};
        while(more)
        {
            auto byte{static_cast<wasm_byte>(static_cast<unsigned long long>(value) & 0x7Fu)};
            auto const sign_bit{(byte & 0x40u) != 0u};
            value = static_cast<Signed>(value >> 7);
            if((value == 0 && !sign_bit) || (value == -1 && sign_bit))
            {
                more = false;
            }
            else
            {
                byte = static_cast<wasm_byte>(byte | 0x80u);
            }
            out.push_back(byte);
        }
    }

    template <typename T>
    inline void emit_fixed(::std::vector<wasm_byte>& out, T value)
    {
        auto const old_size{out.size()};
        out.resize(old_size + sizeof(T));
        ::std::memcpy(out.data() + old_size, ::std::addressof(value), sizeof(T));
    }
}

static_assert(optable::translate::get_SetRegister_fptr<wasm_i32>() != nullptr);

int main()
{
    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_add);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32, value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 1; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{17u, 25u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 2; }
        if(exec.control.signal != optable::m3_control_signal_t::returned) { return 3; }
        if(sl::get_result<wasm_i32>(exec) != 42) { return 4; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 2);
        emit_opcode(expr, op_basic::select);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i64, value_type::i64, value_type::i32};
        constexpr value_type results[]{value_type::i64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 5; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 3> args{11u, 22u, 1u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 6; }
        if(sl::get_result<wasm_i64>(exec) != 11) { return 7; }

        args[2] = 0u;
        if(!sl::execute_function(compiled.function, args, exec)) { return 8; }
        if(sl::get_result<wasm_i64>(exec) != 22) { return 9; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_tee);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_add);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::i32};
        constexpr sl::local_entry_t locals[]{sl::local_entry_t{.count = 1u, .type = value_type::i32}};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = locals, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 10; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{9u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 11; }
        if(sl::get_result<wasm_i32>(exec) != 18) { return 12; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{7});
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{35});
        emit_opcode(expr, op_basic::i32_add);
        emit_opcode(expr, op_basic::end);

        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = {}, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 13; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 0> args{};
        if(!sl::execute_function(compiled.function, args, exec)) { return 14; }
        if(sl::get_result<wasm_i32>(exec) != 42) { return 15; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::f64_add);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::f64, value_type::f64};
        constexpr value_type results[]{value_type::f64};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 16; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 2> args{::std::bit_cast<wasm_u64>(wasm_f64{1.25}), ::std::bit_cast<wasm_u64>(wasm_f64{2.5})};
        if(!sl::execute_function(compiled.function, args, exec)) { return 17; }
        if(sl::get_result<wasm_f64>(exec) != wasm_f64{3.75}) { return 18; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::block);
        emit_fixed(expr, wasm_byte{0x01u});
        emit_opcode(expr, op_basic::end);

        sl::function_descriptor_t desc{.parameters = {}, .results = {}, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(compiled.error != sl::compile_error_t::unsupported_control_flow) { return 19; }
    }

    return 0;
}
