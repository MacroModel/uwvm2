#pragma once

#ifndef UWVM_MODULE
# include <bit>
# include <concepts>
# include <exception>
# include <limits>
# include <type_traits>
# include <uwvm2/runtime/compiler/m3_int/optable/define.h>
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::optable
{
    namespace convert_details
    {
        [[noreturn]] inline void trap_invalid_conversion_to_integer() noexcept
        {
            if(::uwvm2::runtime::compiler::m3_int::optable::trap_invalid_conversion_to_integer_func != nullptr)
            {
                ::uwvm2::runtime::compiler::m3_int::optable::trap_invalid_conversion_to_integer_func();
            }
            ::std::terminate();
        }

        [[noreturn]] inline void trap_integer_overflow() noexcept
        {
            if(::uwvm2::runtime::compiler::m3_int::optable::trap_integer_overflow_func != nullptr)
            {
                ::uwvm2::runtime::compiler::m3_int::optable::trap_integer_overflow_func();
            }
            ::std::terminate();
        }

        template <typename WasmI32>
        inline constexpr ::std::uint_least32_t to_u32_bits(WasmI32 v) noexcept
        {
            ::std::int_least32_t const i{static_cast<::std::int_least32_t>(v)};
            return ::std::bit_cast<::std::uint_least32_t>(i);
        }

        template <typename WasmI64>
        inline constexpr ::std::uint_least64_t to_u64_bits(WasmI64 v) noexcept
        {
            ::std::int_least64_t const i{static_cast<::std::int_least64_t>(v)};
            return static_cast<::std::uint_least64_t>(i);
        }

        template <typename WasmI32>
        inline constexpr WasmI32 from_u32_bits(::std::uint_least32_t u) noexcept
        {
            ::std::int_least32_t const i{::std::bit_cast<::std::int_least32_t>(u)};
            return static_cast<WasmI32>(i);
        }

        template <typename WasmI64>
        inline constexpr WasmI64 from_u64_bits(::std::uint_least64_t u) noexcept
        {
            ::std::int_least64_t const i{::std::bit_cast<::std::int_least64_t>(u)};
            return static_cast<WasmI64>(i);
        }

        template <typename IntOut, typename FloatIn>
        inline IntOut trunc_float_to_int_s(FloatIn x) noexcept
        {
            constexpr FloatIn min_v{static_cast<FloatIn>(::std::numeric_limits<IntOut>::min())};
            constexpr FloatIn max_plus_one{static_cast<FloatIn>(static_cast<long double>(::std::numeric_limits<IntOut>::max()) + 1.0L)};

            if(x >= min_v && x < max_plus_one) [[likely]]
            {
                return static_cast<IntOut>(x);
            }

            if(x != x) [[unlikely]] { trap_invalid_conversion_to_integer(); }
            trap_integer_overflow();
            return IntOut{};
        }

        template <typename UIntOut, typename FloatIn>
        inline UIntOut trunc_float_to_int_u(FloatIn x) noexcept
        {
            constexpr FloatIn max_plus_one{static_cast<FloatIn>(static_cast<long double>(::std::numeric_limits<UIntOut>::max()) + 1.0L)};

            if(x >= static_cast<FloatIn>(0) && x < max_plus_one) [[likely]]
            {
                return static_cast<UIntOut>(x);
            }

            if(x != x) [[unlikely]] { trap_invalid_conversion_to_integer(); }
            trap_integer_overflow();
            return UIntOut{};
        }

        template <typename IntOut, typename FloatIn>
        inline IntOut trunc_sat_float_to_int_s(FloatIn x) noexcept
        {
            if(x != x) [[unlikely]] { return IntOut{}; }
            constexpr FloatIn min_v{static_cast<FloatIn>(::std::numeric_limits<IntOut>::min())};
            constexpr FloatIn max_plus_one{static_cast<FloatIn>(static_cast<long double>(::std::numeric_limits<IntOut>::max()) + 1.0L)};
            if(x <= min_v) { return ::std::numeric_limits<IntOut>::min(); }
            if(x >= max_plus_one) { return ::std::numeric_limits<IntOut>::max(); }
            return static_cast<IntOut>(x);
        }

        template <typename UIntOut, typename FloatIn>
        inline UIntOut trunc_sat_float_to_int_u(FloatIn x) noexcept
        {
            if(x != x) [[unlikely]] { return UIntOut{}; }
            constexpr FloatIn max_plus_one{static_cast<FloatIn>(static_cast<long double>(::std::numeric_limits<UIntOut>::max()) + 1.0L)};
            if(x <= static_cast<FloatIn>(0)) { return UIntOut{}; }
            if(x >= max_plus_one) { return ::std::numeric_limits<UIntOut>::max(); }
            return static_cast<UIntOut>(x);
        }

        struct static_cast_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
            inline ResultType operator()(SourceType value) const noexcept
            {
                return static_cast<ResultType>(value);
            }
        };

        struct reinterpret_cast_operation
        {
            template <m3_register_value_type ResultType, m3_register_value_type SourceType>
                requires(sizeof(ResultType) == sizeof(SourceType) && ::std::is_trivially_copyable_v<ResultType> &&
                         ::std::is_trivially_copyable_v<SourceType>)
            inline ResultType operator()(SourceType value) const noexcept
            {
                return ::std::bit_cast<ResultType>(value);
            }
        };

        struct wrap_i64_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i32> && ::std::same_as<SourceType, wasm_i64>)
            inline wasm_i32 operator()(wasm_i64 value) const noexcept
            {
                return from_u32_bits<wasm_i32>(static_cast<::std::uint_least32_t>(to_u64_bits(value)));
            }
        };

        struct trunc_s_i32_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i32> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i32 operator()(SourceType value) const noexcept
            {
                return static_cast<wasm_i32>(trunc_float_to_int_s<::std::int_least32_t>(value));
            }
        };

        struct trunc_u32_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i32> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i32 operator()(SourceType value) const noexcept
            {
                return from_u32_bits<wasm_i32>(trunc_float_to_int_u<::std::uint_least32_t>(value));
            }
        };

        struct trunc_s_i64_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i64> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i64 operator()(SourceType value) const noexcept
            {
                return static_cast<wasm_i64>(trunc_float_to_int_s<::std::int_least64_t>(value));
            }
        };

        struct trunc_u64_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i64> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i64 operator()(SourceType value) const noexcept
            {
                return from_u64_bits<wasm_i64>(trunc_float_to_int_u<::std::uint_least64_t>(value));
            }
        };

        struct trunc_sat_s_i32_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i32> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i32 operator()(SourceType value) const noexcept
            {
                return static_cast<wasm_i32>(trunc_sat_float_to_int_s<::std::int_least32_t>(value));
            }
        };

        struct trunc_sat_u32_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i32> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i32 operator()(SourceType value) const noexcept
            {
                return from_u32_bits<wasm_i32>(trunc_sat_float_to_int_u<::std::uint_least32_t>(value));
            }
        };

        struct trunc_sat_s_i64_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i64> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i64 operator()(SourceType value) const noexcept
            {
                return static_cast<wasm_i64>(trunc_sat_float_to_int_s<::std::int_least64_t>(value));
            }
        };

        struct trunc_sat_u64_operation
        {
            template <m3_register_value_type ResultType, m3_value_type SourceType>
                requires(::std::same_as<ResultType, wasm_i64> && (::std::same_as<SourceType, wasm_f32> || ::std::same_as<SourceType, wasm_f64>))
            inline wasm_i64 operator()(SourceType value) const noexcept
            {
                return from_u64_bits<wasm_i64>(trunc_sat_float_to_int_u<::std::uint_least64_t>(value));
            }
        };
    }

    template <m3_register_value_type ResultType, m3_value_type SourceType, register_slot_shape_t Shape, typename Operation>
        requires requires(Operation op, SourceType value) {
            { op.template operator()<ResultType, SourceType>(value) } -> ::std::convertible_to<ResultType>;
        }
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void
    m3_register_slot_transform_op(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        if constexpr(Shape == register_slot_shape_t::rr)
        {
            auto const result{static_cast<ResultType>(Operation{}.template operator()<ResultType, SourceType>(details::load_register_value<SourceType>(r0, fp0)))};
            details::store_register_value<ResultType>(r0, fp0, result);
        }
        else if constexpr(Shape == register_slot_shape_t::rs)
        {
            auto const result{static_cast<ResultType>(Operation{}.template operator()<ResultType, SourceType>(details::load_slot<SourceType>(ip, sp)))};
            details::store_register_value<ResultType>(r0, fp0, result);
        }
        else if constexpr(Shape == register_slot_shape_t::sr)
        {
            auto const result{static_cast<ResultType>(Operation{}.template operator()<ResultType, SourceType>(details::load_register_value<SourceType>(r0, fp0)))};
            details::store_slot<ResultType>(ip, sp, result);
        }
        else
        {
            auto const result{static_cast<ResultType>(Operation{}.template operator()<ResultType, SourceType>(details::load_slot<SourceType>(ip, sp)))};
            details::store_slot<ResultType>(ip, sp, result);
        }
    }

    template <m3_register_value_type ResultType, m3_value_type SourceType, unary_shape_t Shape, typename Operation>
        requires requires(Operation op, SourceType value) {
            { op.template operator()<ResultType, SourceType>(value) } -> ::std::convertible_to<ResultType>;
        }
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void
    m3_unary_transform_op(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        SourceType source{};
        if constexpr(Shape == unary_shape_t::r)
        {
            source = details::load_register_value<SourceType>(r0, fp0);
        }
        else
        {
            source = details::load_slot<SourceType>(ip, sp);
        }

        auto const result{static_cast<ResultType>(Operation{}.template operator()<ResultType, SourceType>(source))};
        details::store_register_value<ResultType>(r0, fp0, result);
    }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Wrap_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_transform_op<wasm_i32, wasm_i64, Shape, convert_details::wrap_i64_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Extend_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_transform_op<wasm_i64, wasm_i32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Extend_u32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_transform_op<wasm_i64, wasm_u32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Demote_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_transform_op<wasm_f32, wasm_f64, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Promote_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_transform_op<wasm_f64, wasm_f32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Convert_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f32, wasm_i32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Convert_u32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f32, wasm_u32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Convert_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f32, wasm_i64, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Convert_u64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f32, wasm_u64, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Convert_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f64, wasm_i32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Convert_u32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f64, wasm_u32, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Convert_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f64, wasm_i64, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Convert_u64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f64, wasm_u64, Shape, convert_details::static_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Reinterpret_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f32, Shape, convert_details::reinterpret_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Reinterpret_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f32, wasm_i32, Shape, convert_details::reinterpret_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Reinterpret_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f64, Shape, convert_details::reinterpret_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Reinterpret_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_f64, wasm_i64, Shape, convert_details::reinterpret_cast_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Trunc_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f32, Shape, convert_details::trunc_s_i32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Trunc_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f32, Shape, convert_details::trunc_u32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Trunc_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f64, Shape, convert_details::trunc_s_i32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Trunc_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f64, Shape, convert_details::trunc_u32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Trunc_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f32, Shape, convert_details::trunc_s_i64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Trunc_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f32, Shape, convert_details::trunc_u64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Trunc_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f64, Shape, convert_details::trunc_s_i64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Trunc_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f64, Shape, convert_details::trunc_u64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_TruncSat_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f32, Shape, convert_details::trunc_sat_s_i32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_TruncSat_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f32, Shape, convert_details::trunc_sat_u32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_TruncSat_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f64, Shape, convert_details::trunc_sat_s_i32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_TruncSat_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i32, wasm_f64, Shape, convert_details::trunc_sat_u32_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_TruncSat_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f32, Shape, convert_details::trunc_sat_s_i64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_TruncSat_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f32, Shape, convert_details::trunc_sat_u64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_TruncSat_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f64, Shape, convert_details::trunc_sat_s_i64_operation>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_TruncSat_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_register_slot_transform_op<wasm_i64, wasm_f64, Shape, convert_details::trunc_sat_u64_operation>(ip, sp, r0, fp0); }

    namespace translate
    {
        template <m3_register_value_type ResultType, m3_value_type SourceType, register_slot_shape_t Shape, typename Operation>
            requires requires(Operation op, SourceType value) {
                { op.template operator()<ResultType, SourceType>(value) } -> ::std::convertible_to<ResultType>;
            }
        inline consteval m3_interpreter_opfunc_t get_register_slot_transform_opfunc() noexcept
        {
            return &m3_register_slot_transform_op<ResultType, SourceType, Shape, Operation>;
        }

        template <m3_register_value_type ResultType, m3_value_type SourceType, unary_shape_t Shape, typename Operation>
            requires requires(Operation op, SourceType value) {
                { op.template operator()<ResultType, SourceType>(value) } -> ::std::convertible_to<ResultType>;
            }
        inline consteval m3_interpreter_opfunc_t get_unary_transform_opfunc() noexcept
        {
            return &m3_unary_transform_op<ResultType, SourceType, Shape, Operation>;
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Convert_i32_fptr() noexcept
        {
            return get_register_slot_transform_opfunc<wasm_f64, wasm_i32, Shape, convert_details::static_cast_operation>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Convert_u64_fptr() noexcept
        {
            return get_register_slot_transform_opfunc<wasm_f64, wasm_u64, Shape, convert_details::static_cast_operation>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Reinterpret_f32_fptr() noexcept
        {
            return get_register_slot_transform_opfunc<wasm_i32, wasm_f32, Shape, convert_details::reinterpret_cast_operation>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Trunc_f64_fptr() noexcept
        {
            return get_register_slot_transform_opfunc<wasm_i32, wasm_f64, Shape, convert_details::trunc_s_i32_operation>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_u32_TruncSat_f32_fptr() noexcept
        {
            return get_register_slot_transform_opfunc<wasm_i32, wasm_f32, Shape, convert_details::trunc_sat_u32_operation>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Wrap_i64_fptr() noexcept
        {
            return get_unary_transform_opfunc<wasm_i32, wasm_i64, Shape, convert_details::wrap_i64_operation>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Extend_u32_fptr() noexcept
        {
            return get_unary_transform_opfunc<wasm_i64, wasm_u32, Shape, convert_details::static_cast_operation>();
        }
    }
}
