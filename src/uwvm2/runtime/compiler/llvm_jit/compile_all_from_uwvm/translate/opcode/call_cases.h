#if defined(UWVM_LLVM_JIT_EMIT_OPCODE_CASES)

case wasm1_code::call:
{
    ++code_curr;

    auto const* runtime_module_ptr{local_func_storage.runtime_module_ptr};
    if(runtime_module_ptr == nullptr) [[unlikely]] { return result; }

    validation_module_traits_t::wasm_u32 func_index{};
    if(!parse_wasm_leb128_immediate(code_curr, code_end, func_index)) [[unlikely]] { return result; }

    auto const* callee_type_ptr{resolve_runtime_callee_function_type(*runtime_module_ptr, func_index)};
    if(callee_type_ptr == nullptr) [[unlikely]] { return result; }

    auto const prepared_call{prepare_wasm_call_operands(*callee_type_ptr)};
    if(!prepared_call.valid) [[unlikely]] { return result; }

    auto const abi_layout{prepared_call.abi_layout};

    auto const import_func_count{runtime_module_ptr->imported_function_vec_storage.size()};
    if(static_cast<::std::size_t>(func_index) < import_func_count)
    {
        auto const callee_resolution{resolve_runtime_direct_callee(*runtime_module_ptr, func_index)};
        if(!callee_resolution.state_valid) [[unlikely]] { return result; }

        if(callee_resolution.direct_callable && callee_resolution.function_type_ptr != nullptr &&
           runtime_wasm_function_types_equal(*callee_resolution.function_type_ptr, *callee_type_ptr))
        {
            auto* call_value{
                emit_direct_wasm_call_value(*runtime_module_ptr, callee_resolution.func_index, *callee_resolution.function_type_ptr, prepared_call.arguments)};
            if(!push_wasm_call_result(prepared_call, call_value)) [[unlikely]] { return result; }
            break;
        }

        auto* llvm_intptr_type{::llvm::Type::getIntNTy(llvm_context, static_cast<unsigned>(sizeof(::std::uintptr_t) * 8u))};
        auto* llvm_i32_type{::llvm::Type::getInt32Ty(llvm_context)};
        auto const raw_bridge_result{emit_runtime_raw_host_bridge_call(
            *callee_type_ptr,
            {prepared_call.arguments.data(), prepared_call.arguments.size()},
            "call.params",
            "call.result.buf",
            [&](llvm_jit_runtime_raw_call_buffers_t const& raw_call_buffers) -> ::llvm::CallInst*
            {
                auto* bridge_function_type{get_llvm_runtime_raw_call_bridge_function_type(llvm_context)};
                return emit_runtime_bridge_call(::uwvm2::runtime::lib::llvm_jit_call_raw_host_api,
                                               bridge_function_type,
                                               {::llvm::ConstantInt::get(llvm_intptr_type, reinterpret_cast<::std::uintptr_t>(runtime_module_ptr)),
                                                ::llvm::ConstantInt::get(llvm_i32_type, func_index),
                                                raw_call_buffers.result_buffer_address,
                                                ::llvm::ConstantInt::get(llvm_intptr_type, abi_layout.result_bytes),
                                                raw_call_buffers.param_buffer_address,
                                                ::llvm::ConstantInt::get(llvm_intptr_type, abi_layout.parameter_bytes)});
            })};
        if(!raw_bridge_result.valid) [[unlikely]] { return result; }

        if(!push_wasm_call_result(prepared_call, raw_bridge_result.result_value)) [[unlikely]] { return result; }
        break;
    }

    auto* call_value{emit_direct_wasm_call_value(*runtime_module_ptr, func_index, *callee_type_ptr, prepared_call.arguments)};
    if(!push_wasm_call_result(prepared_call, call_value)) [[unlikely]] { return result; }
    break;
}
case wasm1_code::call_indirect:
{
    ++code_curr;

    auto const* runtime_module_ptr{local_func_storage.runtime_module_ptr};
    if(runtime_module_ptr == nullptr) [[unlikely]] { return result; }

    validation_module_traits_t::wasm_u32 type_index{};
    validation_module_traits_t::wasm_u32 table_index{};
    if(!parse_wasm_leb128_immediate(code_curr, code_end, type_index) || !parse_wasm_leb128_immediate(code_curr, code_end, table_index))
        [[unlikely]]
    {
        return result;
    }

    auto const all_table_count{
        runtime_module_ptr->imported_table_vec_storage.size() + runtime_module_ptr->local_defined_table_vec_storage.size()};
    if(static_cast<::std::size_t>(table_index) >= all_table_count) [[unlikely]] { return result; }

    auto const* callee_type_ptr{resolve_runtime_type_section_function_type(*runtime_module_ptr, type_index)};
    if(callee_type_ptr == nullptr) [[unlikely]] { return result; }

    auto const abi_layout{get_runtime_wasm_call_abi_layout(*callee_type_ptr)};
    if(!abi_layout.valid || operand_stack.size() < abi_layout.parameter_count + 1uz)
        [[unlikely]]
    {
        return result;
    }

    auto const selector{operand_stack.back()};
    operand_stack.pop_back();
    if(selector.type != runtime_operand_stack_value_type::i32 || selector.value == nullptr) [[unlikely]] { return result; }

    auto const prepared_call{prepare_wasm_call_operands(*callee_type_ptr)};
    if(!prepared_call.valid) [[unlikely]] { return result; }

    auto* llvm_i32_type{::llvm::Type::getInt32Ty(llvm_context)};
    auto* llvm_intptr_type{::llvm::Type::getIntNTy(llvm_context, static_cast<unsigned>(sizeof(::std::uintptr_t) * 8u))};

    auto const expected_type_id{resolve_runtime_canonical_type_id(*runtime_module_ptr, type_index)};
    if(expected_type_id == invalid_runtime_canonical_type_id()) [[unlikely]] { return result; }

    auto const* table_view_begin{runtime_module_ptr->llvm_jit_call_indirect_table_views.data()};
    if(table_view_begin == nullptr) [[unlikely]] { return result; }

    auto const raw_bridge_result{emit_runtime_raw_host_bridge_call(
        *callee_type_ptr,
        {prepared_call.arguments.data(), prepared_call.arguments.size()},
        "call_indirect.params",
        "call_indirect.result.buf",
        [&](llvm_jit_runtime_raw_call_buffers_t const& raw_call_buffers) -> ::llvm::CallInst*
        {
            auto* raw_entry_function_type{get_llvm_runtime_raw_call_target_entry_function_type(llvm_context)};
            auto* raw_target_struct_type{get_llvm_runtime_raw_call_target_struct_type(llvm_context)};
            auto* table_view_struct_type{get_llvm_runtime_call_indirect_table_view_struct_type(llvm_context)};
            if(raw_entry_function_type == nullptr || raw_target_struct_type == nullptr || table_view_struct_type == nullptr) [[unlikely]]
            {
                return nullptr;
            }

            auto* table_view_base_ptr{
                get_llvm_host_pointer_constant(reinterpret_cast<::std::uintptr_t>(table_view_begin), get_llvm_pointer_type(table_view_struct_type))};
            if(table_view_base_ptr == nullptr) [[unlikely]] { return nullptr; }

            auto* selector_index{ir_builder.CreateZExt(selector.value, llvm_intptr_type, "call_indirect.selector.index")};
            auto* table_view_ptr{ir_builder.CreateInBoundsGEP(table_view_struct_type,
                                                              table_view_base_ptr,
                                                              {::llvm::ConstantInt::get(llvm_intptr_type, table_index)},
                                                              "call_indirect.table_view.ptr")};
            auto* table_data_address_ptr{ir_builder.CreateStructGEP(table_view_struct_type, table_view_ptr, 0u, "call_indirect.table.data.addr.ptr")};
            auto* table_size_ptr{ir_builder.CreateStructGEP(table_view_struct_type, table_view_ptr, 1u, "call_indirect.table.size.ptr")};
            auto* table_data_address{ir_builder.CreateLoad(llvm_intptr_type, table_data_address_ptr, "call_indirect.table.data.addr")};
            auto* table_size{ir_builder.CreateLoad(llvm_intptr_type, table_size_ptr, "call_indirect.table.size")};

            emit_llvm_conditional_trap(*llvm_module, ir_builder, ir_builder.CreateICmpUGE(selector_index, table_size));
            emit_llvm_conditional_trap(*llvm_module,
                                       ir_builder,
                                       ir_builder.CreateICmpEQ(table_data_address, ::llvm::ConstantInt::get(llvm_intptr_type, 0u)));

            auto* target_base_ptr{
                ir_builder.CreateIntToPtr(table_data_address, get_llvm_pointer_type(raw_target_struct_type), "call_indirect.target.base.ptr")};
            auto* target_ptr{
                ir_builder.CreateInBoundsGEP(raw_target_struct_type, target_base_ptr, selector_index, "call_indirect.target.ptr")};
            auto* entry_address_ptr{ir_builder.CreateStructGEP(raw_target_struct_type, target_ptr, 0u, "call_indirect.entry.addr.ptr")};
            auto* context_address_ptr{ir_builder.CreateStructGEP(raw_target_struct_type, target_ptr, 1u, "call_indirect.context.addr.ptr")};
            auto* encoded_type_id_ptr{ir_builder.CreateStructGEP(raw_target_struct_type, target_ptr, 2u, "call_indirect.type.id.ptr")};
            auto* entry_address{ir_builder.CreateLoad(llvm_intptr_type, entry_address_ptr, "call_indirect.entry.addr")};
            auto* context_address{ir_builder.CreateLoad(llvm_intptr_type, context_address_ptr, "call_indirect.context.addr")};
            auto* encoded_type_id{ir_builder.CreateLoad(llvm_i32_type, encoded_type_id_ptr, "call_indirect.type.id")};

            emit_llvm_conditional_trap(*llvm_module, ir_builder, ir_builder.CreateICmpEQ(entry_address, ::llvm::ConstantInt::get(llvm_intptr_type, 0u)));
            emit_llvm_conditional_trap(
                *llvm_module, ir_builder, ir_builder.CreateICmpNE(encoded_type_id, ::llvm::ConstantInt::get(llvm_i32_type, expected_type_id)));

            auto* raw_entry_function_ptr{
                ir_builder.CreateIntToPtr(entry_address, get_llvm_pointer_type(raw_entry_function_type), "call_indirect.entry.ptr")};
            return ir_builder.CreateCall(raw_entry_function_type,
                                         raw_entry_function_ptr,
                                         {context_address,
                                          raw_call_buffers.result_buffer_address,
                                          ::llvm::ConstantInt::get(llvm_intptr_type, abi_layout.result_bytes),
                                          raw_call_buffers.param_buffer_address,
                                          ::llvm::ConstantInt::get(llvm_intptr_type, abi_layout.parameter_bytes)});
        })};
    if(!raw_bridge_result.valid) [[unlikely]] { return result; }

    if(prepared_call.has_result)
    {
        if(raw_bridge_result.result_value == nullptr) [[unlikely]] { return result; }
        push_operand(prepared_call.result_type, raw_bridge_result.result_value);
    }
    break;
}

#else

case wasm1_code::call:
{
    // call     func_index ...
    // [ safe ] unsafe (could be the section_end)
    // ^^ code_curr

    auto const op_begin{code_curr};

    // call     func_index ...
    // [ safe ] unsafe (could be the section_end)
    // ^^ op_begin

    ++code_curr;

    // call     func_index ...
    // [ safe ] unsafe (could be the section_end)
    //          ^^ code_curr

    ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32 func_index;  // No initialization necessary

    using char8_t_const_may_alias_ptr UWVM_GNU_MAY_ALIAS = char8_t const*;

    auto const [func_next, func_err]{::fast_io::parse_by_scan(reinterpret_cast<char8_t_const_may_alias_ptr>(code_curr),
                                                              reinterpret_cast<char8_t_const_may_alias_ptr>(code_end),
                                                              ::fast_io::mnp::leb128_get(func_index))};
    if(func_err != ::fast_io::parse_code::ok) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::invalid_function_index_encoding;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(func_err);
    }

    // call func_index ...
    // [      safe   ] unsafe (could be the section_end)
    //      ^^ code_curr

    code_curr = reinterpret_cast<::std::byte const*>(func_next);

    // call func_index ...
    // [      safe   ] unsafe (could be the section_end)
    //                ^^ code_curr

    // Validate function index range (imports + locals)
    auto const all_function_size{import_func_count + local_func_count};
    if(static_cast<::std::size_t>(func_index) >= all_function_size) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.invalid_function_index.function_index = static_cast<::std::size_t>(func_index);
        err.err_selectable.invalid_function_index.all_function_size = all_function_size;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::invalid_function_index;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    // Resolve callee type
    ::uwvm2::uwvm::runtime::storage::wasm_binfmt1_final_function_type_t const* callee_type_ptr{};
    if(static_cast<::std::size_t>(func_index) < import_func_count)
    {
        auto const& imported_funcs{importsec.importdesc.index_unchecked(0u)};
        auto const imported_func_ptr{imported_funcs.index_unchecked(static_cast<::std::size_t>(func_index))};

#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
        if(imported_func_ptr == nullptr) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
        callee_type_ptr = imported_func_ptr->imports.storage.function;
    }
    else
    {
        auto const local_idx{static_cast<::std::size_t>(func_index) - import_func_count};
        callee_type_ptr = typesec.types.cbegin() + funcsec.funcs.index_unchecked(local_idx);
    }

#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
    if(callee_type_ptr == nullptr) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif

    auto const& callee_type{*callee_type_ptr};

    auto const param_count{static_cast<::std::size_t>(callee_type.parameter.end - callee_type.parameter.begin)};
    auto const result_count{static_cast<::std::size_t>(callee_type.result.end - callee_type.result.begin)};

    if(!is_polymorphic && operand_stack.size() < param_count) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.operand_stack_underflow.op_code_name = u8"call";
        err.err_selectable.operand_stack_underflow.stack_size_actual = operand_stack.size();
        err.err_selectable.operand_stack_underflow.stack_size_required = param_count;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::operand_stack_underflow;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    auto const stack_size{operand_stack.size()};

    // Type-check arguments when the stack is non-polymorphic.
    if(!is_polymorphic && param_count != 0uz && stack_size >= param_count)
    {
        for(::std::size_t i{}; i != param_count; ++i)
        {
            auto const expected_type{callee_type.parameter.begin[param_count - 1uz - i]};
            auto const actual_type{operand_stack[stack_size - 1uz - i].type};
            if(actual_type != expected_type) [[unlikely]]
            {
                err.err_curr = op_begin;
                err.err_selectable.br_value_type_mismatch.op_code_name = u8"call";
                err.err_selectable.br_value_type_mismatch.expected_type = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::value_type>(expected_type);
                err.err_selectable.br_value_type_mismatch.actual_type = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::value_type>(actual_type);
                err.err_code = ::uwvm2::validation::error::code_validation_error_code::br_value_type_mismatch;
                ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
            }
        }
    }

    // Consume parameters if present.
    if(param_count != 0uz) { operand_stack_pop_n(param_count); }

    // Push results.
    if(result_count != 0uz)
    {
        for(::std::size_t i{}; i != result_count; ++i) { operand_stack_push(callee_type.result.begin[i]); }
    }

    break;
}
case wasm1_code::call_indirect:
{
    // call_indirect  type_index table_index ...
    // [ safe      ] unsafe (could be the section_end)
    // ^^ code_curr

    auto const op_begin{code_curr};

    // call_indirect  type_index table_index ...
    // [ safe      ] unsafe (could be the section_end)
    // ^^ op_begin

    ++code_curr;

    // call_indirect type_index table_index ...
    // [    safe   ] unsafe (could be the section_end)
    //               ^^ code_curr

    using char8_t_const_may_alias_ptr UWVM_GNU_MAY_ALIAS = char8_t const*;

    ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32 type_index;  // No initialization necessary
    auto const [type_next, type_err]{::fast_io::parse_by_scan(reinterpret_cast<char8_t_const_may_alias_ptr>(code_curr),
                                                              reinterpret_cast<char8_t_const_may_alias_ptr>(code_end),
                                                              ::fast_io::mnp::leb128_get(type_index))};
    if(type_err != ::fast_io::parse_code::ok) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::invalid_type_index;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(type_err);
    }

    // call_indirect type_index table_index ...
    // [          safe        ] unsafe (could be the section_end)
    //               ^^ code_curr

    code_curr = reinterpret_cast<::std::byte const*>(type_next);

    // call_indirect type_index table_index ...
    // [          safe        ] unsafe (could be the section_end)
    //                          ^^ code_curr

    auto const all_type_count_uz{typesec.types.size()};
    if(static_cast<::std::size_t>(type_index) >= all_type_count_uz) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.illegal_type_index.type_index = type_index;
        err.err_selectable.illegal_type_index.all_type_count = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32>(all_type_count_uz);
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::illegal_type_index;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32 table_index;  // No initialization necessary
    auto const [table_next, table_err]{::fast_io::parse_by_scan(reinterpret_cast<char8_t_const_may_alias_ptr>(code_curr),
                                                                reinterpret_cast<char8_t_const_may_alias_ptr>(code_end),
                                                                ::fast_io::mnp::leb128_get(table_index))};
    if(table_err != ::fast_io::parse_code::ok) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::invalid_table_index;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(table_err);
    }

    // call_indirect type_index table_index ...
    // [                safe              ] unsafe (could be the section_end)
    //                          ^^ code_curr

    code_curr = reinterpret_cast<::std::byte const*>(table_next);

    // call_indirect type_index table_index ...
    // [                safe              ] unsafe (could be the section_end)
    //                                      ^^ code_curr

    if(table_index >= all_table_count) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.illegal_table_index.table_index = table_index;
        err.err_selectable.illegal_table_index.all_table_count = all_table_count;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::illegal_table_index;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    // Resolve the function signature by type index.
    auto const& callee_type{typesec.types.index_unchecked(static_cast<::std::size_t>(type_index))};
    auto const param_count{static_cast<::std::size_t>(callee_type.parameter.end - callee_type.parameter.begin)};
    auto const result_count{static_cast<::std::size_t>(callee_type.result.end - callee_type.result.begin)};

    // Stack effect: (args..., i32 func_index) -> (results...)
    constexpr auto max_operand_stack_requirement{::std::numeric_limits<::std::size_t>::max()};
    auto const param_count_plus_table_index_overflows{param_count == max_operand_stack_requirement};
    auto const required_stack_size{param_count_plus_table_index_overflows ? max_operand_stack_requirement : (param_count + 1uz)};

    if(!is_polymorphic && (param_count_plus_table_index_overflows || operand_stack.size() < required_stack_size)) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.operand_stack_underflow.op_code_name = u8"call_indirect";
        err.err_selectable.operand_stack_underflow.stack_size_actual = operand_stack.size();
        err.err_selectable.operand_stack_underflow.stack_size_required = required_stack_size;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::operand_stack_underflow;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    // function index operand (must be i32 if present)
    if(!operand_stack.empty())
    {
        auto const idx{operand_stack_pop_unchecked()};

        if(!is_polymorphic && idx.type != curr_operand_stack_value_type::i32) [[unlikely]]
        {
            err.err_curr = op_begin;
            err.err_selectable.br_cond_type_not_i32.op_code_name = u8"call_indirect";
            err.err_selectable.br_cond_type_not_i32.cond_type = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::value_type>(idx.type);
            err.err_code = ::uwvm2::validation::error::code_validation_error_code::br_cond_type_not_i32;
            ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
        }
    }

    auto const stack_size{operand_stack.size()};
    if(!is_polymorphic && param_count != 0uz && stack_size >= param_count)
    {
        for(::std::size_t i{}; i != param_count; ++i)
        {
            auto const expected_type{callee_type.parameter.begin[param_count - 1uz - i]};
            auto const actual_type{operand_stack[stack_size - 1uz - i].type};
            if(actual_type != expected_type) [[unlikely]]
            {
                err.err_curr = op_begin;
                err.err_selectable.br_value_type_mismatch.op_code_name = u8"call_indirect";
                err.err_selectable.br_value_type_mismatch.expected_type = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::value_type>(expected_type);
                err.err_selectable.br_value_type_mismatch.actual_type = static_cast<::uwvm2::parser::wasm::standard::wasm1::type::value_type>(actual_type);
                err.err_code = ::uwvm2::validation::error::code_validation_error_code::br_value_type_mismatch;
                ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
            }
        }
    }

    if(param_count != 0uz) { operand_stack_pop_n(param_count); }

    if(result_count != 0uz)
    {
        for(::std::size_t i{}; i != result_count; ++i) { operand_stack_push(callee_type.result.begin[i]); }
    }

    break;
}

#endif
