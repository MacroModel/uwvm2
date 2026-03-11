#pragma once

#ifndef UWVM_MODULE
# include <exception>
# include <uwvm2/runtime/compiler/m3_int/optable/define.h>
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::optable
{
    namespace control_details
    {
        [[noreturn]] inline void trap_with_hook(trap_hook_t hook) noexcept
        {
            if(hook != nullptr)
            {
                hook();
            }
            ::std::terminate();
        }

        inline void clear_control_state() noexcept
        {
            control_state = {};
        }

        inline void require_runtime_hook(bool configured) noexcept
        {
            if(!configured) [[unlikely]]
            {
                trap_with_hook(unsupported_func);
            }
        }

        inline void clear_registers(m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
        {
            r0 = m3_int_register_t{};
            fp0 = m3_fp_register_t{};
        }

        inline m3_stack_t advance_stack(m3_stack_t sp, m3_slot_offset_t stack_offset) noexcept
        {
            if(sp == nullptr && stack_offset == m3_slot_offset_t{})
            {
                return nullptr;
            }
            return sp + static_cast<::std::ptrdiff_t>(stack_offset);
        }
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Unsupported(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        control_details::trap_with_hook(unsupported_func);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Unreachable(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        control_details::trap_with_hook(unreachable_func);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Call(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        control_details::require_runtime_hook(call_func != nullptr);
        auto const call_pc{details::read_immediate<m3_code_t>(ip)};
        auto const stack_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        control_details::clear_control_state();
        call_func(call_pc, control_details::advance_stack(sp, stack_offset), r0, fp0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Compile(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        control_details::require_runtime_hook(compile_func != nullptr && call_func != nullptr);
        auto* const function_handle{details::read_immediate<void*>(ip)};
        auto const stack_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const compiled_pc{compile_func(function_handle)};
        control_details::require_runtime_hook(compiled_pc != nullptr);
        control_details::clear_control_state();
        call_func(compiled_pc, control_details::advance_stack(sp, stack_offset), r0, fp0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void CallIndirect(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        control_details::require_runtime_hook(indirect_call_func != nullptr);
        auto const table_index{details::load_slot<wasm_u32>(ip, sp)};
        auto* const module_handle{details::read_immediate<void*>(ip)};
        auto* const type_handle{details::read_immediate<void*>(ip)};
        auto const stack_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        control_details::clear_control_state();
        indirect_call_func(table_index, module_handle, type_handle, control_details::advance_stack(sp, stack_offset), r0, fp0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void CallRawFunction(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        control_details::require_runtime_hook(raw_call_func != nullptr);
        auto* const raw_call_target{details::read_immediate<void*>(ip)};
        auto* const function_handle{details::read_immediate<void*>(ip)};
        auto* const user_data{details::read_immediate<void*>(ip)};
        control_details::clear_control_state();
        raw_call_func(raw_call_target, function_handle, user_data, sp, r0, fp0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Entry(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        auto* const function_handle{details::read_immediate<void*>(ip)};
        control_details::clear_registers(r0, fp0);
        control_details::clear_control_state();
        if(entry_func != nullptr)
        {
            entry_func(function_handle, sp, r0, fp0);
        }
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Loop(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        control_details::require_runtime_hook(loop_func != nullptr);
        auto const loop_id{static_cast<void const*>(ip)};

        for(;;)
        {
            control_details::clear_control_state();
            loop_func(ip, sp, r0, fp0);
            if(control_state.signal == m3_control_signal_t::continue_loop && control_state.payload == loop_id)
            {
                continue;
            }
            return;
        }
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Branch(m3_code_t& ip, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        ip = details::read_immediate<m3_code_t>(ip);
    }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void If(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const condition = [&]() noexcept -> wasm_i32 {
            if constexpr(Shape == unary_shape_t::r)
            {
                return static_cast<wasm_i32>(r0);
            }
            else
            {
                return details::load_slot<wasm_i32>(ip, sp);
            }
        }();

        auto const else_pc{details::read_immediate<m3_code_t>(ip)};
        if(condition == wasm_i32{}) { ip = else_pc; }
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void BranchTable(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto branch_index{details::load_slot<wasm_u32>(ip, sp)};
        auto const num_targets{details::read_immediate<wasm_u32>(ip)};
        if(branch_index > num_targets) { branch_index = num_targets; }

        m3_code_t branch{};
        auto const table_offset{static_cast<::std::ptrdiff_t>(branch_index) * static_cast<::std::ptrdiff_t>(sizeof(m3_code_t))};
        ::std::memcpy(::std::addressof(branch), ip + table_offset, sizeof(branch));
        ip = branch;
    }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void BranchIf(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const condition = [&]() noexcept -> wasm_i32 {
            if constexpr(Shape == unary_shape_t::r)
            {
                return static_cast<wasm_i32>(r0);
            }
            else
            {
                return details::load_slot<wasm_i32>(ip, sp);
            }
        }();

        auto const branch{details::read_immediate<m3_code_t>(ip)};
        if(condition != wasm_i32{}) { ip = branch; }
    }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void BranchIfPrologue(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const condition = [&]() noexcept -> wasm_i32 {
            if constexpr(Shape == unary_shape_t::r)
            {
                return static_cast<wasm_i32>(r0);
            }
            else
            {
                return details::load_slot<wasm_i32>(ip, sp);
            }
        }();

        auto const branch{details::read_immediate<m3_code_t>(ip)};
        if(condition == wasm_i32{}) { ip = branch; }
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Return(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        control_state.signal = m3_control_signal_t::returned;
        control_state.payload = nullptr;
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void End(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        control_state.signal = m3_control_signal_t::returned;
        control_state.payload = nullptr;
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void ContinueLoop(m3_code_t& ip, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        control_state.signal = m3_control_signal_t::continue_loop;
        control_state.payload = details::read_immediate<void const*>(ip);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void ContinueLoopIf(m3_code_t& ip, m3_stack_t, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const loop_id{details::read_immediate<void const*>(ip)};
        if(static_cast<wasm_i32>(r0) != wasm_i32{})
        {
            control_state.signal = m3_control_signal_t::continue_loop;
            control_state.payload = loop_id;
        }
    }

    template <m3_value_type ValueType, select_shape_t Shape>
    inline constexpr bool valid_select_shape_v =
        (::std::same_as<ValueType, wasm_f32> || ::std::same_as<ValueType, wasm_f64>) ||
        (Shape != select_shape_t::rrs && Shape != select_shape_t::rsr);

    template <m3_register_value_type ValueType, select_shape_t Shape>
        requires(valid_select_shape_v<ValueType, Shape>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void m3_select(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        auto ip_copy{ip};
        auto const condition{details::load_select_condition<Shape>(ip_copy, sp, r0)};
        auto const operand2{details::load_select_operand2<ValueType, Shape>(ip_copy, sp, r0, fp0)};
        auto const operand1{details::load_select_operand1<ValueType, Shape>(ip_copy, sp, r0, fp0)};
        ip = ip_copy;
        details::store_register_value<ValueType>(r0, fp0, condition != 0 ? operand1 : operand2);
    }

    template <select_shape_t Shape>
        requires(valid_select_shape_v<wasm_i32, Shape>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Select_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_select<wasm_i32, Shape>(ip, sp, r0, fp0); }

    template <select_shape_t Shape>
        requires(valid_select_shape_v<wasm_i64, Shape>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Select_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_select<wasm_i64, Shape>(ip, sp, r0, fp0); }

    template <select_shape_t Shape>
        requires(valid_select_shape_v<wasm_f32, Shape>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Select_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_select<wasm_f32, Shape>(ip, sp, r0, fp0); }

    template <select_shape_t Shape>
        requires(valid_select_shape_v<wasm_f64, Shape>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Select_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_select<wasm_f64, Shape>(ip, sp, r0, fp0); }

    namespace translate
    {
        inline consteval m3_interpreter_opfunc_t get_Unsupported_fptr() noexcept
        {
            return &Unsupported;
        }

        inline consteval m3_interpreter_opfunc_t get_Unreachable_fptr() noexcept
        {
            return &Unreachable;
        }

        inline consteval m3_interpreter_opfunc_t get_Call_fptr() noexcept
        {
            return &Call;
        }

        inline consteval m3_interpreter_opfunc_t get_Compile_fptr() noexcept
        {
            return &Compile;
        }

        inline consteval m3_interpreter_opfunc_t get_CallIndirect_fptr() noexcept
        {
            return &CallIndirect;
        }

        inline consteval m3_interpreter_opfunc_t get_CallRawFunction_fptr() noexcept
        {
            return &CallRawFunction;
        }

        inline consteval m3_interpreter_opfunc_t get_Entry_fptr() noexcept
        {
            return &Entry;
        }

        inline consteval m3_interpreter_opfunc_t get_Loop_fptr() noexcept
        {
            return &Loop;
        }

        inline consteval m3_interpreter_opfunc_t get_Branch_fptr() noexcept
        {
            return &Branch;
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_If_fptr() noexcept
        {
            return &If<Shape>;
        }

        inline consteval m3_interpreter_opfunc_t get_BranchTable_fptr() noexcept
        {
            return &BranchTable;
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_BranchIf_fptr() noexcept
        {
            return &BranchIf<Shape>;
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_BranchIfPrologue_fptr() noexcept
        {
            return &BranchIfPrologue<Shape>;
        }

        inline consteval m3_interpreter_opfunc_t get_Return_fptr() noexcept
        {
            return &Return;
        }

        inline consteval m3_interpreter_opfunc_t get_End_fptr() noexcept
        {
            return &End;
        }

        inline consteval m3_interpreter_opfunc_t get_ContinueLoop_fptr() noexcept
        {
            return &ContinueLoop;
        }

        inline consteval m3_interpreter_opfunc_t get_ContinueLoopIf_fptr() noexcept
        {
            return &ContinueLoopIf;
        }

        template <m3_register_value_type ValueType, select_shape_t Shape>
            requires(valid_select_shape_v<ValueType, Shape>)
        inline consteval m3_interpreter_opfunc_t get_select_opfunc() noexcept
        {
            return &m3_select<ValueType, Shape>;
        }

        template <select_shape_t Shape>
            requires(valid_select_shape_v<wasm_i32, Shape>)
        inline consteval m3_interpreter_opfunc_t get_Select_i32_fptr() noexcept
        {
            return get_select_opfunc<wasm_i32, Shape>();
        }

        template <select_shape_t Shape>
            requires(valid_select_shape_v<wasm_i64, Shape>)
        inline consteval m3_interpreter_opfunc_t get_Select_i64_fptr() noexcept
        {
            return get_select_opfunc<wasm_i64, Shape>();
        }

        template <select_shape_t Shape>
            requires(valid_select_shape_v<wasm_f32, Shape>)
        inline consteval m3_interpreter_opfunc_t get_Select_f32_fptr() noexcept
        {
            return get_select_opfunc<wasm_f32, Shape>();
        }

        template <select_shape_t Shape>
            requires(valid_select_shape_v<wasm_f64, Shape>)
        inline consteval m3_interpreter_opfunc_t get_Select_f64_fptr() noexcept
        {
            return get_select_opfunc<wasm_f64, Shape>();
        }
    }
}
