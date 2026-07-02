#include "../uwvm_int_translate_strict_common.h"

#include <string_view>

namespace
{
    using namespace ::uwvm2test::uwvm_int_strict;

    [[nodiscard]] std::string read_text_file(char const* path)
    {
        ::std::FILE* fp{::std::fopen(path, "rb")};
        if(fp == nullptr) { return {}; }

        std::string out{};
        char buf[4096]{};
        for(;;)
        {
            auto const n{::std::fread(buf, 1u, sizeof(buf), fp)};
            if(n != 0u) { out.append(buf, n); }
            if(n != sizeof(buf)) { break; }
        }

        ::std::fclose(fp);
        return out;
    }

    [[nodiscard]] bool line_contains_all(std::string_view text, std::initializer_list<std::string_view> needles) noexcept
    {
        std::size_t line_begin{};
        for(;;)
        {
            auto const line_end{text.find('\n', line_begin)};
            auto const line{text.substr(line_begin, line_end == std::string_view::npos ? std::string_view::npos : line_end - line_begin)};

            bool ok{true};
            for(auto needle: needles)
            {
                if(line.find(needle) == std::string_view::npos)
                {
                    ok = false;
                    break;
                }
            }
            if(ok) { return true; }

            if(line_end == std::string_view::npos) { break; }
            line_begin = line_end + 1uz;
        }
        return false;
    }

    [[nodiscard]] byte_vec build_fv_slot_selection_module()
    {
        module_builder mb{};

        auto op = [&](byte_vec& c, wasm_op o) { append_u8(c, u8(o)); };
        auto f32 = [&](byte_vec& c, float v) { append_f32_ieee(c, v); };

        // f0: two cached f32 operands. With a 3-slot FV ring this selects top=reg[1], next=reg[2].
        {
            func_type ty{{}, {k_val_f32}};
            func_body fb{};
            auto& c = fb.code;

            op(c, wasm_op::f32_const);
            f32(c, 1.0f);
            op(c, wasm_op::f32_const);
            f32(c, 2.0f);
            op(c, wasm_op::nop);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        // f1: three cached f32 operands. The first add selects top=reg[0], next=reg[1].
        {
            func_type ty{{}, {k_val_f32}};
            func_body fb{};
            auto& c = fb.code;

            op(c, wasm_op::f32_const);
            f32(c, 1.0f);
            op(c, wasm_op::f32_const);
            f32(c, 2.0f);
            op(c, wasm_op::f32_const);
            f32(c, 3.0f);
            op(c, wasm_op::nop);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::f32_add);
            op(c, wasm_op::end);

            (void)mb.add_func(::std::move(ty), ::std::move(fb));
        }

        return mb.build();
    }

    template <optable::uwvm_interpreter_translate_option_t Opt>
    [[nodiscard]] compiled_module_t compile_with_runtime_log(runtime_module_t const& rt)
    {
        ::uwvm2::validation::error::code_validation_error_impl err{};
        optable::compile_option cop{};
        auto cm = compiler::compile_all_from_uwvm_single_func<Opt>(rt, cop, err);
        if(err.err_code != ::uwvm2::validation::error::code_validation_error_code::ok) { ::fast_io::fast_terminate(); }
        if(cm.local_funcs.size() != 2uz) { ::fast_io::fast_terminate(); }
        return cm;
    }

    template <optable::uwvm_interpreter_translate_option_t Opt>
    [[nodiscard]] int run_f32_func(compiled_module_t const& cm, runtime_module_t const& rt, std::size_t index, float expected)
    {
        auto rr = interpreter_runner<Opt>::run(cm.local_funcs.index_unchecked(index),
                                               rt.local_defined_function_vec_storage.index_unchecked(index),
                                               pack_no_params(),
                                               nullptr,
                                               nullptr);
        UWVM2TEST_REQUIRE(rr.results.size() == sizeof(float));
        UWVM2TEST_REQUIRE(load_f32(rr.results) == expected);
        return 0;
    }

    [[nodiscard]] int test_translate_runtime_log_fv_slot_selection()
    {
        ::uwvm2test::uwvm_int_strict::install_unexpected_traps();
        optable::call_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call;
        optable::call_indirect_func = ::uwvm2test::uwvm_int_strict::strict_terminate_call_indirect;

        auto const wasm = build_fv_slot_selection_module();
        auto prep = prepare_runtime_from_wasm(wasm, u8"uwvm2test_runtime_log_fv_slot_selection");
        UWVM2TEST_REQUIRE(prep.mod != nullptr);
        runtime_module_t const& rt = *prep.mod;

        constexpr char kLogPath[]{
#if defined(_WIN32) || defined(_WIN64)
            "uwvm_int_translate_runtime_log_fv_slot_selection_strict.log"
#else
            "/tmp/uwvm_int_translate_runtime_log_fv_slot_selection_strict.log"
#endif
        };

#if defined(_WIN32) || defined(_WIN64)
        ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"uwvm_int_translate_runtime_log_fv_slot_selection_strict.log",
                                                       ::fast_io::open_mode::out);
#else
        ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"/tmp/uwvm_int_translate_runtime_log_fv_slot_selection_strict.log",
                                                       ::fast_io::open_mode::out);
#endif
        ::uwvm2::uwvm::io::enable_runtime_log = true;

        constexpr auto opt3 = make_tailcall_logical_fv_opt<3uz>();
        static_assert(compiler::details::interpreter_tuple_has_no_holes<opt3>());
        auto cm3 = compile_with_runtime_log<opt3>(rt);

        constexpr auto tuple3 =
            compiler::details::make_interpreter_tuple<opt3>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<opt3>()>{});
        constexpr optable::uwvm_interpreter_stacktop_currpos_t curr_f32_1{
            .i32_stack_top_curr_pos = SIZE_MAX,
            .i64_stack_top_curr_pos = SIZE_MAX,
            .f32_stack_top_curr_pos = 1uz,
            .f64_stack_top_curr_pos = 1uz,
            .v128_stack_top_curr_pos = 1uz,
        };
        constexpr optable::uwvm_interpreter_stacktop_currpos_t curr_f32_0{
            .i32_stack_top_curr_pos = SIZE_MAX,
            .i64_stack_top_curr_pos = SIZE_MAX,
            .f32_stack_top_curr_pos = 0uz,
            .f64_stack_top_curr_pos = 0uz,
            .v128_stack_top_curr_pos = 0uz,
        };
        constexpr auto f32_add_curr1 = optable::translate::get_uwvmint_f32_add_fptr_from_tuple<opt3>(curr_f32_1, tuple3);
        constexpr auto f32_add_curr0 = optable::translate::get_uwvmint_f32_add_fptr_from_tuple<opt3>(curr_f32_0, tuple3);
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm3.local_funcs.index_unchecked(0).op.operands, f32_add_curr1));
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm3.local_funcs.index_unchecked(1).op.operands, f32_add_curr0));
        UWVM2TEST_REQUIRE((run_f32_func<opt3>(cm3, rt, 0uz, 3.0f)) == 0);
        UWVM2TEST_REQUIRE((run_f32_func<opt3>(cm3, rt, 1uz, 6.0f)) == 0);

        constexpr auto opt1 = make_tailcall_logical_fv_opt<1uz>();
        static_assert(compiler::details::interpreter_tuple_has_no_holes<opt1>());
        auto cm1 = compile_with_runtime_log<opt1>(rt);
        constexpr auto tuple1 =
            compiler::details::make_interpreter_tuple<opt1>(::std::make_index_sequence<compiler::details::interpreter_tuple_size<opt1>()>{});
        constexpr auto f32_add_ring1 = optable::translate::get_uwvmint_f32_add_fptr_from_tuple<opt1>(curr_f32_0, tuple1);
        constexpr auto f32_add_fill1_ring1 = optable::translate::get_uwvmint_f32_add_then_fill1_fptr_from_tuple<opt1>(curr_f32_0, tuple1);
        UWVM2TEST_REQUIRE(bytecode_contains_fptr(cm1.local_funcs.index_unchecked(0).op.operands, f32_add_ring1) ||
                           bytecode_contains_fptr(cm1.local_funcs.index_unchecked(0).op.operands, f32_add_fill1_ring1));
        UWVM2TEST_REQUIRE((run_f32_func<opt1>(cm1, rt, 0uz, 3.0f)) == 0);
        UWVM2TEST_REQUIRE((run_f32_func<opt1>(cm1, rt, 1uz, 6.0f)) == 0);

        ::uwvm2::uwvm::io::enable_runtime_log = false;
#if defined(_WIN32) || defined(_WIN64)
        ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"NUL", ::fast_io::open_mode::out);
#else
        ::uwvm2::uwvm::io::u8runtime_log_output.reopen(u8"/dev/null", ::fast_io::open_mode::out);
#endif

        auto const log_text{read_text_file(kLogPath)};
        UWVM2TEST_REQUIRE(!log_text.empty());
        UWVM2TEST_REQUIRE(line_contains_all(log_text,
                                            {"fn=0",
                                             "event=wasm.op.before",
                                             "op=f32_add",
                                             "stacktop{mem=0,cache=2",
                                             "f32=2",
                                             "v128=0",
                                             "currpos{",
                                             "f32=1"}));
        UWVM2TEST_REQUIRE(line_contains_all(log_text,
                                            {"fn=1",
                                             "event=wasm.op.before",
                                             "op=f32_add",
                                             "stacktop{mem=0,cache=3",
                                             "f32=3",
                                             "v128=0",
                                             "currpos{",
                                             "f32=0"}));
        UWVM2TEST_REQUIRE(line_contains_all(log_text,
                                            {"fn=0",
                                             "event=wasm.op.before",
                                             "op=f32_add",
                                             "stacktop{mem=1,cache=1",
                                             "f32=1",
                                             "v128=0",
                                             "currpos{",
                                             "f32=0"}));

        return 0;
    }
}  // namespace

int main()
{
    try
    {
        return test_translate_runtime_log_fv_slot_selection();
    }
    catch(...)
    {
        return ::uwvm2test::uwvm_int_strict::fail(__LINE__, "uncaught exception");
    }
}
