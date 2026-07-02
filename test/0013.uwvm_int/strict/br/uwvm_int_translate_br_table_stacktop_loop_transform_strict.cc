#include "../uwvm_int_translate_strict_common.h"

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    [[nodiscard]] byte_vec build_br_table_loop_transform_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };
        auto i32 = [&](byte_vec& c, ::std::int32_t v) { append_i32_leb(c, v); };

        // f0: (param i32 n) -> (result i32)
        //
        // Keep a cached f32 value on the operand stack across a loop, and use `br_table` as a loop back-edge.
        // With stacktop caching enabled, this triggers:
        //   target_frame.type == loop && stacktop_cache_count != 0
        //
        // Always returns 42.
        {
            func_type ty{{k_val_i32}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i32});  // localidx=1: saved selector
            auto& c = fb.code;

            // Keep one fp-cached value across loop re-entry so stacktop_cache_count != 0 after popping the i32 selector.
            op(c, wasm_op::f32_const);
            append_f32_ieee(c, 1.0f);

            op(c, wasm_op::block);
            append_u8(c, k_block_empty);

            op(c, wasm_op::loop);
            append_u8(c, k_block_empty);

            // selector = (n == 0) ? 1 : 0
            op(c, wasm_op::local_get);
            u32(c, 0u);
            op(c, wasm_op::i32_eqz);
            op(c, wasm_op::local_set);
            u32(c, 1u);

            // n = n - 1
            op(c, wasm_op::local_get);
            u32(c, 0u);
            op(c, wasm_op::i32_const);
            i32(c, 1);
            op(c, wasm_op::i32_sub);
            op(c, wasm_op::local_set);
            u32(c, 0u);

            // br_table [0(loop), 1(block)] default=0
            op(c, wasm_op::local_get);
            u32(c, 1u);  // selector

            op(c, wasm_op::br_table);
            u32(c, 2u);  // vec length
            u32(c, 0u);  // continue loop
            u32(c, 1u);  // break outer block
            u32(c, 0u);  // default continue

            op(c, wasm_op::end);  // end loop
            op(c, wasm_op::end);  // end block

            // drop carried f32; return 42
            op(c, wasm_op::drop);
            op(c, wasm_op::i32_const);
            i32(c, 42);
            op(c, wasm_op::end);  // end func

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    [[nodiscard]] int test_translate_br_table_stacktop_loop_transform() noexcept
    {
        ::uwvm2test::uwvm_int_strict::install_unexpected_traps();
        optable::call_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call;
        optable::call_indirect_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call_indirect;

        auto wasm = build_br_table_loop_transform_module();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_br_table_loop_transform");
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        // Mode A: tailcall (no caching) semantics smoke.
        {
            constexpr optable::uwvm_interpreter_translate_option_t opt{.is_tail_call = true};
            ::uwvm2::validation::error::code_validation_error_impl err{};
            optable::compile_option cop{};
            auto cm = compiler::compile_all_from_uwvm_single_func<opt>(rt, cop, err);
            UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);

            using Runner = interpreter_runner<opt>;
            for(::std::int32_t n : {0, 1, 2, 3, 7})
            {
                auto rr = Runner::run(cm.local_funcs.index_unchecked(0),
                                      rt.local_defined_function_vec_storage.index_unchecked(0),
                                      pack_i32(n),
                                      nullptr,
                                      nullptr);
                auto const got{load_i32(rr.results)};
                if(got != 42)
                {
                    ::std::fprintf(stderr, "br_table_stacktop_loop_transform: n=%d got=%d expected=42\n", static_cast<int>(n), static_cast<int>(got));
                    return fail(__LINE__, "unexpected br_table loop result");
                }
            }
        }

        // Mode B: tailcall + stacktop caching (i32 stack-top window). Cover br_table's loop-target transform thunk path.
        {
            constexpr optable::uwvm_interpreter_translate_option_t opt{
                .is_tail_call = true,
                // translate.h requires all scalar ranges enabled together.
                .i32_stack_top_begin_pos = SIZE_MAX,
                .i32_stack_top_end_pos = SIZE_MAX,  // int stack-top window size=2: enough for (value,selector)
                .i64_stack_top_begin_pos = SIZE_MAX,
                .i64_stack_top_end_pos = SIZE_MAX,
                .f32_stack_top_begin_pos = 0uz,
                .f32_stack_top_end_pos = 2uz,  // fp stack-top window size=2 (unused here, but required by contract)
                .f64_stack_top_begin_pos = 0uz,
                .f64_stack_top_end_pos = 2uz,
                .v128_stack_top_begin_pos = 0uz,
                .v128_stack_top_end_pos = 2uz,
            };
            static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());

            ::uwvm2::validation::error::code_validation_error_impl err{};
            optable::compile_option cop{};
            auto cm = compiler::compile_all_from_uwvm_single_func<opt>(rt, cop, err);
            UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);

#if defined(UWVM_ENABLE_UWVM_INT_COMBINE_OPS)
            constexpr auto tuple =
                compiler::details::make_interpreter_tuple<opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<opt>()>{});

            constexpr optable::uwvm_interpreter_stacktop_currpos_t c0{
                .i32_stack_top_curr_pos = SIZE_MAX,
                .i64_stack_top_curr_pos = SIZE_MAX,
                .f32_stack_top_curr_pos = 0uz,
                .f64_stack_top_curr_pos = 0uz,
                .v128_stack_top_curr_pos = 0uz,
            };
            constexpr optable::uwvm_interpreter_stacktop_currpos_t c1{
                .i32_stack_top_curr_pos = SIZE_MAX,
                .i64_stack_top_curr_pos = SIZE_MAX,
                .f32_stack_top_curr_pos = 1uz,
                .f64_stack_top_curr_pos = 1uz,
                .v128_stack_top_curr_pos = 1uz,
            };

            constexpr auto f0 = optable::translate::get_uwvmint_br_stacktop_transform_to_begin_fptr_from_tuple<opt>(c0, tuple);
            constexpr auto f1 = optable::translate::get_uwvmint_br_stacktop_transform_to_begin_fptr_from_tuple<opt>(c1, tuple);

            [[maybe_unused]] bool const has_transform{bytecode_contains_fptr(cm.local_funcs.index_unchecked(0).op.operands, f0) ||
                                                       bytecode_contains_fptr(cm.local_funcs.index_unchecked(0).op.operands, f1)};
#endif

            using Runner = interpreter_runner<opt>;
            for(::std::int32_t n : {0, 1, 2, 3, 7})
            {
                auto rr = Runner::run(cm.local_funcs.index_unchecked(0),
                                      rt.local_defined_function_vec_storage.index_unchecked(0),
                                      pack_i32(n),
                                      nullptr,
                                      nullptr);
                auto const got{load_i32(rr.results)};
                if(got != 42)
                {
                    ::std::fprintf(stderr, "br_table_stacktop_loop_transform_fv: n=%d got=%d expected=42\n", static_cast<int>(n), static_cast<int>(got));
                    return fail(__LINE__, "unexpected br_table loop result");
                }
            }
        }

        return 0;
    }
}  // namespace

int main()
{
    return test_translate_br_table_stacktop_loop_transform();
}
