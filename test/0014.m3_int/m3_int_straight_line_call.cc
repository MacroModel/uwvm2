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
        ::std::vector<wasm_byte> caller_expr{};
        emit_opcode(caller_expr, op_basic::local_get);
        emit_uleb(caller_expr, 0);
        emit_opcode(caller_expr, op_basic::call);
        emit_uleb(caller_expr, 1);
        emit_opcode(caller_expr, op_basic::end);

        ::std::vector<wasm_byte> callee_expr{};
        emit_opcode(callee_expr, op_basic::local_get);
        emit_uleb(callee_expr, 0);
        emit_opcode(callee_expr, op_basic::i32_const);
        emit_sleb(callee_expr, wasm_i32{1});
        emit_opcode(callee_expr, op_basic::i32_add);
        emit_opcode(callee_expr, op_basic::end);

        constexpr value_type caller_params[]{value_type::i32};
        constexpr value_type caller_results[]{value_type::i32};
        constexpr value_type callee_params[]{value_type::i32};
        constexpr value_type callee_results[]{value_type::i32};

        std::array<sl::module_function_t, 2> module_functions{};
        std::array<sl::function_reference_t, 2> functions{
            sl::function_reference_t{.parameters = caller_params, .results = caller_results, .handle = &module_functions[0]},
            sl::function_reference_t{.parameters = callee_params, .results = callee_results, .handle = &module_functions[1]}};
        module_functions[0].descriptor = sl::function_descriptor_t{
            .parameters = caller_params, .results = caller_results, .locals = {}, .expression = caller_expr, .functions = functions};
        module_functions[1].descriptor = sl::function_descriptor_t{
            .parameters = callee_params, .results = callee_results, .locals = {}, .expression = callee_expr, .functions = functions};

        auto const compiled{sl::compile_function(module_functions[0].descriptor)};
        if(!compiled) { return 1; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 1> args{41u};
        if(!sl::execute_function(compiled.function, args, exec, module_functions)) { return 2; }
        if(sl::get_result<wasm_i32>(exec) != 42) { return 3; }
        if(!module_functions[1].compile_attempted) { return 4; }
    }

    {
        ::std::vector<wasm_byte> caller_expr{};
        emit_opcode(caller_expr, op_basic::call);
        emit_uleb(caller_expr, 1);
        emit_opcode(caller_expr, op_basic::end);

        ::std::vector<wasm_byte> callee_expr{};
        emit_opcode(callee_expr, op_basic::i32_const);
        emit_sleb(callee_expr, wasm_i32{77});
        emit_opcode(callee_expr, op_basic::end);

        constexpr value_type caller_results[]{value_type::i32};
        constexpr value_type callee_results[]{value_type::i32};

        std::array<sl::module_function_t, 2> module_functions{};
        std::array<sl::function_reference_t, 2> functions{
            sl::function_reference_t{.parameters = {}, .results = caller_results, .handle = &module_functions[0]},
            sl::function_reference_t{.parameters = {}, .results = callee_results, .handle = &module_functions[1]}};
        module_functions[0].descriptor = sl::function_descriptor_t{
            .parameters = {}, .results = caller_results, .locals = {}, .expression = caller_expr, .functions = functions};
        module_functions[1].descriptor = sl::function_descriptor_t{
            .parameters = {}, .results = callee_results, .locals = {}, .expression = callee_expr, .functions = functions};

        auto const compiled{sl::compile_function(module_functions[0].descriptor)};
        if(!compiled) { return 5; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 0> args{};
        if(!sl::execute_function(compiled.function, args, exec, module_functions)) { return 6; }
        if(sl::get_result<wasm_i32>(exec) != 77) { return 7; }
    }

    {
        ::std::vector<wasm_byte> caller_expr{};
        emit_opcode(caller_expr, op_basic::i32_const);
        emit_sleb(caller_expr, wasm_i32{5});
        emit_opcode(caller_expr, op_basic::call);
        emit_uleb(caller_expr, 1);
        emit_opcode(caller_expr, op_basic::i32_const);
        emit_sleb(caller_expr, wasm_i32{9});
        emit_opcode(caller_expr, op_basic::end);

        ::std::vector<wasm_byte> callee_expr{};
        emit_opcode(callee_expr, op_basic::local_get);
        emit_uleb(callee_expr, 0);
        emit_opcode(callee_expr, op_basic::drop);
        emit_opcode(callee_expr, op_basic::end);

        constexpr value_type caller_results[]{value_type::i32};
        constexpr value_type callee_params[]{value_type::i32};

        std::array<sl::module_function_t, 2> module_functions{};
        std::array<sl::function_reference_t, 2> functions{
            sl::function_reference_t{.parameters = {}, .results = caller_results, .handle = &module_functions[0]},
            sl::function_reference_t{.parameters = callee_params, .results = {}, .handle = &module_functions[1]}};
        module_functions[0].descriptor = sl::function_descriptor_t{
            .parameters = {}, .results = caller_results, .locals = {}, .expression = caller_expr, .functions = functions};
        module_functions[1].descriptor = sl::function_descriptor_t{
            .parameters = callee_params, .results = {}, .locals = {}, .expression = callee_expr, .functions = functions};

        auto const compiled{sl::compile_function(module_functions[0].descriptor)};
        if(!compiled) { return 8; }

        sl::execution_result_t exec{};
        ::std::array<wasm_u64, 0> args{};
        if(!sl::execute_function(compiled.function, args, exec, module_functions)) { return 9; }
        if(sl::get_result<wasm_i32>(exec) != 9) { return 10; }
    }

    return 0;
}
