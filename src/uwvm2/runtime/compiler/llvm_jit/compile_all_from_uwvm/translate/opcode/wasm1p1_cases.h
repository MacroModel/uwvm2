    // WebAssembly 1.1 scalar opcode validation/emission for the LLVM JIT path.
    // Keep feature gates synchronized with validation/standard/wasm1p1/validator.h.  Value spaces that the current
    // LLVM typed ABI cannot represent (funcref/externref/v128/multi-value block signatures) must disable emission rather
    // than manufacture an ABI-incompatible LLVM value.

case static_cast<wasm1_code>(wasm1p1_code::select_t):
{
    auto const op_begin{code_curr};
    ++code_curr;

    if(wasm1p1_para.disable_reference_types) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::select_t),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::reference_types,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    auto const result_type_count{
        read_leb128.template operator()<validation_module_traits_t::wasm_u32>(code_curr, code_end, op_begin, u8"select.result_types")};

    // select_t result_type_count result_type ...
    // [           safe         ] unsafe (could be the section_end)
    //                            ^^ code_curr

    auto const validate_select_condition{[&](concrete_operand_t cond) constexpr UWVM_THROWS
                                         {
                                             if(cond.from_stack && cond.type != curr_operand_stack_value_type::i32) [[unlikely]]
                                             {
                                                 err.err_curr = op_begin;
                                                 err.err_selectable.select_cond_type_not_i32.cond_type = to_wasm1_diagnostic_value_type(cond.type);
                                                 err.err_code = code_validation_error_code::select_cond_type_not_i32;
                                                 ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
                                             }
                                         }};

    if(result_type_count != 1u) [[unlikely]] { fail_invalid_immediate(op_begin, u8"select.result_types"); }

    auto const result_type_byte{read_u8_immediate(code_curr, code_end, op_begin, u8"select.result_type")};

    // select_t result_type_count result_type ...
    // [                 safe               ] unsafe (could be the section_end)
    //                                        ^^ code_curr

    // Field commits are transactional: a count-LEB decode failure leaves the cursor after the opcode; a decoded-count
    // arity rejection or result-type decode failure leaves it after the count; a type-policy rejection follows the type byte.
    auto const result_type{static_cast<curr_operand_stack_value_type>(result_type_byte)};
    ensure_wasm1p1_value_type_enabled(op_begin, result_type, ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);

    if(capability_failure != nullptr && !is_runtime_wasm_value_type_llvm_scalar(result_type)) [[unlikely]]
    {
        report_capability_failure(llvm_jit_capability_failure_kind::instruction,
                                  u8"typed select result is not an LLVM scalar",
                                  op_begin,
                                  opcode_byte(wasm1p1_code::select_t),
                                  true,
                                  0u,
                                  false,
                                  static_cast<::std::int_least64_t>(result_type_byte),
                                  true);
        return;
    }

    if(!is_polymorphic && concrete_operand_count() < 3uz) [[unlikely]] { report_operand_stack_underflow(op_begin, u8"select", 3uz); }

    auto const cond{try_pop_concrete_operand()};
    validate_select_condition(cond);

    auto const v2{try_pop_concrete_operand()};
    if(v2.from_stack && v2.type != result_type) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.select_type_mismatch.type_v1 = to_wasm1_diagnostic_value_type(result_type);
        err.err_selectable.select_type_mismatch.type_v2 = to_wasm1_diagnostic_value_type(v2.type);
        err.err_code = code_validation_error_code::select_type_mismatch;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    auto const v1{try_pop_concrete_operand()};
    if(v1.from_stack && v1.type != result_type) [[unlikely]]
    {
        err.err_curr = op_begin;
        err.err_selectable.select_type_mismatch.type_v1 = to_wasm1_diagnostic_value_type(result_type);
        err.err_selectable.select_type_mismatch.type_v2 = to_wasm1_diagnostic_value_type(v1.type);
        err.err_code = code_validation_error_code::select_type_mismatch;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    operand_stack_push(result_type);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(result_type == curr_operand_stack_value_type::i32 || result_type == curr_operand_stack_value_type::i64 ||
           result_type == curr_operand_stack_value_type::f32 || result_type == curr_operand_stack_value_type::f64)
        {
            if(!try_emit_runtime_local_func_llvm_jit_select(llvm_jit_emit_state)) [[unlikely]] { disable_inline_llvm_jit_emission(); }
        }
        else
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::i32_extend8_s):
{
    auto const op_begin{code_curr};
    if(wasm1p1_para.disable_sign_extension) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::i32_extend8_s),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::sign_extension,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    validate_numeric_unary(u8"i32.extend8_s", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::i32);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(!try_emit_runtime_local_func_llvm_jit_unary(
               llvm_jit_emit_state,
               runtime_operand_stack_value_type::i32,
               runtime_operand_stack_value_type::i32,
               [](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept
               {
                   auto i8_type{::llvm::Type::getInt8Ty(ir_builder.getContext())};
                   auto i32_type{::llvm::Type::getInt32Ty(ir_builder.getContext())};
                   return ir_builder.CreateSExt(ir_builder.CreateTrunc(operand.value, i8_type), i32_type);
               })) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::i32_extend16_s):
{
    auto const op_begin{code_curr};
    if(wasm1p1_para.disable_sign_extension) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::i32_extend16_s),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::sign_extension,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    validate_numeric_unary(u8"i32.extend16_s", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::i32);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(!try_emit_runtime_local_func_llvm_jit_unary(
               llvm_jit_emit_state,
               runtime_operand_stack_value_type::i32,
               runtime_operand_stack_value_type::i32,
               [](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept
               {
                   auto i16_type{::llvm::Type::getInt16Ty(ir_builder.getContext())};
                   auto i32_type{::llvm::Type::getInt32Ty(ir_builder.getContext())};
                   return ir_builder.CreateSExt(ir_builder.CreateTrunc(operand.value, i16_type), i32_type);
               })) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::i64_extend8_s):
{
    auto const op_begin{code_curr};
    if(wasm1p1_para.disable_sign_extension) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::i64_extend8_s),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::sign_extension,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    validate_numeric_unary(u8"i64.extend8_s", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::i64);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(!try_emit_runtime_local_func_llvm_jit_unary(
               llvm_jit_emit_state,
               runtime_operand_stack_value_type::i64,
               runtime_operand_stack_value_type::i64,
               [](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept
               {
                   auto i8_type{::llvm::Type::getInt8Ty(ir_builder.getContext())};
                   auto i64_type{::llvm::Type::getInt64Ty(ir_builder.getContext())};
                   return ir_builder.CreateSExt(ir_builder.CreateTrunc(operand.value, i8_type), i64_type);
               })) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::i64_extend16_s):
{
    auto const op_begin{code_curr};
    if(wasm1p1_para.disable_sign_extension) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::i64_extend16_s),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::sign_extension,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    validate_numeric_unary(u8"i64.extend16_s", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::i64);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(!try_emit_runtime_local_func_llvm_jit_unary(
               llvm_jit_emit_state,
               runtime_operand_stack_value_type::i64,
               runtime_operand_stack_value_type::i64,
               [](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept
               {
                   auto i16_type{::llvm::Type::getInt16Ty(ir_builder.getContext())};
                   auto i64_type{::llvm::Type::getInt64Ty(ir_builder.getContext())};
                   return ir_builder.CreateSExt(ir_builder.CreateTrunc(operand.value, i16_type), i64_type);
               })) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::i64_extend32_s):
{
    auto const op_begin{code_curr};
    if(wasm1p1_para.disable_sign_extension) [[unlikely]]
    {
        fail_wasm1p1_feature_required(op_begin,
                                      opcode_byte(wasm1p1_code::i64_extend32_s),
                                      ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::sign_extension,
                                      ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
    }

    validate_numeric_unary(u8"i64.extend32_s", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::i64);

    if(emit_llvm_jit_active)
    {
        llvm_jit_instruction_emitted_inline = true;
        if(!try_emit_runtime_local_func_llvm_jit_unary(
               llvm_jit_emit_state,
               runtime_operand_stack_value_type::i64,
               runtime_operand_stack_value_type::i64,
               [](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept
               {
                   auto i32_type{::llvm::Type::getInt32Ty(ir_builder.getContext())};
                   auto i64_type{::llvm::Type::getInt64Ty(ir_builder.getContext())};
                   return ir_builder.CreateSExt(ir_builder.CreateTrunc(operand.value, i32_type), i64_type);
               })) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }

    break;
}

case static_cast<wasm1_code>(wasm1p1_code::numeric_prefix):
{
    auto const op_begin{code_curr};
    ++code_curr;

    auto const subopcode{
        read_leb128.template operator()<validation_module_traits_t::wasm_u32>(code_curr, code_end, op_begin, u8"numeric_prefix")};
    auto const numeric_code{static_cast<wasm1p1_numeric_code>(subopcode)};

    auto const validate_nontrapping_float_to_int{
        [&](::uwvm2::utils::container::u8string_view op_name,
            curr_operand_stack_value_type operand_type,
            curr_operand_stack_value_type result_type) constexpr UWVM_THROWS
        {
            if(wasm1p1_para.disable_nontrapping_float_to_int) [[unlikely]]
            {
                fail_wasm1p1_feature_required(op_begin,
                                              subopcode,
                                              ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::nontrapping_float_to_int,
                                              ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
            }
            validate_numeric_unary_stack_effect(op_begin, op_name, operand_type, result_type);
        }};

    auto const emit_nontrapping_float_to_int{
        [&](runtime_operand_stack_value_type operand_type,
            runtime_operand_stack_value_type result_type,
            bool is_signed,
            auto min_bounds,
            auto max_bounds,
            ::std::uint_least64_t min_result,
            ::std::uint_least64_t max_result) constexpr noexcept
        {
            if(emit_llvm_jit_active)
            {
                llvm_jit_instruction_emitted_inline = true;
                if(!try_emit_runtime_local_func_llvm_jit_unary(
                       llvm_jit_emit_state,
                       operand_type,
                       result_type,
                       [=](::llvm::IRBuilder<>& ir_builder, llvm_jit_stack_value_t const& operand) constexpr noexcept -> ::llvm::Value*
                       {
                           auto dest_type{result_type == runtime_operand_stack_value_type::i32
                                              ? ::llvm::Type::getInt32Ty(ir_builder.getContext())
                                              : ::llvm::Type::getInt64Ty(ir_builder.getContext())};
                           return emit_llvm_trunc_sat_float_to_int(
                               ir_builder, dest_type, is_signed, min_bounds, max_bounds, min_result, max_result, operand.value);
                       })) [[unlikely]]
                {
                    disable_inline_llvm_jit_emission();
                }
            }
        }};

    auto const check_memory_index_zero{
        [&](validation_module_traits_t::wasm_u32 memory_index, ::uwvm2::utils::container::u8string_view op_name) constexpr UWVM_THROWS
        {
            if(all_memory_count == 0u) [[unlikely]]
            {
                err.err_curr = op_begin;
                err.err_selectable.no_memory.op_code_name = op_name;
                err.err_selectable.no_memory.align = 0u;
                err.err_selectable.no_memory.offset = 0u;
                err.err_code = code_validation_error_code::no_memory;
                ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
            }

            if(memory_index != 0u) [[unlikely]]
            {
                err.err_curr = op_begin;
                err.err_selectable.illegal_memory_index.memory_index = memory_index;
                err.err_selectable.illegal_memory_index.all_memory_count = all_memory_count;
                err.err_code = code_validation_error_code::illegal_memory_index;
                ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
            }
        }};

    auto const validate_i32_operands{[&](::uwvm2::utils::container::u8string_view op_name, ::std::size_t operand_count) constexpr UWVM_THROWS
                                     {
                                         if(!is_polymorphic && concrete_operand_count() < operand_count) [[unlikely]]
                                         {
                                             report_operand_stack_underflow(op_begin, op_name, operand_count);
                                         }

                                         for(::std::size_t i{}; i != operand_count; ++i)
                                         {
                                             auto const operand{try_pop_concrete_operand()};
                                             if(operand.from_stack && operand.type != curr_operand_stack_value_type::i32) [[unlikely]]
                                             {
                                                 err.err_curr = op_begin;
                                                 err.err_selectable.numeric_operand_type_mismatch.op_code_name = op_name;
                                                 err.err_selectable.numeric_operand_type_mismatch.expected_type =
                                                     static_cast<wasm_value_type>(curr_operand_stack_value_type::i32);
                                                 err.err_selectable.numeric_operand_type_mismatch.actual_type =
                                                     to_wasm1_diagnostic_value_type(operand.type);
                                                 err.err_code = code_validation_error_code::numeric_operand_type_mismatch;
                                                 ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
                                             }
                                         }
                                     }};

    auto const emit_validated_bulk_memory_instruction{[&]() constexpr noexcept
                                                      {
                                                          if(emit_llvm_jit_active)
                                                          {
                                                              llvm_jit_instruction_emitted_inline = true;
                                                              if(!try_emit_runtime_local_func_llvm_jit_instruction(llvm_jit_emit_state, op_begin, code_curr))
                                                                  [[unlikely]]
                                                              {
                                                                  disable_inline_llvm_jit_emission();
                                                              }
                                                          }
                                                      }};

    switch(numeric_code)
    {
        case wasm1p1_numeric_code::i32_trunc_sat_f32_s:
            validate_nontrapping_float_to_int(u8"i32.trunc_sat_f32_s", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i32);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f32,
                                          runtime_operand_stack_value_type::i32,
                                          true,
                                          -2147483648.0f,
                                          2147483648.0f,
                                          0x80000000ull,
                                          0x7fffffffull);
            break;
        case wasm1p1_numeric_code::i32_trunc_sat_f32_u:
            validate_nontrapping_float_to_int(u8"i32.trunc_sat_f32_u", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i32);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f32,
                                          runtime_operand_stack_value_type::i32,
                                          false,
                                          0.0f,
                                          4294967296.0f,
                                          0u,
                                          0xffffffffull);
            break;
        case wasm1p1_numeric_code::i32_trunc_sat_f64_s:
            validate_nontrapping_float_to_int(u8"i32.trunc_sat_f64_s", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i32);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f64,
                                          runtime_operand_stack_value_type::i32,
                                          true,
                                          -2147483648.0,
                                          2147483648.0,
                                          0x80000000ull,
                                          0x7fffffffull);
            break;
        case wasm1p1_numeric_code::i32_trunc_sat_f64_u:
            validate_nontrapping_float_to_int(u8"i32.trunc_sat_f64_u", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i32);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f64,
                                          runtime_operand_stack_value_type::i32,
                                          false,
                                          0.0,
                                          4294967296.0,
                                          0u,
                                          0xffffffffull);
            break;
        case wasm1p1_numeric_code::i64_trunc_sat_f32_s:
            validate_nontrapping_float_to_int(u8"i64.trunc_sat_f32_s", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i64);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f32,
                                          runtime_operand_stack_value_type::i64,
                                          true,
                                          -9223372036854775808.0f,
                                          9223372036854775808.0f,
                                          0x8000000000000000ull,
                                          0x7fffffffffffffffull);
            break;
        case wasm1p1_numeric_code::i64_trunc_sat_f32_u:
            validate_nontrapping_float_to_int(u8"i64.trunc_sat_f32_u", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i64);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f32,
                                          runtime_operand_stack_value_type::i64,
                                          false,
                                          0.0f,
                                          18446744073709551616.0f,
                                          0u,
                                          0xffffffffffffffffull);
            break;
        case wasm1p1_numeric_code::i64_trunc_sat_f64_s:
            validate_nontrapping_float_to_int(u8"i64.trunc_sat_f64_s", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i64);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f64,
                                          runtime_operand_stack_value_type::i64,
                                          true,
                                          -9223372036854775808.0,
                                          9223372036854775808.0,
                                          0x8000000000000000ull,
                                          0x7fffffffffffffffull);
            break;
        case wasm1p1_numeric_code::i64_trunc_sat_f64_u:
            validate_nontrapping_float_to_int(u8"i64.trunc_sat_f64_u", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i64);
            emit_nontrapping_float_to_int(runtime_operand_stack_value_type::f64,
                                          runtime_operand_stack_value_type::i64,
                                          false,
                                          0.0,
                                          18446744073709551616.0,
                                          0u,
                                          0xffffffffffffffffull);
            break;
        case wasm1p1_numeric_code::memory_copy:
        {
            if(wasm1p1_para.disable_bulk_memory) [[unlikely]]
            {
                fail_wasm1p1_feature_required(op_begin,
                                              subopcode,
                                              ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::bulk_memory,
                                              ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
            }
            auto const dst_memory_index{read_u8_immediate(code_curr, code_end, op_begin, u8"memory.copy.dst")};
            check_memory_index_zero(dst_memory_index, u8"memory.copy");
            auto const src_memory_index{read_u8_immediate(code_curr, code_end, op_begin, u8"memory.copy.src")};
            check_memory_index_zero(src_memory_index, u8"memory.copy");
            validate_i32_operands(u8"memory.copy", 3uz);
            emit_validated_bulk_memory_instruction();
            break;
        }
        case wasm1p1_numeric_code::memory_fill:
        {
            if(wasm1p1_para.disable_bulk_memory) [[unlikely]]
            {
                fail_wasm1p1_feature_required(op_begin,
                                              subopcode,
                                              ::uwvm2::parser::wasm::base::wasm1p1_feature_kind::bulk_memory,
                                              ::uwvm2::parser::wasm::base::wasm1p1_error_subject::instruction);
            }
            auto const memory_index{read_u8_immediate(code_curr, code_end, op_begin, u8"memory.fill")};
            check_memory_index_zero(memory_index, u8"memory.fill");
            validate_i32_operands(u8"memory.fill", 3uz);
            emit_validated_bulk_memory_instruction();
            break;
        }
        [[unlikely]] default:
            // The standard validator already accepted this prefixed opcode. It is valid Wasm 1.1 but has no native
            // LLVM AOT lowering here; invalidate the module rather than misreporting it as malformed or falling back
            // to the interpreter.
            if(capability_failure != nullptr)
            {
                ::uwvm2::utils::container::u8string_view reason{u8"numeric-prefixed instruction has no LLVM lowering"};
                switch(numeric_code)
                {
                    case wasm1p1_numeric_code::memory_init: reason = u8"memory.init has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::data_drop: reason = u8"data.drop has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::table_init: reason = u8"table.init has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::elem_drop: reason = u8"elem.drop has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::table_copy: reason = u8"table.copy has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::table_grow: reason = u8"table.grow has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::table_size: reason = u8"table.size has no LLVM lowering"; break;
                    case wasm1p1_numeric_code::table_fill: reason = u8"table.fill has no LLVM lowering"; break;
                    default: break;
                }
                report_capability_failure(llvm_jit_capability_failure_kind::instruction,
                                          reason,
                                          op_begin,
                                          opcode_byte(wasm1p1_code::numeric_prefix),
                                          true,
                                          subopcode,
                                          true);
                return;
            }
            if(emitted_llvm_jit_ir_storage != nullptr)
            {
                disable_inline_llvm_jit_emission();
                return;
            }
            err.err_curr = op_begin;
            err.err_selectable.u8 = static_cast<::std::uint_least8_t>(subopcode);
            err.err_code = code_validation_error_code::illegal_opbase;
            ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    break;
}
