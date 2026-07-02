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

    [[nodiscard]] byte_vec build_dirty_writeback_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto u32 = [&](byte_vec& c, ::std::uint32_t v) { append_u32_leb(c, v); };

        {
            func_type ty{{k_val_i32}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i32});
            auto& c = fb.code;

            op(c, wasm_op::block); append_u8(c, 0x40u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_set); u32(c, 1u);
            op(c, wasm_op::br); u32(c, 0u);
            op(c, wasm_op::end);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        {
            func_type ty{{k_val_i64}, {k_val_i64}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_i64});
            auto& c = fb.code;

            op(c, wasm_op::block); append_u8(c, 0x40u);
            op(c, wasm_op::local_get); u32(c, 0u);
            op(c, wasm_op::local_set); u32(c, 1u);
            op(c, wasm_op::br); u32(c, 0u);
            op(c, wasm_op::end);
            op(c, wasm_op::local_get); u32(c, 1u);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        {
            func_type ty{{}, {k_val_i32}};
            func_body fb{};
            fb.locals.push_back({1u, k_val_v128});
            auto& c = fb.code;

            op(c, wasm_op::block); append_u8(c, 0x40u);
            v128_i32x4(c, 0x13579bdfu, 0x2468ace0u, 0x10203040u, 0x55667788u);
            op(c, wasm_op::local_set); u32(c, 0u);
            op(c, wasm_op::br); u32(c, 0u);
            op(c, wasm_op::end);
            op(c, wasm_op::local_get); u32(c, 0u);
            simd(c, wasm1p1_simd_op::i32x4_extract_lane);
            append_u8(c, 0u);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    template <optable::uwvm_interpreter_translate_option_t Opt>
    [[nodiscard]] int run_suite(runtime_module_t const& rt, wasm_feature_parameter_t const& features) noexcept
    {
        if(::std::getenv("UWVM2TEST_DEBUG_DIRTY_WRITEBACK_LOG") != nullptr)
        {
            ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"/tmp/uwvm2_dirty_writeback_translate.log", ::fast_io::open_mode::out);
            ::uwvm2::uwvm::io::enable_runtime_log = true;
        }

        ::uwvm2::validation::error::code_validation_error_impl err{};
        optable::compile_option cop{};
        auto cm = compiler::compile_all_from_uwvm_single_func<Opt>(rt, cop, err, ::std::addressof(features));
        UWVM2TEST_REQUIRE(err.err_code == ::uwvm2::validation::error::code_validation_error_code::ok);
        UWVM2TEST_REQUIRE(cm.local_funcs.size() == 3uz);

        using Runner = interpreter_runner<Opt>;
        UWVM2TEST_REQUIRE(load_i32(Runner::run(cm.local_funcs.index_unchecked(0),
                                              rt.local_defined_function_vec_storage.index_unchecked(0),
                                              pack_i32(0x12345678),
                                              nullptr,
                                              nullptr)
                                       .results) == 0x12345678);
        UWVM2TEST_REQUIRE(load_i64(Runner::run(cm.local_funcs.index_unchecked(1),
                                              rt.local_defined_function_vec_storage.index_unchecked(1),
                                              pack_i64(0x123456789abcdef0ll),
                                              nullptr,
                                              nullptr)
                                       .results) == 0x123456789abcdef0ll);
        UWVM2TEST_REQUIRE(static_cast<::std::uint32_t>(load_i32(Runner::run(cm.local_funcs.index_unchecked(2),
                                                                            rt.local_defined_function_vec_storage.index_unchecked(2),
                                                                            pack_no_params(),
                                                                            nullptr,
                                                                            nullptr)
                                                                     .results)) == 0x13579bdfu);
        return 0;
    }

    [[nodiscard]] int test_dirty_writeback() noexcept
    {
        install_unexpected_traps();
        optable::call_func = strict_terminate_call;
        optable::call_indirect_func = strict_terminate_call_indirect;

        auto wasm = build_dirty_writeback_module();
        auto features = make_wasm1p1_feature_parameter();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_local_spot_dirty_writeback", {}, features);
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        if(abi_mode_enabled("tail-intspot-fv2") || abi_mode_enabled("tail-sysv"))
        {
            constexpr auto opt{make_tailcall_int_spot_logical_fv_opt<1uz, 2uz>()};
            static_assert(compiler::details::interpreter_tuple_has_no_holes<opt>());
            UWVM2TEST_REQUIRE(run_suite<opt>(rt, features) == 0);
        }
        if(abi_mode_enabled("tail-intspot3-fv2") || abi_mode_enabled("tail-sysv"))
        {
            constexpr auto opt{make_tailcall_int_spot_logical_fv_opt<3uz, 2uz>()};
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
        return test_dirty_writeback();
    }
    catch(...)
    {
        return ::uwvm2test::uwvm_int_strict::fail(__LINE__, "uncaught exception");
    }
}
