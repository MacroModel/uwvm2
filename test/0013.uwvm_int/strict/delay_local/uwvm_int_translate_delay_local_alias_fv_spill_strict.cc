#include "../uwvm_int_translate_strict_common.h"

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

    inline constexpr auto opt_tail_fv2{make_tailcall_logical_fv_opt<2uz>()};

    template <typename... T>
    [[nodiscard]] byte_vec pack_values(T... v)
    {
        byte_vec out((sizeof(T) + ... + 0uz));
        ::std::byte* p{out.data()};
        ((::std::memcpy(p, ::std::addressof(v), sizeof(T)), p += sizeof(T)), ...);
        return out;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ValType, typename ByteStorage>
    [[nodiscard]] bool contains_typed_spill1(ByteStorage const& bc) noexcept
    {
        if constexpr(optable::details::stacktop_range_begin_pos<Opt, ValType>() == optable::details::stacktop_range_end_pos<Opt, ValType>())
        {
            return false;
        }
        else
        {
            constexpr auto tuple =
                compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});

            for(::std::size_t start_pos{optable::details::stacktop_range_begin_pos<Opt, ValType>()};
                start_pos != optable::details::stacktop_range_end_pos<Opt, ValType>();
                ++start_pos)
            {
                auto const fptr =
                    optable::translate::get_uwvmint_stacktop_to_operand_stack_typed_single_fptr_from_tuple<Opt, ValType>(start_pos, tuple);
                if(bytecode_contains_fptr(bc, fptr)) { return true; }
            }

            return false;
        }
    }

    [[nodiscard]] byte_vec build_delay_local_alias_fv_spill_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };
        auto i32 = [&](byte_vec& c, ::std::int32_t v) { append_i32_leb(c, v); };

        // f0: local.tee overwrites local0 while an older local0 value is still buried below a full f32 window.
        {
            func_type ty{{k_val_i32, k_val_f32, k_val_f32, k_val_f32}, {k_val_f32}};
            func_body fb{};
            auto& c = fb.code;

            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 2u);
            op(c, wasm_op::local_get); u32(c, 3u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_const); i32(c, 5);
            op(c, wasm_op::local_tee); u32(c, 0u);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::f32_convert_i32_s);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f1: local.set overwrites local0 while an older local0 value is still buried below a full f64 window.
        {
            func_type ty{{k_val_i32, k_val_f64, k_val_f64, k_val_f64}, {k_val_f64}};
            func_body fb{};
            auto& c = fb.code;

            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 2u);
            op(c, wasm_op::local_get); u32(c, 3u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_const); i32(c, 7);
            op(c, wasm_op::local_set); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::f64_convert_i32_s);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f2: f32/f64 share the FV window while the overwritten i32 local is copied through another local.
        {
            func_type ty{{k_val_i32, k_val_f32, k_val_f64, k_val_f32, k_val_f64}, {k_val_f64}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i32});  // local5
            fb.locals.push_back({1u, k_val_f32});  // local6
            fb.locals.push_back({1u, k_val_f64});  // local7
            fb.locals.push_back({1u, k_val_f32});  // local8
            fb.locals.push_back({1u, k_val_f64});  // local9
            auto& c = fb.code;

            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 2u);
            op(c, wasm_op::local_get); u32(c, 3u);
            op(c, wasm_op::local_get); u32(c, 4u);

            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_const); i32(c, 3);
            op(c, wasm_op::local_tee); u32(c, 0u);
            op(c, wasm_op::local_set); u32(c, 5u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::local_set); u32(c, 5u);

            op(c, wasm_op::local_set); u32(c, 9u);
            op(c, wasm_op::local_set); u32(c, 8u);
            op(c, wasm_op::local_set); u32(c, 7u);
            op(c, wasm_op::local_set); u32(c, 6u);

            op(c, wasm_op::local_get); u32(c, 6u);
            op(c, wasm_op::f64_promote_f32);
            op(c, wasm_op::local_get); u32(c, 7u);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::local_get); u32(c, 8u);
            op(c, wasm_op::f64_promote_f32);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::local_get); u32(c, 9u);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::local_get); u32(c, 5u);
            op(c, wasm_op::f64_convert_i32_s);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    template <optable::uwvm_interpreter_translate_option_t Opt>
    [[nodiscard]] int run_suite(runtime_module_t const& rt, bool expect_fv_spill) noexcept
    {
        ::uwvm2::validation::error::code_validation_error_impl err{};
        optable::compile_option cop{};
        auto cm = compiler::compile_all_from_uwvm_single_func<Opt>(rt, cop, err);
        UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);
        UWVM2TEST_REQUIRE(cm.local_funcs.size() == 3uz);

        if(expect_fv_spill)
        {
            UWVM2TEST_REQUIRE((contains_typed_spill1<Opt, wasm_f32>(cm.local_funcs.index_unchecked(0).op.operands)));
            UWVM2TEST_REQUIRE((contains_typed_spill1<Opt, wasm_f64>(cm.local_funcs.index_unchecked(1).op.operands)));
            bool const mixed_spilled{contains_typed_spill1<Opt, wasm_f32>(cm.local_funcs.index_unchecked(2).op.operands) ||
                                     contains_typed_spill1<Opt, wasm_f64>(cm.local_funcs.index_unchecked(2).op.operands)};
            UWVM2TEST_REQUIRE(mixed_spilled);
        }

        using Runner = interpreter_runner<Opt>;

        {
            auto rr = Runner::run(cm.local_funcs.index_unchecked(0),
                                  rt.local_defined_function_vec_storage.index_unchecked(0),
                                  pack_values(static_cast<wasm_i32>(10), wasm_f32{1.25f}, wasm_f32{2.5f}, wasm_f32{3.75f}),
                                  nullptr,
                                  nullptr);
            UWVM2TEST_REQUIRE(load_f32(rr.results) == wasm_f32{22.5f});
        }

        {
            auto rr = Runner::run(cm.local_funcs.index_unchecked(1),
                                  rt.local_defined_function_vec_storage.index_unchecked(1),
                                  pack_values(static_cast<wasm_i32>(20), wasm_f64{1.25}, wasm_f64{2.5}, wasm_f64{4.25}),
                                  nullptr,
                                  nullptr);
            UWVM2TEST_REQUIRE(load_f64(rr.results) == wasm_f64{35.0});
        }

        {
            auto rr = Runner::run(cm.local_funcs.index_unchecked(2),
                                  rt.local_defined_function_vec_storage.index_unchecked(2),
                                  pack_values(static_cast<wasm_i32>(5), wasm_f32{1.5f}, wasm_f64{2.25}, wasm_f32{3.5f}, wasm_f64{4.75}),
                                  nullptr,
                                  nullptr);
            UWVM2TEST_REQUIRE(load_f64(rr.results) == wasm_f64{20.0});
        }

        return 0;
    }

    [[nodiscard]] int test_translate_delay_local_alias_fv_spill() noexcept
    {
        install_unexpected_traps();
        optable::call_func = strict_terminate_call;
        optable::call_indirect_func = strict_terminate_call_indirect;

        auto wasm = build_delay_local_alias_fv_spill_module();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_delay_local_alias_fv_spill");
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        if(abi_mode_enabled("byref"))
        {
            constexpr auto opt{k_test_byref_opt};
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, false) == 0);
        }

        if(abi_mode_enabled("tail-min"))
        {
            constexpr auto opt{k_test_tail_min_opt};
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, false) == 0);
        }

        if(abi_mode_enabled("tail-sysv"))
        {
            constexpr auto opt{k_test_tail_sysv_opt};
            static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, false) == 0);
        }

        if(abi_mode_enabled("tail-aapcs64"))
        {
            constexpr auto opt{k_test_tail_aapcs64_opt};
            static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, false) == 0);
        }

        static_assert(opt_tail_fv2.i32_stack_top_begin_pos == SIZE_MAX && opt_tail_fv2.i32_stack_top_end_pos == SIZE_MAX);
        static_assert(opt_tail_fv2.i64_stack_top_begin_pos == SIZE_MAX && opt_tail_fv2.i64_stack_top_end_pos == SIZE_MAX);
        static_assert(opt_tail_fv2.f32_stack_top_begin_pos == 0uz);
        static_assert(opt_tail_fv2.f32_stack_top_end_pos == 2uz);
        static_assert(opt_tail_fv2.f64_stack_top_begin_pos == 0uz);
        static_assert(opt_tail_fv2.f64_stack_top_end_pos == 2uz);
        static_assert(opt_tail_fv2.v128_stack_top_begin_pos == 0uz);
        static_assert(opt_tail_fv2.v128_stack_top_end_pos == 2uz);
        static_assert(compiler::details::interpreter_tuple_has_no_holes<opt_tail_fv2>());
        UWVM2TEST_REQUIRE(run_suite<opt_tail_fv2>(rt, true) == 0);

        return 0;
    }
}  // namespace

int main()
{
    return test_translate_delay_local_alias_fv_spill();
}
