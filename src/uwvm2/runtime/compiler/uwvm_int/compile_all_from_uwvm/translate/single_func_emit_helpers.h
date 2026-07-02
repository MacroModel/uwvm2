bool const runtime_log_on{uwvm2::uwvm::io::enable_runtime_log};
using runtime_uwvm_int_opcode_conbination_level_t = ::uwvm2::uwvm::runtime::runtime_mode::runtime_uwvm_int_opcode_conbination_level_t;
#if defined(UWVM_ENABLE_UWVM_INT_COMBINE_OPS)
[[maybe_unused]] auto const runtime_uwvm_int_opcode_conbination_level{::uwvm2::uwvm::runtime::runtime_mode::global_runtime_uwvm_int_opcode_conbination_level};
#else
[[maybe_unused]] constexpr auto runtime_uwvm_int_opcode_conbination_level{runtime_uwvm_int_opcode_conbination_level_t::disable};
#endif
[[maybe_unused]] auto const runtime_uwvm_int_opcode_conbination_level_at_least{
    [](runtime_uwvm_int_opcode_conbination_level_t curr, runtime_uwvm_int_opcode_conbination_level_t required) constexpr noexcept -> bool
    { return static_cast<unsigned>(curr) >= static_cast<unsigned>(required); }};
bool const runtime_uwvm_int_opcode_conbination_enabled{
    runtime_uwvm_int_opcode_conbination_level_at_least(runtime_uwvm_int_opcode_conbination_level, runtime_uwvm_int_opcode_conbination_level_t::soft)};
[[maybe_unused]] bool const runtime_uwvm_int_opcode_conbination_soft_enabled{runtime_uwvm_int_opcode_conbination_enabled};
[[maybe_unused]] bool const runtime_uwvm_int_opcode_conbination_heavy_enabled{
    runtime_uwvm_int_opcode_conbination_level_at_least(runtime_uwvm_int_opcode_conbination_level, runtime_uwvm_int_opcode_conbination_level_t::heavy)};
[[maybe_unused]] bool const runtime_uwvm_int_opcode_conbination_extra_enabled{
    runtime_uwvm_int_opcode_conbination_level_at_least(runtime_uwvm_int_opcode_conbination_level, runtime_uwvm_int_opcode_conbination_level_t::extra)};
[[maybe_unused]] bool const runtime_uwvm_int_delay_local_enabled{runtime_uwvm_int_opcode_conbination_enabled &&
                                                                 !::uwvm2::uwvm::runtime::runtime_mode::runtime_uwvm_int_disable_delay_local};
[[maybe_unused]] bool const runtime_uwvm_int_instruction_reorder_enabled{
#if defined(UWVM_ENABLE_UWVM_INT_INSTRUCTION_REORDER)
    ::uwvm2::uwvm::runtime::runtime_mode::runtime_uwvm_int_enable_instruction_reorder
#else
    false
#endif
};
[[maybe_unused]] bool const runtime_uwvm_int_loop_unwind_enabled{
#if defined(UWVM_ENABLE_UWVM_INT_LOOP_UNWIND)
    !::uwvm2::uwvm::runtime::runtime_mode::runtime_uwvm_int_disable_loop_unwind
#else
    false
#endif
};
// This include fragment is expanded inside one local-function translation frame. The lambdas below
// deliberately capture parser state, bytecode buffers, label tables, and stack-top state by reference
// so opcode case files can emit compact bytecode without passing a large context object through every
// helper call.
// Runtime compiler logs are intentionally verbose under `-Rclog`; tests use them to audit exact translator choices.
constexpr bool runtime_log_emit_opfuncs{true};
constexpr bool runtime_log_emit_cf{true};
constexpr bool runtime_log_emit_wasm_ops{true};
constexpr bool runtime_log_emit_stacktop{true};
[[maybe_unused]] constexpr bool runtime_log_emit_conbine{true};
constexpr bool runtime_log_emit_func_stats{true};

[[maybe_unused]] auto const anonymous_stacktop_param_count_for_numeric_i32{
    [&]() constexpr noexcept -> ::std::size_t
    {
        if constexpr(CompileOption.is_tail_call && stacktop_i32_spot_enabled) { return stacktop_cache_i32_count; }
        else
        {
            return 0uz;
        }
    }};

[[maybe_unused]] auto const anonymous_stacktop_param_count_for_numeric_i64{
    [&]() constexpr noexcept -> ::std::size_t
    {
        if constexpr(CompileOption.is_tail_call && stacktop_i64_spot_enabled) { return stacktop_cache_i64_count; }
        else
        {
            return 0uz;
        }
    }};

[[maybe_unused]] auto const anonymous_stacktop_param_capacity_for_numeric_i32{
    []<typename... TypeInTuple>(::uwvm2::utils::container::tuple<TypeInTuple...> const*) constexpr noexcept -> ::std::size_t
    {
        if constexpr(stacktop_i32_spot_enabled)
        {
            return ::uwvm2::runtime::compiler::uwvm_int::optable::details::anonymous_stacktop_param_capacity<
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32,
                TypeInTuple...>();
        }
        else
        {
            return 0uz;
        }
    }(static_cast<::std::remove_cvref_t<decltype(interpreter_tuple)> const*>(nullptr))};

[[maybe_unused]] auto const anonymous_stacktop_param_capacity_for_numeric_i64{
    []<typename... TypeInTuple>(::uwvm2::utils::container::tuple<TypeInTuple...> const*) constexpr noexcept -> ::std::size_t
    {
        if constexpr(stacktop_i64_spot_enabled)
        {
            return ::uwvm2::runtime::compiler::uwvm_int::optable::details::anonymous_stacktop_param_capacity<
                ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64,
                TypeInTuple...>();
        }
        else
        {
            return 0uz;
        }
    }(static_cast<::std::remove_cvref_t<decltype(interpreter_tuple)> const*>(nullptr))};

struct runtime_log_stats_t
{
    // These counters are intentionally cheap and coarse-grained: they explain translator decisions
    // such as thunk creation, branch transforms, and stack-top spill/fill pressure without requiring
    // every opfunc pointer to be logged.
    ::std::uint_least64_t wasm_op_count{};
    ::std::uint_least64_t opfunc_main_count{};
    ::std::uint_least64_t opfunc_thunk_count{};
    ::std::uint_least64_t label_placeholder_main_count{};
    ::std::uint_least64_t label_placeholder_thunk_count{};
    ::std::uint_least64_t cf_br_count{};
    ::std::uint_least64_t cf_br_transform_count{};
    ::std::uint_least64_t cf_br_if_count{};
    ::std::uint_least64_t cf_loop_entry_transform_count{};
    ::std::uint_least64_t cf_loop_entry_canonicalize_to_mem_count{};
    ::std::uint_least64_t loop_unwind_candidate_count{};
    ::std::uint_least64_t loop_unwind_applied_count{};
    ::std::uint_least64_t loop_unwind_rejected_count{};
    ::std::uint_least64_t loop_unwind_full_count{};
    ::std::uint_least64_t loop_unwind_partial_count{};
    ::std::uint_least64_t loop_unwind_replayed_body_count{};
    ::std::uint_least64_t loop_unwind_replayed_wasm_bytes{};
    ::std::uint_least64_t loop_unwind_replayed_bytecode_bytes{};
    ::std::uint_least64_t instr_reorder_candidate_count{};
    ::std::uint_least64_t instr_reorder_applied_count{};
    ::std::uint_least64_t instr_reorder_local_preload_count{};
    ::std::uint_least64_t instr_reorder_local_reduce_count{};
    ::std::uint_least64_t instr_reorder_local_reduce_set_count{};
    ::std::uint_least64_t instr_reorder_local_reduce_tee_count{};
    ::std::uint_least64_t instr_reorder_expr_fold_count{};
    ::std::uint_least64_t instr_reorder_expr_local_set_count{};
    ::std::uint_least64_t instr_reorder_expr_local_tee_count{};
    ::std::uint_least64_t instr_reorder_const_binop_local_set_count{};
    ::std::uint_least64_t instr_reorder_const_binop_local_tee_count{};
    ::std::uint_least64_t instr_reorder_stacktop_slot_reject_count{};
    ::std::uint_least64_t instr_reorder_stacktop_slot_used_count{};
    ::std::uint_least64_t instr_reorder_expr_step_count{};
    ::std::uint_least64_t instr_reorder_local_read_count{};
    ::std::uint_least64_t stacktop_spill1_count{};
    ::std::uint_least64_t stacktop_spillN_count{};
    ::std::uint_least64_t stacktop_fill1_count{};
    ::std::uint_least64_t stacktop_fillN_count{};
};

runtime_log_stats_t runtime_log_stats{};

// Best-effort: current Wasm IP for emit logs.
::std::size_t runtime_log_curr_ip{};

auto const runtime_log_bc_name{[](bool in_thunk) constexpr noexcept -> ::uwvm2::utils::container::u8string_view
                               {
                                   if(in_thunk) { return ::uwvm2::utils::container::u8string_view{u8"thunk"}; }
                                   return ::uwvm2::utils::container::u8string_view{u8"main"};
                               }};

// Keep opcode names centralized here so runtime logs remain stable even when opcode case files are
// reorganized or compiled out under feature flags.
auto const runtime_log_op_name{[](::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code op) constexpr noexcept -> ::uwvm2::utils::container::u8string_view
                               {
                                   switch(op)
                                   {
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::unreachable: return u8"unreachable";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::nop: return u8"nop";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::block: return u8"block";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::loop: return u8"loop";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::if_: return u8"if";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::else_: return u8"else";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::end: return u8"end";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::br: return u8"br";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::br_if: return u8"br_if";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::br_table: return u8"br_table";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::return_: return u8"return";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::call: return u8"call";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::call_indirect: return u8"call_indirect";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::drop: return u8"drop";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::select: return u8"select";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::local_get: return u8"local_get";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::local_set: return u8"local_set";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::local_tee: return u8"local_tee";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::global_get: return u8"global_get";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::global_set: return u8"global_set";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_load: return u8"i32_load";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load: return u8"i64_load";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_load: return u8"f32_load";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_load: return u8"f64_load";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_load8_s: return u8"i32_load8_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_load8_u: return u8"i32_load8_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_load16_s: return u8"i32_load16_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_load16_u: return u8"i32_load16_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load8_s: return u8"i64_load8_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load8_u: return u8"i64_load8_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load16_s: return u8"i64_load16_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load16_u: return u8"i64_load16_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load32_s: return u8"i64_load32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_load32_u: return u8"i64_load32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_store: return u8"i32_store";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_store: return u8"i64_store";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_store: return u8"f32_store";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_store: return u8"f64_store";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_store8: return u8"i32_store8";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_store16: return u8"i32_store16";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_store8: return u8"i64_store8";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_store16: return u8"i64_store16";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_store32: return u8"i64_store32";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::memory_size: return u8"memory_size";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::memory_grow: return u8"memory_grow";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_const: return u8"i32_const";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_const: return u8"i64_const";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_const: return u8"f32_const";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_const: return u8"f64_const";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_eqz: return u8"i32_eqz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_eq: return u8"i32_eq";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_ne: return u8"i32_ne";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_lt_s: return u8"i32_lt_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_lt_u: return u8"i32_lt_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_gt_s: return u8"i32_gt_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_gt_u: return u8"i32_gt_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_le_s: return u8"i32_le_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_le_u: return u8"i32_le_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_ge_s: return u8"i32_ge_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_ge_u: return u8"i32_ge_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_eqz: return u8"i64_eqz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_eq: return u8"i64_eq";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_ne: return u8"i64_ne";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_lt_s: return u8"i64_lt_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_lt_u: return u8"i64_lt_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_gt_s: return u8"i64_gt_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_gt_u: return u8"i64_gt_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_le_s: return u8"i64_le_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_le_u: return u8"i64_le_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_ge_s: return u8"i64_ge_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_ge_u: return u8"i64_ge_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_eq: return u8"f32_eq";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_ne: return u8"f32_ne";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_lt: return u8"f32_lt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_gt: return u8"f32_gt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_le: return u8"f32_le";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_ge: return u8"f32_ge";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_eq: return u8"f64_eq";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_ne: return u8"f64_ne";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_lt: return u8"f64_lt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_gt: return u8"f64_gt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_le: return u8"f64_le";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_ge: return u8"f64_ge";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_clz: return u8"i32_clz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_ctz: return u8"i32_ctz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_popcnt: return u8"i32_popcnt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_add: return u8"i32_add";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_sub: return u8"i32_sub";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_mul: return u8"i32_mul";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_div_s: return u8"i32_div_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_div_u: return u8"i32_div_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_rem_s: return u8"i32_rem_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_rem_u: return u8"i32_rem_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_and: return u8"i32_and";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_or: return u8"i32_or";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_xor: return u8"i32_xor";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_shl: return u8"i32_shl";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_shr_s: return u8"i32_shr_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_shr_u: return u8"i32_shr_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_rotl: return u8"i32_rotl";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_rotr: return u8"i32_rotr";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_clz: return u8"i64_clz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_ctz: return u8"i64_ctz";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_popcnt: return u8"i64_popcnt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_add: return u8"i64_add";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_sub: return u8"i64_sub";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_mul: return u8"i64_mul";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_div_s: return u8"i64_div_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_div_u: return u8"i64_div_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_rem_s: return u8"i64_rem_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_rem_u: return u8"i64_rem_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_and: return u8"i64_and";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_or: return u8"i64_or";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_xor: return u8"i64_xor";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_shl: return u8"i64_shl";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_shr_s: return u8"i64_shr_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_shr_u: return u8"i64_shr_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_rotl: return u8"i64_rotl";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_rotr: return u8"i64_rotr";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_abs: return u8"f32_abs";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_neg: return u8"f32_neg";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_ceil: return u8"f32_ceil";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_floor: return u8"f32_floor";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_trunc: return u8"f32_trunc";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_nearest: return u8"f32_nearest";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_sqrt: return u8"f32_sqrt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_add: return u8"f32_add";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_sub: return u8"f32_sub";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_mul: return u8"f32_mul";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_div: return u8"f32_div";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_min: return u8"f32_min";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_max: return u8"f32_max";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_copysign: return u8"f32_copysign";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_abs: return u8"f64_abs";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_neg: return u8"f64_neg";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_ceil: return u8"f64_ceil";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_floor: return u8"f64_floor";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_trunc: return u8"f64_trunc";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_nearest: return u8"f64_nearest";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_sqrt: return u8"f64_sqrt";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_add: return u8"f64_add";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_sub: return u8"f64_sub";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_mul: return u8"f64_mul";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_div: return u8"f64_div";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_min: return u8"f64_min";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_max: return u8"f64_max";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_copysign: return u8"f64_copysign";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_wrap_i64: return u8"i32_wrap_i64";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_trunc_f32_s: return u8"i32_trunc_f32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_trunc_f32_u: return u8"i32_trunc_f32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_trunc_f64_s: return u8"i32_trunc_f64_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_trunc_f64_u: return u8"i32_trunc_f64_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_extend_i32_s: return u8"i64_extend_i32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_extend_i32_u: return u8"i64_extend_i32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_trunc_f32_s: return u8"i64_trunc_f32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_trunc_f32_u: return u8"i64_trunc_f32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_trunc_f64_s: return u8"i64_trunc_f64_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_trunc_f64_u: return u8"i64_trunc_f64_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_convert_i32_s: return u8"f32_convert_i32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_convert_i32_u: return u8"f32_convert_i32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_convert_i64_s: return u8"f32_convert_i64_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_convert_i64_u: return u8"f32_convert_i64_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_demote_f64: return u8"f32_demote_f64";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_convert_i32_s: return u8"f64_convert_i32_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_convert_i32_u: return u8"f64_convert_i32_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_convert_i64_s: return u8"f64_convert_i64_s";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_convert_i64_u: return u8"f64_convert_i64_u";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_promote_f32: return u8"f64_promote_f32";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i32_reinterpret_f32: return u8"i32_reinterpret_f32";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::i64_reinterpret_f64: return u8"i64_reinterpret_f64";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f32_reinterpret_i32: return u8"f32_reinterpret_i32";
                                       case ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code::f64_reinterpret_i64:
                                           return u8"f64_reinterpret_i64";
                                       [[unlikely]] default:
                                           return u8"<unknown>";
                                   }
                               }};

auto const runtime_log_vt_name{[]([[maybe_unused]] curr_operand_stack_value_type vt) constexpr noexcept -> ::uwvm2::utils::container::u8string_view
                               {
                                   switch(vt)
                                   {
                                       case curr_operand_stack_value_type::i32: return u8"i32";
                                       case curr_operand_stack_value_type::i64: return u8"i64";
                                       case curr_operand_stack_value_type::f32: return u8"f32";
                                       case curr_operand_stack_value_type::f64:
                                           return u8"f64";
                                       case curr_operand_stack_value_type::v128:
                                           return u8"v128";
                                       case curr_operand_stack_value_type::funcref:
                                           return u8"funcref";
                                       case curr_operand_stack_value_type::externref:
                                           return u8"externref";
                                       [[unlikely]] default:
                                           return u8"?";
                                   }
                               }};

auto const bytecode_reserve_suggest{[&]() constexpr noexcept
                                    {
                                        // Heuristic: most ops expand from 1 byte opcode to (ptr + immediates). Use a conservative multiplier.
                                        auto const code_size{static_cast<::std::size_t>(code_end - code_begin)};
                                        // Reduce upfront allocations for modules with many small functions.
                                        // The emitter grows geometrically if this estimate is too small.
                                        constexpr ::std::size_t mul{8uz};
                                        if(code_size > (::std::numeric_limits<::std::size_t>::max() / mul))
                                        {
                                            // Overflow-safe fallback: skip the multiplier rather than attempting an impossible reserve().
                                            return code_size;
                                        }
                                        return code_size * mul;
                                    }()};
bytecode.reserve(bytecode_reserve_suggest);

// Bytecode emission is append-heavy. Reserving here rather than at each caller keeps opcode cases
// focused on semantic emission while preserving amortized growth behavior.
auto const ensure_vec_capacity{[&](bytecode_vec_t& dst, ::std::size_t add_bytes) constexpr UWVM_THROWS
                               {
                                   auto const curr{dst.size()};
                                   if(add_bytes > (::std::numeric_limits<::std::size_t>::max() - curr)) [[unlikely]]
                                   {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                       ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                                       ::fast_io::fast_terminate();
                                   }
                                   auto const need{curr + add_bytes};
                                   if(need <= dst.capacity()) { return; }

                                   // Grow geometrically to preserve amortized O(1) push_back.
                                   ::std::size_t new_cap{dst.capacity()};
                                   if(new_cap == 0uz) { new_cap = 1uz; }
                                   while(new_cap < need)
                                   {
                                       if(new_cap > (::std::numeric_limits<::std::size_t>::max() / 2uz))
                                       {
                                           new_cap = need;
                                           break;
                                       }
                                       new_cap *= 2uz;
                                   }
                                   dst.reserve(new_cap);
                               }};

// Raw-byte emission is used for thunk splicing and placeholder patching; it deliberately bypasses
// typed helper overloads because the caller already owns the exact byte representation.
auto const emit_bytes_to{[&](bytecode_vec_t& dst, ::std::byte const* src, ::std::size_t n) constexpr UWVM_THROWS
                         {
                             if(n == 0uz) { return; }
                             ensure_vec_capacity(dst, n);
                             auto out{dst.imp.curr_ptr};
                             dst.imp.curr_ptr += n;
                             ::std::memcpy(out, src, n);
                         }};

// Immediates are copied verbatim into the threaded bytecode stream. Requiring trivially-copyable
// values prevents hidden constructors or lifetime rules from leaking into interpreter decoding.
auto const emit_imm_to{[&]<typename T>(bytecode_vec_t& dst, T const& v) constexpr UWVM_THROWS
                       {
                           static_assert(::std::is_trivially_copyable_v<T>);
                           ensure_vec_capacity(dst, sizeof(T));
                           auto out{dst.imp.curr_ptr};
                           dst.imp.curr_ptr += sizeof(T);
                           ::std::memcpy(out, ::std::addressof(v), sizeof(T));
                       }};

auto const emit_imm{[&]<typename T>(T const& v) constexpr UWVM_THROWS { emit_imm_to(bytecode, v); }};

labels.clear();
ptr_fixups.clear();

// Labels are logical until all bytecode and thunks are emitted. Keeping label ids stable lets branch
// cases emit placeholders early and resolve relative offsets at function finalization.
auto const new_label{[&](bool in_thunk) constexpr UWVM_THROWS -> ::std::size_t
                     {
                         if(labels.size() == labels.capacity()) { labels.reserve(labels.capacity() ? labels.capacity() * 2uz : 64uz); }
                         // Safe: ensured capacity.
                         labels.push_back_unchecked(label_info_t{.offset = SIZE_MAX, .in_thunk = in_thunk});
                         return labels.size() - 1uz;
                     }};

auto const set_label_offset{[&](::std::size_t label_id, ::std::size_t off) constexpr noexcept { labels.index_unchecked(label_id).offset = off; }};

// Thunk bytecode (appended after main `bytecode` so it never shifts main offsets).
thunks.clear();

// Branch immediates cannot be finalized until both main bytecode and thunk bytecode stop moving.
// Emit a fixed-size placeholder now and record the site for the final fixup pass.
auto const emit_ptr_label_placeholder{[&](::std::size_t label_id, bool in_thunk) constexpr UWVM_THROWS
                                      {
                                          rel_offset_t const placeholder{};
                                          ::std::size_t const site{in_thunk ? thunks.size() : bytecode.size()};
                                          if(runtime_log_on) [[unlikely]]
                                          {
                                              if(in_thunk) { ++runtime_log_stats.label_placeholder_thunk_count; }
                                              else
                                              {
                                                  ++runtime_log_stats.label_placeholder_main_count;
                                              }
                                              if(runtime_log_emit_cf)
                                              {
                                                  ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
	                                                       u8"[uwvm-int-translator] fn=",
                                                                       function_index,
                                                                       u8" ip=",
                                                                       runtime_log_curr_ip,
                                                                       u8" event=bytecode.emit.imm | kind=label_placeholder bc=",
                                                                       runtime_log_bc_name(in_thunk),
                                                                       u8" off=",
                                                                       site,
                                                                       u8" label_id=",
                                                                       label_id,
                                                                       u8"\n");
                                              }
                                          }
                                          if(in_thunk) { emit_imm_to(thunks, placeholder); }
                                          else
                                          {
                                              emit_imm(placeholder);
                                          }

                                          if(ptr_fixups.size() == ptr_fixups.capacity())
                                          {
                                              ptr_fixups.reserve(ptr_fixups.capacity() ? ptr_fixups.capacity() * 2uz : 256uz);
                                          }
                                          // Safe: ensured capacity.
                                          ptr_fixups.push_back_unchecked(ptr_fixup_t{.site = site, .label_id = label_id, .in_thunk = in_thunk});
                                      }};

auto const get_branch_target_label_id{[&](block_t const& frame) constexpr noexcept -> ::std::size_t
                                      {
                                          // For Wasm structured control:
                                          // - block/if/else/function: label target is the end.
                                          // - loop: label target is the start.
                                          return frame.type == block_type::loop ? frame.start_label_id : frame.end_label_id;
                                      }};

// Every emitted opfunc pointer must match the active interpreter calling convention. The static
// assertion catches accidental emission of a helper from the wrong tail-call/byref mode at compile time.
	auto const emit_opfunc_to{[&](bytecode_vec_t& dst, auto fptr, ::std::source_location loc = ::std::source_location::current()) constexpr UWVM_THROWS
	                          {
	                              static_assert(::std::same_as<decltype(fptr), details::interpreter_expected_opfunc_ptr_t<CompileOption>>,
	                                            "emitting opfunc pointer type does not match interpreter mode");
	                              if(fptr == nullptr) [[unlikely]]
	                              {
	                                  bool const dst_is_thunk{::std::addressof(dst) == ::std::addressof(thunks)};
	                                  ::std::fprintf(stderr,
                                                     "[uwvm-int-translator] fn=%zu ip=%zu event=bytecode.emit.null_opfunc | bc=%s off=%zu "
                                                     "call_line=%u call_file=%s currpos{i32=%zu,i64=%zu,f32=%zu,f64=%zu,v128=%zu}\n",
                                                     static_cast<::std::size_t>(function_index),
                                                     static_cast<::std::size_t>(runtime_log_curr_ip),
                                                     dst_is_thunk ? "thunk" : "main",
                                                     static_cast<::std::size_t>(dst.size()),
                                                     loc.line(),
                                                     loc.file_name(),
                                                     curr_stacktop.i32_stack_top_curr_pos,
                                                     curr_stacktop.i64_stack_top_curr_pos,
                                                     curr_stacktop.f32_stack_top_curr_pos,
                                                     curr_stacktop.f64_stack_top_curr_pos,
                                                     curr_stacktop.v128_stack_top_curr_pos);
	                                  ::fast_io::fast_terminate();
	                              }
	                              if(runtime_log_on) [[unlikely]]
	                              {
	                                  bool const dst_is_thunk{::std::addressof(dst) == ::std::addressof(thunks)};
                                  ::std::size_t const off{dst.size()};
                                  if(dst_is_thunk) { ++runtime_log_stats.opfunc_thunk_count; }
                                  else
                                  {
                                      ++runtime_log_stats.opfunc_main_count;
                                  }
                                  if(runtime_log_emit_opfuncs)
                                  {
                                      // Print the raw opfunc pointer bits stored into the bytecode stream.
                                      ::std::uintptr_t bits{};
                                      constexpr ::std::size_t copy_n{sizeof(bits) < sizeof(fptr) ? sizeof(bits) : sizeof(fptr)};
                                      ::std::memcpy(::std::addressof(bits), ::std::addressof(fptr), copy_n);
                                      ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                           u8"[uwvm-int-translator] fn=",
                                                           function_index,
                                                           u8" ip=",
                                                           runtime_log_curr_ip,
                                                           u8" event=bytecode.emit.opfunc | bc=",
                                                           runtime_log_bc_name(dst_is_thunk),
                                                           u8" off=",
                                                           off,
                                                           u8" sz=",
                                                           sizeof(fptr),
                                                           u8" fptr_bits=",
                                                           ::fast_io::mnp::hex0x(bits),
                                                           u8" stacktop{mem=",
                                                           stacktop_memory_count,
                                                           u8",cache=",
                                                           stacktop_cache_count,
                                                           u8",i32=",
                                                           stacktop_cache_i32_count,
                                                           u8",i64=",
                                                           stacktop_cache_i64_count,
                                                           u8",f32=",
                                                           stacktop_cache_f32_count,
                                                           u8",f64=",
                                                           stacktop_cache_f64_count,
                                                           u8",v128=",
                                                           stacktop_cache_v128_count,
                                                           u8"} currpos{i32=",
                                                           curr_stacktop.i32_stack_top_curr_pos,
                                                           u8",i64=",
                                                           curr_stacktop.i64_stack_top_curr_pos,
                                                           u8",f32=",
                                                           curr_stacktop.f32_stack_top_curr_pos,
                                                           u8",f64=",
                                                           curr_stacktop.f64_stack_top_curr_pos,
                                                           u8",v128=",
                                                           curr_stacktop.v128_stack_top_curr_pos,
                                                           u8"} call_line=",
                                                           loc.line(),
                                                           u8"\n");
                                  }
                              }

                              // Best-effort: prefetch the opfunc's instruction stream into cache.
                              // This attempts to reduce cold I$ misses when the threaded interpreter dispatches to the opfunc.
                              if UWVM_IF_NOT_CONSTEVAL
                              {
                                  using fptr_t = decltype(fptr);
                                  if constexpr(::std::is_pointer_v<fptr_t>)
                                  {
                                      auto const addr{reinterpret_cast<void const*>(fptr)};
                                      ::uwvm2::utils::intrinsics::universal::prefetch<::uwvm2::utils::intrinsics::universal::pfc_mode::instruction,
                                                                                      ::uwvm2::utils::intrinsics::universal::pfc_level::L2>(addr);
                                  }
                              }

	                              // Note: We intentionally store the raw function pointer bytes into the bytecode stream.
	                              emit_imm_to(dst, fptr);
	                          }};

#if defined(UWVM_ENABLE_UWVM_INT_COMBINE_OPS)
[[maybe_unused]] constexpr ::std::size_t stacktop_i32_call_param_max{3uz};
[[maybe_unused]] constexpr ::std::size_t stacktop_i32_call_indirect_param_max{3uz};
[[maybe_unused]] constexpr ::std::size_t stacktop_fp_call_param_max{4uz};

[[maybe_unused]] auto const dispatch_stacktop_param_count_1_to_3{
    []<typename Fn>(::std::size_t param_count, Fn const& fn) constexpr UWVM_THROWS
    {
        switch(param_count)
        {
            case 1uz: fn.template operator()<1uz>(); break;
            case 2uz: fn.template operator()<2uz>(); break;
            case 3uz: fn.template operator()<3uz>(); break;
            [[unlikely]] default: ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const dispatch_stacktop_param_count_0_to_3{
    []<typename Fn>(::std::size_t param_count, Fn const& fn) constexpr UWVM_THROWS
    {
        switch(param_count)
        {
            case 0uz: fn.template operator()<0uz>(); break;
            case 1uz: fn.template operator()<1uz>(); break;
            case 2uz: fn.template operator()<2uz>(); break;
            case 3uz: fn.template operator()<3uz>(); break;
            [[unlikely]] default: ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const dispatch_stacktop_cache_count_1_to_4{
    []<typename Fn>(::std::size_t cache_count, Fn const& fn) constexpr UWVM_THROWS
    {
        switch(cache_count)
        {
            case 1uz: fn.template operator()<1uz>(); break;
            case 2uz: fn.template operator()<2uz>(); break;
            case 3uz: fn.template operator()<3uz>(); break;
            case 4uz: fn.template operator()<4uz>(); break;
            [[unlikely]] default: ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_stacktop_i32_to{
    [&]<typename RetT>(bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_call_stacktop_i32_fptr_from_tuple<CompileOption, ParamCount, RetT>(curr_stacktop,
                                                                                                                             interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_stacktop_i32_drop_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_call_stacktop_i32_drop_fptr_from_tuple<CompileOption, ParamCount>(curr_stacktop,
                                                                                                                            interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_stacktop_i32_local_set_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_call_stacktop_i32_local_set_fptr_from_tuple<CompileOption, ParamCount>(curr_stacktop, interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_arena_i32_to{
    [&]<typename RetT>(bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_arena_i32_fptr_from_tuple<CompileOption, ParamCount, CacheCount, RetT>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_arena_i32_drop_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(
                                dst,
                                translate::get_uwvmint_call_arena_i32_drop_fptr_from_tuple<CompileOption, ParamCount, CacheCount>(curr_stacktop,
                                                                                                                                 interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_arena_i32_local_set_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_arena_i32_local_set_fptr_from_tuple<CompileOption, ParamCount, CacheCount>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_arena_i32_local_tee_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_1_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_arena_i32_local_tee_fptr_from_tuple<CompileOption, ParamCount, CacheCount>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_stacktop_i32_to{
    [&]<typename RetT>(bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_call_indirect_stacktop_i32_fptr_from_tuple<CompileOption, ParamCount, RetT>(curr_stacktop,
                                                                                                                                        interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_stacktop_i32_drop_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_call_indirect_stacktop_i32_drop_fptr_from_tuple<CompileOption, ParamCount>(curr_stacktop, interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_stacktop_i32_local_set_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_call_indirect_stacktop_i32_local_set_fptr_from_tuple<CompileOption, ParamCount>(curr_stacktop,
                                                                                                                                           interpreter_tuple));
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_arena_i32_to{
    [&]<typename RetT>(bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_indirect_arena_i32_fptr_from_tuple<CompileOption, ParamCount, CacheCount, RetT>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_arena_i32_drop_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_indirect_arena_i32_drop_fptr_from_tuple<CompileOption, ParamCount, CacheCount>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};

[[maybe_unused]] auto const emit_call_indirect_arena_i32_local_set_to{
    [&](bytecode_vec_t& dst, ::std::size_t param_count, ::std::size_t cache_count) constexpr UWVM_THROWS
    {
        if constexpr(CompileOption.is_tail_call)
        {
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            dispatch_stacktop_param_count_0_to_3(
                param_count,
                [&]<::std::size_t ParamCount>() constexpr UWVM_THROWS
                {
                    dispatch_stacktop_cache_count_1_to_4(
                        cache_count,
                        [&]<::std::size_t CacheCount>() constexpr UWVM_THROWS
                        {
                            emit_opfunc_to(dst,
                                           translate::get_uwvmint_call_indirect_arena_i32_local_set_fptr_from_tuple<CompileOption, ParamCount, CacheCount>(
                                               curr_stacktop,
                                               interpreter_tuple));
                        });
                });
        }
        else
        {
            static_cast<void>(dst);
            static_cast<void>(param_count);
            static_cast<void>(cache_count);
            ::fast_io::fast_terminate();
        }
    }};
#endif

// ============================
// Stack-top cache manipulation
// ============================
// Stack-top helpers maintain a compile-time model of values currently resident in register-like
// stack-top rings versus values materialized in operand-stack memory. Most control-flow helpers
// canonicalize at edges so all incoming paths agree on the same runtime layout.

[[maybe_unused]] auto const codegen_stack_push{[&](curr_operand_stack_value_type vt) constexpr UWVM_THROWS
                                               {
                                                   if(codegen_operand_stack.size() == codegen_operand_stack.capacity())
                                                   {
                                                       codegen_operand_stack.reserve(codegen_operand_stack.capacity() ? codegen_operand_stack.capacity() * 2uz
                                                                                                                      : 64uz);
                                                   }
                                                   // Safe: ensured capacity.
                                                   codegen_operand_stack.push_back_unchecked({.type = vt});
                                               }};

[[maybe_unused]] auto const codegen_stack_pop_n{[&](::std::size_t n) constexpr noexcept
                                                {
                                                    while(n-- != 0uz)
                                                    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                                        if(codegen_operand_stack.empty()) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
                                                        if(codegen_operand_stack.empty()) { return; }
                                                        codegen_operand_stack.pop_back_unchecked();
                                                    }
                                                }};

[[maybe_unused]] auto const codegen_stack_set_top{
    [&](curr_operand_stack_value_type vt) constexpr noexcept
    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
        if(codegen_operand_stack.empty()) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
        if(codegen_operand_stack.empty()) { return; }
        if constexpr(stacktop_enabled)
        {
            if(!is_polymorphic && stacktop_cache_count != 0uz)
            {
                auto const old_vt{codegen_operand_stack.back_unchecked().type};
                if(old_vt != vt)
                {
                    auto const cache_count_ref_for_vt{[&]([[maybe_unused]] curr_operand_stack_value_type ty) constexpr noexcept -> ::std::size_t&
                                                      {
                                                          switch(ty)
                                                          {
                                                              case curr_operand_stack_value_type::i32:
                                                              {
                                                                  return stacktop_cache_i32_count;
                                                              }
                                                              case curr_operand_stack_value_type::i64:
                                                              {
                                                                  return stacktop_cache_i64_count;
                                                              }
                                                              case curr_operand_stack_value_type::f32:
                                                              {
                                                                  return stacktop_cache_f32_count;
                                                              }
                                                              case curr_operand_stack_value_type::f64:
                                                              {
                                                                  return stacktop_cache_f64_count;
                                                              }
                                                              case curr_operand_stack_value_type::v128:
                                                              {
                                                                  return stacktop_cache_v128_count;
                                                              }
                                                              [[unlikely]] default:
                                                              {
                                                                  return stacktop_cache_i32_count;
                                                              }
                                                          }
                                                      }};

                    // The top value is always inside the cached segment when `stacktop_cache_count != 0`.
                    --cache_count_ref_for_vt(old_vt);
                    ++cache_count_ref_for_vt(vt);
                }
            }
        }
        codegen_operand_stack.back_unchecked().type = vt;
    }};

constexpr auto stacktop_range_begin_pos{[](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                        {
                                            switch(vt)
                                            {
                                                case curr_operand_stack_value_type::i32:
                                                {
                                                    return stacktop_i32_begin_pos;
                                                }
                                                case curr_operand_stack_value_type::i64:
                                                {
                                                    return stacktop_i64_begin_pos;
                                                }
                                                case curr_operand_stack_value_type::f32:
                                                {
                                                    return CompileOption.f32_stack_top_begin_pos;
                                                }
                                                case curr_operand_stack_value_type::f64:
                                                {
                                                    return CompileOption.f64_stack_top_begin_pos;
                                                }
                                                case curr_operand_stack_value_type::v128:
                                                {
                                                    return CompileOption.v128_stack_top_begin_pos;
                                                }
                                                [[unlikely]] default:
                                                {
                                                    return SIZE_MAX;
                                                }
                                            }
                                        }};

constexpr auto stacktop_range_end_pos{[](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                      {
                                          switch(vt)
                                          {
                                              case curr_operand_stack_value_type::i32:
                                              {
                                                  return stacktop_i32_end_pos;
                                              }
                                              case curr_operand_stack_value_type::i64:
                                              {
                                                  return stacktop_i64_end_pos;
                                              }
                                              case curr_operand_stack_value_type::f32:
                                              {
                                                  return CompileOption.f32_stack_top_end_pos;
                                              }
                                              case curr_operand_stack_value_type::f64:
                                              {
                                                  return CompileOption.f64_stack_top_end_pos;
                                              }
                                              case curr_operand_stack_value_type::v128:
                                              {
                                                  return CompileOption.v128_stack_top_end_pos;
                                              }
                                              [[unlikely]] default:
                                              {
                                                  return SIZE_MAX;
                                              }
                                          }
                                      }};

[[maybe_unused]] constexpr auto stacktop_enabled_for_vt{[](curr_operand_stack_value_type vt) constexpr noexcept -> bool
                                                        {
                                                            switch(vt)
                                                            {
                                                                case curr_operand_stack_value_type::i32:
                                                                {
                                                                    return stacktop_i32_begin_pos != stacktop_i32_end_pos;
                                                                }
                                                                case curr_operand_stack_value_type::i64:
                                                                {
                                                                    return stacktop_i64_begin_pos != stacktop_i64_end_pos;
                                                                }
                                                                case curr_operand_stack_value_type::f32:
                                                                {
                                                                    return CompileOption.f32_stack_top_begin_pos != CompileOption.f32_stack_top_end_pos;
                                                                }
                                                                case curr_operand_stack_value_type::f64:
                                                                {
                                                                    return CompileOption.f64_stack_top_begin_pos != CompileOption.f64_stack_top_end_pos;
                                                                }
                                                                case curr_operand_stack_value_type::v128:
                                                                {
                                                                    return CompileOption.v128_stack_top_begin_pos != CompileOption.v128_stack_top_end_pos;
                                                                }
                                                                [[unlikely]] default:
                                                                {
                                                                    return false;
                                                                }
                                                            }
                                                        }};

// Integer locals use the m3-like stack-spot/local-spot model. Keep legacy integer delay-local
// opfuncs available in the optable, but do not instantiate them from the translator by default.
[[maybe_unused]] constexpr bool delay_local_int_enabled{};

[[maybe_unused]] constexpr auto stacktop_ranges_merged_for{[](curr_operand_stack_value_type l, curr_operand_stack_value_type r) constexpr noexcept -> bool
                                                           {
                                                               auto const is_int_vt{[](curr_operand_stack_value_type vt) constexpr noexcept -> bool
                                                                                    {
                                                                                        return vt == curr_operand_stack_value_type::i32 ||
                                                                                               vt == curr_operand_stack_value_type::i64;
                                                                                    }};
                                                               auto const is_fv_vt{[](curr_operand_stack_value_type vt) constexpr noexcept -> bool
                                                                                   {
                                                                                       return vt == curr_operand_stack_value_type::f32 ||
                                                                                              vt == curr_operand_stack_value_type::f64 ||
                                                                                              vt == curr_operand_stack_value_type::v128;
                                                                                   }};
                                                               if constexpr((stacktop_i32_spot_enabled || stacktop_i64_spot_enabled) &&
                                                                            ::uwvm2::runtime::compiler::uwvm_int::optable::details::
                                                                                uwvm_interpreter_uses_logical_fv_stacktop<CompileOption>())
                                                               {
                                                                   if((is_int_vt(l) && is_fv_vt(r)) || (is_fv_vt(l) && is_int_vt(r))) { return false; }
                                                               }

                                                               auto const begin_pos{[](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                                                                    {
                                                                                        switch(vt)
                                                                                        {
                                                                                            case curr_operand_stack_value_type::i32:
                                                                                                return stacktop_i32_begin_pos;
                                                                                            case curr_operand_stack_value_type::i64:
                                                                                                return stacktop_i64_begin_pos;
                                                                                            case curr_operand_stack_value_type::f32:
                                                                                                return CompileOption.f32_stack_top_begin_pos;
                                                                                            case curr_operand_stack_value_type::f64:
                                                                                                return CompileOption.f64_stack_top_begin_pos;
                                                                                            case curr_operand_stack_value_type::v128:
                                                                                                return CompileOption.v128_stack_top_begin_pos;
                                                                                            [[unlikely]] default:
                                                                                                return SIZE_MAX;
                                                                                        }
                                                                                    }};

                                                               auto const end_pos{[](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                                                                  {
                                                                                      switch(vt)
                                                                                      {
                                                                                          case curr_operand_stack_value_type::i32:
                                                                                              return stacktop_i32_end_pos;
                                                                                          case curr_operand_stack_value_type::i64:
                                                                                              return stacktop_i64_end_pos;
                                                                                          case curr_operand_stack_value_type::f32:
                                                                                              return CompileOption.f32_stack_top_end_pos;
                                                                                          case curr_operand_stack_value_type::f64:
                                                                                              return CompileOption.f64_stack_top_end_pos;
                                                                                          case curr_operand_stack_value_type::v128:
                                                                                              return CompileOption.v128_stack_top_end_pos;
                                                                                          [[unlikely]] default:
                                                                                              return SIZE_MAX;
                                                                                      }
                                                                                  }};

                                                               return begin_pos(l) == begin_pos(r) && end_pos(l) == end_pos(r);
                                                           }};

auto const stacktop_cache_count_ref_for_vt{[&](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t&
                                           {
                                               switch(vt)
                                               {
                                                   case curr_operand_stack_value_type::i32:
                                                   {
                                                       return stacktop_cache_i32_count;
                                                   }
                                                   case curr_operand_stack_value_type::i64:
                                                   {
                                                       return stacktop_cache_i64_count;
                                                   }
                                                   case curr_operand_stack_value_type::f32:
                                                   {
                                                       return stacktop_cache_f32_count;
                                                   }
                                                   case curr_operand_stack_value_type::f64:
                                                   {
                                                       return stacktop_cache_f64_count;
                                                   }
                                                   case curr_operand_stack_value_type::v128:
                                                   {
                                                       return stacktop_cache_v128_count;
                                                   }
                                                   [[unlikely]] default:
                                                   {
                                                       return stacktop_cache_i32_count;
                                                   }
                                               }
                                           }};

auto const stacktop_cache_count_for_range{
    [&](::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
    {
        ::std::size_t sum{};
        if(stacktop_i32_enabled && begin_pos == stacktop_i32_begin_pos && end_pos == stacktop_i32_end_pos)
        {
            sum += stacktop_cache_i32_count;
        }
        if(stacktop_i64_enabled && begin_pos == stacktop_i64_begin_pos && end_pos == stacktop_i64_end_pos)
        {
            sum += stacktop_cache_i64_count;
        }
        if(stacktop_f32_enabled && begin_pos == CompileOption.f32_stack_top_begin_pos && end_pos == CompileOption.f32_stack_top_end_pos)
        {
            sum += stacktop_cache_f32_count;
        }
        if(stacktop_f64_enabled && begin_pos == CompileOption.f64_stack_top_begin_pos && end_pos == CompileOption.f64_stack_top_end_pos)
        {
            sum += stacktop_cache_f64_count;
        }
        if(stacktop_v128_enabled && begin_pos == CompileOption.v128_stack_top_begin_pos && end_pos == CompileOption.v128_stack_top_end_pos)
        {
            sum += stacktop_cache_v128_count;
        }
        return sum;
    }};

[[maybe_unused]] auto const stacktop_window_size_for_vt{[&](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                                      {
                                                          if constexpr(!stacktop_enabled) { return SIZE_MAX; }
                                                          else
                                                          {
                                                              if(!stacktop_enabled_for_vt(vt)) { return 0uz; }
                                                              auto const begin_pos{stacktop_range_begin_pos(vt)};
                                                              auto const end_pos{stacktop_range_end_pos(vt)};
                                                              return end_pos > begin_pos ? (end_pos - begin_pos) : 0uz;
                                                          }
                                                      }};

[[maybe_unused]] auto const stacktop_free_slot_count_for_vt{[&](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
                                                            {
                                                                if constexpr(!stacktop_enabled) { return SIZE_MAX; }
                                                                else
                                                                {
                                                                    auto const window_size{stacktop_window_size_for_vt(vt)};
                                                                    if(window_size == 0uz) { return 0uz; }
                                                                    auto const begin_pos{stacktop_range_begin_pos(vt)};
                                                                    auto const end_pos{stacktop_range_end_pos(vt)};
                                                                    auto const used{stacktop_cache_count_for_range(begin_pos, end_pos)};
                                                                    return used < window_size ? (window_size - used) : 0uz;
                                                                }
                                                            }};

[[maybe_unused]] auto const stacktop_has_push_slots_without_spill{[&](curr_operand_stack_value_type vt, ::std::size_t need) constexpr noexcept -> bool
                                                                  {
                                                                      if constexpr(!stacktop_enabled) { return true; }
                                                                      else
                                                                      {
                                                                          return stacktop_free_slot_count_for_vt(vt) >= need;
                                                                      }
                                                                  }};

// Reconcile per-type cache counters from the codegen type stack.
// This protects against missing/incorrect per-type updates on ops that only retype the top value
// (e.g., reinterpret/extend) while keeping stack depth unchanged.
[[maybe_unused]] auto const stacktop_rebuild_cache_type_counts_from_codegen{[&]() constexpr noexcept
                                                                            {
                                                                                if constexpr(!stacktop_enabled) { return; }
                                                                                if(is_polymorphic) { return; }

                                                                                stacktop_cache_i32_count = 0uz;
	                                                                                stacktop_cache_i64_count = 0uz;
	                                                                                stacktop_cache_f32_count = 0uz;
	                                                                                stacktop_cache_f64_count = 0uz;
	                                                                                stacktop_cache_v128_count = 0uz;

                                                                                auto const total{codegen_operand_stack.size()};
                                                                                auto cache_n{stacktop_cache_count};
                                                                                if(cache_n > total) { cache_n = total; }
                                                                                auto const start{total - cache_n};
                                                                                for(::std::size_t i{}; i != cache_n; ++i)
                                                                                {
                                                                                    auto const vt{codegen_operand_stack.index_unchecked(start + i).type};
                                                                                    ++stacktop_cache_count_ref_for_vt(vt);
                                                                                }
                                                                            }};

auto const stacktop_currpos_for_range{
    [&](::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
    {
        if(stacktop_i32_enabled && begin_pos == stacktop_i32_begin_pos && end_pos == stacktop_i32_end_pos)
        {
            return curr_stacktop.i32_stack_top_curr_pos;
        }
        if(stacktop_i64_enabled && begin_pos == stacktop_i64_begin_pos && end_pos == stacktop_i64_end_pos)
        {
            return curr_stacktop.i64_stack_top_curr_pos;
        }
        if(stacktop_f32_enabled && begin_pos == CompileOption.f32_stack_top_begin_pos && end_pos == CompileOption.f32_stack_top_end_pos)
        {
            return curr_stacktop.f32_stack_top_curr_pos;
        }
        if(stacktop_f64_enabled && begin_pos == CompileOption.f64_stack_top_begin_pos && end_pos == CompileOption.f64_stack_top_end_pos)
        {
            return curr_stacktop.f64_stack_top_curr_pos;
        }
        if(stacktop_v128_enabled && begin_pos == CompileOption.v128_stack_top_begin_pos && end_pos == CompileOption.v128_stack_top_end_pos)
        {
            return curr_stacktop.v128_stack_top_curr_pos;
        }
        return SIZE_MAX;
    }};

auto const stacktop_set_currpos_for_range{
    [&](::std::size_t begin_pos, ::std::size_t end_pos, ::std::size_t pos) constexpr noexcept
    {
        if(stacktop_i32_enabled && begin_pos == stacktop_i32_begin_pos && end_pos == stacktop_i32_end_pos)
        {
            curr_stacktop.i32_stack_top_curr_pos = pos;
        }
        if(stacktop_i64_enabled && begin_pos == stacktop_i64_begin_pos && end_pos == stacktop_i64_end_pos)
        {
            curr_stacktop.i64_stack_top_curr_pos = pos;
        }
        if(stacktop_f32_enabled && begin_pos == CompileOption.f32_stack_top_begin_pos && end_pos == CompileOption.f32_stack_top_end_pos)
        {
            curr_stacktop.f32_stack_top_curr_pos = pos;
        }
        if(stacktop_f64_enabled && begin_pos == CompileOption.f64_stack_top_begin_pos && end_pos == CompileOption.f64_stack_top_end_pos)
        {
            curr_stacktop.f64_stack_top_curr_pos = pos;
        }
        if(stacktop_v128_enabled && begin_pos == CompileOption.v128_stack_top_begin_pos && end_pos == CompileOption.v128_stack_top_end_pos)
        {
            curr_stacktop.v128_stack_top_curr_pos = pos;
        }
    }};

auto const stacktop_window_prev_pos{[&](::std::size_t pos, ::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
                              { return pos == begin_pos ? (end_pos - 1uz) : (pos - 1uz); }};

auto const stacktop_window_next_pos{[&](::std::size_t pos, ::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
                              { return (pos + 1uz == end_pos) ? begin_pos : (pos + 1uz); }};

auto const stacktop_window_advance_next{
    [&](::std::size_t pos, ::std::size_t n, ::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
    {
        for(::std::size_t i{}; i != n; ++i) { pos = stacktop_window_next_pos(pos, begin_pos, end_pos); }
        return pos;
    }};

enum class local_spot_slot_class : unsigned char
{
    none,
    int_,
    fv
};

[[maybe_unused]] auto const local_spot_slot_class_for_vt{
    [](curr_operand_stack_value_type vt) constexpr noexcept -> local_spot_slot_class
    {
        switch(vt)
        {
            case curr_operand_stack_value_type::i32: [[fallthrough]];
            case curr_operand_stack_value_type::i64:
            {
                if constexpr(stacktop_int_spot_enabled) { return local_spot_slot_class::int_; }
                else
                {
                    return local_spot_slot_class::none;
                }
            }
            case curr_operand_stack_value_type::f32: [[fallthrough]];
            case curr_operand_stack_value_type::f64: [[fallthrough]];
            case curr_operand_stack_value_type::v128:
            {
                if constexpr(::uwvm2::runtime::compiler::uwvm_int::optable::details::uwvm_interpreter_uses_logical_fv_stacktop<CompileOption>())
                {
                    return local_spot_slot_class::fv;
                }
                else
                {
                    return local_spot_slot_class::none;
                }
            }
            [[unlikely]] default:
            {
                return local_spot_slot_class::none;
            }
        }
    }};

[[maybe_unused]] auto const local_spot_enabled_for_vt{
    [&](curr_operand_stack_value_type vt) constexpr noexcept -> bool
    {
        if constexpr(!stacktop_enabled || !CompileOption.is_tail_call) { return false; }
        else
        {
            return stacktop_enabled_for_vt(vt) && local_spot_slot_class_for_vt(vt) != local_spot_slot_class::none;
        }
    }};

[[maybe_unused]] auto const local_spot_same_slot{
    [&](local_spot_cache_entry_t const& e, curr_operand_stack_value_type vt, ::std::size_t pos) constexpr noexcept -> bool
    {
        return e.valid && e.pos == pos && local_spot_slot_class_for_vt(e.type) == local_spot_slot_class_for_vt(vt);
    }};

[[maybe_unused]] auto const local_spot_flush_dirty_entry_to{
    [&](bytecode_vec_t& dst, local_spot_cache_entry_t& e) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled || !CompileOption.is_tail_call) { return; }
        else
        {
            if(!e.valid || !e.dirty || e.pos == SIZE_MAX) { return; }

            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
            using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
            using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
            using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

            if(runtime_log_on) [[unlikely]]
            {
                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                     u8"[uwvm-int-translator] fn=",
                                     function_index,
                                     u8" ip=",
                                     runtime_log_curr_ip,
                                     u8" event=local_spot.flush | vt=",
                                     runtime_log_vt_name(e.type),
                                     u8" off=",
                                     e.off,
                                     u8" pos=",
                                     e.pos,
                                     u8"\n");
            }

            switch(e.type)
            {
                case curr_operand_stack_value_type::i32:
                {
                    if constexpr(stacktop_i32_spot_enabled)
                    {
                        emit_opfunc_to(dst,
                                       translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<CompileOption, wasm_i32>(e.pos,
                                                                                                                               curr_stacktop,
                                                                                                                               interpreter_tuple));
                        emit_imm_to(dst, e.off);
                        e.dirty = false;
                    }
                    break;
                }
                case curr_operand_stack_value_type::i64:
                {
                    if constexpr(stacktop_i64_spot_enabled)
                    {
                        emit_opfunc_to(dst,
                                       translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<CompileOption, wasm_i64>(e.pos,
                                                                                                                               curr_stacktop,
                                                                                                                               interpreter_tuple));
                        emit_imm_to(dst, e.off);
                        e.dirty = false;
                    }
                    break;
                }
                case curr_operand_stack_value_type::f32:
                {
                    if constexpr(stacktop_f32_enabled)
                    {
                        emit_opfunc_to(dst,
                                       translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<CompileOption, wasm_f32>(e.pos,
                                                                                                                               curr_stacktop,
                                                                                                                               interpreter_tuple));
                        emit_imm_to(dst, e.off);
                        e.dirty = false;
                    }
                    break;
                }
                case curr_operand_stack_value_type::f64:
                {
                    if constexpr(stacktop_f64_enabled)
                    {
                        emit_opfunc_to(dst,
                                       translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<CompileOption, wasm_f64>(e.pos,
                                                                                                                               curr_stacktop,
                                                                                                                               interpreter_tuple));
                        emit_imm_to(dst, e.off);
                        e.dirty = false;
                    }
                    break;
                }
                case curr_operand_stack_value_type::v128:
                {
                    if constexpr(stacktop_v128_enabled)
                    {
                        emit_opfunc_to(dst,
                                       translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(e.pos,
                                                                                                                                 curr_stacktop,
                                                                                                                                 interpreter_tuple));
                        emit_imm_to(dst, e.off);
                        e.dirty = false;
                    }
                    break;
                }
                [[unlikely]] default:
                {
                    break;
                }
            }
        }
    }};

[[maybe_unused]] auto const local_spot_flush_dirty_all_to{
    [&](bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled || !CompileOption.is_tail_call) { return; }
        else
        {
            for(auto& e: local_spot_cache) { local_spot_flush_dirty_entry_to(dst, e); }
        }
    }};

[[maybe_unused]] auto const local_spot_invalidate_all{[&]() constexpr noexcept
                                                      {
                                                          if constexpr(!stacktop_enabled) { return; }
                                                          else
                                                          {
                                                              for(auto& e: local_spot_cache)
                                                              {
                                                                  e.valid = false;
                                                                  e.dirty = false;
                                                              }
                                                          }
                                                      }};

[[maybe_unused]] auto const local_spot_flush_dirty_all_and_invalidate_to{
    [&](bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        local_spot_flush_dirty_all_to(dst);
        local_spot_invalidate_all();
    }};

[[maybe_unused]] auto const local_spot_invalidate_slot{
    [&](curr_operand_stack_value_type vt, ::std::size_t pos) constexpr noexcept
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(!local_spot_enabled_for_vt(vt)) { return; }
            for(auto& e: local_spot_cache)
            {
                if(local_spot_same_slot(e, vt, pos))
                {
                    e.valid = false;
                    e.dirty = false;
                }
            }
        }
    }};

[[maybe_unused]] auto const local_spot_flush_dirty_slot_and_invalidate_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, ::std::size_t pos) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(!local_spot_enabled_for_vt(vt)) { return; }
            for(auto& e: local_spot_cache)
            {
                if(local_spot_same_slot(e, vt, pos)) { local_spot_flush_dirty_entry_to(dst, e); }
            }
            local_spot_invalidate_slot(vt, pos);
        }
    }};

[[maybe_unused]] auto const local_spot_find{
    [&](local_offset_t off, curr_operand_stack_value_type vt) constexpr noexcept -> local_spot_cache_entry_t*
    {
        if constexpr(!stacktop_enabled) { return nullptr; }
        else
        {
            if(!local_spot_enabled_for_vt(vt)) { return nullptr; }
            for(auto& e: local_spot_cache)
            {
                if(e.valid && e.off == off && e.type == vt) { return ::std::addressof(e); }
            }
            return nullptr;
        }
    }};

[[maybe_unused]] auto const local_spot_bind{
    [&](local_offset_t off, curr_operand_stack_value_type vt, ::std::size_t pos, bool dirty = false) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(!local_spot_enabled_for_vt(vt) || pos == SIZE_MAX) { return; }

            for(auto& e: local_spot_cache)
            {
                if(e.valid && (e.off == off || local_spot_same_slot(e, vt, pos))) { e.valid = false; }
            }

            auto const put_entry{[&](local_spot_cache_entry_t& e) constexpr noexcept
                                 {
                                     e.valid = true;
                                     e.dirty = dirty;
                                     e.type = vt;
                                     e.off = off;
                                     e.pos = pos;
                                 }};

            for(auto& e: local_spot_cache)
            {
                if(!e.valid)
                {
                    put_entry(e);
                    return;
                }
            }

            if(local_spot_cache.size() == local_spot_cache.capacity())
            {
                local_spot_cache.reserve(local_spot_cache.capacity() ? local_spot_cache.capacity() * 2uz : 16uz);
            }
            local_spot_cache.push_back_unchecked({});
            put_entry(local_spot_cache.back_unchecked());
        }
    }};

[[maybe_unused]] auto const local_spot_bind_to{
    [&](bytecode_vec_t& dst, local_offset_t off, curr_operand_stack_value_type vt, ::std::size_t pos, bool dirty = false) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(!local_spot_enabled_for_vt(vt) || pos == SIZE_MAX) { return; }

            for(auto& e: local_spot_cache)
            {
                if(e.valid && e.off != off && local_spot_same_slot(e, vt, pos)) { local_spot_flush_dirty_entry_to(dst, e); }
            }

            local_spot_bind(off, vt, pos, dirty);
        }
    }};

[[maybe_unused]] auto const local_spot_current_pos_for_vt{
    [&](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
    {
        if constexpr(!stacktop_enabled) { return SIZE_MAX; }
        else
        {
            if(!local_spot_enabled_for_vt(vt)) { return SIZE_MAX; }
            auto const begin_pos{stacktop_range_begin_pos(vt)};
            auto const end_pos{stacktop_range_end_pos(vt)};
            return stacktop_currpos_for_range(begin_pos, end_pos);
        }
    }};

[[maybe_unused]] auto const local_spot_push_target_pos_for_vt{
    [&](curr_operand_stack_value_type vt) constexpr noexcept -> ::std::size_t
    {
        auto const currpos{local_spot_current_pos_for_vt(vt)};
        if(currpos == SIZE_MAX) { return SIZE_MAX; }
        auto const begin_pos{stacktop_range_begin_pos(vt)};
        auto const end_pos{stacktop_range_end_pos(vt)};
        return stacktop_window_prev_pos(currpos, begin_pos, end_pos);
    }};

[[maybe_unused]] auto const local_spot_flush_dirty_stacktop_slots_and_invalidate_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, ::std::size_t n) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(n == 0uz || !local_spot_enabled_for_vt(vt)) { return; }
            auto const begin_pos{stacktop_range_begin_pos(vt)};
            auto const end_pos{stacktop_range_end_pos(vt)};
            auto pos{stacktop_currpos_for_range(begin_pos, end_pos)};
            auto const cached{stacktop_cache_count_for_range(begin_pos, end_pos)};
            if(n > cached) { n = cached; }
            for(::std::size_t i{}; i != n; ++i)
            {
                local_spot_flush_dirty_slot_and_invalidate_to(dst, vt, pos);
                pos = stacktop_window_next_pos(pos, begin_pos, end_pos);
            }
        }
    }};

#if 0
            [[maybe_unused]] auto const stacktop_window_advance_prev{
                [&](::std::size_t pos, ::std::size_t n, ::std::size_t begin_pos, ::std::size_t end_pos) constexpr noexcept -> ::std::size_t
                {
                    for(::std::size_t i{}; i != n; ++i) { pos = stacktop_window_prev_pos(pos, begin_pos, end_pos); }
                    return pos;
                }};
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
auto const stacktop_runtime_depth{[&]() constexpr noexcept -> ::std::size_t { return stacktop_memory_count + stacktop_cache_count; }};
#endif

auto const stacktop_assert_invariants{
    [&]() constexpr noexcept
    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
        if constexpr(stacktop_enabled)
        {
            // Stack-top cache model is tied to the emitted bytecode stream; validate against the
            // codegen type stack, not the validator operand stack (which can be ahead during conbine).
            if(stacktop_runtime_depth() != codegen_operand_stack.size()) [[unlikely]]
            {
                using op_underlying_t = ::std::underlying_type_t<::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code>;
                auto const op_u{static_cast<::std::uint_least32_t>(static_cast<op_underlying_t>(stacktop_dbg_last_op))};

                ::fast_io::io::perr(::fast_io::u8err(),
                                    u8"[uwvm-int-translator] stacktop invariant failure: fn=",
                                    function_index,
                                    u8" ip=",
                                    stacktop_dbg_last_ip,
                                    u8" op=",
                                    runtime_log_op_name(stacktop_dbg_last_op),
                                    u8" op_u=",
                                    ::fast_io::mnp::hex0x(op_u),
                                    u8" stacktop{mem=",
                                    stacktop_memory_count,
                                    u8",cache=",
                                    stacktop_cache_count,
                                    u8"} codegen_sz=",
                                    codegen_operand_stack.size(),
                                    u8" operand_sz=",
                                    operand_stack.size(),
                                    u8" polymorphic=",
                                    is_polymorphic,
                                    u8"\n");

                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
            }

	            if(stacktop_cache_i32_count + stacktop_cache_i64_count + stacktop_cache_f32_count + stacktop_cache_f64_count +
	                   stacktop_cache_v128_count !=
	               stacktop_cache_count)
            {
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
            }

            auto const check_currpos_in_range{
                [&]([[maybe_unused]] ::std::size_t pos, [[maybe_unused]] ::std::size_t begin_pos, [[maybe_unused]] ::std::size_t end_pos) constexpr noexcept
                {
                    if(pos < begin_pos || pos >= end_pos) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
                }};

            if(stacktop_i32_enabled)
            {
                check_currpos_in_range(curr_stacktop.i32_stack_top_curr_pos, stacktop_i32_begin_pos, stacktop_i32_end_pos);
            }
            if(stacktop_i64_enabled)
            {
                check_currpos_in_range(curr_stacktop.i64_stack_top_curr_pos, stacktop_i64_begin_pos, stacktop_i64_end_pos);
            }
            if(stacktop_f32_enabled)
            {
                check_currpos_in_range(curr_stacktop.f32_stack_top_curr_pos, CompileOption.f32_stack_top_begin_pos, CompileOption.f32_stack_top_end_pos);
            }
            if(stacktop_f64_enabled)
            {
                check_currpos_in_range(curr_stacktop.f64_stack_top_curr_pos, CompileOption.f64_stack_top_begin_pos, CompileOption.f64_stack_top_end_pos);
            }
            if(stacktop_v128_enabled)
            {
                check_currpos_in_range(curr_stacktop.v128_stack_top_curr_pos, CompileOption.v128_stack_top_begin_pos, CompileOption.v128_stack_top_end_pos);
            }

            // Capacity check per range (use direct sites so the reported line identifies the failing range).
            if(stacktop_i32_enabled)
            {
                auto const begin_pos{stacktop_i32_begin_pos};
                auto const end_pos{stacktop_i32_end_pos};
                auto const cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                auto const cap{end_pos - begin_pos};
                if(cnt > cap) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
            }
            if(stacktop_i64_enabled)
            {
                auto const begin_pos{stacktop_i64_begin_pos};
                auto const end_pos{stacktop_i64_end_pos};
                auto const cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                auto const cap{end_pos - begin_pos};
                if(cnt > cap) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
            }
            if(stacktop_f32_enabled)
            {
                auto const begin_pos{CompileOption.f32_stack_top_begin_pos};
                auto const end_pos{CompileOption.f32_stack_top_end_pos};
                auto const cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                auto const cap{end_pos - begin_pos};
                if(cnt > cap) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
            }
            if(stacktop_f64_enabled)
            {
                auto const begin_pos{CompileOption.f64_stack_top_begin_pos};
                auto const end_pos{CompileOption.f64_stack_top_end_pos};
                auto const cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                auto const cap{end_pos - begin_pos};
                if(cnt > cap) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
            }
            if(stacktop_v128_enabled)
            {
                auto const begin_pos{CompileOption.v128_stack_top_begin_pos};
                auto const end_pos{CompileOption.v128_stack_top_end_pos};
                auto const cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                auto const cap{end_pos - begin_pos};
                if(cnt > cap) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
            }
        }
#endif
    }};

auto const emit_stacktop_spill1_typed_to{
    [&]([[maybe_unused]] bytecode_vec_t& dst, [[maybe_unused]] ::std::size_t slot, [[maybe_unused]] curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        if constexpr(stacktop_enabled)
        {
            if(runtime_log_on) [[unlikely]] { ++runtime_log_stats.stacktop_spill1_count; }
            if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
            {
                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                     u8"[uwvm-int-translator] fn=",
                                     function_index,
                                     u8" event=stacktop.spill1 | vt=",
                                     runtime_log_vt_name(vt),
                                     u8" slot=",
                                     slot,
                                     u8" currpos{i32=",
                                     curr_stacktop.i32_stack_top_curr_pos,
                                     u8",i64=",
                                     curr_stacktop.i64_stack_top_curr_pos,
                                     u8",f32=",
                                     curr_stacktop.f32_stack_top_curr_pos,
                                     u8",f64=",
                                     curr_stacktop.f64_stack_top_curr_pos,
                                     u8",v128=",
                                     curr_stacktop.v128_stack_top_curr_pos,
                                     u8"} stacktop{mem=",
                                     stacktop_memory_count,
                                     u8",cache=",
                                     stacktop_cache_count,
                                     u8"}\n");
            }

            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            switch(vt)
            {
                case curr_operand_stack_value_type::i32:
                {
                    if constexpr(stacktop_i32_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::i64:
                {
                    if constexpr(stacktop_i64_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::f32:
                {
                    if constexpr(stacktop_f32_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::f64:
                {
                    if constexpr(stacktop_f64_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::v128:
                {
                    if constexpr(stacktop_v128_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<CompileOption, wasm_v128_t>(slot,
                                                                                                                                    interpreter_tuple));
                    }
                    break;
                }
                [[unlikely]] default:
                {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                    ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                    break;
                }
            }
        }
    }};

// Compile-time: whether each value-type range is disjoint (not merged) with other enabled ranges.
// If a range is merged, the untyped multi-count spill/fill opfuncs are not usable (they require StartPos to hit exactly one range),
// so we must fall back to single-value typed spill/fill.
[[maybe_unused]] constexpr auto const stacktop_same_range{
    [](::std::size_t a_begin, ::std::size_t a_end, ::std::size_t b_begin, ::std::size_t b_end) constexpr noexcept -> bool
    { return a_begin == b_begin && a_end == b_end; }};

[[maybe_unused]] constexpr bool const i32_range_unique{stacktop_i32_enabled &&
                                                       ((1uz) +
                                                        (stacktop_i64_enabled && stacktop_same_range(stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos,
                                                                                                     stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f32_enabled && stacktop_same_range(stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos,
                                                                                                     CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f64_enabled && stacktop_same_range(stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos,
                                                                                                     CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_v128_enabled && stacktop_same_range(stacktop_i32_begin_pos,
                                                                                                      stacktop_i32_end_pos,
                                                                                                      CompileOption.v128_stack_top_begin_pos,
                                                                                                      CompileOption.v128_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz)) == 1uz};

[[maybe_unused]] constexpr bool const i64_range_unique{stacktop_i64_enabled &&
                                                       ((1uz) +
                                                        (stacktop_i32_enabled && stacktop_same_range(stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos,
                                                                                                     stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f32_enabled && stacktop_same_range(stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos,
                                                                                                     CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f64_enabled && stacktop_same_range(stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos,
                                                                                                     CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_v128_enabled && stacktop_same_range(stacktop_i64_begin_pos,
                                                                                                      stacktop_i64_end_pos,
                                                                                                      CompileOption.v128_stack_top_begin_pos,
                                                                                                      CompileOption.v128_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz)) == 1uz};

[[maybe_unused]] constexpr bool const f32_range_unique{stacktop_f32_enabled &&
                                                       ((1uz) +
                                                        (stacktop_i32_enabled && stacktop_same_range(CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos,
                                                                                                     stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_i64_enabled && stacktop_same_range(CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos,
                                                                                                     stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f64_enabled && stacktop_same_range(CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos,
                                                                                                     CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_v128_enabled && stacktop_same_range(CompileOption.f32_stack_top_begin_pos,
                                                                                                      CompileOption.f32_stack_top_end_pos,
                                                                                                      CompileOption.v128_stack_top_begin_pos,
                                                                                                      CompileOption.v128_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz)) == 1uz};

[[maybe_unused]] constexpr bool const f64_range_unique{stacktop_f64_enabled &&
                                                       ((1uz) +
                                                        (stacktop_i32_enabled && stacktop_same_range(CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos,
                                                                                                     stacktop_i32_begin_pos,
                                                                                                     stacktop_i32_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_i64_enabled && stacktop_same_range(CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos,
                                                                                                     stacktop_i64_begin_pos,
                                                                                                     stacktop_i64_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_f32_enabled && stacktop_same_range(CompileOption.f64_stack_top_begin_pos,
                                                                                                     CompileOption.f64_stack_top_end_pos,
                                                                                                     CompileOption.f32_stack_top_begin_pos,
                                                                                                     CompileOption.f32_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz) +
                                                        (stacktop_v128_enabled && stacktop_same_range(CompileOption.f64_stack_top_begin_pos,
                                                                                                      CompileOption.f64_stack_top_end_pos,
                                                                                                      CompileOption.v128_stack_top_begin_pos,
                                                                                                      CompileOption.v128_stack_top_end_pos)
                                                             ? 1uz
                                                             : 0uz)) == 1uz};

// Emit a multi-value spill (cache -> memory) for a **single scalar type**.
// This reduces bytecode size and dispatch overhead by combining consecutive spills into one threaded-interpreter opfunc.
[[maybe_unused]] auto const emit_stacktop_spilln_same_vt_to{
    [&]([[maybe_unused]] bytecode_vec_t& dst,
        [[maybe_unused]] ::std::size_t start_pos,
        [[maybe_unused]] ::std::size_t count,
        [[maybe_unused]] curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        if(count == 0uz) { return; }
        if(runtime_log_on) [[unlikely]] { ++runtime_log_stats.stacktop_spillN_count; }

        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
        using remain_t = ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_remain_size_t;
        auto tmp_currpos{curr_stacktop};
        remain_t remain{};

        switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                if constexpr(stacktop_i32_enabled)
                {
                    tmp_currpos.i32_stack_top_curr_pos = start_pos;
                    remain.i32_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.spillN | vt=i32 start=",
                                             start_pos,
                                             u8" remain(i32)=",
                                             count,
                                             u8" currpos(i32)=",
                                             curr_stacktop.i32_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_stacktop_to_operand_stack_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                if constexpr(stacktop_i64_enabled)
                {
                    tmp_currpos.i64_stack_top_curr_pos = start_pos;
                    remain.i64_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.spillN | vt=i64 start=",
                                             start_pos,
                                             u8" remain(i64)=",
                                             count,
                                             u8" currpos(i64)=",
                                             curr_stacktop.i64_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_stacktop_to_operand_stack_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::f32:
            {
                if constexpr(stacktop_f32_enabled)
                {
                    tmp_currpos.f32_stack_top_curr_pos = start_pos;
                    remain.f32_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.spillN | vt=f32 start=",
                                             start_pos,
                                             u8" remain(f32)=",
                                             count,
                                             u8" currpos(f32)=",
                                             curr_stacktop.f32_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_stacktop_to_operand_stack_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::f64:
            {
                if constexpr(stacktop_f64_enabled)
                {
                    tmp_currpos.f64_stack_top_curr_pos = start_pos;
                    remain.f64_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.spillN | vt=f64 start=",
                                             start_pos,
                                             u8" remain(f64)=",
                                             count,
                                             u8" currpos(f64)=",
                                             curr_stacktop.f64_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_stacktop_to_operand_stack_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::v128:
            {
                if constexpr(stacktop_v128_enabled)
                {
                    tmp_currpos.v128_stack_top_curr_pos = start_pos;
                    remain.v128_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.spillN | vt=v128 start=",
                                             start_pos,
                                             u8" remain(v128)=",
                                             count,
                                             u8" currpos(v128)=",
                                             curr_stacktop.v128_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_stacktop_to_operand_stack_fptr_from_tuple<CompileOption, wasm_v128_t>(tmp_currpos,
                                                                                                                                remain,
                                                                                                                                interpreter_tuple));
                    return;
                }
                break;
            }
            [[unlikely]] default:
            {
                break;
            }
        }

        // Fallback: emit single-value spills.
        {
            ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
            ::std::size_t const end_pos{stacktop_range_end_pos(vt)};

            // Spill in deepest->top order so operand-stack memory preserves deep->top layout.
            ::std::size_t pos{start_pos};
            pos = stacktop_window_advance_next(pos, count - 1uz, begin_pos, end_pos);  // deepest within the segment
            for(::std::size_t i{}; i != count; ++i)
            {
                emit_stacktop_spill1_typed_to(dst, pos, vt);
                pos = stacktop_window_prev_pos(pos, begin_pos, end_pos);
            }
        }
    }};

auto const emit_stacktop_fill1_typed_to{
    [&]([[maybe_unused]] bytecode_vec_t& dst, [[maybe_unused]] ::std::size_t slot, [[maybe_unused]] curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        if constexpr(stacktop_enabled)
        {
            if(runtime_log_on) [[unlikely]] { ++runtime_log_stats.stacktop_fill1_count; }
            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            switch(vt)
            {
                case curr_operand_stack_value_type::i32:
                {
                    if constexpr(stacktop_i32_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_operand_stack_to_stacktop_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::i64:
                {
                    if constexpr(stacktop_i64_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_operand_stack_to_stacktop_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::f32:
                {
                    if constexpr(stacktop_f32_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_operand_stack_to_stacktop_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::f64:
                {
                    if constexpr(stacktop_f64_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_operand_stack_to_stacktop_typed_single_fptr_from_tuple<CompileOption,
                                                                                                          ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64>(
                                slot,
                                interpreter_tuple));
                    }
                    break;
                }
                case curr_operand_stack_value_type::v128:
                {
                    if constexpr(stacktop_v128_enabled)
                    {
                        emit_opfunc_to(
                            dst,
                            translate::get_uwvmint_operand_stack_to_stacktop_typed_single_fptr_from_tuple<CompileOption, wasm_v128_t>(slot,
                                                                                                                                    interpreter_tuple));
                    }
                    break;
                }
                [[unlikely]] default:
                {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                    ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                    break;
                }
            }
        }
    }};

// Emit a multi-value fill (memory -> cache) for a **single scalar type**.
[[maybe_unused]] auto const emit_stacktop_filln_same_vt_to{
    [&]([[maybe_unused]] bytecode_vec_t& dst,
        [[maybe_unused]] ::std::size_t start_pos,
        [[maybe_unused]] ::std::size_t count,
        [[maybe_unused]] curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        if(count == 0uz) { return; }
        if(runtime_log_on) [[unlikely]] { ++runtime_log_stats.stacktop_fillN_count; }

        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
        using remain_t = ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_remain_size_t;
        auto tmp_currpos{curr_stacktop};
        remain_t remain{};

        switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                if constexpr(stacktop_i32_enabled)
                {
                    tmp_currpos.i32_stack_top_curr_pos = start_pos;
                    remain.i32_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.fillN | vt=i32 start=",
                                             start_pos,
                                             u8" remain(i32)=",
                                             count,
                                             u8" currpos(i32)=",
                                             curr_stacktop.i32_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_operand_stack_to_stacktop_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                if constexpr(stacktop_i64_enabled)
                {
                    tmp_currpos.i64_stack_top_curr_pos = start_pos;
                    remain.i64_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.fillN | vt=i64 start=",
                                             start_pos,
                                             u8" remain(i64)=",
                                             count,
                                             u8" currpos(i64)=",
                                             curr_stacktop.i64_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_operand_stack_to_stacktop_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::f32:
            {
                if constexpr(stacktop_f32_enabled)
                {
                    tmp_currpos.f32_stack_top_curr_pos = start_pos;
                    remain.f32_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.fillN | vt=f32 start=",
                                             start_pos,
                                             u8" remain(f32)=",
                                             count,
                                             u8" currpos(f32)=",
                                             curr_stacktop.f32_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_operand_stack_to_stacktop_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::f64:
            {
                if constexpr(stacktop_f64_enabled)
                {
                    tmp_currpos.f64_stack_top_curr_pos = start_pos;
                    remain.f64_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.fillN | vt=f64 start=",
                                             start_pos,
                                             u8" remain(f64)=",
                                             count,
                                             u8" currpos(f64)=",
                                             curr_stacktop.f64_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(
                        dst,
                        translate::get_uwvmint_operand_stack_to_stacktop_fptr_from_tuple<CompileOption, ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64>(
                            tmp_currpos,
                            remain,
                            interpreter_tuple));
                    return;
                }
                break;
            }
            case curr_operand_stack_value_type::v128:
            {
                if constexpr(stacktop_v128_enabled)
                {
                    tmp_currpos.v128_stack_top_curr_pos = start_pos;
                    remain.v128_stack_top_remain_size = count;
                    if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" event=stacktop.fillN | vt=v128 start=",
                                             start_pos,
                                             u8" remain(v128)=",
                                             count,
                                             u8" currpos(v128)=",
                                             curr_stacktop.v128_stack_top_curr_pos,
                                             u8"\n");
                    }
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_operand_stack_to_stacktop_fptr_from_tuple<CompileOption, wasm_v128_t>(tmp_currpos,
                                                                                                                                remain,
                                                                                                                                interpreter_tuple));
                    return;
                }
                break;
            }
            [[unlikely]] default:
            {
                break;
            }
        }

        // Fallback: emit single-value fills.
        {
            ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
            ::std::size_t const end_pos{stacktop_range_end_pos(vt)};

            // Fill in top->deep order: each fill consumes the current operand-stack top and writes the next stack-top window slot.
            ::std::size_t pos{start_pos};
            for(::std::size_t i{}; i != count; ++i)
            {
                emit_stacktop_fill1_typed_to(dst, pos, vt);
                pos = stacktop_window_next_pos(pos, begin_pos, end_pos);
            }
        }
    }};

auto const stacktop_spill_one_deepest_to{[&](bytecode_vec_t& dst, ::std::size_t max_spill_n = SIZE_MAX) constexpr UWVM_THROWS
                                         {
                                             if constexpr(!stacktop_enabled) { return; }
                                             if(stacktop_cache_count == 0uz) { return; }

                                             stacktop_assert_invariants();

                                             // Deepest cached value is at index `stacktop_memory_count`.
                                             // If several *consecutive* deepest cached values share the same type, batch them into one spill opfunc.
                                             auto const vt{codegen_operand_stack.index_unchecked(stacktop_memory_count).type};
                                             ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                             ::std::size_t const end_pos{stacktop_range_end_pos(vt)};
                                             ::std::size_t const group_cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                                             ::std::size_t const currpos{stacktop_currpos_for_range(begin_pos, end_pos)};
                                             ::std::size_t spill_n{1uz};
                                             {
                                                 auto const cache_end{stacktop_memory_count + stacktop_cache_count};
                                                 while(spill_n < group_cnt && (stacktop_memory_count + spill_n) < cache_end &&
                                                       codegen_operand_stack.index_unchecked(stacktop_memory_count + spill_n).type == vt)
                                                 {
                                                     ++spill_n;
                                                 }
                                             }
                                             if(max_spill_n == 0uz) { return; }
                                             if(spill_n > max_spill_n) { spill_n = max_spill_n; }

                                             // Batch only for disjoint ranges; for merged ranges, fall back to single spills.
                                             // (Typed spill opfunc enforces Count==1.)
                                             if(spill_n > 1uz)
                                             {
                                                 // segment top (least deep among the deepest `spill_n` values)
                                                 ::std::size_t const seg_top_slot{stacktop_window_advance_next(currpos, group_cnt - spill_n, begin_pos, end_pos)};
                                                 if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                                 {
                                                     ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                          u8"[uwvm-int-translator] fn=",
                                                                          function_index,
                                                                          u8" event=stacktop.spill_one_deepest(batch) | vt=",
                                                                          runtime_log_vt_name(vt),
                                                                          u8" spill_n=",
                                                                          spill_n,
                                                                          u8" seg_top_slot=",
                                                                          seg_top_slot,
                                                                          u8" group_cnt=",
                                                                          group_cnt,
                                                                          u8" currpos=",
                                                                          currpos,
                                                                          u8" begin=",
                                                                          begin_pos,
                                                                          u8" end=",
                                                                          end_pos,
                                                                          u8"\n");
                                                 }
                                                 emit_stacktop_spilln_same_vt_to(dst, seg_top_slot, spill_n, vt);
                                             }
                                             else
                                             {
                                                 ::std::size_t const deepest_slot{stacktop_window_advance_next(currpos, group_cnt - 1uz, begin_pos, end_pos)};
                                                 if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                                 {
                                                     ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                          u8"[uwvm-int-translator] fn=",
                                                                          function_index,
                                                                          u8" event=stacktop.spill_one_deepest(single) | vt=",
                                                                          runtime_log_vt_name(vt),
                                                                          u8" slot=",
                                                                          deepest_slot,
                                                                          u8" group_cnt=",
                                                                          group_cnt,
                                                                          u8" currpos=",
                                                                          currpos,
                                                                          u8" begin=",
                                                                          begin_pos,
                                                                          u8" end=",
                                                                          end_pos,
                                                                          u8"\n");
                                                 }
                                                 emit_stacktop_spill1_typed_to(dst, deepest_slot, vt);
                                             }

                                             // Model effects: one value moved cache -> memory.
                                             stacktop_memory_count += spill_n;
                                             stacktop_cache_count -= spill_n;
                                             stacktop_cache_count_ref_for_vt(vt) -= spill_n;

                                             stacktop_assert_invariants();
                                         }};

auto const stacktop_fill_one_from_memory_to{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
                                            {
                                                if constexpr(!stacktop_enabled) { return; }
                                                if(stacktop_memory_count == 0uz) { return; }

                                                stacktop_assert_invariants();

                                                // Fill consecutive same-typed values from memory into cache in one opfunc when possible.
                                                auto const vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};
                                                ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                                ::std::size_t const end_pos{stacktop_range_end_pos(vt)};
                                                ::std::size_t const group_cnt{stacktop_cache_count_for_range(begin_pos, end_pos)};
                                                ::std::size_t const currpos{stacktop_currpos_for_range(begin_pos, end_pos)};
                                                ::std::size_t const window_size{end_pos - begin_pos};
                                                ::std::size_t const free_slots{window_size - group_cnt};
                                                ::std::size_t fill_n{1uz};
                                                if(free_slots > 1uz)
                                                {
                                                    while(fill_n < free_slots && fill_n < stacktop_memory_count &&
                                                          codegen_operand_stack.index_unchecked((stacktop_memory_count - 1uz) - fill_n).type == vt)
                                                    {
                                                        ++fill_n;
                                                    }
                                                }

                                                ::std::size_t const start_slot{stacktop_window_advance_next(currpos, group_cnt, begin_pos, end_pos)};
                                                {
                                                    ::std::size_t pos{start_slot};
                                                    for(::std::size_t i{}; i != fill_n; ++i)
                                                    {
                                                        local_spot_flush_dirty_slot_and_invalidate_to(dst, vt, pos);
                                                        pos = stacktop_window_next_pos(pos, begin_pos, end_pos);
                                                    }
                                                }

                                                if(fill_n > 1uz) { emit_stacktop_filln_same_vt_to(dst, start_slot, fill_n, vt); }
                                                else
                                                {
                                                    emit_stacktop_fill1_typed_to(dst, start_slot, vt);
                                                }

                                                stacktop_memory_count -= fill_n;
                                                stacktop_cache_count += fill_n;
                                                stacktop_cache_count_ref_for_vt(vt) += fill_n;

                                                stacktop_assert_invariants();
                                            }};

auto const stacktop_prepare_push1_typed{[&](bytecode_vec_t& dst, curr_operand_stack_value_type vt) constexpr UWVM_THROWS
                                        {
                                            if constexpr(!stacktop_enabled) { return; }
                                            if(!stacktop_enabled_for_vt(vt))
                                            {
                                                while(stacktop_cache_count != 0uz) { stacktop_spill_one_deepest_to(dst); }
                                                return;
                                            }

                                            ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                            ::std::size_t const end_pos{stacktop_range_end_pos(vt)};
                                            ::std::size_t const window_size{end_pos - begin_pos};

                                            // Critical correctness: push opfuncs write into `stacktop_window_prev_pos(currpos)`. If the stack-top window is full, that slot
                                            // is occupied by the deepest cached value of that range; spill from the deepest cached overall until
                                            // the target range has a free slot.
                                            if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                            {
                                                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                     u8"[uwvm-int-translator] fn=",
                                                                     function_index,
                                                                     u8" event=stacktop.prepare_push1(begin) | vt=",
                                                                     runtime_log_vt_name(vt),
                                                                     u8" begin=",
                                                                     begin_pos,
                                                                     u8" end=",
                                                                     end_pos,
                                                                     u8" window=",
                                                                     window_size,
                                                                     u8" range_cache=",
                                                                     stacktop_cache_count_for_range(begin_pos, end_pos),
                                                                     u8" stacktop{mem=",
                                                                     stacktop_memory_count,
                                                                     u8",cache=",
                                                                     stacktop_cache_count,
                                                                     u8"}\n");
                                            }
                                            ::std::size_t spill_cnt{};
                                            while(stacktop_cache_count_for_range(begin_pos, end_pos) >= window_size)
                                            {
                                                // Only spill what is needed to free one slot for this push.
                                                // Over-spilling can strand subsequent opcodes (e.g. `select`) with operands in memory while
                                                // the corresponding opfunc expects them to still reside in the stack-top cache.
                                                stacktop_spill_one_deepest_to(dst, 1uz);
                                                ++spill_cnt;
                                            }

                                            ::std::size_t const target_pos{
                                                stacktop_window_prev_pos(stacktop_currpos_for_range(begin_pos, end_pos), begin_pos, end_pos)};
                                            local_spot_flush_dirty_slot_and_invalidate_to(dst, vt, target_pos);

                                            stacktop_assert_invariants();
                                            if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                            {
                                                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                     u8"[uwvm-int-translator] fn=",
                                                                     function_index,
                                                                     u8" event=stacktop.prepare_push1(end) | vt=",
                                                                     runtime_log_vt_name(vt),
                                                                     u8" spills=",
                                                                     spill_cnt,
                                                                     u8" range_cache=",
                                                                     stacktop_cache_count_for_range(begin_pos, end_pos),
                                                                     u8" stacktop{mem=",
                                                                     stacktop_memory_count,
                                                                     u8",cache=",
                                                                     stacktop_cache_count,
                                                                     u8"} currpos{i32=",
                                                                     curr_stacktop.i32_stack_top_curr_pos,
                                                                     u8",i64=",
                                                                     curr_stacktop.i64_stack_top_curr_pos,
                                                                     u8",f32=",
                                                                     curr_stacktop.f32_stack_top_curr_pos,
                                                                     u8",f64=",
                                                                     curr_stacktop.f64_stack_top_curr_pos,
                                                                     u8"}\n");
                                            }
                                        }};

auto const stacktop_commit_push1_typed{[&](curr_operand_stack_value_type vt) constexpr noexcept
                                       {
                                           if constexpr(!stacktop_enabled) { return; }
                                           if(!stacktop_enabled_for_vt(vt))
                                           {
                                               ++stacktop_memory_count;
                                               return;
                                           }

                                           ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                           ::std::size_t const end_pos{stacktop_range_end_pos(vt)};

                                           ::std::size_t const currpos{stacktop_currpos_for_range(begin_pos, end_pos)};
                                           ::std::size_t const new_pos{stacktop_window_prev_pos(currpos, begin_pos, end_pos)};
                                           stacktop_set_currpos_for_range(begin_pos, end_pos, new_pos);

                                           ++stacktop_cache_count;
                                           ++stacktop_cache_count_ref_for_vt(vt);
                                           if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                           {
                                               ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                    u8"[uwvm-int-translator] fn=",
                                                                    function_index,
                                                                    u8" event=stacktop.commit_push1 | vt=",
                                                                    runtime_log_vt_name(vt),
                                                                    u8" currpos=",
                                                                    currpos,
                                                                    u8"->",
                                                                    new_pos,
                                                                    u8" stacktop{mem=",
                                                                    stacktop_memory_count,
                                                                    u8",cache=",
                                                                    stacktop_cache_count,
                                                                    u8"}\n");
                                           }
                                       }};

// Commit-only pop updates the compiler's cache model after an already-emitted opfunc consumes stack
// values. It emits no bytecode because the runtime helper has already performed the actual pop.
auto const stacktop_commit_pop_n{[&](::std::size_t n) constexpr noexcept
                                 {
                                     if constexpr(!stacktop_enabled) { return; }
                                     else
                                     {
                                         if(n == 0uz) { return; }

                                         auto const before_curr_stacktop{curr_stacktop};
                                         auto const before_stacktop_memory_count{stacktop_memory_count};
                                         auto const before_stacktop_cache_count{stacktop_cache_count};
                                         auto const before_stacktop_cache_i32_count{stacktop_cache_i32_count};
	                                         auto const before_stacktop_cache_i64_count{stacktop_cache_i64_count};
	                                         auto const before_stacktop_cache_f32_count{stacktop_cache_f32_count};
	                                         auto const before_stacktop_cache_f64_count{stacktop_cache_f64_count};
	                                         auto const before_stacktop_cache_v128_count{stacktop_cache_v128_count};

                                         // Pop from the top; if cache becomes empty, remaining pops consume the memory-only stack.
                                         for(::std::size_t i{}; i != n; ++i)
                                         {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                             if(codegen_operand_stack.empty()) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
                                             if(codegen_operand_stack.empty()) { return; }

                                             // Pop i-th value from top (type stack is updated by the caller after this commit).
                                             auto const vt{codegen_operand_stack.index_unchecked((codegen_operand_stack.size() - 1uz) - i).type};

                                             if(!stacktop_enabled_for_vt(vt))
                                             {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                                 if(stacktop_memory_count == 0uz) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
                                                 if(stacktop_memory_count != 0uz) { --stacktop_memory_count; }
                                                 continue;
                                             }

                                             ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                             ::std::size_t const end_pos{stacktop_range_end_pos(vt)};

                                             ::std::size_t const currpos{stacktop_currpos_for_range(begin_pos, end_pos)};
                                             ::std::size_t const new_pos{stacktop_window_next_pos(currpos, begin_pos, end_pos)};
                                             stacktop_set_currpos_for_range(begin_pos, end_pos, new_pos);

                                             if(stacktop_cache_count != 0uz)
                                             {
                                                 --stacktop_cache_count;
                                                 --stacktop_cache_count_ref_for_vt(vt);
                                             }
                                             else
                                             {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                                                 if(stacktop_memory_count == 0uz) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
                                                 --stacktop_memory_count;
                                             }
                                         }

                                         if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
                                         {
                                             ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                                  u8"[uwvm-int-translator] fn=",
                                                                  function_index,
                                                                  u8" event=stacktop.commit_pop_n | n=",
                                                                  n,
                                                                  u8" stacktop{mem=",
                                                                  before_stacktop_memory_count,
                                                                  u8",cache=",
                                                                  before_stacktop_cache_count,
                                                                  u8"}->",
                                                                  u8"{mem=",
                                                                  stacktop_memory_count,
                                                                  u8",cache=",
                                                                  stacktop_cache_count,
                                                                  u8"} cache_type{i32=",
                                                                  before_stacktop_cache_i32_count,
                                                                  u8",i64=",
                                                                  before_stacktop_cache_i64_count,
                                                                  u8",f32=",
                                                                  before_stacktop_cache_f32_count,
	                                                                  u8",f64=",
	                                                                  before_stacktop_cache_f64_count,
	                                                                  u8",v128=",
	                                                                  before_stacktop_cache_v128_count,
	                                                                  u8"}->",
                                                                  u8"{i32=",
                                                                  stacktop_cache_i32_count,
                                                                  u8",i64=",
                                                                  stacktop_cache_i64_count,
                                                                  u8",f32=",
                                                                  stacktop_cache_f32_count,
	                                                                  u8",f64=",
	                                                                  stacktop_cache_f64_count,
	                                                                  u8",v128=",
	                                                                  stacktop_cache_v128_count,
	                                                                  u8"} currpos{i32=",
                                                                  before_curr_stacktop.i32_stack_top_curr_pos,
                                                                  u8"->",
                                                                  curr_stacktop.i32_stack_top_curr_pos,
                                                                  u8",i64=",
                                                                  before_curr_stacktop.i64_stack_top_curr_pos,
                                                                  u8"->",
                                                                  curr_stacktop.i64_stack_top_curr_pos,
                                                                  u8",f32=",
                                                                  before_curr_stacktop.f32_stack_top_curr_pos,
                                                                  u8"->",
                                                                  curr_stacktop.f32_stack_top_curr_pos,
                                                                  u8",f64=",
	                                                                  before_curr_stacktop.f64_stack_top_curr_pos,
	                                                                  u8"->",
	                                                                  curr_stacktop.f64_stack_top_curr_pos,
	                                                                  u8",v128=",
	                                                                  before_curr_stacktop.v128_stack_top_curr_pos,
	                                                                  u8"->",
	                                                                  curr_stacktop.v128_stack_top_curr_pos,
	                                                                  u8"}\n");
                                         }
                                     }
                                 }};

// Canonical fill tries to keep the top values cached after a stack-shape change. This is the normal
// post-op state used by fallthrough code where register-like stack-top values are profitable.
auto const stacktop_fill_to_canonical{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
                                      {
                                          if constexpr(!stacktop_enabled) { return; }
                                          else
                                          {

                                              stacktop_assert_invariants();

                                              // Fill from memory into cache as deep as possible while respecting per-range stack-top window capacities.
                                              while(stacktop_memory_count != 0uz)
                                              {
                                                  auto const vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};
                                                  ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
                                                  ::std::size_t const end_pos{stacktop_range_end_pos(vt)};
                                                  ::std::size_t const window_size{end_pos - begin_pos};
                                                  if(stacktop_cache_count_for_range(begin_pos, end_pos) == window_size) { break; }
                                                  stacktop_fill_one_from_memory_to(dst);
                                              }

                                              stacktop_assert_invariants();
                                          }
                                      }};

// Calls and control-flow joins are layout barriers: all cached values are spilled so every incoming
// path observes the same operand-stack memory representation.
auto const stacktop_flush_all_to_operand_stack{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
                                               {
                                                   if constexpr(!stacktop_enabled) { return; }
                                                   else
                                                   {
                                                       stacktop_assert_invariants();
                                                       local_spot_flush_dirty_all_to(dst);
                                                       // Spill from deepest to top so operand stack memory ends up in correct deep->top order.
                                                       while(stacktop_cache_count != 0uz) { stacktop_spill_one_deepest_to(dst); }

                                                       local_spot_invalidate_all();
                                                       stacktop_assert_invariants();
                                                   }
                                               }};

[[maybe_unused]] auto const spill_i32_stacktop_if_anonymous_capacity_insufficient{
    [&](bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        if constexpr(stacktop_i32_spot_enabled)
        {
            if(stacktop_cache_i32_count > anonymous_stacktop_param_capacity_for_numeric_i32 ||
               (stacktop_cache_i32_count != 0uz && curr_stacktop.i32_stack_top_curr_pos != stacktop_i32_begin_pos))
            {
                stacktop_flush_all_to_operand_stack(dst);
            }
        }
    }};

[[maybe_unused]] auto const spill_i64_stacktop_if_anonymous_capacity_insufficient{
    [&](bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        if constexpr(stacktop_i64_spot_enabled)
        {
            if(stacktop_cache_i64_count > anonymous_stacktop_param_capacity_for_numeric_i64 ||
               (stacktop_cache_i64_count != 0uz && curr_stacktop.i64_stack_top_curr_pos != stacktop_i64_begin_pos))
            {
                stacktop_flush_all_to_operand_stack(dst);
            }
        }
    }};

[[maybe_unused]] auto const emit_i32_anonymous_binop_to{
    [&]<::uwvm2::runtime::compiler::uwvm_int::optable::numeric_details::int_binop Op>(bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
        if constexpr(stacktop_i32_spot_enabled)
        {
            spill_i32_stacktop_if_anonymous_capacity_insufficient(dst);
            local_spot_flush_dirty_stacktop_slots_and_invalidate_to(dst,
                                                                    curr_operand_stack_value_type::i32,
                                                                    anonymous_stacktop_param_count_for_numeric_i32());
            emit_opfunc_to(dst,
                           translate::get_uwvmint_i32_binop_anonymous_stacktop_fptr_from_tuple<CompileOption, Op>(
                               anonymous_stacktop_param_count_for_numeric_i32(),
                               interpreter_tuple));
        }
        else
        {
            local_spot_flush_dirty_stacktop_slots_and_invalidate_to(dst, curr_operand_stack_value_type::i32, 2uz);
            emit_opfunc_to(dst, translate::get_uwvmint_i32_binop_fptr_from_tuple<CompileOption, Op>(curr_stacktop, interpreter_tuple));
        }
    }};

[[maybe_unused]] auto const emit_i64_anonymous_binop_to{
    [&]<::uwvm2::runtime::compiler::uwvm_int::optable::numeric_details::int_binop Op>(bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
        if constexpr(stacktop_i64_spot_enabled)
        {
            spill_i64_stacktop_if_anonymous_capacity_insufficient(dst);
            local_spot_flush_dirty_stacktop_slots_and_invalidate_to(dst,
                                                                    curr_operand_stack_value_type::i64,
                                                                    anonymous_stacktop_param_count_for_numeric_i64());
            emit_opfunc_to(dst,
                           translate::get_uwvmint_i64_binop_anonymous_stacktop_fptr_from_tuple<CompileOption, Op>(
                               anonymous_stacktop_param_count_for_numeric_i64(),
                               interpreter_tuple));
        }
        else
        {
            local_spot_flush_dirty_stacktop_slots_and_invalidate_to(dst, curr_operand_stack_value_type::i64, 2uz);
            emit_opfunc_to(dst, translate::get_uwvmint_i64_binop_fptr_from_tuple<CompileOption, Op>(curr_stacktop, interpreter_tuple));
        }
    }};

[[maybe_unused]] auto const stacktop_reset_currpos_to_begin{
    [&]() constexpr noexcept
    {
        if constexpr(!stacktop_enabled) { return; }
        curr_stacktop.i32_stack_top_curr_pos = stacktop_i32_enabled ? stacktop_i32_begin_pos : SIZE_MAX;
        curr_stacktop.i64_stack_top_curr_pos = stacktop_i64_enabled ? stacktop_i64_begin_pos : SIZE_MAX;
        curr_stacktop.f32_stack_top_curr_pos = stacktop_f32_enabled ? CompileOption.f32_stack_top_begin_pos : SIZE_MAX;
        curr_stacktop.f64_stack_top_curr_pos = stacktop_f64_enabled ? CompileOption.f64_stack_top_begin_pos : SIZE_MAX;
        curr_stacktop.v128_stack_top_curr_pos = stacktop_v128_enabled ? CompileOption.v128_stack_top_begin_pos : SIZE_MAX;
    }};

[[maybe_unused]] auto const stacktop_currpos_is_begin{[&]() constexpr noexcept -> bool
                                                      {
                                                          if constexpr(!stacktop_enabled) { return true; }
                                                          if constexpr(stacktop_i32_enabled)
                                                          {
                                                              if(curr_stacktop.i32_stack_top_curr_pos != stacktop_i32_begin_pos)
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          if constexpr(stacktop_i64_enabled)
                                                          {
                                                              if(curr_stacktop.i64_stack_top_curr_pos != stacktop_i64_begin_pos)
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          if constexpr(stacktop_f32_enabled)
                                                          {
                                                              if(curr_stacktop.f32_stack_top_curr_pos != CompileOption.f32_stack_top_begin_pos)
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          if constexpr(stacktop_f64_enabled)
                                                          {
                                                              if(curr_stacktop.f64_stack_top_curr_pos != CompileOption.f64_stack_top_begin_pos)
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          if constexpr(stacktop_v128_enabled)
                                                          {
                                                              if(curr_stacktop.v128_stack_top_curr_pos != CompileOption.v128_stack_top_begin_pos)
                                                              {
                                                                  return false;
                                                              }
                                                          }
                                                          return true;
                                                      }};

[[maybe_unused]] auto const loop_unwind_gcd_size{[](::std::size_t a, ::std::size_t b) constexpr noexcept -> ::std::size_t
                                                 {
                                                     while(b != 0uz)
                                                     {
                                                         auto const r{a % b};
                                                         a = b;
                                                         b = r;
                                                     }
                                                     return a;
                                                 }};

[[maybe_unused]] auto const loop_unwind_lcm_size{[&](::std::size_t a, ::std::size_t b) constexpr noexcept -> ::std::size_t
                                                 {
                                                     if(a == 0uz || b == 0uz) { return 0uz; }
                                                     auto const g{loop_unwind_gcd_size(a, b)};
                                                     auto const div{a / g};
                                                     if(div > (::std::numeric_limits<::std::size_t>::max() / b))
                                                     {
                                                         return ::std::numeric_limits<::std::size_t>::max();
                                                     }
                                                     return div * b;
                                                 }};

// Loop-unwind uses the minimum recovery period for each enabled stack-top window:
// window_size / gcd(window_size, delta). The global period is the lcm across ranges.
// See src/uwvm2/runtime/compiler/uwvm_int/loop_unwind.md.
[[maybe_unused]] auto const loop_unwind_currpos_period{
    [&]() constexpr noexcept -> ::std::size_t
    {
        if constexpr(!stacktop_enabled) { return 1uz; }
        ::std::size_t period{1uz};

        auto const add_range{[&](bool enabled, ::std::size_t begin_pos, ::std::size_t end_pos, ::std::size_t currpos) constexpr noexcept
                             {
                                 if(!enabled || currpos == SIZE_MAX || end_pos <= begin_pos) { return; }
                                 auto const window_size{end_pos - begin_pos};
                                 auto const delta{(currpos + window_size - begin_pos) % window_size};
                                 if(delta == 0uz) { return; }
                                 auto const range_period{window_size / loop_unwind_gcd_size(window_size, delta)};
                                 period = loop_unwind_lcm_size(period, range_period);
                             }};

        add_range(stacktop_i32_enabled, stacktop_i32_begin_pos, stacktop_i32_end_pos, curr_stacktop.i32_stack_top_curr_pos);
        add_range(stacktop_i64_enabled, stacktop_i64_begin_pos, stacktop_i64_end_pos, curr_stacktop.i64_stack_top_curr_pos);
        add_range(stacktop_f32_enabled, CompileOption.f32_stack_top_begin_pos, CompileOption.f32_stack_top_end_pos, curr_stacktop.f32_stack_top_curr_pos);
        add_range(stacktop_f64_enabled, CompileOption.f64_stack_top_begin_pos, CompileOption.f64_stack_top_end_pos, curr_stacktop.f64_stack_top_curr_pos);
        add_range(stacktop_v128_enabled, CompileOption.v128_stack_top_begin_pos, CompileOption.v128_stack_top_end_pos, curr_stacktop.v128_stack_top_curr_pos);
        return period;
    }};

[[maybe_unused]] auto const stacktop_transform_currpos_to_begin{
    [&](bytecode_vec_t& dst) constexpr UWVM_THROWS
    {
        if constexpr(stacktop_enabled && CompileOption.is_tail_call && stacktop_regtransform_cf_entry && stacktop_regtransform_supported)
        {
            if(is_polymorphic)
            {
                // Unreachable region: keep compiler-side state deterministic without emitting runtime code.
                local_spot_invalidate_all();
                stacktop_reset_currpos_to_begin();
                return;
            }

            // If cache is empty there is no live register state to preserve; reset cursors without emitting a transform opfunc.
            if(stacktop_cache_count == 0uz)
            {
                local_spot_flush_dirty_all_and_invalidate_to(dst);
                stacktop_reset_currpos_to_begin();
                return;
            }

            namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
            local_spot_flush_dirty_all_and_invalidate_to(dst);
            emit_opfunc_to(dst, translate::get_uwvmint_stacktop_transform_to_begin_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
            stacktop_reset_currpos_to_begin();
        }
    }};

// Edge canonicalization is stricter than a normal spill: it also resets stack-top window cursors and counters so
// branch targets can start from a deterministic empty-cache state.
[[maybe_unused]] auto const stacktop_canonicalize_edge_to_memory{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
                                                                 {
                                                                     if constexpr(!stacktop_enabled) { return; }
                                                                     // Move all cached values to operand-stack memory (call-like barrier).
                                                                     stacktop_flush_all_to_operand_stack(dst);
                                                                     // Make the empty-cache state deterministic for subsequent codegen.
                                                                     stacktop_reset_currpos_to_begin();
                                                                     stacktop_cache_count = 0uz;
                                                                     stacktop_cache_i32_count = 0uz;
	                                                                     stacktop_cache_i64_count = 0uz;
	                                                                     stacktop_cache_f32_count = 0uz;
	                                                                     stacktop_cache_f64_count = 0uz;
	                                                                     stacktop_cache_v128_count = 0uz;
                                                                     stacktop_memory_count = codegen_operand_stack.size();
                                                                 }};

[[maybe_unused]] auto const stacktop_loop_entry_residence_matches{
    [&](curr_block_type const& frame) constexpr noexcept -> bool
    {
        if constexpr(!stacktop_enabled) { return true; }
        else
        {
            if(frame.type != block_type::loop || !frame.stacktop_has_loop_entry_state) { return true; }
            if(stacktop_memory_count != frame.stacktop_memory_count_at_loop_entry) { return false; }
            if(stacktop_cache_count != frame.stacktop_cache_count_at_loop_entry) { return false; }
            if(stacktop_cache_i32_count != frame.stacktop_cache_i32_count_at_loop_entry) { return false; }
            if(stacktop_cache_i64_count != frame.stacktop_cache_i64_count_at_loop_entry) { return false; }
            if(stacktop_cache_f32_count != frame.stacktop_cache_f32_count_at_loop_entry) { return false; }
            if(stacktop_cache_f64_count != frame.stacktop_cache_f64_count_at_loop_entry) { return false; }
            if(stacktop_cache_v128_count != frame.stacktop_cache_v128_count_at_loop_entry) { return false; }
            if(codegen_operand_stack.size() != frame.codegen_operand_stack_at_loop_entry.size()) { return false; }
            for(::std::size_t i{}; i != codegen_operand_stack.size(); ++i)
            {
                if(codegen_operand_stack.index_unchecked(i).type != frame.codegen_operand_stack_at_loop_entry.index_unchecked(i).type) { return false; }
            }
            return true;
        }
    }};

[[maybe_unused]] auto const stacktop_loop_entry_currpos_matches{
    [&](curr_block_type const& frame) constexpr noexcept -> bool
    {
        if constexpr(!stacktop_enabled) { return true; }
        else
        {
            if(frame.type != block_type::loop || !frame.stacktop_has_loop_entry_state) { return true; }
            return curr_stacktop.i32_stack_top_curr_pos == frame.stacktop_currpos_at_loop_entry.i32_stack_top_curr_pos &&
                   curr_stacktop.i64_stack_top_curr_pos == frame.stacktop_currpos_at_loop_entry.i64_stack_top_curr_pos &&
                   curr_stacktop.f32_stack_top_curr_pos == frame.stacktop_currpos_at_loop_entry.f32_stack_top_curr_pos &&
                   curr_stacktop.f64_stack_top_curr_pos == frame.stacktop_currpos_at_loop_entry.f64_stack_top_curr_pos &&
                   curr_stacktop.v128_stack_top_curr_pos == frame.stacktop_currpos_at_loop_entry.v128_stack_top_curr_pos;
        }
    }};

[[maybe_unused]] auto const stacktop_restore_loop_entry_residence_to{
    [&](bytecode_vec_t& dst, curr_block_type const& frame) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(frame.type != block_type::loop || !frame.stacktop_has_loop_entry_state) { return; }
            if(stacktop_loop_entry_residence_matches(frame)) { return; }

            stacktop_canonicalize_edge_to_memory(dst);
            while(stacktop_cache_count < frame.stacktop_cache_count_at_loop_entry)
            {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                if(stacktop_memory_count == 0uz) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
                if(stacktop_memory_count == 0uz) [[unlikely]] { break; }
                stacktop_fill_one_from_memory_to(dst);
            }

            curr_stacktop = frame.stacktop_currpos_at_loop_entry;
            stacktop_memory_count = frame.stacktop_memory_count_at_loop_entry;
            stacktop_cache_count = frame.stacktop_cache_count_at_loop_entry;
            stacktop_cache_i32_count = frame.stacktop_cache_i32_count_at_loop_entry;
            stacktop_cache_i64_count = frame.stacktop_cache_i64_count_at_loop_entry;
            stacktop_cache_f32_count = frame.stacktop_cache_f32_count_at_loop_entry;
            stacktop_cache_f64_count = frame.stacktop_cache_f64_count_at_loop_entry;
            stacktop_cache_v128_count = frame.stacktop_cache_v128_count_at_loop_entry;
            codegen_operand_stack = frame.codegen_operand_stack_at_loop_entry;
            local_spot_flush_dirty_all_and_invalidate_to(dst);
        }
    }};

auto const stacktop_prepare_push1_if_reachable{[&](bytecode_vec_t& dst, curr_operand_stack_value_type vt) constexpr UWVM_THROWS
                                               {
                                                   if constexpr(!stacktop_enabled) { return; }
                                                   else
                                                   {
                                                       if(is_polymorphic) { return; }
                                                       stacktop_prepare_push1_typed(dst, vt);
                                                   }
                                               }};

[[maybe_unused]] auto const stacktop_prepare_push_n_same_vt_if_reachable{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, ::std::size_t n) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(is_polymorphic || n == 0uz) { return; }
            if(!stacktop_enabled_for_vt(vt)) { return; }

            ::std::size_t const begin_pos{stacktop_range_begin_pos(vt)};
            ::std::size_t const end_pos{stacktop_range_end_pos(vt)};
            ::std::size_t const window_size{end_pos - begin_pos};

#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
            if(window_size == 0uz || n > window_size) [[unlikely]] { ::uwvm2::utils::debug::trap_and_inform_bug_pos(); }
#endif
            if(window_size == 0uz || n > window_size) [[unlikely]] { return; }

            if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
            {
                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                     u8"[uwvm-int-translator] fn=",
                                     function_index,
                                     u8" event=stacktop.prepare_pushN(begin) | vt=",
                                     runtime_log_vt_name(vt),
                                     u8" n=",
                                     n,
                                     u8" begin=",
                                     begin_pos,
                                     u8" end=",
                                     end_pos,
                                     u8" range_cache=",
                                     stacktop_cache_count_for_range(begin_pos, end_pos),
                                     u8" stacktop{mem=",
                                     stacktop_memory_count,
                                     u8",cache=",
                                     stacktop_cache_count,
                                     u8"}\n");
            }

            ::std::size_t spill_cnt{};
            while(stacktop_cache_count_for_range(begin_pos, end_pos) + n > window_size)
            {
                stacktop_spill_one_deepest_to(dst, 1uz);
                ++spill_cnt;
            }

            stacktop_assert_invariants();
            if(runtime_log_on && runtime_log_emit_stacktop) [[unlikely]]
            {
                ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                     u8"[uwvm-int-translator] fn=",
                                     function_index,
                                     u8" event=stacktop.prepare_pushN(end) | vt=",
                                     runtime_log_vt_name(vt),
                                     u8" n=",
                                     n,
                                     u8" spills=",
                                     spill_cnt,
                                     u8" range_cache=",
                                     stacktop_cache_count_for_range(begin_pos, end_pos),
                                     u8" stacktop{mem=",
                                     stacktop_memory_count,
                                     u8",cache=",
                                     stacktop_cache_count,
                                     u8"}\n");
            }
        }
    }};

[[maybe_unused]] auto const stacktop_commit_push1_if_reachable{[&](curr_operand_stack_value_type vt) constexpr noexcept
                                                               {
                                                                   if constexpr(!stacktop_enabled) { return; }
                                                                   else
                                                                   {
                                                                       if(is_polymorphic) { return; }
                                                                       stacktop_commit_push1_typed(vt);
                                                                   }
                                                               }};

[[maybe_unused]] auto const stacktop_commit_push1_typed_if_reachable{[&](curr_operand_stack_value_type vt) constexpr UWVM_THROWS
                                                                     {
                                                                         if constexpr(!stacktop_enabled) { return; }
                                                                         else
                                                                         {
                                                                             if(is_polymorphic) { return; }
                                                                             stacktop_commit_push1_typed(vt);
                                                                             codegen_stack_push(vt);
                                                                         }
                                                                     }};

auto const stacktop_after_pop_n_if_reachable{[&](bytecode_vec_t& dst, ::std::size_t n) constexpr UWVM_THROWS
                                             {
                                                 if constexpr(!stacktop_enabled) { return; }
                                                 else
                                                 {
                                                     if(is_polymorphic) { return; }
                                                     stacktop_commit_pop_n(n);
                                                     codegen_stack_pop_n(n);
                                                     stacktop_fill_to_canonical(dst);
                                                 }
                                             }};

// Pop modeling + retype the new top before canonical fill (used for ops like i64.cmp -> i32).
[[maybe_unused]] auto const stacktop_after_pop_n_retype_top_if_reachable{
    [&](bytecode_vec_t& dst, ::std::size_t n, curr_operand_stack_value_type new_top_type) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(is_polymorphic) { return; }
            stacktop_commit_pop_n(n);
            codegen_stack_pop_n(n);
            codegen_stack_set_top(new_top_type);
            stacktop_fill_to_canonical(dst);
        }
    }};

// Pop+push modeling (used for cross-range ops like f32.cmp -> i32 when i32 range is disjoint from f32 range).
	[[maybe_unused]] auto const stacktop_after_pop_n_push1_typed_if_reachable{
	    [&](bytecode_vec_t& dst, ::std::size_t pop_n, curr_operand_stack_value_type push_type) constexpr UWVM_THROWS
	    {
	        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(is_polymorphic) { return; }
            stacktop_commit_pop_n(pop_n);
            codegen_stack_pop_n(pop_n);
            // Ensure the target stack-top window has a free slot *after* the pop step.
            // This is required for cross-range ops that keep depth unchanged (pop+push),
            // e.g. `f32.load` (i32 addr -> f32 result) in split GPR/FP stack-top window layouts.
            stacktop_prepare_push1_typed(dst, push_type);
            stacktop_commit_push1_typed(push_type);
            codegen_stack_push(push_type);
	            stacktop_fill_to_canonical(dst);
	        }
	    }};

	// Pop+push modeling for opfuncs that have already written the result to operand-stack memory.
	[[maybe_unused]] auto const stacktop_after_pop_n_push1_memory_typed_if_reachable{
	    [&](bytecode_vec_t& dst, ::std::size_t pop_n, curr_operand_stack_value_type push_type) constexpr UWVM_THROWS
	    {
	        if constexpr(!stacktop_enabled) { return; }
	        else
	        {
	            if(is_polymorphic) { return; }
	            stacktop_commit_pop_n(pop_n);
	            codegen_stack_pop_n(pop_n);
	            ++stacktop_memory_count;
	            codegen_stack_push(push_type);
	            stacktop_fill_to_canonical(dst);
	        }
	    }};

	// Pop+push modeling without emitting any fill ops (used to keep `br_if` fusion candidates contiguous).
	[[maybe_unused]] auto const stacktop_after_pop_n_push1_typed_no_fill_if_reachable{
    [&](::std::size_t pop_n, curr_operand_stack_value_type push_type) constexpr noexcept
    {
        if constexpr(!stacktop_enabled) { return; }
        else
        {
            if(is_polymorphic) { return; }
            stacktop_commit_pop_n(pop_n);
            codegen_stack_pop_n(pop_n);
            stacktop_commit_push1_typed(push_type);
            codegen_stack_push(push_type);
        }
    }};

	// Pop modeling without emitting any fill ops (used to keep `br_if` fusion candidates contiguous).
	[[maybe_unused]] auto const stacktop_after_pop_n_no_fill_if_reachable{[&](::std::size_t n) constexpr noexcept
	                                                                      {
	                                                                          if constexpr(!stacktop_enabled) { return; }
	                                                                          else
	                                                                          {
	                                                                              if(is_polymorphic) { return; }
	                                                                              stacktop_commit_pop_n(n);
	                                                                              codegen_stack_pop_n(n);
	                                                                          }
	                                                                      }};

	[[maybe_unused]] auto const stacktop_after_i32_anonymous_binop_no_fill_if_reachable{
	    [&]() constexpr noexcept
	    {
	        if constexpr(!stacktop_enabled) { return; }
	        else
	        {
	            if(is_polymorphic) { return; }
	            if constexpr(stacktop_i32_spot_enabled)
	            {
	                if(anonymous_stacktop_param_count_for_numeric_i32() == 1uz)
	                {
	                    if(stacktop_memory_count != 0uz) { --stacktop_memory_count; }
	                    codegen_stack_pop_n(1uz);
	                }
	                else
	                {
	                    stacktop_commit_pop_n(1uz);
	                    codegen_stack_pop_n(1uz);
	                }
	            }
	            else
	            {
	                stacktop_commit_pop_n(1uz);
	                codegen_stack_pop_n(1uz);
	            }
	        }
	    }};

	[[maybe_unused]] auto const stacktop_after_i64_anonymous_binop_no_fill_if_reachable{
	    [&]() constexpr noexcept
	    {
	        if constexpr(!stacktop_enabled) { return; }
	        else
	        {
	            if(is_polymorphic) { return; }
	            if constexpr(stacktop_i64_spot_enabled)
	            {
	                if(anonymous_stacktop_param_count_for_numeric_i64() == 1uz)
	                {
	                    if(stacktop_memory_count != 0uz) { --stacktop_memory_count; }
	                    codegen_stack_pop_n(1uz);
	                }
	                else
	                {
	                    stacktop_commit_pop_n(1uz);
	                    codegen_stack_pop_n(1uz);
	                }
	            }
	            else
	            {
	                stacktop_commit_pop_n(1uz);
	                codegen_stack_pop_n(1uz);
	            }
	        }
	    }};

	[[maybe_unused]] auto const stacktop_after_i32_anonymous_binop_if_reachable{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
	                                                                           {
	                                                                               if constexpr(!stacktop_enabled) { return; }
	                                                                               else
	                                                                               {
	                                                                                   if(is_polymorphic) { return; }
	                                                                                   stacktop_after_i32_anonymous_binop_no_fill_if_reachable();
	                                                                                   stacktop_fill_to_canonical(dst);
	                                                                               }
	                                                                           }};

	[[maybe_unused]] auto const stacktop_after_i64_anonymous_binop_if_reachable{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
	                                                                           {
	                                                                               if constexpr(!stacktop_enabled) { return; }
	                                                                               else
	                                                                               {
	                                                                                   if(is_polymorphic) { return; }
	                                                                                   stacktop_after_i64_anonymous_binop_no_fill_if_reachable();
	                                                                                   stacktop_fill_to_canonical(dst);
	                                                                               }
	                                                                           }};

	// `drop` emission must account for where the value currently lives. A cached value may be removed by
	// metadata updates only, while a memory-backed value needs a real drop helper.
auto const emit_drop_typed_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        auto const emit_real_drop{
            [&]() constexpr UWVM_THROWS
            {
                switch(vt)
                {
                    case curr_operand_stack_value_type::i32:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::i64:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::f32:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::f64:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::v128:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_v128_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::funcref:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_funcref_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::externref:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_externref_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    [[unlikely]] default:
                    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                        ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                        return;
                    }
                }
            }};

        if constexpr(!stacktop_enabled)
        {
            emit_real_drop();
            return;
        }

        if(is_polymorphic)
        {
            // Unreachable region: emit a real `drop` without mutating compiler-side stack-top model.
            emit_real_drop();
            return;
        }

        // Cache-hit: fold `drop` by updating the compiler-side stack-top cursor (no runtime op).
        // Cache-miss: emit real `drop` to adjust the memory stack pointer, then update the model.
        if(stacktop_cache_count != 0uz)
        {
            stacktop_commit_pop_n(1uz);
            codegen_stack_pop_n(1uz);
            stacktop_fill_to_canonical(dst);
        }
        else
        {
            emit_real_drop();
            stacktop_commit_pop_n(1uz);
            codegen_stack_pop_n(1uz);
            stacktop_fill_to_canonical(dst);
        }
    }};

// Drop modeling without emitting any canonical fills.
// Used for bulk stack-shape repair sequences (br/br_if/br_table/return) where intermediate fills would
// often reload values that are immediately dropped again.
[[maybe_unused]] auto const emit_drop_typed_to_no_fill{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        auto const emit_real_drop{
            [&]() constexpr UWVM_THROWS
            {
                switch(vt)
                {
                    case curr_operand_stack_value_type::i32:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::i64:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::f32:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::f64:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    case curr_operand_stack_value_type::v128:
                    {
                        emit_opfunc_to(dst, translate::get_uwvmint_drop_v128_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                        return;
                    }
                    [[unlikely]] default:
                    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                        ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                        return;
                    }
                }
            }};

        if constexpr(!stacktop_enabled)
        {
            emit_real_drop();
            return;
        }

        if(is_polymorphic)
        {
            // Unreachable region: emit a real `drop` without mutating compiler-side stack-top model.
            emit_real_drop();
            return;
        }

        if(stacktop_cache_count != 0uz)
        {
            stacktop_commit_pop_n(1uz);
            codegen_stack_pop_n(1uz);
        }
        else
        {
            emit_real_drop();
            stacktop_commit_pop_n(1uz);
            codegen_stack_pop_n(1uz);
        }
    }};

// Bulk drop modeling without emitting any canonical fills.
// Used to coalesce long stack-shape repair sequences (br/br_if/br_table/return) into one `drop_bytes` opfunc.
[[maybe_unused]] auto const emit_drop_to_stack_size_no_fill{
    [&](bytecode_vec_t& dst, ::std::size_t target_size) constexpr UWVM_THROWS
    {
        if constexpr(!stacktop_enabled)
        {
            // Fallback: preserve existing behavior (emit per-value drops).
            auto const curr_size{operand_stack.size()};
            if(curr_size <= target_size) { return; }
            for(::std::size_t i{curr_size}; i > target_size; --i) { emit_drop_typed_to_no_fill(dst, operand_stack.index_unchecked(i - 1uz).type); }
            return;
        }

        if(is_polymorphic)
        {
            // Unreachable region: do not mutate compiler-side stack-top model; emit real drops.
            auto const curr_size{operand_stack.size()};
            if(curr_size <= target_size) { return; }
            for(::std::size_t i{curr_size}; i > target_size; --i) { emit_drop_typed_to_no_fill(dst, operand_stack.index_unchecked(i - 1uz).type); }
            return;
        }

        auto const curr_size{codegen_operand_stack.size()};
        if(curr_size <= target_size) { return; }

        ::std::size_t const drop_n{curr_size - target_size};

        // Top `stacktop_cache_count` values are resident in registers (cache); remaining are in operand-stack memory.
        ::std::size_t const cache_pops{stacktop_cache_count < drop_n ? stacktop_cache_count : drop_n};
        ::std::size_t const mem_pops{drop_n - cache_pops};

        if(mem_pops != 0uz)
        {
            ::std::size_t bytes_total{};
            ::std::size_t const mem_end{curr_size - cache_pops};
            for(::std::size_t i{target_size}; i < mem_end; ++i) { bytes_total += operand_stack_valtype_size(codegen_operand_stack.index_unchecked(i).type); }

            if(bytes_total != 0uz)
            {
                using drop_bytes_imm_t = ::std::uint_least32_t;

                namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

                ::std::size_t remaining{bytes_total};
                while(remaining != 0uz)
                {
                    auto const chunk_sz{remaining > ::std::numeric_limits<drop_bytes_imm_t>::max()
                                            ? static_cast<::std::size_t>(::std::numeric_limits<drop_bytes_imm_t>::max())
                                            : remaining};
                    drop_bytes_imm_t const chunk{static_cast<drop_bytes_imm_t>(chunk_sz)};

                    emit_opfunc_to(dst, translate::get_uwvmint_drop_bytes_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, chunk);

                    remaining -= chunk_sz;
                }
            }
        }

        stacktop_commit_pop_n(drop_n);
        codegen_stack_pop_n(drop_n);
    }};

// Local-get emission is optimized around the stack-top cache: when a spill was just emitted to make
// room, this helper can patch the spill into a combined spill-then-local-get dispatch.
auto const emit_local_get_typed_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, local_offset_t off) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        bool fused_spill_and_local_get{};
        bool emitted_local_spot_get{};
        [[maybe_unused]] ::std::size_t fuse_site{};

        if constexpr(stacktop_enabled)
        {
            // local.get pushes 1 value to stack-top cache; spill if stack-top window is full.
            [[maybe_unused]] ::std::size_t const bc_before{dst.size()};
            stacktop_prepare_push1_if_reachable(dst, vt);

            if(!is_polymorphic)
            {
                if(auto* const spot{local_spot_find(off, vt)}; spot != nullptr)
                {
                    using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
                    using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
                    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
                    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

                    if(runtime_log_on) [[unlikely]]
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" ip=",
                                             runtime_log_curr_ip,
                                             u8" event=local_spot.get | vt=",
                                             runtime_log_vt_name(vt),
                                             u8" off=",
                                             off,
                                             u8" pos=",
                                             spot->pos,
                                             u8" dirty=",
                                             spot->dirty,
                                             u8"\n");
                    }

                    switch(vt)
                    {
                        case curr_operand_stack_value_type::i32:
                        {
                            if constexpr(stacktop_i32_spot_enabled)
                            {
                                emit_opfunc_to(dst,
                                               translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<CompileOption, wasm_i32>(spot->pos,
                                                                                                                                    curr_stacktop,
                                                                                                                                    interpreter_tuple));
                                emitted_local_spot_get = true;
                            }
                            break;
                        }
                        case curr_operand_stack_value_type::i64:
                        {
                            if constexpr(stacktop_i64_spot_enabled)
                            {
                                emit_opfunc_to(dst,
                                               translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<CompileOption, wasm_i64>(spot->pos,
                                                                                                                                    curr_stacktop,
                                                                                                                                    interpreter_tuple));
                                emitted_local_spot_get = true;
                            }
                            break;
                        }
                        case curr_operand_stack_value_type::f32:
                        {
                            if constexpr(stacktop_f32_enabled)
                            {
                                emit_opfunc_to(dst,
                                               translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<CompileOption, wasm_f32>(spot->pos,
                                                                                                                                    curr_stacktop,
                                                                                                                                    interpreter_tuple));
                                emitted_local_spot_get = true;
                            }
                            break;
                        }
                        case curr_operand_stack_value_type::f64:
                        {
                            if constexpr(stacktop_f64_enabled)
                            {
                                emit_opfunc_to(dst,
                                               translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<CompileOption, wasm_f64>(spot->pos,
                                                                                                                                    curr_stacktop,
                                                                                                                                    interpreter_tuple));
                                emitted_local_spot_get = true;
                            }
                            break;
                        }
                        case curr_operand_stack_value_type::v128:
                        {
                            if constexpr(stacktop_v128_enabled)
                            {
                                emit_opfunc_to(dst,
                                               translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(spot->pos,
                                                                                                                                      curr_stacktop,
                                                                                                                                      interpreter_tuple));
                                emitted_local_spot_get = true;
                            }
                            break;
                        }
                        [[unlikely]] default:
                        {
                            break;
                        }
                    }
                }
            }

#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
            // If `stacktop_prepare_push1_*` emitted a spill opfunc immediately before this `local.get`, rewrite that spill opfunc
            // into a fused "spill1 + local.get" opfunc, and reuse the would-be `local.get` opfunc slot for the immediate.
            // This avoids an extra dispatch *and* avoids the runtime "skip next opfunc pointer" pattern.
            if(!emitted_local_spot_get && dst.size() != bc_before)
            {
                constexpr bool i32_i64_merge{stacktop_i32_begin_pos == stacktop_i64_begin_pos &&
                                             stacktop_i32_end_pos == stacktop_i64_end_pos};
                constexpr bool i32_f32_merge{stacktop_i32_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                             stacktop_i32_end_pos == CompileOption.f32_stack_top_end_pos};
                constexpr bool i32_f64_merge{stacktop_i32_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             stacktop_i32_end_pos == CompileOption.f64_stack_top_end_pos};
                constexpr bool i64_f32_merge{stacktop_i64_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                             stacktop_i64_end_pos == CompileOption.f32_stack_top_end_pos};
                constexpr bool i64_f64_merge{stacktop_i64_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             stacktop_i64_end_pos == CompileOption.f64_stack_top_end_pos};
                constexpr bool f32_f64_merge{CompileOption.f32_stack_top_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             CompileOption.f32_stack_top_end_pos == CompileOption.f64_stack_top_end_pos};

                using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
                using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
                using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
                using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

                auto const spilled_vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};

                // Patch the *last* emitted spill opfunc (spillN is not emitted here; prepare_push1 emits spill1 only).
                using opfunc_ptr_t = decltype(translate::get_uwvmint_local_get_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                fuse_site = dst.size() - sizeof(opfunc_ptr_t);

	                auto patch_with{[&](auto fused_fptr) constexpr UWVM_THROWS
	                                {
	                                    if(fused_fptr == nullptr) { return; }
	                                    ::std::byte tmp[sizeof(fused_fptr)];
	                                    ::std::memcpy(tmp, ::std::addressof(fused_fptr), sizeof(fused_fptr));
	                                    ::std::memcpy(dst.data() + fuse_site, tmp, sizeof(fused_fptr));
	                                    fused_spill_and_local_get = true;
	                                }};

                // Decide spill-value type (SpilledT) from the translator model.
                switch(vt)
                {
                    case curr_operand_stack_value_type::i32:
                    {
                        if constexpr(stacktop_i32_enabled)
                        {
                            if(spilled_vt == curr_operand_stack_value_type::i32)
                            {
                                patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_i32>(
                                    curr_stacktop,
                                    interpreter_tuple));
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i64)
                            {
                                if constexpr(i32_i64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_i32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f32)
                            {
                                if constexpr(i32_f32_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_i32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f64)
                            {
                                if constexpr(i32_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_i32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                        }
                        break;
                    }
                    case curr_operand_stack_value_type::i64:
                    {
                        if constexpr(stacktop_i64_enabled)
                        {
                            if(spilled_vt == curr_operand_stack_value_type::i64)
                            {
                                patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_i64>(
                                    curr_stacktop,
                                    interpreter_tuple));
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i32)
                            {
                                if constexpr(i32_i64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_i64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f32)
                            {
                                if constexpr(i64_f32_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_i64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f64)
                            {
                                if constexpr(i64_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_i64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                        }
                        break;
                    }
                    case curr_operand_stack_value_type::f32:
                    {
                        if constexpr(stacktop_f32_enabled)
                        {
                            if(spilled_vt == curr_operand_stack_value_type::f32)
                            {
                                patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_f32>(
                                    curr_stacktop,
                                    interpreter_tuple));
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f64)
                            {
                                if constexpr(f32_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_f32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i32)
                            {
                                if constexpr(i32_f32_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_f32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i64)
                            {
                                if constexpr(i64_f32_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_f32>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                        }
                        break;
                    }
                    case curr_operand_stack_value_type::f64:
                    {
                        if constexpr(stacktop_f64_enabled)
                        {
                            if(spilled_vt == curr_operand_stack_value_type::f64)
                            {
                                patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_f64>(
                                    curr_stacktop,
                                    interpreter_tuple));
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::f32)
                            {
                                if constexpr(f32_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_f64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i32)
                            {
                                if constexpr(i32_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_f64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                            else if(spilled_vt == curr_operand_stack_value_type::i64)
                            {
                                if constexpr(i64_f64_merge)
                                {
                                    patch_with(translate::get_uwvmint_stacktop_spill1_then_local_get_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_f64>(
                                        curr_stacktop,
                                        interpreter_tuple));
                                }
                            }
                        }
                        break;
                    }
                    [[unlikely]] default:
                    {
                        break;
                    }
                }

                if(fused_spill_and_local_get)
                {
                    // Emit only the immediate; opfunc slot was patched above.
                    emit_imm_to(dst, off);
                }
            }
#endif
        }

        if(!emitted_local_spot_get && !fused_spill_and_local_get) switch(vt)
            {
                case curr_operand_stack_value_type::i32:
                {
                    emit_opfunc_to(dst, translate::get_uwvmint_local_get_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::i64:
                {
                    emit_opfunc_to(dst, translate::get_uwvmint_local_get_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::f32:
                {
                    emit_opfunc_to(dst, translate::get_uwvmint_local_get_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::f64:
                {
                    emit_opfunc_to(dst, translate::get_uwvmint_local_get_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::v128:
                {
                    emit_opfunc_to(dst, translate::get_uwvmint_local_get_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::funcref:
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_local_get_typed_fptr_from_tuple<CompileOption, wasm_funcref_t>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                case curr_operand_stack_value_type::externref:
                {
                    emit_opfunc_to(dst,
                                   translate::get_uwvmint_local_get_typed_fptr_from_tuple<CompileOption, wasm_externref_t>(curr_stacktop, interpreter_tuple));
                    emit_imm_to(dst, off);
                    break;
                }
                [[unlikely]] default:
                {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                    ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                    break;
                }
            }

        if constexpr(stacktop_enabled)
        {
            // Model effects: 1 value pushed into stack-top cache.
            stacktop_commit_push1_typed_if_reachable(vt);
        }
    }};

// Constants share the same spill-patching strategy as local.get so immediate producers do not add an
// extra dispatch after the cache has just been adjusted.
[[maybe_unused]] auto const emit_const_i32_to{
    [&](bytecode_vec_t& dst, wasm_i32 imm) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        bool fused_spill_and_const{};

        if constexpr(stacktop_enabled)
        {
            [[maybe_unused]] ::std::size_t const bc_before{dst.size()};
            stacktop_prepare_push1_if_reachable(dst, curr_operand_stack_value_type::i32);

#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
            if constexpr(stacktop_i32_enabled)
            {
                if(dst.size() != bc_before)
                {
                    constexpr bool i32_i64_merge{stacktop_i32_begin_pos == stacktop_i64_begin_pos &&
                                                 stacktop_i32_end_pos == stacktop_i64_end_pos};
                    constexpr bool i32_f32_merge{stacktop_i32_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                                 stacktop_i32_end_pos == CompileOption.f32_stack_top_end_pos};
                    constexpr bool i32_f64_merge{stacktop_i32_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                                 stacktop_i32_end_pos == CompileOption.f64_stack_top_end_pos};

                    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
                    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

                    auto const spilled_vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};

                    using opfunc_ptr_t = decltype(translate::get_uwvmint_i32_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    ::std::size_t const fuse_site{dst.size() - sizeof(opfunc_ptr_t)};

	                    auto patch_with{[&](auto fused_fptr) constexpr UWVM_THROWS
	                                    {
	                                        if(fused_fptr == nullptr) { return; }
	                                        ::std::byte tmp[sizeof(fused_fptr)];
	                                        ::std::memcpy(tmp, ::std::addressof(fused_fptr), sizeof(fused_fptr));
	                                        ::std::memcpy(dst.data() + fuse_site, tmp, sizeof(fused_fptr));
	                                        fused_spill_and_const = true;
	                                    }};

                    if(spilled_vt == curr_operand_stack_value_type::i32)
                    {
                        patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_i32>(
                            curr_stacktop,
                            interpreter_tuple));
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::i64)
                    {
                        if constexpr(i32_i64_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_i32>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::f32)
                    {
                        if constexpr(i32_f32_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_i32>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::f64)
                    {
                        if constexpr(i32_f64_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_i32>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }

                    if(fused_spill_and_const) { emit_imm_to(dst, imm); }
                }
            }
#endif
        }

        if(!fused_spill_and_const)
        {
            emit_opfunc_to(dst, translate::get_uwvmint_i32_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
            emit_imm_to(dst, imm);
        }

        if constexpr(stacktop_enabled) { stacktop_commit_push1_typed_if_reachable(curr_operand_stack_value_type::i32); }
    }};

[[maybe_unused]] auto const emit_const_i64_to{
    [&](bytecode_vec_t& dst, wasm_i64 imm) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        bool fused_spill_and_const{};

        if constexpr(stacktop_enabled)
        {
            [[maybe_unused]] ::std::size_t const bc_before{dst.size()};
            stacktop_prepare_push1_if_reachable(dst, curr_operand_stack_value_type::i64);

#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
            if constexpr(stacktop_i64_enabled)
            {
                if(dst.size() != bc_before)
                {
                    constexpr bool i32_i64_merge{stacktop_i32_begin_pos == stacktop_i64_begin_pos &&
                                                 stacktop_i32_end_pos == stacktop_i64_end_pos};
                    constexpr bool i64_f32_merge{stacktop_i64_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                                 stacktop_i64_end_pos == CompileOption.f32_stack_top_end_pos};
                    constexpr bool i64_f64_merge{stacktop_i64_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                                 stacktop_i64_end_pos == CompileOption.f64_stack_top_end_pos};

                    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
                    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

                    auto const spilled_vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};

                    using opfunc_ptr_t = decltype(translate::get_uwvmint_i64_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                    ::std::size_t const fuse_site{dst.size() - sizeof(opfunc_ptr_t)};

	                    auto patch_with{[&](auto fused_fptr) constexpr UWVM_THROWS
	                                    {
	                                        if(fused_fptr == nullptr) { return; }
	                                        ::std::byte tmp[sizeof(fused_fptr)];
	                                        ::std::memcpy(tmp, ::std::addressof(fused_fptr), sizeof(fused_fptr));
	                                        ::std::memcpy(dst.data() + fuse_site, tmp, sizeof(fused_fptr));
	                                        fused_spill_and_const = true;
	                                    }};

                    if(spilled_vt == curr_operand_stack_value_type::i64)
                    {
                        patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_i64>(
                            curr_stacktop,
                            interpreter_tuple));
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::i32)
                    {
                        if constexpr(i32_i64_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_i64>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::f32)
                    {
                        if constexpr(i64_f32_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_i64>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }
                    else if(spilled_vt == curr_operand_stack_value_type::f64)
                    {
                        if constexpr(i64_f64_merge)
                        {
                            patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_i64>(
                                curr_stacktop,
                                interpreter_tuple));
                        }
                    }

                    if(fused_spill_and_const) { emit_imm_to(dst, imm); }
                }
            }
#endif
        }

        if(!fused_spill_and_const)
        {
            emit_opfunc_to(dst, translate::get_uwvmint_i64_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
            emit_imm_to(dst, imm);
        }

        if constexpr(stacktop_enabled) { stacktop_commit_push1_typed_if_reachable(curr_operand_stack_value_type::i64); }
    }};

[[maybe_unused]] auto const emit_const_f32_to{
    [&](bytecode_vec_t& dst, wasm_f32 imm) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        bool fused_spill_and_const{};

        if constexpr(stacktop_enabled)
        {
            [[maybe_unused]] ::std::size_t const bc_before{dst.size()};
            stacktop_prepare_push1_if_reachable(dst, curr_operand_stack_value_type::f32);

#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
            if(dst.size() != bc_before)
            {
                constexpr bool i32_f32_merge{stacktop_i32_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                             stacktop_i32_end_pos == CompileOption.f32_stack_top_end_pos};
                constexpr bool i64_f32_merge{stacktop_i64_begin_pos == CompileOption.f32_stack_top_begin_pos &&
                                             stacktop_i64_end_pos == CompileOption.f32_stack_top_end_pos};
                constexpr bool f32_f64_merge{CompileOption.f32_stack_top_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             CompileOption.f32_stack_top_end_pos == CompileOption.f64_stack_top_end_pos};

                using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

                auto const spilled_vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};

                using opfunc_ptr_t = decltype(translate::get_uwvmint_f32_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                ::std::size_t const fuse_site{dst.size() - sizeof(opfunc_ptr_t)};

	                auto patch_with{[&](auto fused_fptr) constexpr UWVM_THROWS
	                                {
	                                    if(fused_fptr == nullptr) { return; }
	                                    ::std::byte tmp[sizeof(fused_fptr)];
	                                    ::std::memcpy(tmp, ::std::addressof(fused_fptr), sizeof(fused_fptr));
	                                    ::std::memcpy(dst.data() + fuse_site, tmp, sizeof(fused_fptr));
	                                    fused_spill_and_const = true;
	                                }};

                if(spilled_vt == curr_operand_stack_value_type::f32)
                {
                    patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_f32>(curr_stacktop,
                                                                                                                                          interpreter_tuple));
                }
                else if(spilled_vt == curr_operand_stack_value_type::f64)
                {
                    if constexpr(f32_f64_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_f32>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }
                else if(spilled_vt == curr_operand_stack_value_type::i32)
                {
                    if constexpr(i32_f32_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_f32>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }
                else if(spilled_vt == curr_operand_stack_value_type::i64)
                {
                    if constexpr(i64_f32_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_f32>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }

                if(fused_spill_and_const) { emit_imm_to(dst, imm); }
            }
#endif
        }

        if(!fused_spill_and_const)
        {
            emit_opfunc_to(dst, translate::get_uwvmint_f32_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
            emit_imm_to(dst, imm);
        }

        if constexpr(stacktop_enabled) { stacktop_commit_push1_typed_if_reachable(curr_operand_stack_value_type::f32); }
    }};

[[maybe_unused]] auto const emit_const_f64_to{
    [&](bytecode_vec_t& dst, wasm_f64 imm) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        bool fused_spill_and_const{};

        if constexpr(stacktop_enabled)
        {
            [[maybe_unused]] ::std::size_t const bc_before{dst.size()};
            stacktop_prepare_push1_if_reachable(dst, curr_operand_stack_value_type::f64);

#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
            if(dst.size() != bc_before)
            {
                constexpr bool i32_f64_merge{stacktop_i32_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             stacktop_i32_end_pos == CompileOption.f64_stack_top_end_pos};
                constexpr bool i64_f64_merge{stacktop_i64_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             stacktop_i64_end_pos == CompileOption.f64_stack_top_end_pos};
                constexpr bool f32_f64_merge{CompileOption.f32_stack_top_begin_pos == CompileOption.f64_stack_top_begin_pos &&
                                             CompileOption.f32_stack_top_end_pos == CompileOption.f64_stack_top_end_pos};

                using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;

                auto const spilled_vt{codegen_operand_stack.index_unchecked(stacktop_memory_count - 1uz).type};

                using opfunc_ptr_t = decltype(translate::get_uwvmint_f64_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                ::std::size_t const fuse_site{dst.size() - sizeof(opfunc_ptr_t)};

	                auto patch_with{[&](auto fused_fptr) constexpr UWVM_THROWS
	                                {
	                                    if(fused_fptr == nullptr) { return; }
	                                    ::std::byte tmp[sizeof(fused_fptr)];
	                                    ::std::memcpy(tmp, ::std::addressof(fused_fptr), sizeof(fused_fptr));
	                                    ::std::memcpy(dst.data() + fuse_site, tmp, sizeof(fused_fptr));
	                                    fused_spill_and_const = true;
	                                }};

                if(spilled_vt == curr_operand_stack_value_type::f64)
                {
                    patch_with(translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f64, wasm_f64>(curr_stacktop,
                                                                                                                                          interpreter_tuple));
                }
                else if(spilled_vt == curr_operand_stack_value_type::f32)
                {
                    if constexpr(f32_f64_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_f32, wasm_f64>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }
                else if(spilled_vt == curr_operand_stack_value_type::i32)
                {
                    if constexpr(i32_f64_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i32, wasm_f64>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }
                else if(spilled_vt == curr_operand_stack_value_type::i64)
                {
                    if constexpr(i64_f64_merge)
                    {
                        patch_with(
                            translate::get_uwvmint_stacktop_spill1_then_const_typed_fptr_from_tuple<CompileOption, wasm_i64, wasm_f64>(curr_stacktop,
                                                                                                                                       interpreter_tuple));
                    }
                }

                if(fused_spill_and_const) { emit_imm_to(dst, imm); }
            }
#endif
        }

        if(!fused_spill_and_const)
        {
            emit_opfunc_to(dst, translate::get_uwvmint_f64_const_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
            emit_imm_to(dst, imm);
        }

        if constexpr(stacktop_enabled) { stacktop_commit_push1_typed_if_reachable(curr_operand_stack_value_type::f64); }
    }};

// Local-set consumes the top value. When the cache is empty, the helper first fills from memory so
// the typed setter can read a normal top value instead of depending on a memory-only special case.
auto const emit_local_set_typed_to{
    [&](bytecode_vec_t& dst,
        curr_operand_stack_value_type vt,
        local_offset_t off,
        bool allow_lazy_local_spot = true,
        bool bind_local_spot_after_store = true) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        if constexpr(stacktop_enabled)
        {
            // local.set reads its operand from stack-top cache; ensure cache is populated.
            if(!is_polymorphic && stacktop_cache_count == 0uz) { stacktop_fill_to_canonical(dst); }
        }

        ::std::size_t const local_spot_src_pos{!is_polymorphic ? local_spot_current_pos_for_vt(vt) : SIZE_MAX};
        bool const lazy_local_spot_store{allow_lazy_local_spot && local_spot_src_pos != SIZE_MAX};

        if(!lazy_local_spot_store) switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::v128:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::funcref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_funcref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::externref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_externref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            [[unlikely]] default:
            {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                break;
            }
        }

        if(!is_polymorphic && bind_local_spot_after_store) { local_spot_bind_to(dst, off, vt, local_spot_src_pos, lazy_local_spot_store); }

        if constexpr(stacktop_enabled)
        {
            // Model effects: 1 value popped (compiler-managed stack-top cursor).
            if(!is_polymorphic)
            {
                stacktop_commit_pop_n(1uz);
                codegen_stack_pop_n(1uz);
                stacktop_fill_to_canonical(dst);
            }
        }
    }};

// Fused `local.set x; local.get x` for integer stack-top locals. The emitted opfunc stores the
// current top value to the local arena and keeps that same top value resident, so the compiler stack
// model intentionally does not pop or push.
[[maybe_unused]] auto const emit_local_set_keep_typed_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, local_offset_t off) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        if constexpr(stacktop_enabled)
        {
            if(!is_polymorphic && stacktop_cache_count == 0uz) { stacktop_fill_to_canonical(dst); }
        }

        ::std::size_t const local_spot_src_pos{!is_polymorphic ? local_spot_current_pos_for_vt(vt) : SIZE_MAX};
        bool const lazy_local_spot_store{};

        if(!lazy_local_spot_store) switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_keep_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_keep_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            [[unlikely]] default:
            {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                break;
            }
        }

        if(!is_polymorphic) { local_spot_bind_to(dst, off, vt, local_spot_src_pos, lazy_local_spot_store); }
    }};

// Local.set without emitting any canonical fills (see `emit_drop_typed_to_no_fill` rationale).
[[maybe_unused]] auto const emit_local_set_typed_to_no_fill{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, local_offset_t off) constexpr UWVM_THROWS
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        if constexpr(stacktop_enabled)
        {
            // local.set reads its operand from stack-top cache; ensure the top value is resident in cache.
            // Avoid a full canonical refill here; bulk repair will canonicalize once at the end when needed.
            if(!is_polymorphic && stacktop_cache_count == 0uz && stacktop_enabled_for_vt(vt)) { stacktop_fill_one_from_memory_to(dst); }
        }

        ::std::size_t const local_spot_src_pos{!is_polymorphic ? local_spot_current_pos_for_vt(vt) : SIZE_MAX};
        bool const lazy_local_spot_store{local_spot_src_pos != SIZE_MAX};

        if(!lazy_local_spot_store) switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::v128:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::funcref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_funcref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::externref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_set_typed_fptr_from_tuple<CompileOption, wasm_externref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            [[unlikely]] default:
            {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                break;
            }
        }

        if(!is_polymorphic) { local_spot_bind_to(dst, off, vt, local_spot_src_pos, lazy_local_spot_store); }

        if constexpr(stacktop_enabled)
        {
            if(!is_polymorphic)
            {
                stacktop_commit_pop_n(1uz);
                codegen_stack_pop_n(1uz);
            }
        }
    }};

// Local-tee writes a local but keeps the value alive. The helper therefore emits a store-like opfunc
// without committing a stack pop in the compiler model.
auto const emit_local_tee_typed_to{
    [&](bytecode_vec_t& dst, curr_operand_stack_value_type vt, local_offset_t off, bool bind_local_spot_after_store = true) constexpr UWVM_THROWS
        -> ::std::size_t
    {
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        if constexpr(stacktop_enabled)
        {
            // local.tee reads the top value from stack-top cache; ensure cache is populated.
            if(!is_polymorphic && stacktop_cache_count == 0uz) { stacktop_fill_to_canonical(dst); }
        }

        ::std::size_t const site{dst.size()};
        ::std::size_t const local_spot_src_pos{!is_polymorphic ? local_spot_current_pos_for_vt(vt) : SIZE_MAX};
        bool const lazy_local_spot_store{};

        if(!lazy_local_spot_store) switch(vt)
        {
            case curr_operand_stack_value_type::i32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_i32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::i64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_i64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f32:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_f32_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::f64:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_f64_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::v128:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_typed_fptr_from_tuple<CompileOption, wasm_v128_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::funcref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_typed_fptr_from_tuple<CompileOption, wasm_funcref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            case curr_operand_stack_value_type::externref:
            {
                emit_opfunc_to(dst, translate::get_uwvmint_local_tee_typed_fptr_from_tuple<CompileOption, wasm_externref_t>(curr_stacktop, interpreter_tuple));
                emit_imm_to(dst, off);
                break;
            }
            [[unlikely]] default:
            {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
#endif
                break;
            }
        }

        if(!is_polymorphic && bind_local_spot_after_store) { local_spot_bind_to(dst, off, vt, local_spot_src_pos, lazy_local_spot_store); }

        return site;
    }};

[[maybe_unused]] auto const emit_preserve_top_values_drop_to_base_restore{
    [&](bytecode_vec_t& dst,
        block_result_type value_types,
        ::std::size_t target_base,
        ::std::size_t original_stack_size,
        bool fill_to_canonical_before_restore) constexpr UWVM_THROWS
    {
        auto const value_count{static_cast<::std::size_t>(value_types.end - value_types.begin)};
        if(value_count == 0uz)
        {
            if constexpr(stacktop_enabled) { emit_drop_to_stack_size_no_fill(dst, target_base); }
            else
            {
                for(::std::size_t i{original_stack_size}; i-- > target_base;) { emit_drop_typed_to_no_fill(dst, operand_stack.index_unchecked(i).type); }
            }
            return;
        }

        if(value_count > internal_temp_local_offsets.size()) [[unlikely]] { ::fast_io::fast_terminate(); }

        for(::std::size_t i{value_count}; i-- != 0uz;)
        {
            emit_local_set_typed_to_no_fill(dst, value_types.begin[i], internal_temp_local_offsets.index_unchecked(i));
        }

        if constexpr(stacktop_enabled) { emit_drop_to_stack_size_no_fill(dst, target_base); }
        else
        {
            auto const first_preserved_index{original_stack_size - value_count};
            for(::std::size_t i{first_preserved_index}; i-- > target_base;) { emit_drop_typed_to_no_fill(dst, operand_stack.index_unchecked(i).type); }
        }

        if(fill_to_canonical_before_restore)
        {
            if constexpr(stacktop_enabled)
            {
                if constexpr(!strict_cf_entry_like_call) { stacktop_fill_to_canonical(dst); }
            }
        }

        for(::std::size_t i{}; i != value_count; ++i)
        {
            emit_local_get_typed_to(dst, value_types.begin[i], internal_temp_local_offsets.index_unchecked(i));
        }
    }};

// Branch emission records a relative-offset placeholder rather than computing it immediately because
// targets may be in thunks appended after main bytecode.
auto const emit_br_to{[&](bytecode_vec_t& dst, ::std::size_t label_id, bool dst_is_thunk) constexpr UWVM_THROWS
                      {
                          namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
                          if(runtime_log_on) [[unlikely]]
                          {
                              ++runtime_log_stats.cf_br_count;
                              if(runtime_log_emit_cf)
                              {
                                  ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                                       u8"[uwvm-int-translator] fn=",
                                                       function_index,
                                                       u8" ip=",
                                                       runtime_log_curr_ip,
                                                       u8" event=bytecode.emit.cf | op=br bc=",
                                                       runtime_log_bc_name(dst_is_thunk),
                                                       u8" off=",
                                                       dst.size(),
                                                       u8" label_id=",
                                                       label_id,
                                                       u8"\n");
                                                      }
                                                  }
                          local_spot_flush_dirty_all_and_invalidate_to(dst);
                          emit_opfunc_to(dst, translate::get_uwvmint_br_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                          emit_ptr_label_placeholder(label_id, dst_is_thunk);
                      }};

[[maybe_unused]] auto const emit_br_to_with_stacktop_transform{
    [&](bytecode_vec_t& dst, ::std::size_t label_id, bool dst_is_thunk) constexpr UWVM_THROWS
    {
#ifdef UWVM_ENABLE_UWVM_INT_COMBINE_OPS
        namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;

        if constexpr(stacktop_enabled && CompileOption.is_tail_call && stacktop_regtransform_cf_entry && stacktop_regtransform_supported)
        {
            if(!is_polymorphic && stacktop_cache_count != 0uz)
            {
                if(stacktop_currpos_is_begin())
                {
                    emit_br_to(dst, label_id, dst_is_thunk);
                    return;
                }

                if(runtime_log_on) [[unlikely]]
                {
                    ++runtime_log_stats.cf_br_transform_count;
                    if(runtime_log_emit_cf)
                    {
                        ::fast_io::io::print(::uwvm2::uwvm::io::u8runtime_log_output,
                                             u8"[uwvm-int-translator] fn=",
                                             function_index,
                                             u8" ip=",
                                             runtime_log_curr_ip,
                                             u8" event=bytecode.emit.cf | op=br_stacktop_transform_to_begin bc=",
                                             runtime_log_bc_name(dst_is_thunk),
                                             u8" off=",
                                             dst.size(),
                                             u8" label_id=",
                                             label_id,
                                             u8" cache=",
                                             stacktop_cache_count,
                                             u8"\n");
                    }
                }
                local_spot_flush_dirty_all_and_invalidate_to(dst);
                emit_opfunc_to(dst, translate::get_uwvmint_br_stacktop_transform_to_begin_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                emit_ptr_label_placeholder(label_id, dst_is_thunk);
                return;
            }
        }
#endif

        // Fallback: plain `br`.
        emit_br_to(dst, label_id, dst_is_thunk);
    }};

// Return is a call-like boundary for stack-top state: flush cached values before emitting the runtime
// return helper so the caller-frame handoff sees a materialized operand stack.
auto const emit_return_to{[&](bytecode_vec_t& dst) constexpr UWVM_THROWS
                          {
                              namespace translate = ::uwvm2::runtime::compiler::uwvm_int::optable::translate;
                              // Tail-call `return` requires flushing cached stack-top values back to the operand stack memory.
                              // See optable/control.h `uwvmint_return` contract.
                              stacktop_flush_all_to_operand_stack(dst);
                              emit_opfunc_to(dst, translate::get_uwvmint_return_fptr_from_tuple<CompileOption>(curr_stacktop, interpreter_tuple));
                          }};
