#ifndef UWVM2TEST_RUNNER_USE_LLVM_JIT
# define UWVM2TEST_RUNNER_USE_LLVM_JIT 1
#endif
#define UWVM2TEST_STRICT_NO_INTERPRETER 1

#include "../0013.uwvm_int/strict/uwvm_int_translate_strict_common.h"

#include <bit>
#include <cfenv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
# include <xmmintrin.h>
#endif

namespace
{
    namespace strict = ::uwvm2test::uwvm_int_strict;
    namespace wasm_type = ::uwvm2::uwvm::wasm::type;

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
# if defined(__x86_64__) || defined(_M_X64) || defined(__SSE2__)
    inline constexpr unsigned flush_control_mask{(1u << 15u) | (1u << 6u)};
# else
    inline constexpr unsigned flush_control_mask{1u << 15u};
# endif
    [[nodiscard]] unsigned read_fp_control() noexcept { return _mm_getcsr(); }
    void write_fp_control(unsigned value) noexcept { _mm_setcsr(value); }
#elif defined(__aarch64__) && (defined(__GNUC__) || defined(__clang__))
    inline constexpr ::std::uint_least64_t flush_control_mask{1ull << 24u};
    [[nodiscard]] ::std::uint_least64_t read_fp_control() noexcept
    {
        ::std::uint_least64_t value{};
        __asm__ volatile("mrs %0, fpcr" : "=r"(value));
        return value;
    }
    void write_fp_control(::std::uint_least64_t value) noexcept
    { __asm__ volatile("msr fpcr, %0" : : "r"(value)); }
#else
    inline constexpr unsigned flush_control_mask{};
    [[nodiscard]] unsigned read_fp_control() noexcept { return 0u; }
    void write_fp_control(unsigned) noexcept {}
#endif

    void poison_fp_environment() noexcept
    {
        static_cast<void>(::std::fesetround(FE_UPWARD));
        write_fp_control(read_fp_control() | flush_control_mask);
    }

    [[nodiscard]] bool flush_controls_enabled() noexcept
    {
        if constexpr(flush_control_mask == 0u) { return true; }
        return (read_fp_control() & flush_control_mask) == flush_control_mask;
    }

    struct initial_environment_restore
    {
        ::std::fenv_t environment{};
        decltype(read_fp_control()) fp_control{};
        bool valid{};

        initial_environment_restore() noexcept
            : fp_control{read_fp_control()}, valid{::std::fegetenv(::std::addressof(environment)) == 0}
        {}

        ~initial_environment_restore() noexcept
        {
            if(valid) { static_cast<void>(::std::fesetenv(::std::addressof(environment))); }
            write_fp_control(fp_control);
        }
    };

    using wasm1 = ::uwvm2::parser::wasm::standard::wasm1::features::wasm1;
    using feature_list = wasm_type::feature_list<wasm1>;
    using value_type = ::uwvm2::parser::wasm::standard::wasm1::type::value_type;

    struct poison_environment_import
    {
        inline static constexpr ::uwvm2::utils::container::u8string_view function_name{u8"poison"};
        using result_tuple = wasm_type::import_function_result_tuple_t<feature_list>;
        using parameter_tuple = wasm_type::import_function_parameter_tuple_t<feature_list>;
        using local_imported_function_type = wasm_type::local_imported_function_type_t<result_tuple, parameter_tuple>;

        inline static ::std::size_t call_count{};

        static void call(local_imported_function_type&) noexcept
        {
            ++call_count;
            poison_fp_environment();
        }
    };

    struct fp_host_module
    {
        ::uwvm2::utils::container::u8string_view module_name{u8"fp-host"};
        using local_function_tuple = ::uwvm2::utils::container::tuple<poison_environment_import>;
    };

    static_assert(wasm_type::is_local_imported_function<poison_environment_import>);
    static_assert(wasm_type::is_local_imported_module<fp_host_module>);

    [[nodiscard]] strict::byte_vec build_fp_module()
    {
        strict::module_builder module{};
        auto op = [&](strict::byte_vec& code, strict::wasm_op opcode) { strict::append_u8(code, strict::u8(opcode)); };
        auto u32 = [&](strict::byte_vec& code, ::std::uint32_t value) { strict::append_u32_leb(code, value); };

        strict::func_type host_type{{}, {}};
        module.types.push_back(host_type);
        module.add_import_func("fp-host", "poison", 0u);

        auto add_binary_function = [&](strict::wasm_op binary_opcode)
        {
            strict::func_type type{{strict::k_val_f32, strict::k_val_f32}, {strict::k_val_f32}};
            strict::func_body body{};
            auto& code{body.code};
            op(code, strict::wasm_op::call);
            u32(code, 0u);
            op(code, strict::wasm_op::local_get);
            u32(code, 0u);
            op(code, strict::wasm_op::local_get);
            u32(code, 1u);
            op(code, binary_opcode);
            op(code, strict::wasm_op::end);
            static_cast<void>(module.add_func(::std::move(type), ::std::move(body)));
        };

        add_binary_function(strict::wasm_op::f32_add);
        add_binary_function(strict::wasm_op::f32_mul);
        return module.build();
    }

    template <typename Value>
    void append_abi_value(strict::byte_vec& bytes, Value value)
    {
        auto const old_size{bytes.size()};
        bytes.resize(old_size + sizeof(Value));
        ::std::memcpy(bytes.data() + old_size, ::std::addressof(value), sizeof(Value));
    }

    [[nodiscard]] ::std::uint32_t run_binary(::uwvm2::uwvm::runtime::storage::wasm_module_storage_t const* module,
                                             ::std::uint_least32_t function_index,
                                             float left,
                                             float right) noexcept
    {
        strict::byte_vec parameters{};
        parameters.reserve(sizeof(float) * 2u);
        append_abi_value(parameters, left);
        append_abi_value(parameters, right);

        float result{};
        ::uwvm2::runtime::lib::llvm_jit_call_raw_host_api(
            module, function_index, ::std::addressof(result), sizeof(result), parameters.data(), parameters.size());
        return ::std::bit_cast<::std::uint32_t>(result);
    }

    [[nodiscard]] int test_llvm_fp_environment() noexcept
    {
        initial_environment_restore restore_initial{};
        if(!restore_initial.valid) { return 1; }

        auto wasm{build_fp_module()};
        wasm_type::local_imported_t host_module{fp_host_module{}};
        auto prepared{strict::prepare_runtime_from_wasm(wasm, u8"llvm_aot_fp_environment", {}, {}, {host_module})};
        if(prepared.mod == nullptr) { return 2; }

        if(::std::fesetround(FE_DOWNWARD) != 0) { return 3; }
        write_fp_control(read_fp_control() | flush_control_mask);
        if(::std::fegetround() != FE_DOWNWARD || !flush_controls_enabled()) { return 4; }

        // 1.0 + 2^-24 is exactly halfway between adjacent f32 values. Wasm must choose the even 1.0 result even though
        // both the embedding thread and the imported callback select hostile rounding modes.
        auto const half_ulp{::std::bit_cast<float>(::std::uint32_t{0x33800000u})};
        if(run_binary(prepared.mod, 1u, 1.0f, half_ulp) != 0x3f800000u) { return 5; }
        if(::std::fegetround() != FE_DOWNWARD || !flush_controls_enabled()) { return 6; }

        // DAZ must not erase a subnormal operand.
        auto const minimum_subnormal{::std::bit_cast<float>(::std::uint32_t{1u})};
        if(run_binary(prepared.mod, 1u, minimum_subnormal, 0.0f) != 1u) { return 7; }
        if(::std::fegetround() != FE_DOWNWARD || !flush_controls_enabled()) { return 8; }

        // FTZ must not erase a subnormal result.
        auto const minimum_normal{::std::numeric_limits<float>::min()};
        if(run_binary(prepared.mod, 2u, minimum_normal, 0.5f) != 0x00400000u) { return 9; }
        if(::std::fegetround() != FE_DOWNWARD || !flush_controls_enabled()) { return 10; }

        return poison_environment_import::call_count == 3uz ? 0 : 11;
    }
}

int main()
{
    return test_llvm_fp_environment();
}
