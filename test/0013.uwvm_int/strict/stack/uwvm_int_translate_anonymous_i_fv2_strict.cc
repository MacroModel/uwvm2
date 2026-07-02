#include <bit>

#include "../uwvm_int_translate_strict_common.h"

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
    using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
    using wasm_v128 = ::uwvm2::parser::wasm::standard::wasm1p1::type::wasm_v128;

    inline constexpr auto opt{make_tailcall_logical_fv_opt<2uz>()};

    static_assert(opt.i32_stack_top_begin_pos == SIZE_MAX && opt.i32_stack_top_end_pos == SIZE_MAX);
    static_assert(opt.i64_stack_top_begin_pos == SIZE_MAX && opt.i64_stack_top_end_pos == SIZE_MAX);
    static_assert(opt.f32_stack_top_begin_pos == 0uz);
    static_assert(opt.f32_stack_top_end_pos == 2uz);
    static_assert(opt.f64_stack_top_begin_pos == 0uz);
    static_assert(opt.f64_stack_top_end_pos == 2uz);
    static_assert(opt.v128_stack_top_begin_pos == 0uz);
    static_assert(opt.v128_stack_top_end_pos == 2uz);
    static_assert(compiler::details::interpreter_tuple_size<opt>() == 4uz);
    static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());
    static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<0uz, opt>, ::std::byte const*>);
    static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<1uz, opt>, ::std::byte*>);
    static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<2uz, opt>, wasm_v128>);
    static_assert(::std::same_as<compiler::details::interpreter_tuple_arg_t<3uz, opt>, wasm_v128>);

    template <typename... T>
    [[nodiscard]] byte_vec pack_values(T... v)
    {
        byte_vec out((sizeof(T) + ... + 0uz));
        ::std::byte* p{out.data()};
        ((::std::memcpy(p, ::std::addressof(v), sizeof(T)), p += sizeof(T)), ...);
        return out;
    }

    template <typename ByteStorage, typename MakeFptr>
    [[nodiscard]] bool bytecode_contains_f64_variant(ByteStorage const& bc, MakeFptr&& make_fptr) noexcept
    {
        auto curr = make_initial_stacktop_currpos<opt>();
        if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }

        for(::std::size_t pos{opt.f64_stack_top_begin_pos}; pos != opt.f64_stack_top_end_pos; ++pos)
        {
            curr.f64_stack_top_curr_pos = pos;
            curr.v128_stack_top_curr_pos = pos;
            if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }
        }

        return false;
    }

    [[nodiscard]] byte_vec build_anonymous_i_fv2_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };
        auto i32 = [&](byte_vec& c, ::std::int32_t v) { append_i32_leb(c, v); };
        auto i64 = [&](byte_vec& c, ::std::int64_t v) { append_i64_leb(c, v); };
        auto f64 = [&](byte_vec& c, double v) { append_f64_ieee(c, v); };

        // f0: ((a + b) * 3) keeps integer values off the FV stack-top window.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::i32_const); i32(c, 3);
            op(c, wasm_op::i32_mul);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f1: ((a + b) * 2.0) should live in the two-slot FV stack-top window.
        {
            func_type ty{{k_val_f64, k_val_f64}, {k_val_f64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::f64_const); f64(c, 2.0);
            op(c, wasm_op::f64_mul);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f2: (a * b) + f64.convert_i32_s(i) mixes anonymous integer state with FV state.
        {
            func_type ty{{k_val_i32, k_val_f64, k_val_f64}, {k_val_f64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 2u);
            op(c, wasm_op::f64_mul);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::f64_convert_i32_s);
            op(c, wasm_op::f64_add);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f3: ((x + y) ^ 17) checks the i64 anonymous/memory binary fallback.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_add);
            op(c, wasm_op::i64_const); i64(c, 17);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f4: i32.clz(a) + i32.ctz(b) intentionally avoids delayed local/const RHS fusions.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i32_clz);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_ctz);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f5: i64.clz(a) + i64.ctz(b) covers the same anonymous fallback for i64.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::i64_clz);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_ctz);
            op(c, wasm_op::i64_add);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f6: two independent i32 2-local bitops should both use the no-int-stack combine path.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_and);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_or);
            op(c, wasm_op::i32_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f7: direct i32 xor 2-local combine.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f8: i64 sub/mul 2-local combines.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_sub);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_mul);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f9: i64 and/or 2-local combines.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_and);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_or);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f10: direct i64 xor 2-local combine.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f11: i64 shl/shr_u 2-local combines.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_shl);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_shr_u);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f12: i64 shr_s 2-local combine.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_shr_s);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f13: i64 rotl/rotr 2-local combines.
        {
            func_type ty{{k_val_i64, k_val_i64}, {k_val_i64}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_rotl);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_rotr);
            op(c, wasm_op::i64_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f14: i32 shl/shr_u 2-local combines.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_shl);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_shr_u);
            op(c, wasm_op::i32_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f15: i32 shr_s 2-local combine.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_shr_s);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f16: i32 rotl/rotr 2-local combines.
        {
            func_type ty{{k_val_i32, k_val_i32}, {k_val_i32}};
            func_body fb{};
            auto& c = fb.code;
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_rotl);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_rotr);
            op(c, wasm_op::i32_xor);
            op(c, wasm_op::end);
            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    [[nodiscard]] int check_bytecode(compiled_module_t const& cm) noexcept
    {
        constexpr auto tuple =
            compiler::details::make_interpreter_tuple<opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<opt>()>{});

        auto const curr = make_initial_stacktop_currpos<opt>();
        auto const exp_i32_add_2local = optable::translate::get_uwvmint_i32_add_2localget_fptr_from_tuple<opt>(curr, tuple);
        auto const exp_i32_and_2local = optable::translate::get_uwvmint_i32_and_2localget_fptr_from_tuple<opt>(curr, tuple);
        auto const exp_i32_or_2local = optable::translate::get_uwvmint_i32_or_2localget_fptr_from_tuple<opt>(curr, tuple);
        auto const exp_i32_xor_2local = optable::translate::get_uwvmint_i32_xor_2localget_fptr_from_tuple<opt>(curr, tuple);
        auto const exp_i32_shl_2local = optable::translate::get_uwvmint_i32_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shl>(curr, tuple);
        auto const exp_i32_shr_s_2local = optable::translate::get_uwvmint_i32_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shr_s>(curr, tuple);
        auto const exp_i32_shr_u_2local = optable::translate::get_uwvmint_i32_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shr_u>(curr, tuple);
        auto const exp_i32_rotl_2local = optable::translate::get_uwvmint_i32_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::rotl>(curr, tuple);
        auto const exp_i32_rotr_2local = optable::translate::get_uwvmint_i32_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::rotr>(curr, tuple);
        auto const exp_i64_add_2local = optable::translate::get_uwvmint_i64_add_2localget_fptr_from_tuple<opt>(curr, tuple);
        auto const exp_i64_sub_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::sub>(curr, tuple);
        auto const exp_i64_mul_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::mul>(curr, tuple);
        auto const exp_i64_and_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::and_>(curr, tuple);
        auto const exp_i64_or_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::or_>(curr, tuple);
        auto const exp_i64_xor_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::xor_>(curr, tuple);
        auto const exp_i64_shl_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shl>(curr, tuple);
        auto const exp_i64_shr_s_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shr_s>(curr, tuple);
        auto const exp_i64_shr_u_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::shr_u>(curr, tuple);
        auto const exp_i64_rotl_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::rotl>(curr, tuple);
        auto const exp_i64_rotr_2local = optable::translate::get_uwvmint_i64_binop_2localget_fptr_from_tuple<
            opt,
            optable::numeric_details::int_binop::rotr>(curr, tuple);

        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(0).op.operands, exp_i32_add_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(3).op.operands, exp_i64_add_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_f64_variant(
            cm.local_funcs.index_unchecked(2).op.operands,
            [&](auto const& curr_variant) constexpr noexcept
            { return optable::translate::get_uwvmint_f64_from_i32_s_fptr_from_tuple<opt>(curr_variant, tuple); }));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(6).op.operands, exp_i32_and_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(6).op.operands, exp_i32_or_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(7).op.operands, exp_i32_xor_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(8).op.operands, exp_i64_sub_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(8).op.operands, exp_i64_mul_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(9).op.operands, exp_i64_and_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(9).op.operands, exp_i64_or_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(10).op.operands, exp_i64_xor_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(11).op.operands, exp_i64_shl_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(11).op.operands, exp_i64_shr_u_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(12).op.operands, exp_i64_shr_s_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(13).op.operands, exp_i64_rotl_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(13).op.operands, exp_i64_rotr_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(14).op.operands, exp_i32_shl_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(14).op.operands, exp_i32_shr_u_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(15).op.operands, exp_i32_shr_s_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(16).op.operands, exp_i32_rotl_2local));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm.local_funcs.index_unchecked(16).op.operands, exp_i32_rotr_2local));
        return 0;
    }

    [[nodiscard]] int run_test() noexcept
    {
        install_unexpected_traps();
        optable::call_func = strict_terminate_call;
        optable::call_indirect_func = strict_terminate_call_indirect;

        auto wasm = build_anonymous_i_fv2_module();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_anonymous_i_fv2");
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        ::uwvm2::validation::error::code_validation_error_impl err{};
        optable::compile_option cop{};
        auto cm = compiler::compile_all_from_uwvm_single_func<opt>(rt, cop, err);
        UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);
        UWVM2TEST_REQUIRE(cm.local_funcs.size() == 17uz);
        UWVM2TEST_REQUIRE(check_bytecode(cm) == 0);

        using Runner = interpreter_runner<opt>;

        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(0),
                                              rt.local_defined_function_vec_storage.index_unchecked(0),
                                              pack_values(static_cast<wasm_i32>(7), static_cast<wasm_i32>(5)),
                                              nullptr,
                                              nullptr)
                                       .results) == 36);

        UWVM2TEST_REQUIRE(load_f64(Runner::run(cm.local_funcs.index_unchecked(1),
                                              rt.local_defined_function_vec_storage.index_unchecked(1),
                                              pack_values(1.5, 2.5),
                                              nullptr,
                                              nullptr)
                                       .results) == 8.0);

        UWVM2TEST_REQUIRE(load_f64(Runner::run(cm.local_funcs.index_unchecked(2),
                                              rt.local_defined_function_vec_storage.index_unchecked(2),
                                              pack_values(static_cast<wasm_i32>(3), 2.0, 4.0),
                                              nullptr,
                                              nullptr)
                                       .results) == 11.0);

        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(3),
                                              rt.local_defined_function_vec_storage.index_unchecked(3),
                                              pack_values(static_cast<wasm_i64>(1000), static_cast<wasm_i64>(24)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i64>((1000 + 24) ^ 17));

        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(4),
                                              rt.local_defined_function_vec_storage.index_unchecked(4),
                                              pack_values(static_cast<wasm_i32>(16), static_cast<wasm_i32>(32)),
                                              nullptr,
                                              nullptr)
                                       .results) == 32);

        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(5),
                                              rt.local_defined_function_vec_storage.index_unchecked(5),
                                              pack_values(static_cast<wasm_i64>(16), static_cast<wasm_i64>(32)),
                                              nullptr,
                                              nullptr)
                                       .results) == 64);

        {
            wasm_i32 const a{0x0f0f};
            wasm_i32 const b{0x00ff};
            wasm_i32 const expected{static_cast<wasm_i32>((static_cast<::std::uint32_t>(a) & static_cast<::std::uint32_t>(b)) ^
                                                          (static_cast<::std::uint32_t>(a) | static_cast<::std::uint32_t>(b)))};
            UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(6),
                                                  rt.local_defined_function_vec_storage.index_unchecked(6),
                                                  pack_values(a, b),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(7),
                                              rt.local_defined_function_vec_storage.index_unchecked(7),
                                              pack_values(static_cast<wasm_i32>(0x12345678), static_cast<wasm_i32>(0x00ff00ff)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i32>(0x12cb5687));

        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(8),
                                              rt.local_defined_function_vec_storage.index_unchecked(8),
                                              pack_values(static_cast<wasm_i64>(11), static_cast<wasm_i64>(3)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i64>((11 - 3) ^ (11 * 3)));

        {
            wasm_i64 const a{0x0f0f0f0f0f0f0f0fLL};
            wasm_i64 const b{0x00ff00ff00ff00ffLL};
            wasm_i64 const expected{static_cast<wasm_i64>((static_cast<::std::uint64_t>(a) & static_cast<::std::uint64_t>(b)) ^
                                                          (static_cast<::std::uint64_t>(a) | static_cast<::std::uint64_t>(b)))};
            UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(9),
                                                  rt.local_defined_function_vec_storage.index_unchecked(9),
                                                  pack_values(a, b),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(10),
                                              rt.local_defined_function_vec_storage.index_unchecked(10),
                                              pack_values(static_cast<wasm_i64>(0x123456789abcdef0LL), static_cast<wasm_i64>(0x00ff00ff00ff00ffLL)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i64>(0x12cb56879a43de0fLL));

        {
            ::std::uint64_t const x{0x123456789abcdef0ULL};
            ::std::uint64_t const sh{4u};
            wasm_i64 const expected{static_cast<wasm_i64>((x << sh) ^ (x >> sh))};
            UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(11),
                                                  rt.local_defined_function_vec_storage.index_unchecked(11),
                                                  pack_values(static_cast<wasm_i64>(x), static_cast<wasm_i64>(sh)),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(12),
                                              rt.local_defined_function_vec_storage.index_unchecked(12),
                                              pack_values(static_cast<wasm_i64>(64), static_cast<wasm_i64>(3)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i64>(8));

        {
            ::std::uint64_t const x{0x123456789abcdef0ULL};
            unsigned const sh{9u};
            wasm_i64 const expected{static_cast<wasm_i64>(::std::rotl(x, static_cast<int>(sh)) ^ ::std::rotr(x, static_cast<int>(sh)))};
            UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(13),
                                                  rt.local_defined_function_vec_storage.index_unchecked(13),
                                                  pack_values(static_cast<wasm_i64>(x), static_cast<wasm_i64>(sh)),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        {
            ::std::uint32_t const x{0x12345678U};
            ::std::uint32_t const sh{5u};
            wasm_i32 const expected{static_cast<wasm_i32>((x << sh) ^ (x >> sh))};
            UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(14),
                                                  rt.local_defined_function_vec_storage.index_unchecked(14),
                                                  pack_values(static_cast<wasm_i32>(x), static_cast<wasm_i32>(sh)),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(15),
                                              rt.local_defined_function_vec_storage.index_unchecked(15),
                                              pack_values(static_cast<wasm_i32>(64), static_cast<wasm_i32>(3)),
                                              nullptr,
                                              nullptr)
                                       .results) == static_cast<wasm_i32>(8));

        {
            ::std::uint32_t const x{0x12345678U};
            unsigned const sh{7u};
            wasm_i32 const expected{static_cast<wasm_i32>(::std::rotl(x, static_cast<int>(sh)) ^ ::std::rotr(x, static_cast<int>(sh)))};
            UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(16),
                                                  rt.local_defined_function_vec_storage.index_unchecked(16),
                                                  pack_values(static_cast<wasm_i32>(x), static_cast<wasm_i32>(sh)),
                                                  nullptr,
                                                  nullptr)
                                           .results) == expected);
        }

        return 0;
    }
}  // namespace

int main()
{
    try
    {
        return run_test();
    }
    catch(...)
    {
        return ::uwvm2test::uwvm_int_strict::fail(__LINE__, "uncaught exception");
    }
}
