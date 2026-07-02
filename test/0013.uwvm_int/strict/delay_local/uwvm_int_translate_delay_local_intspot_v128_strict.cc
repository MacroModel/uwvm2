#include "../uwvm_int_translate_strict_common.h"

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    constexpr ::std::uint8_t k_val_v128 = 0x7bu;

    inline void simd(byte_vec& c, wasm1p1_simd_op o)
    {
        append_u8(c, u8(wasm1p1_op::simd_prefix));
        append_u32_leb(c, u32(o));
    }

    inline void append_u32_le(byte_vec& c, ::std::uint32_t v)
    {
        append_u8(c, static_cast<::std::uint8_t>(v & 0xffu));
        append_u8(c, static_cast<::std::uint8_t>((v >> 8u) & 0xffu));
        append_u8(c, static_cast<::std::uint8_t>((v >> 16u) & 0xffu));
        append_u8(c, static_cast<::std::uint8_t>((v >> 24u) & 0xffu));
    }

    inline void v128_i32x4(byte_vec& c, ::std::uint32_t a, ::std::uint32_t b, ::std::uint32_t d, ::std::uint32_t e)
    {
        simd(c, wasm1p1_simd_op::v128_const);
        append_u32_le(c, a);
        append_u32_le(c, b);
        append_u32_le(c, d);
        append_u32_le(c, e);
    }

    [[nodiscard]] byte_vec build_delay_local_intspot_v128_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };
        auto i32 = [&](byte_vec& c, ::std::int32_t v) { append_i32_leb(c, v); };
        auto i64 = [&](byte_vec& c, ::std::int64_t v) { append_i64_leb(c, v); };

        {
            func_type ty{{k_val_i32}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i32});
            auto& c = fb.code;

            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_set); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i32_const); i32(c, 7);
            op(c, wasm_op::i32_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        {
            func_type ty{{k_val_i64}, {k_val_i64}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i64});
            auto& c = fb.code;

            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_set); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::i64_const); i64(c, 11);
            op(c, wasm_op::i64_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        {
            func_type ty{{}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_v128});
            fb.locals.push_back({1u, k_val_v128});
            auto& c = fb.code;

            v128_i32x4(c, 0x10203040u, 0x11111111u, 0x22222222u, 0x33333333u);
            op(c, wasm_op::local_set); u32(c, 0u);
            v128_i32x4(c, 0x01010101u, 0x55555555u, 0x44444444u, 0x77777777u);
            op(c, wasm_op::local_set); u32(c, 1u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_get); u32(c, 1u);
            simd(c, wasm1p1_simd_op::v128_xor);
            simd(c, wasm1p1_simd_op::i32x4_extract_lane);
            append_u8(c, 0u);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        {
            func_type ty{{}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_v128});
            auto& c = fb.code;

            v128_i32x4(c, 0x01010101u, 0x22222222u, 0x33333333u, 0x44444444u);
            op(c, wasm_op::local_set); u32(c, 0u);
            v128_i32x4(c, 0xf0f0f0f0u, 0xaaaaaaaau, 0xbbbbbbbbu, 0xccccccccu);
            op(c, wasm_op::local_get); u32(c, 0u);
            simd(c, wasm1p1_simd_op::v128_xor);
            simd(c, wasm1p1_simd_op::i32x4_extract_lane);
            append_u8(c, 0u);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage, typename MakeFptr>
    [[nodiscard]] bool bytecode_contains_i32_cache_variant(ByteStorage const& bc, MakeFptr&& make_fptr) noexcept
    {
        auto curr{make_entry_stacktop_currpos<Opt>()};
        if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }

        constexpr ::std::size_t begin{Opt.i32_stack_spot_begin_pos != SIZE_MAX ? Opt.i32_stack_spot_begin_pos : Opt.i32_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i32_stack_spot_end_pos != SIZE_MAX ? Opt.i32_stack_spot_end_pos : Opt.i32_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t pos{begin}; pos < end; ++pos)
            {
                curr.i32_stack_top_curr_pos = pos;
                if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }
            }
        }

        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage, typename MakeFptr>
    [[nodiscard]] bool bytecode_contains_i64_cache_variant(ByteStorage const& bc, MakeFptr&& make_fptr) noexcept
    {
        auto curr{make_entry_stacktop_currpos<Opt>()};
        if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }

        constexpr ::std::size_t begin{Opt.i64_stack_spot_begin_pos != SIZE_MAX ? Opt.i64_stack_spot_begin_pos : Opt.i64_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i64_stack_spot_end_pos != SIZE_MAX ? Opt.i64_stack_spot_end_pos : Opt.i64_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t pos{begin}; pos < end; ++pos)
            {
                curr.i64_stack_top_curr_pos = pos;
                if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }
            }
        }

        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage>
    [[nodiscard]] bool bytecode_contains_i32_local_spot_get(ByteStorage const& bc) noexcept
    {
        using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
        constexpr auto tuple =
            compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});
        constexpr ::std::size_t begin{Opt.i32_stack_spot_begin_pos != SIZE_MAX ? Opt.i32_stack_spot_begin_pos : Opt.i32_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i32_stack_spot_end_pos != SIZE_MAX ? Opt.i32_stack_spot_end_pos : Opt.i32_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t src{begin}; src < end; ++src)
            {
                auto curr{make_entry_stacktop_currpos<Opt>()};
                if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<Opt, wasm_i32>(src, curr, tuple)))
                {
                    return true;
                }
                for(::std::size_t pos{begin}; pos < end; ++pos)
                {
                    curr.i32_stack_top_curr_pos = pos;
                    if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<Opt, wasm_i32>(src, curr, tuple)))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage>
    [[nodiscard]] bool bytecode_contains_i32_local_spot_flush(ByteStorage const& bc) noexcept
    {
        using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
        constexpr auto tuple =
            compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});
        constexpr ::std::size_t begin{Opt.i32_stack_spot_begin_pos != SIZE_MAX ? Opt.i32_stack_spot_begin_pos : Opt.i32_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i32_stack_spot_end_pos != SIZE_MAX ? Opt.i32_stack_spot_end_pos : Opt.i32_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t src{begin}; src < end; ++src)
            {
                auto curr{make_entry_stacktop_currpos<Opt>()};
                if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<Opt, wasm_i32>(src, curr, tuple)))
                {
                    return true;
                }
                for(::std::size_t pos{begin}; pos < end; ++pos)
                {
                    curr.i32_stack_top_curr_pos = pos;
                    if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<Opt, wasm_i32>(src, curr, tuple)))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage>
    [[nodiscard]] bool bytecode_contains_i64_local_spot_get(ByteStorage const& bc) noexcept
    {
        using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
        constexpr auto tuple =
            compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});
        constexpr ::std::size_t begin{Opt.i64_stack_spot_begin_pos != SIZE_MAX ? Opt.i64_stack_spot_begin_pos : Opt.i64_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i64_stack_spot_end_pos != SIZE_MAX ? Opt.i64_stack_spot_end_pos : Opt.i64_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t src{begin}; src < end; ++src)
            {
                auto curr{make_entry_stacktop_currpos<Opt>()};
                if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<Opt, wasm_i64>(src, curr, tuple)))
                {
                    return true;
                }
                for(::std::size_t pos{begin}; pos < end; ++pos)
                {
                    curr.i64_stack_top_curr_pos = pos;
                    if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_get_spot_typed_fptr_from_tuple<Opt, wasm_i64>(src, curr, tuple)))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage>
    [[nodiscard]] bool bytecode_contains_i64_local_spot_flush(ByteStorage const& bc) noexcept
    {
        using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
        constexpr auto tuple =
            compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});
        constexpr ::std::size_t begin{Opt.i64_stack_spot_begin_pos != SIZE_MAX ? Opt.i64_stack_spot_begin_pos : Opt.i64_stack_top_begin_pos};
        constexpr ::std::size_t end{Opt.i64_stack_spot_end_pos != SIZE_MAX ? Opt.i64_stack_spot_end_pos : Opt.i64_stack_top_end_pos};
        if constexpr(begin != SIZE_MAX && begin != end)
        {
            for(::std::size_t src{begin}; src < end; ++src)
            {
                auto curr{make_entry_stacktop_currpos<Opt>()};
                if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<Opt, wasm_i64>(src, curr, tuple)))
                {
                    return true;
                }
                for(::std::size_t pos{begin}; pos < end; ++pos)
                {
                    curr.i64_stack_top_curr_pos = pos;
                    if(bytecode_contains_fptr(bc, optable::translate::get_uwvmint_local_spot_flush_typed_fptr_from_tuple<Opt, wasm_i64>(src, curr, tuple)))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt, typename ByteStorage, typename MakeFptr>
    [[nodiscard]] bool bytecode_contains_v128_cache_variant(ByteStorage const& bc, MakeFptr&& make_fptr) noexcept
    {
        auto curr{make_entry_stacktop_currpos<Opt>()};
        if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }

        if constexpr(Opt.v128_stack_top_begin_pos != SIZE_MAX && Opt.v128_stack_top_begin_pos != Opt.v128_stack_top_end_pos)
        {
            for(::std::size_t pos{Opt.v128_stack_top_begin_pos}; pos < Opt.v128_stack_top_end_pos; ++pos)
            {
                curr.v128_stack_top_curr_pos = pos;
                if(bytecode_contains_fptr(bc, make_fptr(curr))) { return true; }
            }
        }

        return false;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt>
    [[nodiscard]] int run_suite(runtime_module_t const& rt, wasm_feature_parameter_t const& features) noexcept
    {
        if(::std::getenv("UWVM2TEST_DEBUG_INTSLOT_V128_LOG") != nullptr)
        {
            ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"/tmp/uwvm2_intslot_v128_translate.log", ::fast_io::open_mode::out);
            ::uwvm2::uwvm::io::enable_runtime_log = true;
        }

        ::uwvm2::validation::error::code_validation_error_impl err{};
        optable::compile_option cop{};
        auto cm = compiler::compile_all_from_uwvm_single_func<Opt>(rt, cop, err, ::std::addressof(features));
        UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);
        UWVM2TEST_REQUIRE(cm.local_funcs.size() == 4uz);

        [[maybe_unused]] constexpr auto tuple =
            compiler::details::make_interpreter_tuple<Opt>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<Opt>()>{});

        if constexpr(Opt.is_tail_call)
        {
            UWVM2TEST_REQUIRE(bytecode_contains_i32_local_spot_get<Opt>(cm.local_funcs.index_unchecked(0).op.operands) ||
                              bytecode_contains_i32_local_spot_flush<Opt>(cm.local_funcs.index_unchecked(0).op.operands) ||
                              bytecode_contains_i32_cache_variant<Opt>(
                                  cm.local_funcs.index_unchecked(0).op.operands,
                                  [&](auto const& curr_variant) constexpr noexcept
                                  { return optable::translate::get_uwvmint_local_set_keep_i32_fptr_from_tuple<Opt>(curr_variant, tuple); }));
            UWVM2TEST_REQUIRE(bytecode_contains_i64_local_spot_get<Opt>(cm.local_funcs.index_unchecked(1).op.operands) ||
                              bytecode_contains_i64_local_spot_flush<Opt>(cm.local_funcs.index_unchecked(1).op.operands) ||
                              bytecode_contains_i64_cache_variant<Opt>(
                                  cm.local_funcs.index_unchecked(1).op.operands,
                                  [&](auto const& curr_variant) constexpr noexcept
                                  { return optable::translate::get_uwvmint_local_set_keep_i64_fptr_from_tuple<Opt>(curr_variant, tuple); }));

#if defined(UWVM_ENABLE_UWVM_INT_COMBINE_OPS) && defined(UWVM_ENABLE_UWVM_INT_DELAY_LOCAL_SOFT)
            UWVM2TEST_REQUIRE(bytecode_contains_v128_cache_variant<Opt>(
                cm.local_funcs.index_unchecked(2).op.operands,
                [&](auto const& curr_variant) constexpr noexcept
                {
                    return optable::translate::get_uwvmint_simd_v128_binop_2localget_fptr_from_tuple<
                        Opt,
                        optable::wasm1p1_simd_details::v128_binop::xor_>(curr_variant, tuple);
                }));
            UWVM2TEST_REQUIRE(bytecode_contains_v128_cache_variant<Opt>(
                cm.local_funcs.index_unchecked(3).op.operands,
                [&](auto const& curr_variant) constexpr noexcept
                {
                    return optable::translate::get_uwvmint_simd_v128_binop_localget_rhs_fptr_from_tuple<
                        Opt,
                        optable::wasm1p1_simd_details::v128_binop::xor_>(curr_variant, tuple);
                }));
#endif
        }

        using Runner = interpreter_runner<Opt>;
        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(0),
                                              rt.local_defined_function_vec_storage.index_unchecked(0),
                                              pack_i32(35),
                                              nullptr,
                                              nullptr)
                                       .results) == 42);
        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(1),
                                              rt.local_defined_function_vec_storage.index_unchecked(1),
                                              pack_i64(0x123456780000ll),
                                              nullptr,
                                              nullptr)
                                       .results) == 0x12345678000bll);
        UWVM2TEST_REQUIRE(static_cast<::std::uint32_t>(load_i32(Runner::run(cm.local_funcs.index_unchecked(2),
                                                                            rt.local_defined_function_vec_storage.index_unchecked(2),
                                                                            pack_no_params(),
                                                                            nullptr,
                                                                            nullptr)
                                                                     .results)) == (0x10203040u ^ 0x01010101u));
        UWVM2TEST_REQUIRE(static_cast<::std::uint32_t>(load_i32(Runner::run(cm.local_funcs.index_unchecked(3),
                                                                            rt.local_defined_function_vec_storage.index_unchecked(3),
                                                                            pack_no_params(),
                                                                            nullptr,
                                                                            nullptr)
                                                                     .results)) == (0xf0f0f0f0u ^ 0x01010101u));

        return 0;
    }

    [[nodiscard]] int test_translate_delay_local_intspot_v128() noexcept
    {
        install_unexpected_traps();
        optable::call_func = strict_terminate_call;
        optable::call_indirect_func = strict_terminate_call_indirect;

        auto wasm = build_delay_local_intspot_v128_module();
        auto features = make_wasm1p1_feature_parameter();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_delay_local_intspot_v128", {}, features);
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        if(abi_mode_enabled("tail-intspot-fv2") || abi_mode_enabled("tail-sysv"))
        {
            constexpr auto opt{make_tailcall_int_spot_logical_fv_opt<2uz, 2uz>()};
            static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, features) == 0);
        }

        return 0;
    }
}  // namespace

int main()
{
    try
    {
        return test_translate_delay_local_intspot_v128();
    }
    catch(...)
    {
        return ::uwvm2test::uwvm_int_strict::fail(__LINE__, "uncaught exception");
    }
}
