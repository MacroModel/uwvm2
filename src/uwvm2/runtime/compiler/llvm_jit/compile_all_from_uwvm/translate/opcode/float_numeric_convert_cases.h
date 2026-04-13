#if defined(UWVM_LLVM_JIT_EMIT_OPCODE_CASES)

case wasm1_code::f32_abs:
case wasm1_code::f64_abs:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_abs ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::fabs, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_neg:
case wasm1_code::f64_neg:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_neg ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateFNeg(operand.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_ceil:
case wasm1_code::f64_ceil:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_ceil ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::ceil, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_floor:
case wasm1_code::f64_floor:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_floor ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::floor, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_trunc:
case wasm1_code::f64_trunc:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_trunc ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::trunc, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_nearest:
case wasm1_code::f64_nearest:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_nearest ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::rint, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_sqrt:
case wasm1_code::f64_sqrt:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_sqrt ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_unary(expected_type,
                   expected_type,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       ::llvm::Type* overloaded_types[]{operand.value->getType()};
                       ::llvm::Value* arguments[]{operand.value};
                       return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::sqrt, overloaded_types, arguments);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_add:
case wasm1_code::f64_add:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_add ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return ir_builder.CreateFAdd(left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_sub:
case wasm1_code::f64_sub:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_sub ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return ir_builder.CreateFSub(left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_mul:
case wasm1_code::f64_mul:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_mul ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return ir_builder.CreateFMul(left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_div:
case wasm1_code::f64_div:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_div ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return ir_builder.CreateFDiv(left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_min:
case wasm1_code::f64_min:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_min ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return emit_llvm_float_min(ir_builder, left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_max:
case wasm1_code::f64_max:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_max ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type, expected_type, [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right) { return emit_llvm_float_max(ir_builder, left.value, right.value); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_copysign:
case wasm1_code::f64_copysign:
{
    auto const expected_type{curr_opbase == wasm1_code::f32_copysign ? runtime_operand_stack_value_type::f32 : runtime_operand_stack_value_type::f64};
    ++code_curr;
    if(!emit_binary(expected_type,
                    expected_type,
                    [&](llvm_jit_stack_value_t const& left, llvm_jit_stack_value_t const& right)
                    {
                        ::llvm::Type* overloaded_types[]{left.value->getType()};
                        ::llvm::Value* arguments[]{left.value, right.value};
                        return call_llvm_intrinsic(*llvm_module, ir_builder, ::llvm::Intrinsic::copysign, overloaded_types, arguments);
                    }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_wrap_i64:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateTrunc(operand.value, ::llvm::Type::getInt32Ty(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_trunc_f32_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<float>(
                           *llvm_module, ir_builder, ::llvm::Type::getInt32Ty(llvm_context), true, -2147483904.0f, 2147483648.0f, operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_trunc_f64_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<double>(
                           *llvm_module, ir_builder, ::llvm::Type::getInt32Ty(llvm_context), true, -2147483649.0, 2147483648.0, operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_trunc_f32_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<float>(
                           *llvm_module, ir_builder, ::llvm::Type::getInt32Ty(llvm_context), false, -1.0f, 4294967296.0f, operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_trunc_f64_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<double>(
                           *llvm_module, ir_builder, ::llvm::Type::getInt32Ty(llvm_context), false, -1.0, 4294967296.0, operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_extend_i32_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateSExt(operand.value, ::llvm::Type::getInt64Ty(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_extend_i32_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateZExt(operand.value, ::llvm::Type::getInt64Ty(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_trunc_f32_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<float>(*llvm_module,
                                                                  ir_builder,
                                                                  ::llvm::Type::getInt64Ty(llvm_context),
                                                                  true,
                                                                  -9223373136366403584.0f,
                                                                  9223372036854775808.0f,
                                                                  operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_trunc_f64_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<double>(*llvm_module,
                                                                   ir_builder,
                                                                   ::llvm::Type::getInt64Ty(llvm_context),
                                                                   true,
                                                                   -9223372036854777856.0,
                                                                   9223372036854775808.0,
                                                                   operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_trunc_f32_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<float>(*llvm_module,
                                                                  ir_builder,
                                                                  ::llvm::Type::getInt64Ty(llvm_context),
                                                                  false,
                                                                  -1.0f,
                                                                  18446744073709551616.0f,
                                                                  operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_trunc_f64_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand)
                   {
                       return emit_llvm_trunc_float_to_int<double>(*llvm_module,
                                                                   ir_builder,
                                                                   ::llvm::Type::getInt64Ty(llvm_context),
                                                                   false,
                                                                   -1.0,
                                                                   18446744073709551616.0,
                                                                   operand.value);
                   }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_convert_i32_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateSIToFP(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_convert_i32_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateUIToFP(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_convert_i64_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateSIToFP(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_convert_i64_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateUIToFP(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_demote_f64:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateFPTrunc(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_convert_i32_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateSIToFP(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_convert_i32_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateUIToFP(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_convert_i64_s:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateSIToFP(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_convert_i64_u:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateUIToFP(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_promote_f32:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateFPExt(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::i32_reinterpret_f32:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f32,
                   runtime_operand_stack_value_type::i32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateBitCast(operand.value, ::llvm::Type::getInt32Ty(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::i64_reinterpret_f64:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::f64,
                   runtime_operand_stack_value_type::i64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateBitCast(operand.value, ::llvm::Type::getInt64Ty(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f32_reinterpret_i32:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i32,
                   runtime_operand_stack_value_type::f32,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateBitCast(operand.value, ::llvm::Type::getFloatTy(llvm_context)); }))
    {
        return result;
    }
    break;
}
case wasm1_code::f64_reinterpret_i64:
{
    ++code_curr;
    if(!emit_unary(runtime_operand_stack_value_type::i64,
                   runtime_operand_stack_value_type::f64,
                   [&](llvm_jit_stack_value_t const& operand) { return ir_builder.CreateBitCast(operand.value, ::llvm::Type::getDoubleTy(llvm_context)); }))
    {
        return result;
    }
    break;
}

#else

case wasm1_code::f32_abs:
{
    validate_numeric_unary(u8"f32.abs", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_neg:
{
    validate_numeric_unary(u8"f32.neg", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_ceil:
{
    validate_numeric_unary(u8"f32.ceil", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_floor:
{
    validate_numeric_unary(u8"f32.floor", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_trunc:
{
    validate_numeric_unary(u8"f32.trunc", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_nearest:
{
    validate_numeric_unary(u8"f32.nearest", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_sqrt:
{
    validate_numeric_unary(u8"f32.sqrt", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_add:
{
    validate_numeric_binary(u8"f32.add", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_sub:
{
    validate_numeric_binary(u8"f32.sub", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_mul:
{
    validate_numeric_binary(u8"f32.mul", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_div:
{
    validate_numeric_binary(u8"f32.div", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_min:
{
    validate_numeric_binary(u8"f32.min", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_max:
{
    validate_numeric_binary(u8"f32.max", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_copysign:
{
    validate_numeric_binary(u8"f32.copysign", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f64_abs:
{
    validate_numeric_unary(u8"f64.abs", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_neg:
{
    validate_numeric_unary(u8"f64.neg", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_ceil:
{
    validate_numeric_unary(u8"f64.ceil", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_floor:
{
    validate_numeric_unary(u8"f64.floor", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_trunc:
{
    validate_numeric_unary(u8"f64.trunc", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_nearest:
{
    validate_numeric_unary(u8"f64.nearest", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_sqrt:
{
    validate_numeric_unary(u8"f64.sqrt", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_add:
{
    validate_numeric_binary(u8"f64.add", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_sub:
{
    validate_numeric_binary(u8"f64.sub", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_mul:
{
    validate_numeric_binary(u8"f64.mul", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_div:
{
    validate_numeric_binary(u8"f64.div", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_min:
{
    validate_numeric_binary(u8"f64.min", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_max:
{
    validate_numeric_binary(u8"f64.max", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_copysign:
{
    validate_numeric_binary(u8"f64.copysign", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::i32_wrap_i64:
{
    validate_numeric_unary(u8"i32.wrap_i64", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i32_trunc_f32_s:
{
    validate_numeric_unary(u8"i32.trunc_f32_s", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i32_trunc_f32_u:
{
    validate_numeric_unary(u8"i32.trunc_f32_u", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i32_trunc_f64_s:
{
    validate_numeric_unary(u8"i32.trunc_f64_s", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i32_trunc_f64_u:
{
    validate_numeric_unary(u8"i32.trunc_f64_u", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i64_extend_i32_s:
{
    validate_numeric_unary(u8"i64.extend_i32_s", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::i64_extend_i32_u:
{
    validate_numeric_unary(u8"i64.extend_i32_u", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::i64_trunc_f32_s:
{
    validate_numeric_unary(u8"i64.trunc_f32_s", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::i64_trunc_f32_u:
{
    validate_numeric_unary(u8"i64.trunc_f32_u", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::i64_trunc_f64_s:
{
    validate_numeric_unary(u8"i64.trunc_f64_s", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::i64_trunc_f64_u:
{
    validate_numeric_unary(u8"i64.trunc_f64_u", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::f32_convert_i32_s:
{
    validate_numeric_unary(u8"f32.convert_i32_s", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_convert_i32_u:
{
    validate_numeric_unary(u8"f32.convert_i32_u", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_convert_i64_s:
{
    validate_numeric_unary(u8"f32.convert_i64_s", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_convert_i64_u:
{
    validate_numeric_unary(u8"f32.convert_i64_u", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f32_demote_f64:
{
    validate_numeric_unary(u8"f32.demote_f64", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f64_convert_i32_s:
{
    validate_numeric_unary(u8"f64.convert_i32_s", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_convert_i32_u:
{
    validate_numeric_unary(u8"f64.convert_i32_u", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_convert_i64_s:
{
    validate_numeric_unary(u8"f64.convert_i64_s", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_convert_i64_u:
{
    validate_numeric_unary(u8"f64.convert_i64_u", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::f64_promote_f32:
{
    validate_numeric_unary(u8"f64.promote_f32", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::f64);
    break;
}
case wasm1_code::i32_reinterpret_f32:
{
    validate_numeric_unary(u8"i32.reinterpret_f32", curr_operand_stack_value_type::f32, curr_operand_stack_value_type::i32);
    break;
}
case wasm1_code::i64_reinterpret_f64:
{
    validate_numeric_unary(u8"i64.reinterpret_f64", curr_operand_stack_value_type::f64, curr_operand_stack_value_type::i64);
    break;
}
case wasm1_code::f32_reinterpret_i32:
{
    validate_numeric_unary(u8"f32.reinterpret_i32", curr_operand_stack_value_type::i32, curr_operand_stack_value_type::f32);
    break;
}
case wasm1_code::f64_reinterpret_i64:
{
    validate_numeric_unary(u8"f64.reinterpret_i64", curr_operand_stack_value_type::i64, curr_operand_stack_value_type::f64);
    break;
}

#endif
