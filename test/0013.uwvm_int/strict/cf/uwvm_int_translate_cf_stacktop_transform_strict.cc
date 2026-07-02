#include "../uwvm_int_translate_strict_common.h"

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    [[nodiscard]] byte_vec build_cf_stacktop_transform_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };
        auto i32 = [&](byte_vec& c, ::std::int32_t v) { append_i32_leb(c, v); };
        auto f32 = [&](byte_vec& c, float v) { append_f32_ieee(c, v); };

        // f0: (param i32) (result i32) -> 123 (for param!=0)
        // Contains an unreachable-at-runtime `br 0` that exercises control-flow edge repair when stacktop
        // caching is enabled. New strict control-flow mode canonicalizes loop entries to memory instead of
        // requiring the old branch-local transform opfunc.
        {
            func_type ty{{k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;

            // Keep one FV-cached stack value across the loop so the branch sees stacktop_cache_count != 0.
            op(c, wasm_op::f32_const); f32(c, 7.0f);

            op(c, wasm_op::block); append_u8(c, k_block_empty);
            op(c, wasm_op::loop); append_u8(c, k_block_empty);

            // if (param != 0) break out of loop+block
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::br_if); u32(c, 1u);

            // else path: never execute in test (we pass param=1), but it must compile.
            op(c, wasm_op::nop);
            op(c, wasm_op::br); u32(c, 0u);

            op(c, wasm_op::end);  // end loop
            op(c, wasm_op::end);  // end block

            // Drop carried value; return 123
            op(c, wasm_op::drop);
            op(c, wasm_op::i32_const); i32(c, 123);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    [[nodiscard]] int test_translate_cf_stacktop_transform() noexcept
    {
        ::uwvm2test::uwvm_int_strict::install_unexpected_traps();
        optable::call_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call;
        optable::call_indirect_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call_indirect;

        auto wasm = build_cf_stacktop_transform_module();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_cf_stacktop_transform");
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        // Mode A: byref semantics smoke
        {
            constexpr optable::uwvm_interpreter_translate_option_t opt{.is_tail_call = false};
            ::uwvm2::validation::error::code_validation_error_impl err{};
            optable::compile_option cop{};
            auto cm = compiler::compile_all_from_uwvm_single_func<opt>(rt, cop, err);
            UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);

            using Runner = interpreter_runner<opt>;
            auto rr = Runner::run(cm.local_funcs.index_unchecked(0),
                                  rt.local_defined_function_vec_storage.index_unchecked(0),
                                  pack_i32(1),
                                  nullptr,
                                  nullptr);
            UWVM2TEST_REQUIRE(load_i32(rr.results) == 123);
        }

        // Mode C: tailcall + stacktop caching (merged rings)
        {
            constexpr optable::uwvm_interpreter_translate_option_t opt{
                .is_tail_call = true,
                .i32_stack_top_begin_pos = SIZE_MAX,
                .i32_stack_top_end_pos = SIZE_MAX,
                .i64_stack_top_begin_pos = SIZE_MAX,
                .i64_stack_top_end_pos = SIZE_MAX,
                .f32_stack_top_begin_pos = 0uz,
                .f32_stack_top_end_pos = 2uz,
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

            using Runner = interpreter_runner<opt>;
            auto rr = Runner::run(cm.local_funcs.index_unchecked(0),
                                  rt.local_defined_function_vec_storage.index_unchecked(0),
                                  pack_i32(1),
                                  nullptr,
                                  nullptr);
            UWVM2TEST_REQUIRE(load_i32(rr.results) == 123);
        }

        return 0;
    }
}  // namespace

int main()
{
    return test_translate_cf_stacktop_transform();
}
