#pragma once

#ifndef UWVM_MODULE
# include <bit>
# include <cmath>
# include <concepts>
# include <exception>
# include <limits>
# include <type_traits>
# include <uwvm2/runtime/compiler/m3_int/optable/define.h>
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::optable
{
    namespace numeric_details
    {
        template <m3_value_type T>
            requires(::std::integral<T>)
        inline constexpr unsigned wasm_bit_width_v = (::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_u32>) ? 32u : 64u;

        template <m3_register_value_type ResultType, typename Value>
        inline ResultType normalize_result(Value value) noexcept
        {
            if constexpr(::std::same_as<ResultType, wasm_i32> && ::std::same_as<Value, wasm_u32>)
            {
                return ::std::bit_cast<wasm_i32>(value);
            }
            else if constexpr(::std::same_as<ResultType, wasm_i64> && ::std::same_as<Value, wasm_u64>)
            {
                return ::std::bit_cast<wasm_i64>(value);
            }
            else
            {
                return static_cast<ResultType>(value);
            }
        }

        template <m3_value_type T>
            requires(::std::signed_integral<T>)
        inline T arithmetic_shift_right(T value, ::std::make_unsigned_t<T> shift_count) noexcept
        {
            using unsigned_t = ::std::make_unsigned_t<T>;
            constexpr auto width{wasm_bit_width_v<T>};
            auto const amount{static_cast<unsigned>(shift_count % width)};
            if(amount == 0u) { return value; }

            auto const bits{::std::bit_cast<unsigned_t>(value)};
            auto shifted{static_cast<unsigned_t>(bits >> amount)};
            if(value < 0)
            {
                auto const fill_mask{static_cast<unsigned_t>(~unsigned_t{} << (width - amount))};
                shifted = static_cast<unsigned_t>(shifted | fill_mask);
            }
            return ::std::bit_cast<T>(shifted);
        }


        [[noreturn]] inline void trap_integer_divide_by_zero() noexcept
        {
            if(::uwvm2::runtime::compiler::m3_int::optable::trap_integer_divide_by_zero_func != nullptr)
            {
                ::uwvm2::runtime::compiler::m3_int::optable::trap_integer_divide_by_zero_func();
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

        struct add_operation
        {
            template <m3_value_type T>
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if constexpr(::std::integral<T>)
                {
                    using unsigned_t = ::std::make_unsigned_t<T>;
                    return static_cast<T>(static_cast<unsigned_t>(lhs) + static_cast<unsigned_t>(rhs));
                }
                else
                {
                    return static_cast<T>(lhs + rhs);
                }
            }
        };

        struct subtract_operation
        {
            template <m3_value_type T>
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if constexpr(::std::integral<T>)
                {
                    using unsigned_t = ::std::make_unsigned_t<T>;
                    return static_cast<T>(static_cast<unsigned_t>(lhs) - static_cast<unsigned_t>(rhs));
                }
                else
                {
                    return static_cast<T>(lhs - rhs);
                }
            }
        };

        struct multiply_operation
        {
            template <m3_value_type T>
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if constexpr(::std::integral<T>)
                {
                    using unsigned_t = ::std::make_unsigned_t<T>;
                    return static_cast<T>(static_cast<unsigned_t>(lhs) * static_cast<unsigned_t>(rhs));
                }
                else
                {
                    return static_cast<T>(lhs * rhs);
                }
            }
        };

        struct float_divide_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<T>(lhs / rhs);
            }
        };

        struct min_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(::std::isnan(lhs) || ::std::isnan(rhs)) [[unlikely]]
                {
                    return ::std::numeric_limits<T>::quiet_NaN();
                }
                if(lhs == T{} && rhs == T{}) [[unlikely]]
                {
                    return ::std::signbit(lhs) ? lhs : rhs;
                }
                return lhs > rhs ? rhs : lhs;
            }
        };

        struct max_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(::std::isnan(lhs) || ::std::isnan(rhs)) [[unlikely]]
                {
                    return ::std::numeric_limits<T>::quiet_NaN();
                }
                if(lhs == T{} && rhs == T{}) [[unlikely]]
                {
                    return ::std::signbit(lhs) ? rhs : lhs;
                }
                return lhs > rhs ? lhs : rhs;
            }
        };

        struct copysign_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<T>(::std::copysign(lhs, rhs));
            }
        };

        struct signed_divide_operation
        {
            template <m3_value_type T>
                requires(::std::signed_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(rhs == T{}) [[unlikely]] { trap_integer_divide_by_zero(); }
                if(lhs == ::std::numeric_limits<T>::min() && rhs == T{-1}) [[unlikely]] { trap_integer_overflow(); }
                return static_cast<T>(lhs / rhs);
            }
        };

        struct unsigned_divide_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(rhs == T{}) [[unlikely]] { trap_integer_divide_by_zero(); }
                return static_cast<T>(lhs / rhs);
            }
        };

        struct signed_remainder_operation
        {
            template <m3_value_type T>
                requires(::std::signed_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(rhs == T{}) [[unlikely]] { trap_integer_divide_by_zero(); }
                if(lhs == ::std::numeric_limits<T>::min() && rhs == T{-1}) [[unlikely]] { return T{}; }
                return static_cast<T>(lhs % rhs);
            }
        };

        struct unsigned_remainder_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                if(rhs == T{}) [[unlikely]] { trap_integer_divide_by_zero(); }
                return static_cast<T>(lhs % rhs);
            }
        };

        struct abs_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::fabs;
                return static_cast<T>(fabs(value));
            }
        };

        struct ceil_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::ceil;
                return static_cast<T>(ceil(value));
            }
        };

        struct floor_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::floor;
                return static_cast<T>(floor(value));
            }
        };

        struct trunc_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::trunc;
                return static_cast<T>(trunc(value));
            }
        };

        struct sqrt_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::sqrt;
                return static_cast<T>(sqrt(value));
            }
        };

        struct nearest_operation
        {
            template <m3_value_type T>
                requires(::std::floating_point<T>)
            inline T operator()(T value) const noexcept
            {
                using ::std::rint;
                return static_cast<T>(rint(value));
            }
        };

        struct equal_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs == rhs);
            }
        };

        struct not_equal_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs != rhs);
            }
        };

        struct less_than_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs < rhs);
            }
        };

        struct greater_than_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs > rhs);
            }
        };

        struct less_equal_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs <= rhs);
            }
        };

        struct greater_equal_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<wasm_i32>(lhs >= rhs);
            }
        };

        struct and_operation
        {
            template <m3_value_type T>
                requires(::std::integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<T>(lhs & rhs);
            }
        };

        struct or_operation
        {
            template <m3_value_type T>
                requires(::std::integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<T>(lhs | rhs);
            }
        };

        struct xor_operation
        {
            template <m3_value_type T>
                requires(::std::integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                return static_cast<T>(lhs ^ rhs);
            }
        };

        struct shift_left_operation
        {
            template <m3_value_type T>
                requires(::std::integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                using unsigned_t = ::std::make_unsigned_t<T>;
                constexpr auto width{wasm_bit_width_v<T>};
                auto const amount{static_cast<unsigned_t>(rhs) % width};
                return static_cast<T>(static_cast<unsigned_t>(lhs) << amount);
            }
        };

        struct shift_right_operation
        {
            template <m3_value_type T>
                requires(::std::integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                using unsigned_t = ::std::make_unsigned_t<T>;
                constexpr auto width{wasm_bit_width_v<T>};
                auto const amount{static_cast<unsigned_t>(rhs) % width};
                if constexpr(::std::signed_integral<T>)
                {
                    return arithmetic_shift_right(lhs, amount);
                }
                else
                {
                    return static_cast<T>(static_cast<unsigned_t>(lhs) >> amount);
                }
            }
        };

        struct rotate_left_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                constexpr auto width{wasm_bit_width_v<T>};
                return static_cast<T>(::std::rotl(lhs, static_cast<int>(rhs % width)));
            }
        };

        struct rotate_right_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T lhs, T rhs) const noexcept
            {
                constexpr auto width{wasm_bit_width_v<T>};
                return static_cast<T>(::std::rotr(lhs, static_cast<int>(rhs % width)));
            }
        };

        struct equal_to_zero_operation
        {
            template <m3_value_type T>
            inline wasm_i32 operator()(T value) const noexcept
            {
                return static_cast<wasm_i32>(value == static_cast<T>(0));
            }
        };

        struct countl_zero_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(::std::countl_zero(value));
            }
        };

        struct countr_zero_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(::std::countr_zero(value));
            }
        };

        struct popcount_operation
        {
            template <m3_value_type T>
                requires(::std::unsigned_integral<T>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(::std::popcount(value));
            }
        };

        struct negate_operation
        {
            template <m3_value_type T>
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(-value);
            }
        };

        struct extend8_s_operation
        {
            template <m3_value_type T>
                requires(::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_i64>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(static_cast<::std::int_least8_t>(value));
            }
        };

        struct extend16_s_operation
        {
            template <m3_value_type T>
                requires(::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_i64>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(static_cast<::std::int_least16_t>(value));
            }
        };

        struct extend32_s_operation
        {
            template <m3_value_type T>
                requires(::std::same_as<T, wasm_i64>)
            inline T operator()(T value) const noexcept
            {
                return static_cast<T>(static_cast<::std::int_least32_t>(value));
            }
        };
    }

    template <m3_register_value_type ResultType, m3_value_type ValueType, binary_shape_t Shape, typename Operation>
        requires requires(Operation op, ValueType lhs, ValueType rhs) {
            { op(lhs, rhs) };
        }
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void m3_binary_op(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        ValueType lhs{};
        ValueType rhs{};

        if constexpr(Shape == binary_shape_t::sr)
        {
            lhs = details::load_register_value<ValueType>(r0, fp0);
            rhs = details::load_slot<ValueType>(ip, sp);
        }
        else if constexpr(Shape == binary_shape_t::rs)
        {
            lhs = details::load_slot<ValueType>(ip, sp);
            rhs = details::load_register_value<ValueType>(r0, fp0);
        }
        else
        {
            rhs = details::load_slot<ValueType>(ip, sp);
            lhs = details::load_slot<ValueType>(ip, sp);
        }

        auto const result{Operation{}(lhs, rhs)};
        details::store_register_value<ResultType>(r0, fp0, numeric_details::normalize_result<ResultType>(result));
    }

    template <m3_register_value_type ResultType, m3_value_type ValueType, unary_shape_t Shape, typename Operation>
        requires requires(Operation op, ValueType value) {
            { op(value) };
        }
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void m3_unary_op(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        ValueType value{};
        if constexpr(Shape == unary_shape_t::r)
        {
            value = details::load_register_value<ValueType>(r0, fp0);
        }
        else
        {
            value = details::load_slot<ValueType>(ip, sp);
        }

        auto const result{Operation{}(value)};
        details::store_register_value<ResultType>(r0, fp0, numeric_details::normalize_result<ResultType>(result));
    }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Add(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::add_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Subtract(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::subtract_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Multiply(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::multiply_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Add(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::add_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Subtract(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::subtract_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Multiply(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::multiply_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::signed_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::signed_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::unsigned_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::unsigned_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Remainder(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::signed_remainder_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Remainder(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::signed_remainder_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Remainder(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::unsigned_remainder_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Remainder(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::unsigned_remainder_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Add(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::add_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Subtract(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::subtract_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Multiply(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::multiply_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::float_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Min(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::min_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Max(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::max_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_CopySign(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f32, wasm_f32, Shape, numeric_details::copysign_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Add(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::add_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Subtract(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::subtract_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Multiply(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::multiply_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Divide(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::float_divide_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Min(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::min_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Max(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::max_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_CopySign(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_f64, wasm_f64, Shape, numeric_details::copysign_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Equal(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_NotEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::not_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Equal(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_NotEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::not_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i64, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u64, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u64, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u64, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u64, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Equal(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_NotEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::not_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f32, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Equal(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_NotEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::not_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_LessThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::less_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_GreaterThan(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::greater_than_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_LessThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::less_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_GreaterThanOrEqual(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_f64, Shape, numeric_details::greater_equal_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_And(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::and_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Or(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::or_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Xor(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::xor_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_And(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::and_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Or(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::or_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Xor(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::xor_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_ShiftLeft(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::shift_left_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_ShiftLeft(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::shift_left_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_ShiftRight(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_i32, Shape, numeric_details::shift_right_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_ShiftRight(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::shift_right_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_ShiftRight(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_i64, Shape, numeric_details::shift_right_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_ShiftRight(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::shift_right_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Rotl(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::rotate_left_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Rotr(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i32, wasm_u32, Shape, numeric_details::rotate_right_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Rotl(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::rotate_left_operation>(ip, sp, r0, fp0); }

    template <binary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Rotr(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_binary_op<wasm_i64, wasm_u64, Shape, numeric_details::rotate_right_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_EqualToZero(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_i32, Shape, numeric_details::equal_to_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_EqualToZero(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_i64, Shape, numeric_details::equal_to_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Clz(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_u32, Shape, numeric_details::countl_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Ctz(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_u32, Shape, numeric_details::countr_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u32_Popcnt(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_u32, Shape, numeric_details::popcount_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Clz(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_u64, Shape, numeric_details::countl_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Ctz(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_u64, Shape, numeric_details::countr_zero_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void u64_Popcnt(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_u64, Shape, numeric_details::popcount_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Negate(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::negate_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Negate(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::negate_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Abs(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::abs_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Ceil(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::ceil_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Floor(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::floor_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Trunc(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::trunc_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Sqrt(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::sqrt_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Nearest(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f32, wasm_f32, Shape, numeric_details::nearest_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Abs(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::abs_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Ceil(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::ceil_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Floor(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::floor_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Trunc(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::trunc_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Sqrt(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::sqrt_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Nearest(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_f64, wasm_f64, Shape, numeric_details::nearest_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Extend8_s(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_i32, Shape, numeric_details::extend8_s_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Extend16_s(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i32, wasm_i32, Shape, numeric_details::extend16_s_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Extend8_s(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_i64, Shape, numeric_details::extend8_s_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Extend16_s(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_i64, Shape, numeric_details::extend16_s_operation>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Extend32_s(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_unary_op<wasm_i64, wasm_i64, Shape, numeric_details::extend32_s_operation>(ip, sp, r0, fp0); }

    namespace translate
    {
        template <m3_register_value_type ResultType, m3_value_type ValueType, binary_shape_t Shape, typename Operation>
            requires requires(Operation op, ValueType lhs, ValueType rhs) {
                { op(lhs, rhs) };
            }
        inline consteval m3_interpreter_opfunc_t get_binary_opfunc() noexcept
        {
            return &m3_binary_op<ResultType, ValueType, Shape, Operation>;
        }

        template <m3_register_value_type ResultType, m3_value_type ValueType, binary_shape_t Shape, typename Operation>
            requires requires(Operation op, ValueType lhs, ValueType rhs) {
                { op(lhs, rhs) };
            }
        inline consteval m3_interpreter_opfunc_t get_commutative_binary_opfunc() noexcept
        {
            if constexpr(Shape == binary_shape_t::sr)
            {
                return &m3_binary_op<ResultType, ValueType, binary_shape_t::rs, Operation>;
            }
            else
            {
                return &m3_binary_op<ResultType, ValueType, Shape, Operation>;
            }
        }

        template <m3_register_value_type ResultType, m3_value_type ValueType, unary_shape_t Shape, typename Operation>
            requires requires(Operation op, ValueType value) {
                { op(value) };
            }
        inline consteval m3_interpreter_opfunc_t get_unary_opfunc() noexcept
        {
            return &m3_unary_op<ResultType, ValueType, Shape, Operation>;
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Add_fptr() noexcept
        {
            return get_commutative_binary_opfunc<wasm_i32, wasm_i32, Shape, numeric_details::add_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Subtract_fptr() noexcept
        {
            return get_binary_opfunc<wasm_i32, wasm_i32, Shape, numeric_details::subtract_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Divide_fptr() noexcept
        {
            return get_binary_opfunc<wasm_i32, wasm_i32, Shape, numeric_details::signed_divide_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_u32_Remainder_fptr() noexcept
        {
            return get_binary_opfunc<wasm_i32, wasm_u32, Shape, numeric_details::unsigned_remainder_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_NotEqual_fptr() noexcept
        {
            return get_commutative_binary_opfunc<wasm_i32, wasm_i32, Shape, numeric_details::not_equal_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f32_Min_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f32, wasm_f32, Shape, numeric_details::min_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f32_Max_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f32, wasm_f32, Shape, numeric_details::max_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f32_CopySign_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f32, wasm_f32, Shape, numeric_details::copysign_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Min_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f64, wasm_f64, Shape, numeric_details::min_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Max_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f64, wasm_f64, Shape, numeric_details::max_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_CopySign_fptr() noexcept
        {
            return get_binary_opfunc<wasm_f64, wasm_f64, Shape, numeric_details::copysign_operation>();
        }

        template <binary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_u32_Rotl_fptr() noexcept
        {
            return get_binary_opfunc<wasm_i32, wasm_u32, Shape, numeric_details::rotate_left_operation>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_EqualToZero_fptr() noexcept
        {
            return get_unary_opfunc<wasm_i32, wasm_i32, Shape, numeric_details::equal_to_zero_operation>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Sqrt_fptr() noexcept
        {
            return get_unary_opfunc<wasm_f64, wasm_f64, Shape, numeric_details::sqrt_operation>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_u32_Clz_fptr() noexcept
        {
            return get_unary_opfunc<wasm_i32, wasm_u32, Shape, numeric_details::countl_zero_operation>();
        }
    }
}
