#include <uwvm2/runtime/compiler/m3_int/straight_line.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sl = ::uwvm2::runtime::compiler::m3_int::straight_line;

namespace
{
    using wasm_byte = sl::wasm_byte;
    using wasm_i32 = sl::wasm_i32;
    using wasm_u32 = sl::wasm_u32;
    using wasm_u64 = sl::wasm_u64;
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
}

int main()
{
    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{0});
        emit_opcode(expr, op_basic::local_set);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::block);
        expr.push_back(wasm_byte{0x40u});
        emit_opcode(expr, op_basic::block);
        expr.push_back(wasm_byte{0x40u});
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::br_table);
        emit_uleb(expr, 1);
        emit_uleb(expr, 1);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{99});
        emit_opcode(expr, op_basic::local_set);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{7});
        emit_opcode(expr, op_basic::local_set);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::i32};
        constexpr sl::local_entry_t locals[]{sl::local_entry_t{.count = 1u, .type = value_type::i32}};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = locals, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 1; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 2; }
        if(sl::get_result<wasm_i32>(exec) != 0) { return 3; }

        args[0] = 1u;
        if(!sl::execute_function(compiled.function, args, exec)) { return 4; }
        if(sl::get_result<wasm_i32>(exec) != 7) { return 5; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::block);
        expr.push_back(static_cast<wasm_byte>(value_type::i32));
        emit_opcode(expr, op_basic::block);
        expr.push_back(static_cast<wasm_byte>(value_type::i32));
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{11});
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::br_table);
        emit_uleb(expr, 1);
        emit_uleb(expr, 1);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{99});
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::drop);
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{22});
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::i32};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = {}, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 6; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{0u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 7; }
        if(sl::get_result<wasm_i32>(exec) != 11) { return 8; }

        args[0] = 1u;
        if(!sl::execute_function(compiled.function, args, exec)) { return 9; }
        if(sl::get_result<wasm_i32>(exec) != 22) { return 10; }
    }

    {
        ::std::vector<wasm_byte> expr{};
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{0});
        emit_opcode(expr, op_basic::local_set);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::block);
        expr.push_back(wasm_byte{0x40u});
        emit_opcode(expr, op_basic::loop);
        expr.push_back(wasm_byte{0x40u});
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::i32_const);
        emit_sleb(expr, wasm_i32{1});
        emit_opcode(expr, op_basic::i32_add);
        emit_opcode(expr, op_basic::local_set);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::i32_lt_s);
        emit_opcode(expr, op_basic::br_table);
        emit_uleb(expr, 1);
        emit_uleb(expr, 1);
        emit_uleb(expr, 0);
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::end);
        emit_opcode(expr, op_basic::local_get);
        emit_uleb(expr, 1);
        emit_opcode(expr, op_basic::end);

        constexpr value_type params[]{value_type::i32};
        constexpr value_type results[]{value_type::i32};
        constexpr sl::local_entry_t locals[]{sl::local_entry_t{.count = 1u, .type = value_type::i32}};
        sl::function_descriptor_t desc{.parameters = params, .results = results, .locals = locals, .expression = expr};
        auto const compiled{sl::compile_function(desc)};
        if(!compiled) { return 11; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{3u};
        if(!sl::execute_function(compiled.function, args, exec)) { return 12; }
        if(sl::get_result<wasm_i32>(exec) != 3) { return 13; }
    }

    return 0;
}
