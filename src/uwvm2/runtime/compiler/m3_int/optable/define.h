#pragma once

#ifndef UWVM_MODULE
# include <bit>
# include <concepts>
# include <cstddef>
# include <cstdint>
# include <cstring>
# include <type_traits>
# include <utility>
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/runtime/compiler/m3_int/macro/push_macros.h>
# include <uwvm2/parser/wasm/standard/wasm1/type/value_type.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::optable
{
    using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
    using wasm_u32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32;
    using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
    using wasm_u64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64;
    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;

    using m3_code_t = ::std::byte const*;
    using m3_stack_t = ::std::byte*;
    using m3_int_register_t = wasm_i64;
    using m3_fp_register_t = wasm_f64;
    using m3_slot_offset_t = wasm_i32;

    struct m3_memory_view_t
    {
        ::std::byte* data{};
        ::std::size_t size{};
        wasm_u32 page_size{wasm_u32{65536u}};
        wasm_u32 page_count{};
    };

    enum class binary_shape_t : ::std::uint_least8_t
    {
        rs,
        sr,
        ss
    };

    enum class unary_shape_t : ::std::uint_least8_t
    {
        r,
        s
    };

    enum class register_slot_shape_t : ::std::uint_least8_t
    {
        rr,
        rs,
        sr,
        ss
    };

    enum class select_shape_t : ::std::uint_least8_t
    {
        rss,
        rrs,
        rsr,
        srs,
        ssr,
        sss
    };

    template <typename T>
    concept m3_value_type = ::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_u32> || ::std::same_as<T, wasm_i64> ||
                            ::std::same_as<T, wasm_u64> || ::std::same_as<T, wasm_f32> || ::std::same_as<T, wasm_f64>;

    template <typename T>
    concept m3_register_value_type = ::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_i64> || ::std::same_as<T, wasm_f32> ||
                                     ::std::same_as<T, wasm_f64>;

    using m3_interpreter_opfunc_t = void(UWVM_INTERPRETER_OPFUNC_TYPE_MACRO*)(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;
    using trap_hook_t = void (*)() noexcept;
    using memory_grow_func_t = wasm_i32 (*)(wasm_i32) noexcept;
    using call_func_t = void (*)(m3_code_t, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;
    using compile_func_t = m3_code_t (*)(void*) noexcept;
    using indirect_call_func_t = void (*)(wasm_u32, void*, void*, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;
    using raw_call_func_t = void (*)(void*, void*, void*, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;
    using entry_func_t = void (*)(void*, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;
    using loop_func_t = void (*)(m3_code_t&, m3_stack_t, m3_int_register_t&, m3_fp_register_t&) noexcept;

    enum class m3_control_signal_t : ::std::uint_least8_t
    {
        none,
        returned,
        continue_loop
    };

    struct m3_control_state_t
    {
        m3_control_signal_t signal{m3_control_signal_t::none};
        void const* payload{};
    };

    inline trap_hook_t unsupported_func{};
    inline trap_hook_t unreachable_func{};
    inline trap_hook_t trap_invalid_conversion_to_integer_func{};
    inline trap_hook_t trap_integer_divide_by_zero_func{};
    inline trap_hook_t trap_integer_overflow_func{};
    inline trap_hook_t trap_out_of_bounds_memory_access_func{};
    inline memory_grow_func_t memory_grow_func{};
    inline call_func_t call_func{};
    inline compile_func_t compile_func{};
    inline indirect_call_func_t indirect_call_func{};
    inline raw_call_func_t raw_call_func{};
    inline entry_func_t entry_func{};
    inline loop_func_t loop_func{};

    inline m3_memory_view_t memory_view{};
    inline m3_control_state_t control_state{};

    namespace details
    {
        template <typename T>
            requires(::std::is_trivially_copyable_v<T>)
        inline T read_immediate(m3_code_t& ip) noexcept
        {
            T out{};
            ::std::memcpy(::std::addressof(out), ip, sizeof(T));
            ip += sizeof(T);
            return out;
        }

        template <typename T>
            requires(::std::is_trivially_copyable_v<T>)
        inline T load_slot_at(m3_stack_t sp, m3_slot_offset_t offset) noexcept
        {
            T out{};
            ::std::memcpy(::std::addressof(out), sp + static_cast<::std::ptrdiff_t>(offset), sizeof(T));
            return out;
        }

        template <typename T>
            requires(::std::is_trivially_copyable_v<T>)
        inline void store_slot_at(m3_stack_t sp, m3_slot_offset_t offset, T const& value) noexcept
        {
            ::std::memcpy(sp + static_cast<::std::ptrdiff_t>(offset), ::std::addressof(value), sizeof(T));
        }

        template <typename T>
            requires(::std::is_trivially_copyable_v<T>)
        inline T load_slot(m3_code_t& ip, m3_stack_t sp) noexcept
        {
            auto const offset{read_immediate<m3_slot_offset_t>(ip)};
            return load_slot_at<T>(sp, offset);
        }

        template <typename T>
            requires(::std::is_trivially_copyable_v<T>)
        inline void store_slot(m3_code_t& ip, m3_stack_t sp, T const& value) noexcept
        {
            auto const offset{read_immediate<m3_slot_offset_t>(ip)};
            store_slot_at(sp, offset, value);
        }

        template <m3_value_type T>
        inline T load_register_value(m3_int_register_t const& r0, m3_fp_register_t const& fp0) noexcept
        {
            if constexpr(::std::same_as<T, wasm_f32> || ::std::same_as<T, wasm_f64>)
            {
                return static_cast<T>(fp0);
            }
            else
            {
                return static_cast<T>(r0);
            }
        }

        template <m3_register_value_type T>
        inline void store_register_value(m3_int_register_t& r0, m3_fp_register_t& fp0, T value) noexcept
        {
            if constexpr(::std::same_as<T, wasm_f32> || ::std::same_as<T, wasm_f64>)
            {
                fp0 = static_cast<m3_fp_register_t>(value);
            }
            else
            {
                r0 = static_cast<m3_int_register_t>(value);
            }
        }

        template <m3_value_type T>
        inline T load_binary_lhs(binary_shape_t const shape, m3_code_t& ip, m3_stack_t sp, m3_int_register_t const& r0, m3_fp_register_t const& fp0) noexcept
        {
            switch(shape)
            {
                case binary_shape_t::sr:
                    return load_register_value<T>(r0, fp0);
                case binary_shape_t::rs:
                    return load_slot<T>(ip, sp);
                case binary_shape_t::ss:
                default:
                {
                    auto const rhs{load_slot<T>(ip, sp)};
                    (void)rhs;
                    return load_slot<T>(ip, sp);
                }
            }
        }

        template <m3_value_type T>
        inline T load_binary_rhs(binary_shape_t const shape, m3_code_t& ip, m3_stack_t sp, m3_int_register_t const& r0, m3_fp_register_t const& fp0) noexcept
        {
            switch(shape)
            {
                case binary_shape_t::sr:
                    return load_slot<T>(ip, sp);
                case binary_shape_t::rs:
                    return load_register_value<T>(r0, fp0);
                case binary_shape_t::ss:
                default:
                {
                    auto ip_copy{ip};
                    auto const rhs{load_slot<T>(ip_copy, sp)};
                    auto const lhs{load_slot<T>(ip_copy, sp)};
                    (void)lhs;
                    ip = ip_copy;
                    return rhs;
                }
            }
        }

        template <select_shape_t Shape>
        inline wasm_i32 load_select_condition(m3_code_t& ip, m3_stack_t sp, m3_int_register_t const& r0) noexcept
        {
            if constexpr(Shape == select_shape_t::rss || Shape == select_shape_t::rrs || Shape == select_shape_t::rsr)
            {
                return static_cast<wasm_i32>(r0);
            }
            else
            {
                return load_slot<wasm_i32>(ip, sp);
            }
        }

        template <m3_value_type T, select_shape_t Shape>
        inline T load_select_operand2(m3_code_t& ip, m3_stack_t sp, m3_int_register_t const& r0, m3_fp_register_t const& fp0) noexcept
        {
            if constexpr(Shape == select_shape_t::rrs || Shape == select_shape_t::srs)
            {
                return load_register_value<T>(r0, fp0);
            }
            else
            {
                return load_slot<T>(ip, sp);
            }
        }

        template <m3_value_type T, select_shape_t Shape>
        inline T load_select_operand1(m3_code_t& ip, m3_stack_t sp, m3_int_register_t const& r0, m3_fp_register_t const& fp0) noexcept
        {
            if constexpr(Shape == select_shape_t::ssr || Shape == select_shape_t::rsr)
            {
                return load_register_value<T>(r0, fp0);
            }
            else
            {
                return load_slot<T>(ip, sp);
            }
        }
    }
}
