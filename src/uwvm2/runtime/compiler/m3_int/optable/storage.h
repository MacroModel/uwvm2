#pragma once

#ifndef UWVM_MODULE
# include <limits>
# include <uwvm2/runtime/compiler/m3_int/optable/define.h>
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::optable
{
    namespace storage_details
    {
        template <typename T>
        concept memory_storage_value_type = (::std::integral<T> || ::std::floating_point<T>) && ::std::is_trivially_copyable_v<T>;

        [[noreturn]] inline void trap_out_of_bounds_memory_access() noexcept
        {
            if(::uwvm2::runtime::compiler::m3_int::optable::trap_out_of_bounds_memory_access_func != nullptr)
            {
                ::uwvm2::runtime::compiler::m3_int::optable::trap_out_of_bounds_memory_access_func();
            }
            ::std::terminate();
        }

        inline constexpr wasm_u32 default_memory_page_size{wasm_u32{65536u}};

        inline wasm_u32 current_memory_page_size() noexcept
        {
            return memory_view.page_size == wasm_u32{} ? default_memory_page_size : memory_view.page_size;
        }

        inline wasm_u32 current_memory_page_count() noexcept
        {
            if(memory_view.page_count != wasm_u32{}) { return memory_view.page_count; }
            if(memory_view.size == ::std::size_t{}) { return wasm_u32{}; }
            auto const page_size{static_cast<::std::size_t>(current_memory_page_size())};
            return static_cast<wasm_u32>(((memory_view.size - 1u) / page_size) + 1u);
        }

        inline bool memory_range_valid(wasm_u64 operand, ::std::size_t width) noexcept
        {
            if(memory_view.data == nullptr) { return false; }
            if(operand > static_cast<wasm_u64>((::std::numeric_limits<::std::size_t>::max)())) { return false; }
            auto const offset{static_cast<::std::size_t>(operand)};
            if(offset > memory_view.size) { return false; }
            return width <= memory_view.size - offset;
        }

        template <memory_storage_value_type T>
        inline T swap_memory_order(T value) noexcept
        {
            if constexpr(sizeof(T) == 1 || ::std::endian::native == ::std::endian::little)
            {
                return value;
            }
            else if constexpr(::std::integral<T>)
            {
                return ::std::byteswap(value);
            }
            else if constexpr(::std::same_as<T, wasm_f32>)
            {
                return ::std::bit_cast<wasm_f32>(::std::byteswap(::std::bit_cast<wasm_u32>(value)));
            }
            else
            {
                return ::std::bit_cast<wasm_f64>(::std::byteswap(::std::bit_cast<wasm_u64>(value)));
            }
        }

        template <memory_storage_value_type T>
        inline T load_memory_value(wasm_u64 operand) noexcept
        {
            if(!memory_range_valid(operand, sizeof(T))) [[unlikely]] { trap_out_of_bounds_memory_access(); }
            auto const offset{static_cast<::std::size_t>(operand)};
            T value{};
            ::std::memcpy(::std::addressof(value), memory_view.data + static_cast<::std::ptrdiff_t>(offset), sizeof(T));
            return swap_memory_order(value);
        }

        template <memory_storage_value_type T>
        inline void store_memory_value(wasm_u64 operand, T value) noexcept
        {
            if(!memory_range_valid(operand, sizeof(T))) [[unlikely]] { trap_out_of_bounds_memory_access(); }
            auto const offset{static_cast<::std::size_t>(operand)};
            auto const stored{swap_memory_order(value)};
            ::std::memcpy(memory_view.data + static_cast<::std::ptrdiff_t>(offset), ::std::addressof(stored), sizeof(T));
        }
    }

    template <m3_register_value_type ValueType>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetRegister(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        details::store_register_value<ValueType>(r0, fp0, details::load_slot<ValueType>(ip, sp));
    }

    template <m3_register_value_type ValueType>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetSlot(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        details::store_slot<ValueType>(ip, sp, details::load_register_value<ValueType>(r0, fp0));
    }

    template <m3_register_value_type ValueType>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void PreserveSetSlot(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        auto const stack_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const preserve_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const previous{details::load_slot_at<ValueType>(sp, stack_offset)};
        details::store_slot_at<ValueType>(sp, preserve_offset, previous);
        details::store_slot_at<ValueType>(sp, stack_offset, details::load_register_value<ValueType>(r0, fp0));
    }

    template <typename ValueType>
        requires(::std::is_trivially_copyable_v<ValueType>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void CopySlot(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const dest_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const src_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        details::store_slot_at<ValueType>(sp, dest_offset, details::load_slot_at<ValueType>(sp, src_offset));
    }

    template <typename ValueType>
        requires(::std::is_trivially_copyable_v<ValueType>)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void PreserveCopySlot(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const dest_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const src_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const preserve_offset{details::read_immediate<m3_slot_offset_t>(ip)};
        auto const previous{details::load_slot_at<ValueType>(sp, dest_offset)};
        details::store_slot_at<ValueType>(sp, preserve_offset, previous);
        details::store_slot_at<ValueType>(sp, dest_offset, details::load_slot_at<ValueType>(sp, src_offset));
    }

    template <m3_register_value_type ResultType, storage_details::memory_storage_value_type SourceType, unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void m3_load(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        wasm_u64 operand{};
        if constexpr(Shape == unary_shape_t::r)
        {
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand = static_cast<wasm_u64>(static_cast<wasm_u32>(r0));
            operand += offset;
        }
        else
        {
            operand = static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp));
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand += offset;
        }

        auto const value{storage_details::load_memory_value<SourceType>(operand)};
        details::store_register_value<ResultType>(r0, fp0, static_cast<ResultType>(value));
    }

    template <storage_details::memory_storage_value_type SourceType, storage_details::memory_storage_value_type DestType, register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void m3_store(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    {
        SourceType value{};
        wasm_u64 operand{};

        if constexpr(Shape == register_slot_shape_t::rr)
        {
            value = details::load_register_value<SourceType>(r0, fp0);
            operand = static_cast<wasm_u64>(static_cast<wasm_u32>(r0));
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand += offset;
        }
        else if constexpr(Shape == register_slot_shape_t::rs)
        {
            operand = static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp));
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand += offset;
            value = details::load_register_value<SourceType>(r0, fp0);
        }
        else if constexpr(Shape == register_slot_shape_t::sr)
        {
            value = details::load_slot<SourceType>(ip, sp);
            operand = static_cast<wasm_u64>(static_cast<wasm_u32>(r0));
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand += offset;
        }
        else
        {
            value = details::load_slot<SourceType>(ip, sp);
            operand = static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp));
            auto const offset{details::read_immediate<wasm_u32>(ip)};
            operand += offset;
        }

        storage_details::store_memory_value<DestType>(operand, static_cast<DestType>(value));
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void MemSize(m3_code_t&, m3_stack_t, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        r0 = static_cast<m3_int_register_t>(static_cast<wasm_i32>(storage_details::current_memory_page_count()));
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void MemGrow(m3_code_t&, m3_stack_t, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const requested_pages{static_cast<wasm_i32>(r0)};
        if(requested_pages < wasm_i32{})
        {
            r0 = static_cast<m3_int_register_t>(wasm_i32{-1});
            return;
        }

        auto const previous_pages{static_cast<wasm_i32>(storage_details::current_memory_page_count())};
        if(requested_pages == wasm_i32{})
        {
            r0 = static_cast<m3_int_register_t>(previous_pages);
            return;
        }

        if(memory_grow_func == nullptr)
        {
            r0 = static_cast<m3_int_register_t>(wasm_i32{-1});
            return;
        }

        r0 = static_cast<m3_int_register_t>(memory_grow_func(requested_pages));
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void MemCopy(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const size{static_cast<wasm_u32>(r0)};
        auto const source{static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp))};
        auto const destination{static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp))};

        if(!storage_details::memory_range_valid(destination, static_cast<::std::size_t>(size))) [[unlikely]]
        {
            storage_details::trap_out_of_bounds_memory_access();
        }
        if(!storage_details::memory_range_valid(source, static_cast<::std::size_t>(size))) [[unlikely]]
        {
            storage_details::trap_out_of_bounds_memory_access();
        }

        ::std::memmove(memory_view.data + static_cast<::std::ptrdiff_t>(destination),
                       memory_view.data + static_cast<::std::ptrdiff_t>(source),
                       static_cast<::std::size_t>(size));
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void MemFill(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const size{static_cast<wasm_u32>(r0)};
        auto const byte_value{details::load_slot<wasm_u32>(ip, sp)};
        auto const destination{static_cast<wasm_u64>(details::load_slot<wasm_u32>(ip, sp))};

        if(!storage_details::memory_range_valid(destination, static_cast<::std::size_t>(size))) [[unlikely]]
        {
            storage_details::trap_out_of_bounds_memory_access();
        }

        ::std::memset(memory_view.data + static_cast<::std::ptrdiff_t>(destination),
                      static_cast<int>(static_cast<::std::uint_least8_t>(byte_value)),
                      static_cast<::std::size_t>(size));
    }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Load_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_f32, wasm_f32, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Load_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_f64, wasm_f64, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Load_i8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i32, ::std::int_least8_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Load_u8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i32, ::std::uint_least8_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Load_i16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i32, ::std::int_least16_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Load_u16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i32, ::std::uint_least16_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Load_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i32, wasm_i32, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_i8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, ::std::int_least8_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_u8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, ::std::uint_least8_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_i16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, ::std::int_least16_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_u16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, ::std::uint_least16_t, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, wasm_i32, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_u32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, wasm_u32, Shape>(ip, sp, r0, fp0); }

    template <unary_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Load_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_load<wasm_i64, wasm_i64, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f32_Store_f32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_f32, wasm_f32, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void f64_Store_f64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_f64, wasm_f64, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Store_u8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i32, ::std::uint_least8_t, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Store_i16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i32, ::std::int_least16_t, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i32_Store_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i32, wasm_i32, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Store_u8(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i64, ::std::uint_least8_t, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Store_i16(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i64, ::std::int_least16_t, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Store_i32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i64, wasm_i32, Shape>(ip, sp, r0, fp0); }

    template <register_slot_shape_t Shape>
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void i64_Store_i64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t& r0, m3_fp_register_t& fp0) noexcept
    { m3_store<wasm_i64, wasm_i64, Shape>(ip, sp, r0, fp0); }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Const32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const value{details::read_immediate<wasm_u32>(ip)};
        details::store_slot<wasm_u32>(ip, sp, value);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void Const64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const value{details::read_immediate<wasm_u64>(ip)};
        details::store_slot<wasm_u64>(ip, sp, value);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void GetGlobal_s32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u32 const*>(ip)};
        details::store_slot<wasm_u32>(ip, sp, *global);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void GetGlobal_s64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u64 const*>(ip)};
        details::store_slot<wasm_u64>(ip, sp, *global);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_i32(m3_code_t& ip, m3_stack_t, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u32*>(ip)};
        *global = static_cast<wasm_u32>(static_cast<wasm_i32>(r0));
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_i64(m3_code_t& ip, m3_stack_t, m3_int_register_t& r0, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u64*>(ip)};
        *global = static_cast<wasm_u64>(r0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_s32(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u32*>(ip)};
        *global = details::load_slot<wasm_u32>(ip, sp);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_s64(m3_code_t& ip, m3_stack_t sp, m3_int_register_t&, m3_fp_register_t&) noexcept
    {
        auto const global{details::read_immediate<wasm_u64*>(ip)};
        *global = details::load_slot<wasm_u64>(ip, sp);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_f32(m3_code_t& ip, m3_stack_t, m3_int_register_t&, m3_fp_register_t& fp0) noexcept
    {
        auto const global{details::read_immediate<wasm_f32*>(ip)};
        *global = static_cast<wasm_f32>(fp0);
    }

    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline void SetGlobal_f64(m3_code_t& ip, m3_stack_t, m3_int_register_t&, m3_fp_register_t& fp0) noexcept
    {
        auto const global{details::read_immediate<wasm_f64*>(ip)};
        *global = static_cast<wasm_f64>(fp0);
    }

    namespace translate
    {
        template <m3_register_value_type ResultType, storage_details::memory_storage_value_type SourceType, unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_load_fptr() noexcept
        {
            return &m3_load<ResultType, SourceType, Shape>;
        }

        template <storage_details::memory_storage_value_type SourceType, storage_details::memory_storage_value_type DestType, register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_store_fptr() noexcept
        {
            return &m3_store<SourceType, DestType, Shape>;
        }

        inline consteval m3_interpreter_opfunc_t get_MemSize_fptr() noexcept
        {
            return &MemSize;
        }

        inline consteval m3_interpreter_opfunc_t get_MemGrow_fptr() noexcept
        {
            return &MemGrow;
        }

        inline consteval m3_interpreter_opfunc_t get_MemCopy_fptr() noexcept
        {
            return &MemCopy;
        }

        inline consteval m3_interpreter_opfunc_t get_MemFill_fptr() noexcept
        {
            return &MemFill;
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f32_Load_f32_fptr() noexcept
        {
            return get_load_fptr<wasm_f32, wasm_f32, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Load_f64_fptr() noexcept
        {
            return get_load_fptr<wasm_f64, wasm_f64, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Load_i8_fptr() noexcept
        {
            return get_load_fptr<wasm_i32, ::std::int_least8_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Load_u8_fptr() noexcept
        {
            return get_load_fptr<wasm_i32, ::std::uint_least8_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Load_i16_fptr() noexcept
        {
            return get_load_fptr<wasm_i32, ::std::int_least16_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Load_u16_fptr() noexcept
        {
            return get_load_fptr<wasm_i32, ::std::uint_least16_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Load_i32_fptr() noexcept
        {
            return get_load_fptr<wasm_i32, wasm_i32, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_i8_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, ::std::int_least8_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_u8_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, ::std::uint_least8_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_i16_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, ::std::int_least16_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_u16_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, ::std::uint_least16_t, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_i32_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, wasm_i32, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_u32_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, wasm_u32, Shape>();
        }

        template <unary_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Load_i64_fptr() noexcept
        {
            return get_load_fptr<wasm_i64, wasm_i64, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f32_Store_f32_fptr() noexcept
        {
            return get_store_fptr<wasm_f32, wasm_f32, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_f64_Store_f64_fptr() noexcept
        {
            return get_store_fptr<wasm_f64, wasm_f64, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Store_u8_fptr() noexcept
        {
            return get_store_fptr<wasm_i32, ::std::uint_least8_t, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Store_i16_fptr() noexcept
        {
            return get_store_fptr<wasm_i32, ::std::int_least16_t, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i32_Store_i32_fptr() noexcept
        {
            return get_store_fptr<wasm_i32, wasm_i32, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Store_u8_fptr() noexcept
        {
            return get_store_fptr<wasm_i64, ::std::uint_least8_t, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Store_i16_fptr() noexcept
        {
            return get_store_fptr<wasm_i64, ::std::int_least16_t, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Store_i32_fptr() noexcept
        {
            return get_store_fptr<wasm_i64, wasm_i32, Shape>();
        }

        template <register_slot_shape_t Shape>
        inline consteval m3_interpreter_opfunc_t get_i64_Store_i64_fptr() noexcept
        {
            return get_store_fptr<wasm_i64, wasm_i64, Shape>();
        }

        inline consteval m3_interpreter_opfunc_t get_Const32_fptr() noexcept
        {
            return &Const32;
        }

        inline consteval m3_interpreter_opfunc_t get_Const64_fptr() noexcept
        {
            return &Const64;
        }

        inline consteval m3_interpreter_opfunc_t get_GetGlobal_s32_fptr() noexcept
        {
            return &GetGlobal_s32;
        }

        inline consteval m3_interpreter_opfunc_t get_GetGlobal_s64_fptr() noexcept
        {
            return &GetGlobal_s64;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_i32_fptr() noexcept
        {
            return &SetGlobal_i32;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_i64_fptr() noexcept
        {
            return &SetGlobal_i64;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_s32_fptr() noexcept
        {
            return &SetGlobal_s32;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_s64_fptr() noexcept
        {
            return &SetGlobal_s64;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_f32_fptr() noexcept
        {
            return &SetGlobal_f32;
        }

        inline consteval m3_interpreter_opfunc_t get_SetGlobal_f64_fptr() noexcept
        {
            return &SetGlobal_f64;
        }

        template <m3_register_value_type ValueType>
        inline consteval m3_interpreter_opfunc_t get_SetRegister_fptr() noexcept
        {
            return &SetRegister<ValueType>;
        }

        template <m3_register_value_type ValueType>
        inline consteval m3_interpreter_opfunc_t get_SetSlot_fptr() noexcept
        {
            return &SetSlot<ValueType>;
        }

        template <m3_register_value_type ValueType>
        inline consteval m3_interpreter_opfunc_t get_PreserveSetSlot_fptr() noexcept
        {
            return &PreserveSetSlot<ValueType>;
        }

        template <typename ValueType>
            requires(::std::is_trivially_copyable_v<ValueType>)
        inline consteval m3_interpreter_opfunc_t get_CopySlot_fptr() noexcept
        {
            return &CopySlot<ValueType>;
        }

        template <typename ValueType>
            requires(::std::is_trivially_copyable_v<ValueType>)
        inline consteval m3_interpreter_opfunc_t get_PreserveCopySlot_fptr() noexcept
        {
            return &PreserveCopySlot<ValueType>;
        }
    }
}
