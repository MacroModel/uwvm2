#pragma once

#ifndef UWVM_MODULE
# include <bit>
# include <cstddef>
# include <cstdint>
# include <cstring>
# include <optional>
# include <span>
# include <type_traits>
# include <utility>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/runtime/compiler/m3_int/optable/impl.h>
# include <uwvm2/parser/wasm/standard/wasm1/opcode/mvp.h>
# include <uwvm2/parser/wasm/standard/wasm1/type/value_binfmt.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::m3_int::straight_line
{
    using wasm_byte = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_byte;
    using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
    using wasm_u32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u32;
    using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;
    using wasm_u64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64;
    using wasm_f32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f32;
    using wasm_f64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_f64;
    using value_type = ::uwvm2::parser::wasm::standard::wasm1::type::value_type;
    using op_basic = ::uwvm2::parser::wasm::standard::wasm1::opcode::op_basic;

    struct local_entry_t
    {
        wasm_u32 count{};
        value_type type{};
    };

    struct function_reference_t
    {
        ::std::span<value_type const> parameters{};
        ::std::span<value_type const> results{};
        void* handle{};
    };

    struct indirect_type_reference_t
    {
        ::std::span<value_type const> parameters{};
        ::std::span<value_type const> results{};
    };

    struct table_reference_t
    {
        ::std::span<void* const> elements{};
    };

    struct function_descriptor_t
    {
        ::std::span<value_type const> parameters{};
        ::std::span<value_type const> results{};
        ::std::span<local_entry_t const> locals{};
        ::std::span<wasm_byte const> expression{};
        ::std::span<function_reference_t const> functions{};
        ::std::span<indirect_type_reference_t const> types{};
        ::std::span<table_reference_t const> tables{};
    };

    enum class compile_error_t : unsigned
    {
        none,
        missing_end,
        invalid_immediate,
        invalid_local_index,
        stack_underflow,
        type_mismatch,
        invalid_result_arity,
        invalid_final_stack,
        unsupported_opcode,
        unsupported_control_flow
    };

    struct compiled_function_t
    {
        ::uwvm2::utils::container::vector<::std::byte> bytecode{};
        ::std::size_t parameter_count{};
        ::std::size_t local_count{};
        ::std::size_t stack_bytes{};
        ::std::optional<value_type> result_type{};
    };

    struct compile_result_t
    {
        compiled_function_t function{};
        compile_error_t error{compile_error_t::none};
        ::std::size_t expression_offset{};

        [[nodiscard]] explicit constexpr operator bool() const noexcept { return error == compile_error_t::none; }
    };

    struct module_function_t
    {
        function_descriptor_t descriptor{};
        mutable compile_result_t compiled{};
        mutable bool compile_attempted{};
    };

    struct execution_result_t
    {
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        ::uwvm2::utils::container::vector<::std::byte> stack{};
        optable::m3_control_state_t control{};
    };

    namespace details
    {
        inline constexpr ::std::size_t slot_size{sizeof(wasm_u64)};

        enum class location_kind_t : ::std::uint_least8_t
        {
            slot,
            int_register,
            fp_register
        };

        struct stack_value_t
        {
            value_type type{};
            location_kind_t location{location_kind_t::slot};
            optable::m3_slot_offset_t slot_offset{};
        };

        struct control_patch_t
        {
            ::std::size_t immediate_offset{};
            ::std::size_t target_offset{};
        };

        enum class control_frame_kind_t : ::std::uint_least8_t
        {
            block,
            loop,
            if_
        };

        struct control_frame_t
        {
            control_frame_kind_t kind{};
            ::uwvm2::utils::container::vector<stack_value_t> entry_stack{};
            ::uwvm2::utils::container::vector<::std::size_t> pending_end_patches{};
            ::std::optional<::std::size_t> false_patch{};
            ::std::optional<value_type> result_type{};
            optable::m3_slot_offset_t result_slot_offset{};
            ::std::size_t loop_target_offset{};
            bool has_else{};
        };

        struct compiler_state_t
        {
            function_descriptor_t descriptor{};
            ::uwvm2::utils::container::vector<::std::byte> bytecode{};
            ::uwvm2::utils::container::vector<stack_value_t> stack{};
            ::std::optional<::std::size_t> int_register_index{};
            ::std::optional<::std::size_t> fp_register_index{};
            ::std::size_t next_temp_slot_offset{};
            ::std::size_t max_slot_bytes{};
            ::uwvm2::utils::container::vector<control_patch_t> control_patches{};
            ::uwvm2::utils::container::vector<control_frame_t> control_frames{};
            ::uwvm2::utils::container::vector<::std::size_t> function_end_patches{};
            ::std::size_t function_end_offset{};
            bool path_reachable{true};
            ::std::size_t unreachable_control_depth{};
            bool function_end_offset_ready{};
            wasm_byte const* curr{};
            wasm_byte const* end{};
            bool terminated{};
            compile_error_t error{compile_error_t::none};
            ::std::size_t error_offset{};
        };

        struct execution_runtime_context_t
        {
            ::std::span<module_function_t const> module_functions{};
        };

        inline thread_local execution_runtime_context_t const* current_execution_runtime_context{};

        [[nodiscard]] inline bool read_u8(compiler_state_t& state, wasm_byte& out) noexcept;

        template <typename T>
        inline void spill_entry_to_slot(compiler_state_t& state, ::std::size_t index);

        inline bool finish_function(compiler_state_t& state, bool emit_return) noexcept;
        inline bool compile_end_block(compiler_state_t& state);

        template <typename T>
        inline void emit_object(::uwvm2::utils::container::vector<::std::byte>& out, T const& value)
        {
            static_assert(::std::is_trivially_copyable_v<T>);
            auto const old_size{out.size()};
            out.resize(old_size + sizeof(T));
            ::std::memcpy(out.data() + static_cast<::std::ptrdiff_t>(old_size), ::std::addressof(value), sizeof(T));
        }

        inline void emit_slot_offset(::uwvm2::utils::container::vector<::std::byte>& out, ::std::size_t slot_offset)
        {
            emit_object(out, static_cast<optable::m3_slot_offset_t>(slot_offset));
        }

        inline void set_error(compiler_state_t& state, compile_error_t err) noexcept
        {
            if(state.error == compile_error_t::none)
            {
                state.error = err;
                state.error_offset = static_cast<::std::size_t>(state.curr - state.descriptor.expression.data());
            }
        }

        inline void add_control_patch(compiler_state_t& state, ::std::size_t immediate_offset, ::std::size_t target_offset)
        {
            state.control_patches.push_back(control_patch_t{.immediate_offset = immediate_offset, .target_offset = target_offset});
        }

        inline void resolve_control_patch_to_current_offset(compiler_state_t& state, ::std::size_t immediate_offset)
        {
            add_control_patch(state, immediate_offset, state.bytecode.size());
        }

        [[nodiscard]] inline bool stack_matches_snapshot(compiler_state_t const& state,
                                                         ::uwvm2::utils::container::vector<stack_value_t> const& snapshot) noexcept
        {
            if(state.stack.size() != snapshot.size())
            {
                return false;
            }
            for(::std::size_t i{}; i != snapshot.size(); ++i)
            {
                if(state.stack[i].type != snapshot[i].type)
                {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] inline bool stack_snapshot_types_match(
            ::uwvm2::utils::container::vector<stack_value_t> const& lhs,
            ::uwvm2::utils::container::vector<stack_value_t> const& rhs) noexcept
        {
            if(lhs.size() != rhs.size())
            {
                return false;
            }
            for(::std::size_t i{}; i != lhs.size(); ++i)
            {
                if(lhs[i].type != rhs[i].type)
                {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] inline bool value_type_span_equal(::std::span<value_type const> lhs, ::std::span<value_type const> rhs) noexcept
        {
            if(lhs.size() != rhs.size())
            {
                return false;
            }
            for(::std::size_t i{}; i != lhs.size(); ++i)
            {
                if(lhs[i] != rhs[i])
                {
                    return false;
                }
            }
            return true;
        }

        inline void restore_stack_snapshot(compiler_state_t& state, ::uwvm2::utils::container::vector<stack_value_t> const& snapshot)
        {
            state.stack = snapshot;
            state.int_register_index.reset();
            state.fp_register_index.reset();
        }

        [[nodiscard]] inline ::uwvm2::utils::container::vector<stack_value_t>
        make_frame_stack_snapshot(control_frame_t const& frame, bool for_branch_target) noexcept
        {
            auto snapshot{frame.entry_stack};
            if(frame.result_type.has_value() && (!for_branch_target || frame.kind != control_frame_kind_t::loop))
            {
                snapshot.push_back(stack_value_t{.type = *frame.result_type,
                                                .location = location_kind_t::slot,
                                                .slot_offset = frame.result_slot_offset});
            }
            return snapshot;
        }

        [[nodiscard]] inline bool read_block_result_type(compiler_state_t& state, ::std::optional<value_type>& out)
        {
            wasm_byte block_type{};
            if(!read_u8(state, block_type))
            {
                return false;
            }
            switch(block_type)
            {
                case wasm_byte{0x40u}:
                    out.reset();
                    return true;
                case static_cast<wasm_byte>(value_type::i32):
                    out = value_type::i32;
                    return true;
                case static_cast<wasm_byte>(value_type::i64):
                    out = value_type::i64;
                    return true;
                case static_cast<wasm_byte>(value_type::f32):
                    out = value_type::f32;
                    return true;
                case static_cast<wasm_byte>(value_type::f64):
                    out = value_type::f64;
                    return true;
                default:
                    set_error(state, compile_error_t::unsupported_control_flow);
                    return false;
            }
        }

        [[nodiscard]] inline constexpr bool is_int_type(value_type type) noexcept
        {
            return type == value_type::i32 || type == value_type::i64;
        }

        [[nodiscard]] inline constexpr bool is_fp_type(value_type type) noexcept
        {
            return type == value_type::f32 || type == value_type::f64;
        }

        [[nodiscard]] inline constexpr location_kind_t register_location_for(value_type type) noexcept
        {
            return is_fp_type(type) ? location_kind_t::fp_register : location_kind_t::int_register;
        }

        [[nodiscard]] inline constexpr bool is_register_location(location_kind_t location) noexcept
        {
            return location == location_kind_t::int_register || location == location_kind_t::fp_register;
        }

        template <value_type Type>
        struct cpp_type_from_value_type;

        template <>
        struct cpp_type_from_value_type<value_type::i32>
        {
            using type = wasm_i32;
        };

        template <>
        struct cpp_type_from_value_type<value_type::i64>
        {
            using type = wasm_i64;
        };

        template <>
        struct cpp_type_from_value_type<value_type::f32>
        {
            using type = wasm_f32;
        };

        template <>
        struct cpp_type_from_value_type<value_type::f64>
        {
            using type = wasm_f64;
        };

        template <value_type Type>
        using cpp_type_from_value_type_t = typename cpp_type_from_value_type<Type>::type;

        template <typename T>
        inline constexpr value_type value_type_from_cpp_v = []() consteval {
            if constexpr(::std::same_as<T, wasm_i32> || ::std::same_as<T, wasm_u32>)
            {
                return value_type::i32;
            }
            else if constexpr(::std::same_as<T, wasm_i64> || ::std::same_as<T, wasm_u64>)
            {
                return value_type::i64;
            }
            else if constexpr(::std::same_as<T, wasm_f32>)
            {
                return value_type::f32;
            }
            else
            {
                return value_type::f64;
            }
        }();

        [[nodiscard]] inline bool read_u8(compiler_state_t& state, wasm_byte& out) noexcept
        {
            if(state.curr == state.end)
            {
                set_error(state, compile_error_t::invalid_immediate);
                return false;
            }
            out = *state.curr;
            ++state.curr;
            return true;
        }

        template <typename T>
        [[nodiscard]] inline bool read_fixed(compiler_state_t& state, T& out) noexcept
        {
            static_assert(::std::is_trivially_copyable_v<T>);
            auto const remaining{static_cast<::std::size_t>(state.end - state.curr)};
            if(remaining < sizeof(T))
            {
                set_error(state, compile_error_t::invalid_immediate);
                return false;
            }
            ::std::memcpy(::std::addressof(out), state.curr, sizeof(T));
            state.curr += sizeof(T);
            return true;
        }

        [[nodiscard]] inline bool read_uleb_u32(compiler_state_t& state, wasm_u32& out) noexcept
        {
            out = wasm_u32{};
            unsigned shift{};
            for(unsigned i{}; i != 5u; ++i)
            {
                wasm_byte byte{};
                if(!read_u8(state, byte))
                {
                    return false;
                }
                out |= static_cast<wasm_u32>(static_cast<wasm_u32>(byte & 0x7Fu) << shift);
                if((byte & 0x80u) == 0u)
                {
                    return true;
                }
                shift += 7u;
            }
            set_error(state, compile_error_t::invalid_immediate);
            return false;
        }

        template <typename SignedType>
        [[nodiscard]] inline bool read_sleb(compiler_state_t& state, SignedType& out) noexcept
        {
            static_assert(::std::is_signed_v<SignedType>);
            using unsigned_type = ::std::make_unsigned_t<SignedType>;

            unsigned_type result{};
            unsigned shift{};
            wasm_byte byte{};
            constexpr unsigned max_bytes{(sizeof(SignedType) * 8u + 6u) / 7u};

            for(unsigned i{}; i != max_bytes; ++i)
            {
                if(!read_u8(state, byte))
                {
                    return false;
                }
                result |= static_cast<unsigned_type>(static_cast<unsigned_type>(byte & 0x7Fu) << shift);
                shift += 7u;
                if((byte & 0x80u) == 0u)
                {
                    if((shift < sizeof(SignedType) * 8u) && ((byte & 0x40u) != 0u))
                    {
                        result |= static_cast<unsigned_type>(~unsigned_type{}) << shift;
                    }
                    out = static_cast<SignedType>(result);
                    return true;
                }
            }

            set_error(state, compile_error_t::invalid_immediate);
            return false;
        }

        [[nodiscard]] inline ::std::size_t total_local_count(function_descriptor_t const& descriptor) noexcept
        {
            ::std::size_t count{descriptor.parameters.size()};
            for(auto const& entry : descriptor.locals)
            {
                count += static_cast<::std::size_t>(entry.count);
            }
            return count;
        }

        [[nodiscard]] inline value_type get_local_type(function_descriptor_t const& descriptor, ::std::size_t local_index, bool& ok) noexcept
        {
            if(local_index < descriptor.parameters.size())
            {
                ok = true;
                return descriptor.parameters[local_index];
            }

            auto remaining{local_index - descriptor.parameters.size()};
            for(auto const& entry : descriptor.locals)
            {
                auto const count{static_cast<::std::size_t>(entry.count)};
                if(remaining < count)
                {
                    ok = true;
                    return entry.type;
                }
                remaining -= count;
            }

            ok = false;
            return value_type{};
        }

        [[nodiscard]] inline ::std::size_t allocate_temp_slot(compiler_state_t& state) noexcept
        {
            auto const slot_offset{state.next_temp_slot_offset};
            state.next_temp_slot_offset += slot_size;
            if(state.next_temp_slot_offset > state.max_slot_bytes)
            {
                state.max_slot_bytes = state.next_temp_slot_offset;
            }
            return slot_offset;
        }

        inline void spill_all_registers(compiler_state_t& state)
        {
            for(::std::size_t i{}; i != state.stack.size(); ++i)
            {
                switch(state.stack[i].location)
                {
                    case location_kind_t::slot:
                        break;
                    case location_kind_t::int_register:
                        switch(state.stack[i].type)
                        {
                            case value_type::i32:
                                spill_entry_to_slot<wasm_i32>(state, i);
                                break;
                            case value_type::i64:
                                spill_entry_to_slot<wasm_i64>(state, i);
                                break;
                            default:
                                ::fast_io::fast_terminate();
                        }
                        break;
                    case location_kind_t::fp_register:
                        switch(state.stack[i].type)
                        {
                            case value_type::f32:
                                spill_entry_to_slot<wasm_f32>(state, i);
                                break;
                            case value_type::f64:
                                spill_entry_to_slot<wasm_f64>(state, i);
                                break;
                            default:
                                ::fast_io::fast_terminate();
                        }
                        break;
                }
            }
        }

        template <typename T>
        inline void emit_set_slot(compiler_state_t& state, ::std::size_t slot_offset)
        {
            emit_object(state.bytecode, optable::translate::get_SetSlot_fptr<T>());
            emit_slot_offset(state.bytecode, slot_offset);
        }

        template <typename T>
        inline void emit_set_register(compiler_state_t& state, ::std::size_t slot_offset)
        {
            emit_object(state.bytecode, optable::translate::get_SetRegister_fptr<T>());
            emit_slot_offset(state.bytecode, slot_offset);
        }

        template <typename T>
        inline void emit_copy_slot(compiler_state_t& state, ::std::size_t dest_offset, ::std::size_t src_offset)
        {
            emit_object(state.bytecode, optable::translate::get_CopySlot_fptr<T>());
            emit_slot_offset(state.bytecode, dest_offset);
            emit_slot_offset(state.bytecode, src_offset);
        }

        template <typename T>
        inline void spill_entry_to_slot(compiler_state_t& state, ::std::size_t index)
        {
            auto& entry{state.stack[index]};
            auto const slot_offset{allocate_temp_slot(state)};
            emit_set_slot<T>(state, slot_offset);
            entry.location = location_kind_t::slot;
            entry.slot_offset = static_cast<optable::m3_slot_offset_t>(slot_offset);
            if constexpr(is_fp_type(value_type_from_cpp_v<T>))
            {
                state.fp_register_index.reset();
            }
            else
            {
                state.int_register_index.reset();
            }
        }

        inline void spill_register_if_needed(compiler_state_t& state, value_type type, bool consumed_register) 
        {
            if(consumed_register)
            {
                return;
            }

            auto& register_index{is_fp_type(type) ? state.fp_register_index : state.int_register_index};
            if(!register_index.has_value())
            {
                return;
            }

            auto const index{*register_index};
            auto const entry_type{state.stack[index].type};
            switch(entry_type)
            {
                case value_type::i32:
                    spill_entry_to_slot<wasm_i32>(state, index);
                    break;
                case value_type::i64:
                    spill_entry_to_slot<wasm_i64>(state, index);
                    break;
                case value_type::f32:
                    spill_entry_to_slot<wasm_f32>(state, index);
                    break;
                case value_type::f64:
                    spill_entry_to_slot<wasm_f64>(state, index);
                    break;
            }
        }

        inline void pop_value(compiler_state_t& state) noexcept
        {
            auto const entry{state.stack.back()};
            if(entry.location == location_kind_t::int_register)
            {
                state.int_register_index.reset();
            }
            else if(entry.location == location_kind_t::fp_register)
            {
                state.fp_register_index.reset();
            }
            state.stack.pop_back();
        }

        inline void push_register_value(compiler_state_t& state, value_type type) noexcept
        {
            state.stack.push_back(stack_value_t{.type = type, .location = register_location_for(type), .slot_offset = {}});
            auto const index{state.stack.size() - 1u};
            if(is_fp_type(type))
            {
                state.fp_register_index = index;
            }
            else
            {
                state.int_register_index = index;
            }
        }

        inline void push_slot_value(compiler_state_t& state, value_type type, ::std::size_t slot_offset) noexcept
        {
            state.stack.push_back(stack_value_t{.type = type, .location = location_kind_t::slot, .slot_offset = static_cast<optable::m3_slot_offset_t>(slot_offset)});
        }

        [[nodiscard]] inline bool require_stack_size(compiler_state_t& state, ::std::size_t size) noexcept
        {
            if(state.stack.size() < size)
            {
                set_error(state, compile_error_t::stack_underflow);
                return false;
            }
            return true;
        }

        [[nodiscard]] inline bool require_top_type(compiler_state_t& state, value_type type, ::std::size_t from_top = 0u) noexcept
        {
            if(!require_stack_size(state, from_top + 1u))
            {
                return false;
            }
            if(state.stack[state.stack.size() - 1u - from_top].type != type)
            {
                set_error(state, compile_error_t::type_mismatch);
                return false;
            }
            return true;
        }

        template <value_type Type, typename Getter>
        inline bool compile_local_get(compiler_state_t& state, Getter getter)
        {
            wasm_u32 local_index_u32{};
            if(!read_uleb_u32(state, local_index_u32))
            {
                return false;
            }

            bool valid_local{};
            auto const local_type{get_local_type(state.descriptor, static_cast<::std::size_t>(local_index_u32), valid_local)};
            if(!valid_local)
            {
                set_error(state, compile_error_t::invalid_local_index);
                return false;
            }
            if(local_type != Type)
            {
                set_error(state, compile_error_t::type_mismatch);
                return false;
            }

            spill_register_if_needed(state, Type, false);
            emit_object(state.bytecode, getter());
            emit_slot_offset(state.bytecode, static_cast<::std::size_t>(local_index_u32) * slot_size);
            push_register_value(state, Type);
            return true;
        }

        template <value_type Type>
        inline bool compile_const(compiler_state_t& state)
        {
            auto const slot_offset{allocate_temp_slot(state)};

            if constexpr(Type == value_type::i32)
            {
                wasm_i32 value{};
                if(!read_sleb(state, value))
                {
                    return false;
                }
                emit_object(state.bytecode, optable::translate::get_Const32_fptr());
                emit_object(state.bytecode, static_cast<wasm_u32>(value));
            }
            else if constexpr(Type == value_type::i64)
            {
                wasm_i64 value{};
                if(!read_sleb(state, value))
                {
                    return false;
                }
                emit_object(state.bytecode, optable::translate::get_Const64_fptr());
                emit_object(state.bytecode, static_cast<wasm_u64>(value));
            }
            else if constexpr(Type == value_type::f32)
            {
                wasm_f32 value{};
                if(!read_fixed(state, value))
                {
                    return false;
                }
                emit_object(state.bytecode, optable::translate::get_Const32_fptr());
                emit_object(state.bytecode, ::std::bit_cast<wasm_u32>(value));
            }
            else
            {
                wasm_f64 value{};
                if(!read_fixed(state, value))
                {
                    return false;
                }
                emit_object(state.bytecode, optable::translate::get_Const64_fptr());
                emit_object(state.bytecode, ::std::bit_cast<wasm_u64>(value));
            }

            emit_slot_offset(state.bytecode, slot_offset);
            push_slot_value(state, Type, slot_offset);
            return true;
        }

        template <typename T>
        inline void emit_value_write_to_slot(compiler_state_t& state, stack_value_t const& source, ::std::size_t slot_offset)
        {
            if(source.location == location_kind_t::slot)
            {
                emit_copy_slot<T>(state, slot_offset, source.slot_offset);
            }
            else
            {
                emit_set_slot<T>(state, slot_offset);
            }
        }

        template <typename T>
        inline void emit_local_write(compiler_state_t& state, stack_value_t const& source, ::std::size_t local_index)
        {
            emit_value_write_to_slot<T>(state, source, local_index * slot_size);
        }

        template <value_type Type>
        inline bool compile_local_set(compiler_state_t& state, bool preserve_value)
        {
            wasm_u32 local_index_u32{};
            if(!read_uleb_u32(state, local_index_u32))
            {
                return false;
            }

            bool valid_local{};
            auto const local_type{get_local_type(state.descriptor, static_cast<::std::size_t>(local_index_u32), valid_local)};
            if(!valid_local)
            {
                set_error(state, compile_error_t::invalid_local_index);
                return false;
            }
            if(local_type != Type)
            {
                set_error(state, compile_error_t::type_mismatch);
                return false;
            }
            if(!require_top_type(state, Type))
            {
                return false;
            }

            auto const source{state.stack.back()};
            if constexpr(Type == value_type::i32)
            {
                emit_local_write<wasm_i32>(state, source, local_index_u32);
            }
            else if constexpr(Type == value_type::i64)
            {
                emit_local_write<wasm_i64>(state, source, local_index_u32);
            }
            else if constexpr(Type == value_type::f32)
            {
                emit_local_write<wasm_f32>(state, source, local_index_u32);
            }
            else
            {
                emit_local_write<wasm_f64>(state, source, local_index_u32);
            }

            if(!preserve_value)
            {
                pop_value(state);
            }
            return true;
        }

        template <typename ResultType, typename OperandType, typename Operation, bool Commutative>
        inline bool compile_binary_numeric(compiler_state_t& state)
        {
            constexpr auto operand_type{value_type_from_cpp_v<OperandType>};
            constexpr auto result_type{value_type_from_cpp_v<ResultType>};

            if(!require_top_type(state, operand_type) || !require_top_type(state, operand_type, 1u))
            {
                return false;
            }

            auto const rhs{state.stack.back()};
            auto const lhs{state.stack[state.stack.size() - 2u]};

            auto const consumed_int_register{rhs.location == location_kind_t::int_register || lhs.location == location_kind_t::int_register};
            auto const consumed_fp_register{rhs.location == location_kind_t::fp_register || lhs.location == location_kind_t::fp_register};
            if(is_fp_type(result_type))
            {
                spill_register_if_needed(state, result_type, consumed_fp_register);
            }
            else
            {
                spill_register_if_needed(state, result_type, consumed_int_register);
            }

            auto const shape = [&]() noexcept {
                if(rhs.location == location_kind_t::slot)
                {
                    if(lhs.location == location_kind_t::slot)
                    {
                        return optable::binary_shape_t::ss;
                    }
                    return optable::binary_shape_t::sr;
                }
                return optable::binary_shape_t::rs;
            }();

            emit_binary_by_shape(
                state,
                shape,
                rhs,
                lhs,
                []<optable::binary_shape_t Shape>(::std::integral_constant<optable::binary_shape_t, Shape>) -> optable::m3_interpreter_opfunc_t {
                    if constexpr(Commutative)
                    {
                        return optable::translate::get_commutative_binary_opfunc<ResultType, OperandType, Shape, Operation>();
                    }
                    else
                    {
                        return optable::translate::get_binary_opfunc<ResultType, OperandType, Shape, Operation>();
                    }
                });

            pop_value(state);
            pop_value(state);
            push_register_value(state, result_type);
            return true;
        }

        template <typename Getter>
        inline void emit_binary_by_shape(compiler_state_t& state,
                                         optable::binary_shape_t shape,
                                         stack_value_t const& rhs,
                                         stack_value_t const& lhs,
                                         Getter getter,
                                         int = 0)
        {
            switch(shape)
            {
                case optable::binary_shape_t::rs:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::binary_shape_t, optable::binary_shape_t::rs>{}));
                    emit_slot_offset(state.bytecode, lhs.slot_offset);
                    break;
                case optable::binary_shape_t::sr:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::binary_shape_t, optable::binary_shape_t::sr>{}));
                    emit_slot_offset(state.bytecode, rhs.slot_offset);
                    break;
                case optable::binary_shape_t::ss:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::binary_shape_t, optable::binary_shape_t::ss>{}));
                    emit_slot_offset(state.bytecode, rhs.slot_offset);
                    emit_slot_offset(state.bytecode, lhs.slot_offset);
                    break;
            }
        }

        template <typename ResultType, typename OperandType, typename Operation>
        inline bool compile_unary_numeric(compiler_state_t& state)
        {
            constexpr auto operand_type{value_type_from_cpp_v<OperandType>};
            constexpr auto result_type{value_type_from_cpp_v<ResultType>};

            if(!require_top_type(state, operand_type))
            {
                return false;
            }

            auto const operand{state.stack.back()};
            auto const consumed_int_register{operand.location == location_kind_t::int_register};
            auto const consumed_fp_register{operand.location == location_kind_t::fp_register};
            if(is_fp_type(result_type))
            {
                spill_register_if_needed(state, result_type, consumed_fp_register);
            }
            else
            {
                spill_register_if_needed(state, result_type, consumed_int_register);
            }

            auto const shape{operand.location == location_kind_t::slot ? optable::unary_shape_t::s : optable::unary_shape_t::r};
            emit_unary_by_shape(
                state,
                shape,
                operand,
                []<optable::unary_shape_t Shape>(::std::integral_constant<optable::unary_shape_t, Shape>) -> optable::m3_interpreter_opfunc_t {
                    return optable::translate::get_unary_opfunc<ResultType, OperandType, Shape, Operation>();
                });

            pop_value(state);
            push_register_value(state, result_type);
            return true;
        }

        template <typename Getter>
        inline void emit_unary_by_shape(compiler_state_t& state,
                                        optable::unary_shape_t shape,
                                        stack_value_t const& operand,
                                        Getter getter,
                                        int = 0)
        {
            switch(shape)
            {
                case optable::unary_shape_t::r:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::unary_shape_t, optable::unary_shape_t::r>{}));
                    break;
                case optable::unary_shape_t::s:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::unary_shape_t, optable::unary_shape_t::s>{}));
                    emit_slot_offset(state.bytecode, operand.slot_offset);
                    break;
            }
        }

        template <typename Getter>
        inline void emit_register_slot_transform_by_shape(compiler_state_t& state,
                                                          optable::register_slot_shape_t shape,
                                                          stack_value_t const& operand,
                                                          Getter getter,
                                                          int = 0)
        {
            switch(shape)
            {
                case optable::register_slot_shape_t::rr:
                    emit_object(
                        state.bytecode,
                        getter(::std::integral_constant<optable::register_slot_shape_t, optable::register_slot_shape_t::rr>{}));
                    break;
                case optable::register_slot_shape_t::rs:
                    emit_object(
                        state.bytecode,
                        getter(::std::integral_constant<optable::register_slot_shape_t, optable::register_slot_shape_t::rs>{}));
                    emit_slot_offset(state.bytecode, operand.slot_offset);
                    break;
                default:
                    ::fast_io::fast_terminate();
            }
        }

        [[nodiscard]] inline ::std::size_t emit_pc_immediate_placeholder(compiler_state_t& state)
        {
            auto const immediate_offset{state.bytecode.size()};
            emit_object(state.bytecode, optable::m3_code_t{});
            return immediate_offset;
        }

        [[nodiscard]] inline ::std::size_t emit_branch_placeholder(compiler_state_t& state)
        {
            emit_object(state.bytecode, optable::translate::get_Branch_fptr());
            return emit_pc_immediate_placeholder(state);
        }

        template <typename Getter>
        [[nodiscard]] inline ::std::size_t emit_conditional_branch_placeholder(compiler_state_t& state,
                                                                               stack_value_t const& condition,
                                                                               Getter getter)
        {
            auto const shape{condition.location == location_kind_t::slot ? optable::unary_shape_t::s : optable::unary_shape_t::r};
            switch(shape)
            {
                case optable::unary_shape_t::r:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::unary_shape_t, optable::unary_shape_t::r>{}));
                    break;
                case optable::unary_shape_t::s:
                    emit_object(state.bytecode, getter(::std::integral_constant<optable::unary_shape_t, optable::unary_shape_t::s>{}));
                    emit_slot_offset(state.bytecode, condition.slot_offset);
                    break;
            }
            return emit_pc_immediate_placeholder(state);
        }

        template <typename ResultType, typename SourceType, typename Operation>
        inline bool compile_unary_transform(compiler_state_t& state)
        {
            constexpr auto source_type{value_type_from_cpp_v<SourceType>};
            constexpr auto result_type{value_type_from_cpp_v<ResultType>};

            if(!require_top_type(state, source_type))
            {
                return false;
            }

            auto const operand{state.stack.back()};
            auto const consumed_int_register{operand.location == location_kind_t::int_register};
            auto const consumed_fp_register{operand.location == location_kind_t::fp_register};
            if(is_fp_type(result_type))
            {
                spill_register_if_needed(state, result_type, consumed_fp_register);
            }
            else
            {
                spill_register_if_needed(state, result_type, consumed_int_register);
            }

            auto const shape{operand.location == location_kind_t::slot ? optable::register_slot_shape_t::rs : optable::register_slot_shape_t::rr};
            emit_register_slot_transform_by_shape(
                state,
                shape,
                operand,
                []<optable::register_slot_shape_t Shape>(::std::integral_constant<optable::register_slot_shape_t, Shape>)
                    -> optable::m3_interpreter_opfunc_t {
                    return optable::translate::get_register_slot_transform_opfunc<ResultType, SourceType, Shape, Operation>();
                });

            pop_value(state);
            push_register_value(state, result_type);
            return true;
        }

        inline bool compile_enter_block(compiler_state_t& state, control_frame_kind_t kind)
        {
            ::std::optional<value_type> result_type{};
            if(!read_block_result_type(state, result_type))
            {
                return false;
            }

            spill_all_registers(state);

            control_frame_t frame{};
            frame.kind = kind;
            frame.entry_stack = state.stack;
            frame.result_type = result_type;
            if(result_type.has_value())
            {
                frame.result_slot_offset = static_cast<optable::m3_slot_offset_t>(allocate_temp_slot(state));
            }
            if(kind == control_frame_kind_t::loop)
            {
                frame.loop_target_offset = state.bytecode.size();
            }
            state.control_frames.push_back(::std::move(frame));
            return true;
        }

        inline bool compile_if_block(compiler_state_t& state)
        {
            ::std::optional<value_type> result_type{};
            if(!read_block_result_type(state, result_type))
            {
                return false;
            }
            if(!require_top_type(state, value_type::i32))
            {
                return false;
            }

            spill_all_registers(state);
            auto const condition{state.stack.back()};
            auto const false_patch{emit_conditional_branch_placeholder(
                state,
                condition,
                []<optable::unary_shape_t Shape>(::std::integral_constant<optable::unary_shape_t, Shape>) -> optable::m3_interpreter_opfunc_t {
                    return optable::translate::get_If_fptr<Shape>();
                })};
            pop_value(state);

            control_frame_t frame{};
            frame.kind = control_frame_kind_t::if_;
            frame.entry_stack = state.stack;
            frame.false_patch = false_patch;
            frame.result_type = result_type;
            if(result_type.has_value())
            {
                frame.result_slot_offset = static_cast<optable::m3_slot_offset_t>(allocate_temp_slot(state));
            }
            state.control_frames.push_back(::std::move(frame));
            return true;
        }

        inline bool get_branch_target(compiler_state_t& state,
                                      wasm_u32 depth,
                                      control_frame_t*& frame,
                                      bool& function_target) noexcept
        {
            auto const frame_count{state.control_frames.size()};
            if(depth < frame_count)
            {
                frame = ::std::addressof(state.control_frames.index_unchecked(frame_count - 1u - static_cast<::std::size_t>(depth)));
                function_target = false;
                return true;
            }
            if(depth == frame_count)
            {
                frame = nullptr;
                function_target = true;
                return true;
            }
            return false;
        }

        inline bool materialize_branch_result_to_frame(compiler_state_t& state, control_frame_t const& frame)
        {
            if(!frame.result_type.has_value())
            {
                return true;
            }

            auto const result{state.stack.back()};
            switch(*frame.result_type)
            {
                case value_type::i32:
                    if(result.location == location_kind_t::slot)
                    {
                        emit_copy_slot<wasm_i32>(state, frame.result_slot_offset, result.slot_offset);
                    }
                    else
                    {
                        emit_set_slot<wasm_i32>(state, frame.result_slot_offset);
                    }
                    return true;
                case value_type::i64:
                    if(result.location == location_kind_t::slot)
                    {
                        emit_copy_slot<wasm_i64>(state, frame.result_slot_offset, result.slot_offset);
                    }
                    else
                    {
                        emit_set_slot<wasm_i64>(state, frame.result_slot_offset);
                    }
                    return true;
                case value_type::f32:
                    if(result.location == location_kind_t::slot)
                    {
                        emit_copy_slot<wasm_f32>(state, frame.result_slot_offset, result.slot_offset);
                    }
                    else
                    {
                        emit_set_slot<wasm_f32>(state, frame.result_slot_offset);
                    }
                    return true;
                case value_type::f64:
                    if(result.location == location_kind_t::slot)
                    {
                        emit_copy_slot<wasm_f64>(state, frame.result_slot_offset, result.slot_offset);
                    }
                    else
                    {
                        emit_set_slot<wasm_f64>(state, frame.result_slot_offset);
                    }
                    return true;
            }
            ::fast_io::fast_terminate();
        }

        inline bool validate_branch_target(compiler_state_t& state,
                                           control_frame_t const* frame,
                                           ::uwvm2::utils::container::vector<stack_value_t> const& snapshot)
        {
            spill_all_registers(state);
            if(!stack_matches_snapshot(state, snapshot))
            {
                set_error(state, compile_error_t::type_mismatch);
                return false;
            }
            if(frame != nullptr)
            {
                auto const expects_result{frame->result_type.has_value() && snapshot.size() == frame->entry_stack.size() + 1u};
                if(expects_result)
                {
                    return materialize_branch_result_to_frame(state, *frame);
                }
            }
            return true;
        }

        inline void add_branch_target_patch(compiler_state_t& state,
                                            control_frame_t* target_frame,
                                            bool function_target,
                                            ::std::size_t immediate_offset)
        {
            if(function_target)
            {
                state.function_end_patches.push_back(immediate_offset);
                return;
            }
            if(target_frame->kind == control_frame_kind_t::loop)
            {
                add_control_patch(state, immediate_offset, target_frame->loop_target_offset);
                return;
            }
            target_frame->pending_end_patches.push_back(immediate_offset);
        }

        inline bool compile_branch(compiler_state_t& state, bool conditional)
        {
            stack_value_t condition{};
            if(conditional)
            {
                if(!require_top_type(state, value_type::i32))
                {
                    return false;
                }
            }

            wasm_u32 depth{};
            if(!read_uleb_u32(state, depth))
            {
                return false;
            }

            if(conditional)
            {
                spill_all_registers(state);
                condition = state.stack.back();
                pop_value(state);
            }

            control_frame_t* target_frame{};
            bool function_target{};
            if(!get_branch_target(state, depth, target_frame, function_target))
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }

            if(function_target)
            {
                if(!state.descriptor.results.empty())
                {
                    set_error(state, compile_error_t::unsupported_control_flow);
                    return false;
                }
                if(!validate_branch_target(state, nullptr, {}))
                {
                    return false;
                }

                auto const patch{conditional ?
                                     emit_conditional_branch_placeholder(
                                         state,
                                         condition,
                                         []<optable::unary_shape_t Shape>(::std::integral_constant<optable::unary_shape_t, Shape>)
                                             -> optable::m3_interpreter_opfunc_t {
                                             return optable::translate::get_BranchIf_fptr<Shape>();
                                         }) :
                                     emit_branch_placeholder(state)};
                state.function_end_patches.push_back(patch);
                if(!conditional)
                {
                    state.path_reachable = false;
                }
                return true;
            }

            auto const target_snapshot{make_frame_stack_snapshot(*target_frame, true)};
            if(!validate_branch_target(state, target_frame, target_snapshot))
            {
                return false;
            }

            auto const patch{conditional ?
                                 emit_conditional_branch_placeholder(
                                     state,
                                     condition,
                                     []<optable::unary_shape_t Shape>(::std::integral_constant<optable::unary_shape_t, Shape>)
                                         -> optable::m3_interpreter_opfunc_t {
                                         return optable::translate::get_BranchIf_fptr<Shape>();
                                     }) :
                                 emit_branch_placeholder(state)};

            if(target_frame->kind == control_frame_kind_t::loop)
            {
                add_control_patch(state, patch, target_frame->loop_target_offset);
            }
            else
            {
                target_frame->pending_end_patches.push_back(patch);
            }

            if(!conditional)
            {
                state.path_reachable = false;
            }
            return true;
        }

        inline bool compile_branch_table(compiler_state_t& state)
        {
            if(!require_top_type(state, value_type::i32))
            {
                return false;
            }

            wasm_u32 target_count{};
            if(!read_uleb_u32(state, target_count))
            {
                return false;
            }

            spill_all_registers(state);
            auto const selector{state.stack.back()};
            pop_value(state);

            ::uwvm2::utils::container::vector<wasm_u32> depths{};
            ::uwvm2::utils::container::vector<stack_value_t> reference_snapshot{};
            bool has_reference_snapshot{};
            auto const total_targets{static_cast<::std::size_t>(target_count) + 1u};
            for(::std::size_t i{}; i != total_targets; ++i)
            {
                wasm_u32 depth{};
                if(!read_uleb_u32(state, depth))
                {
                    return false;
                }
                depths.push_back(depth);

                control_frame_t* target_frame{};
                bool function_target{};
                if(!get_branch_target(state, depth, target_frame, function_target))
                {
                    set_error(state, compile_error_t::unsupported_control_flow);
                    return false;
                }

                auto const target_snapshot{[&]() -> ::uwvm2::utils::container::vector<stack_value_t> {
                    if(function_target)
                    {
                        return {};
                    }
                    return make_frame_stack_snapshot(*target_frame, true);
                }()};

                if(function_target && !state.descriptor.results.empty())
                {
                    set_error(state, compile_error_t::unsupported_control_flow);
                    return false;
                }

                if(!has_reference_snapshot)
                {
                    reference_snapshot = target_snapshot;
                    has_reference_snapshot = true;
                }
                else if(!stack_snapshot_types_match(reference_snapshot, target_snapshot))
                {
                    set_error(state, compile_error_t::type_mismatch);
                    return false;
                }
            }

            if(!stack_matches_snapshot(state, reference_snapshot))
            {
                set_error(state, compile_error_t::type_mismatch);
                return false;
            }

            for(auto const depth : depths)
            {
                control_frame_t* target_frame{};
                bool function_target{};
                get_branch_target(state, depth, target_frame, function_target);
                if(function_target || target_frame == nullptr)
                {
                    continue;
                }
                auto const target_snapshot{make_frame_stack_snapshot(*target_frame, true)};
                auto const expects_result{target_frame->result_type.has_value() && target_snapshot.size() == target_frame->entry_stack.size() + 1u};
                if(expects_result)
                {
                    if(!materialize_branch_result_to_frame(state, *target_frame))
                    {
                        return false;
                    }
                }
            }

            emit_object(state.bytecode, optable::translate::get_BranchTable_fptr());
            emit_slot_offset(state.bytecode, selector.slot_offset);
            emit_object(state.bytecode, target_count);
            for(auto const depth : depths)
            {
                control_frame_t* target_frame{};
                bool function_target{};
                get_branch_target(state, depth, target_frame, function_target);
                add_branch_target_patch(state, target_frame, function_target, emit_pc_immediate_placeholder(state));
            }

            state.path_reachable = false;
            return true;
        }

        inline bool compile_call_indirect(compiler_state_t& state)
        {
            wasm_u32 type_index{};
            wasm_u32 table_index{};
            if(!read_uleb_u32(state, type_index) || !read_uleb_u32(state, table_index))
            {
                return false;
            }

            auto const type_slot{static_cast<::std::size_t>(type_index)};
            auto const table_slot{static_cast<::std::size_t>(table_index)};
            if(type_slot >= state.descriptor.types.size() || table_slot >= state.descriptor.tables.size())
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }

            auto const& callee_type{state.descriptor.types[type_slot]};
            auto const& table{state.descriptor.tables[table_slot]};
            if(callee_type.results.size() > 1u)
            {
                set_error(state, compile_error_t::invalid_result_arity);
                return false;
            }
            if(!require_stack_size(state, callee_type.parameters.size() + 1u))
            {
                return false;
            }
            if(!require_top_type(state, value_type::i32))
            {
                return false;
            }

            for(::std::size_t i{}; i != callee_type.parameters.size(); ++i)
            {
                auto const expected{callee_type.parameters[callee_type.parameters.size() - 1u - i]};
                if(!require_top_type(state, expected, i + 1u))
                {
                    return false;
                }
            }

            spill_all_registers(state);
            auto const selector{state.stack.back()};
            pop_value(state);

            auto const parameter_base_offset{state.next_temp_slot_offset};
            auto const stack_base_index{state.stack.size() - callee_type.parameters.size()};
            for(::std::size_t i{}; i != callee_type.parameters.size(); ++i)
            {
                auto const parameter_slot_offset{allocate_temp_slot(state)};
                auto const argument{state.stack[stack_base_index + i]};
                switch(callee_type.parameters[i])
                {
                    case value_type::i32:
                        emit_value_write_to_slot<wasm_i32>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::i64:
                        emit_value_write_to_slot<wasm_i64>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::f32:
                        emit_value_write_to_slot<wasm_f32>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::f64:
                        emit_value_write_to_slot<wasm_f64>(state, argument, parameter_slot_offset);
                        break;
                }
            }

            for(::std::size_t i{}; i != callee_type.parameters.size(); ++i)
            {
                pop_value(state);
            }

            emit_object(state.bytecode, optable::translate::get_CallIndirect_fptr());
            emit_slot_offset(state.bytecode, selector.slot_offset);
            emit_object(state.bytecode, const_cast<table_reference_t*>(::std::addressof(table)));
            emit_object(state.bytecode, const_cast<indirect_type_reference_t*>(::std::addressof(callee_type)));
            emit_slot_offset(state.bytecode, parameter_base_offset);

            if(callee_type.results.size() == 1u)
            {
                push_register_value(state, callee_type.results.front());
            }
            return true;
        }

        inline bool compile_call(compiler_state_t& state)
        {
            wasm_u32 function_index{};
            if(!read_uleb_u32(state, function_index))
            {
                return false;
            }

            auto const function_slot{static_cast<::std::size_t>(function_index)};
            if(function_slot >= state.descriptor.functions.size())
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }

            auto const& callee{state.descriptor.functions[function_slot]};
            if(callee.handle == nullptr)
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }
            if(callee.results.size() > 1u)
            {
                set_error(state, compile_error_t::invalid_result_arity);
                return false;
            }
            if(!require_stack_size(state, callee.parameters.size()))
            {
                return false;
            }

            for(::std::size_t i{}; i != callee.parameters.size(); ++i)
            {
                auto const expected{callee.parameters[callee.parameters.size() - 1u - i]};
                if(!require_top_type(state, expected, i))
                {
                    return false;
                }
            }

            spill_all_registers(state);
            auto const parameter_base_offset{state.next_temp_slot_offset};
            auto const stack_base_index{state.stack.size() - callee.parameters.size()};
            for(::std::size_t i{}; i != callee.parameters.size(); ++i)
            {
                auto const parameter_slot_offset{allocate_temp_slot(state)};
                auto const argument{state.stack[stack_base_index + i]};
                switch(callee.parameters[i])
                {
                    case value_type::i32:
                        emit_value_write_to_slot<wasm_i32>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::i64:
                        emit_value_write_to_slot<wasm_i64>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::f32:
                        emit_value_write_to_slot<wasm_f32>(state, argument, parameter_slot_offset);
                        break;
                    case value_type::f64:
                        emit_value_write_to_slot<wasm_f64>(state, argument, parameter_slot_offset);
                        break;
                }
            }

            for(::std::size_t i{}; i != callee.parameters.size(); ++i)
            {
                pop_value(state);
            }

            emit_object(state.bytecode, optable::translate::get_Compile_fptr());
            emit_object(state.bytecode, callee.handle);
            emit_slot_offset(state.bytecode, parameter_base_offset);

            if(callee.results.size() == 1u)
            {
                push_register_value(state, callee.results.front());
            }
            return true;
        }

        inline bool compile_else_block(compiler_state_t& state)
        {
            if(state.control_frames.empty())
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }

            auto& frame{state.control_frames.back()};
            if(frame.kind != control_frame_kind_t::if_ || frame.has_else)
            {
                set_error(state, compile_error_t::unsupported_control_flow);
                return false;
            }

            if(state.path_reachable)
            {
                auto const end_snapshot{make_frame_stack_snapshot(frame, false)};
                if(!validate_branch_target(state, ::std::addressof(frame), end_snapshot))
                {
                    return false;
                }

                frame.pending_end_patches.push_back(emit_branch_placeholder(state));
            }

            if(frame.false_patch.has_value())
            {
                resolve_control_patch_to_current_offset(state, *frame.false_patch);
                frame.false_patch.reset();
            }

            frame.has_else = true;
            restore_stack_snapshot(state, frame.entry_stack);
            state.path_reachable = true;
            return true;
        }

        [[nodiscard]] inline bool skip_unreachable_opcode(compiler_state_t& state, op_basic opcode)
        {
            switch(opcode)
            {
                case op_basic::block:
                case op_basic::loop:
                case op_basic::if_:
                {
                    ::std::optional<value_type> result_type{};
                    if(!read_block_result_type(state, result_type))
                    {
                        return false;
                    }
                    ++state.unreachable_control_depth;
                    return true;
                }
                case op_basic::else_:
                    if(state.unreachable_control_depth == 0u)
                    {
                        return compile_else_block(state);
                    }
                    return true;
                case op_basic::end:
                    if(state.unreachable_control_depth != 0u)
                    {
                        --state.unreachable_control_depth;
                        return true;
                    }
                    return compile_end_block(state);
                case op_basic::local_get:
                case op_basic::local_set:
                case op_basic::local_tee:
                case op_basic::br:
                case op_basic::br_if:
                case op_basic::call:
                {
                    wasm_u32 imm{};
                    return read_uleb_u32(state, imm);
                }
                case op_basic::call_indirect:
                {
                    wasm_u32 imm0{};
                    wasm_u32 imm1{};
                    return read_uleb_u32(state, imm0) && read_uleb_u32(state, imm1);
                }
                case op_basic::i32_const:
                {
                    wasm_i32 imm{};
                    return read_sleb(state, imm);
                }
                case op_basic::i64_const:
                {
                    wasm_i64 imm{};
                    return read_sleb(state, imm);
                }
                case op_basic::f32_const:
                {
                    wasm_f32 imm{};
                    return read_fixed(state, imm);
                }
                case op_basic::f64_const:
                {
                    wasm_f64 imm{};
                    return read_fixed(state, imm);
                }
                case op_basic::br_table:
                {
                    wasm_u32 count{};
                    if(!read_uleb_u32(state, count))
                    {
                        return false;
                    }
                    for(wasm_u32 i{}; i != count + 1u; ++i)
                    {
                        wasm_u32 target{};
                        if(!read_uleb_u32(state, target))
                        {
                            return false;
                        }
                    }
                    return true;
                }
                default:
                    return true;
            }
        }

        inline bool compile_end_block(compiler_state_t& state)
        {
            if(state.control_frames.empty())
            {
                return finish_function(state, false);
            }

            auto frame{::std::move(state.control_frames.back())};
            state.control_frames.pop_back();

            bool reachable_after{};
            switch(frame.kind)
            {
                case control_frame_kind_t::block:
                {
                    if(state.path_reachable)
                    {
                        auto const end_snapshot{make_frame_stack_snapshot(frame, false)};
                        if(!validate_branch_target(state, ::std::addressof(frame), end_snapshot))
                        {
                            return false;
                        }
                    }
                    for(auto const patch : frame.pending_end_patches)
                    {
                        resolve_control_patch_to_current_offset(state, patch);
                    }
                    reachable_after = state.path_reachable || !frame.pending_end_patches.empty();
                    restore_stack_snapshot(state, make_frame_stack_snapshot(frame, false));
                    state.path_reachable = reachable_after;
                    return true;
                }
                case control_frame_kind_t::loop:
                {
                    if(state.path_reachable)
                    {
                        auto const end_snapshot{make_frame_stack_snapshot(frame, false)};
                        if(!validate_branch_target(state, ::std::addressof(frame), end_snapshot))
                        {
                            return false;
                        }
                    }
                    restore_stack_snapshot(state, make_frame_stack_snapshot(frame, false));
                    return true;
                }
                case control_frame_kind_t::if_:
                {
                    if(frame.false_patch.has_value())
                    {
                        resolve_control_patch_to_current_offset(state, *frame.false_patch);
                    }
                    if(frame.result_type.has_value() && !frame.has_else)
                    {
                        set_error(state, compile_error_t::unsupported_control_flow);
                        return false;
                    }
                    if(state.path_reachable)
                    {
                        auto const end_snapshot{make_frame_stack_snapshot(frame, false)};
                        if(!validate_branch_target(state, ::std::addressof(frame), end_snapshot))
                        {
                            return false;
                        }
                    }
                    for(auto const patch : frame.pending_end_patches)
                    {
                        resolve_control_patch_to_current_offset(state, patch);
                    }
                    reachable_after = frame.has_else ? (state.path_reachable || !frame.pending_end_patches.empty()) : true;
                    restore_stack_snapshot(state, make_frame_stack_snapshot(frame, false));
                    state.path_reachable = reachable_after;
                    return true;
                }
            }

            ::fast_io::fast_terminate();
        }

        template <value_type Type>
        inline bool compile_select(compiler_state_t& state)
        {
            if(!require_top_type(state, value_type::i32) || !require_top_type(state, Type, 1u) || !require_top_type(state, Type, 2u))
            {
                return false;
            }

            auto const condition{state.stack.back()};
            auto const operand2{state.stack[state.stack.size() - 2u]};
            auto const operand1{state.stack[state.stack.size() - 3u]};

            auto const consumed_int_register{condition.location == location_kind_t::int_register || operand2.location == location_kind_t::int_register ||
                                             operand1.location == location_kind_t::int_register};
            auto const consumed_fp_register{condition.location == location_kind_t::fp_register || operand2.location == location_kind_t::fp_register ||
                                            operand1.location == location_kind_t::fp_register};
            if(is_fp_type(Type))
            {
                spill_register_if_needed(state, Type, consumed_fp_register);
            }
            else
            {
                spill_register_if_needed(state, Type, consumed_int_register);
            }

            auto const shape = [&]() noexcept {
                auto const condition_part{condition.location == location_kind_t::slot ? 's' : 'r'};
                auto const operand2_part{operand2.location == location_kind_t::slot ? 's' : 'r'};
                auto const operand1_part{operand1.location == location_kind_t::slot ? 's' : 'r'};

                if(condition_part == 'r' && operand2_part == 's' && operand1_part == 's') { return optable::select_shape_t::rss; }
                if(condition_part == 'r' && operand2_part == 'r' && operand1_part == 's') { return optable::select_shape_t::rrs; }
                if(condition_part == 'r' && operand2_part == 's' && operand1_part == 'r') { return optable::select_shape_t::rsr; }
                if(condition_part == 's' && operand2_part == 'r' && operand1_part == 's') { return optable::select_shape_t::srs; }
                if(condition_part == 's' && operand2_part == 's' && operand1_part == 'r') { return optable::select_shape_t::ssr; }
                return optable::select_shape_t::sss;
            }();

            switch(Type)
            {
                case value_type::i32:
                    switch(shape)
                    {
                        case optable::select_shape_t::rss:
                            emit_object(state.bytecode, optable::translate::get_Select_i32_fptr<optable::select_shape_t::rss>());
                            break;
                        case optable::select_shape_t::srs:
                            emit_object(state.bytecode, optable::translate::get_Select_i32_fptr<optable::select_shape_t::srs>());
                            break;
                        case optable::select_shape_t::ssr:
                            emit_object(state.bytecode, optable::translate::get_Select_i32_fptr<optable::select_shape_t::ssr>());
                            break;
                        case optable::select_shape_t::sss:
                            emit_object(state.bytecode, optable::translate::get_Select_i32_fptr<optable::select_shape_t::sss>());
                            break;
                        default:
                            set_error(state, compile_error_t::type_mismatch);
                            return false;
                    }
                    break;
                case value_type::i64:
                    switch(shape)
                    {
                        case optable::select_shape_t::rss:
                            emit_object(state.bytecode, optable::translate::get_Select_i64_fptr<optable::select_shape_t::rss>());
                            break;
                        case optable::select_shape_t::srs:
                            emit_object(state.bytecode, optable::translate::get_Select_i64_fptr<optable::select_shape_t::srs>());
                            break;
                        case optable::select_shape_t::ssr:
                            emit_object(state.bytecode, optable::translate::get_Select_i64_fptr<optable::select_shape_t::ssr>());
                            break;
                        case optable::select_shape_t::sss:
                            emit_object(state.bytecode, optable::translate::get_Select_i64_fptr<optable::select_shape_t::sss>());
                            break;
                        default:
                            set_error(state, compile_error_t::type_mismatch);
                            return false;
                    }
                    break;
                case value_type::f32:
                    switch(shape)
                    {
                        case optable::select_shape_t::rss:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::rss>());
                            break;
                        case optable::select_shape_t::rrs:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::rrs>());
                            break;
                        case optable::select_shape_t::rsr:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::rsr>());
                            break;
                        case optable::select_shape_t::srs:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::srs>());
                            break;
                        case optable::select_shape_t::ssr:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::ssr>());
                            break;
                        case optable::select_shape_t::sss:
                            emit_object(state.bytecode, optable::translate::get_Select_f32_fptr<optable::select_shape_t::sss>());
                            break;
                    }
                    break;
                case value_type::f64:
                    switch(shape)
                    {
                        case optable::select_shape_t::rss:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::rss>());
                            break;
                        case optable::select_shape_t::rrs:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::rrs>());
                            break;
                        case optable::select_shape_t::rsr:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::rsr>());
                            break;
                        case optable::select_shape_t::srs:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::srs>());
                            break;
                        case optable::select_shape_t::ssr:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::ssr>());
                            break;
                        case optable::select_shape_t::sss:
                            emit_object(state.bytecode, optable::translate::get_Select_f64_fptr<optable::select_shape_t::sss>());
                            break;
                    }
                    break;
            }

            if(condition.location == location_kind_t::slot)
            {
                emit_slot_offset(state.bytecode, condition.slot_offset);
            }
            if(operand2.location == location_kind_t::slot)
            {
                emit_slot_offset(state.bytecode, operand2.slot_offset);
            }
            if(operand1.location == location_kind_t::slot)
            {
                emit_slot_offset(state.bytecode, operand1.slot_offset);
            }

            pop_value(state);
            pop_value(state);
            pop_value(state);
            push_register_value(state, Type);
            return true;
        }

        inline bool finish_function(compiler_state_t& state, bool emit_return) noexcept
        {
            if(state.descriptor.results.size() > 1u)
            {
                set_error(state, compile_error_t::invalid_result_arity);
                return false;
            }

            if(state.descriptor.results.empty())
            {
                if(!state.stack.empty())
                {
                    set_error(state, compile_error_t::invalid_final_stack);
                    return false;
                }
            }
            else
            {
                auto const result_type{state.descriptor.results.front()};
                if(state.stack.size() != 1u || state.stack.back().type != result_type)
                {
                    set_error(state, compile_error_t::invalid_final_stack);
                    return false;
                }

                auto const result{state.stack.back()};
                if(result.location == location_kind_t::slot)
                {
                    switch(result_type)
                    {
                        case value_type::i32:
                            emit_set_register<wasm_i32>(state, result.slot_offset);
                            break;
                        case value_type::i64:
                            emit_set_register<wasm_i64>(state, result.slot_offset);
                            break;
                        case value_type::f32:
                            emit_set_register<wasm_f32>(state, result.slot_offset);
                            break;
                        case value_type::f64:
                            emit_set_register<wasm_f64>(state, result.slot_offset);
                            break;
                    }
                    state.stack.back().location = register_location_for(result_type);
                    if(is_fp_type(result_type))
                    {
                        state.fp_register_index = 0u;
                    }
                    else
                    {
                        state.int_register_index = 0u;
                    }
                }
            }

            state.function_end_offset = state.bytecode.size();
            state.function_end_offset_ready = true;
            emit_object(state.bytecode, emit_return ? optable::translate::get_Return_fptr() : optable::translate::get_End_fptr());
            state.terminated = true;
            return true;
        }
    }

    inline compile_result_t compile_function(function_descriptor_t const& descriptor)
    {
        compile_result_t result{};

        details::compiler_state_t state{
            .descriptor = descriptor,
            .bytecode = {},
            .stack = {},
            .int_register_index = {},
            .fp_register_index = {},
            .next_temp_slot_offset = details::total_local_count(descriptor) * details::slot_size,
            .max_slot_bytes = details::total_local_count(descriptor) * details::slot_size,
            .curr = descriptor.expression.data(),
            .end = descriptor.expression.data() + descriptor.expression.size(),
            .terminated = false,
            .error = compile_error_t::none,
            .error_offset = 0u};

        while(state.curr != state.end && state.error == compile_error_t::none && !state.terminated)
        {
            auto const opcode{static_cast<op_basic>(*state.curr++)};
            if(!state.path_reachable)
            {
                if(!details::skip_unreachable_opcode(state, opcode))
                {
                    break;
                }
                continue;
            }
            switch(opcode)
            {
                case op_basic::end:
                    details::compile_end_block(state);
                    break;
                case op_basic::return_:
                    details::finish_function(state, true);
                    break;
                case op_basic::drop:
                    if(details::require_stack_size(state, 1u))
                    {
                        details::pop_value(state);
                    }
                    break;
                case op_basic::local_get:
                {
                    auto const saved_curr{state.curr};
                    wasm_u32 local_index_u32{};
                    if(!details::read_uleb_u32(state, local_index_u32))
                    {
                        break;
                    }
                    bool valid_local{};
                    auto const local_type{details::get_local_type(descriptor, static_cast<::std::size_t>(local_index_u32), valid_local)};
                    state.curr = saved_curr;
                    if(!valid_local)
                    {
                        details::set_error(state, compile_error_t::invalid_local_index);
                        break;
                    }
                    switch(local_type)
                    {
                        case value_type::i32:
                            details::compile_local_get<value_type::i32>(state, []() consteval { return optable::translate::get_SetRegister_fptr<wasm_i32>(); });
                            break;
                        case value_type::i64:
                            details::compile_local_get<value_type::i64>(state, []() consteval { return optable::translate::get_SetRegister_fptr<wasm_i64>(); });
                            break;
                        case value_type::f32:
                            details::compile_local_get<value_type::f32>(state, []() consteval { return optable::translate::get_SetRegister_fptr<wasm_f32>(); });
                            break;
                        case value_type::f64:
                            details::compile_local_get<value_type::f64>(state, []() consteval { return optable::translate::get_SetRegister_fptr<wasm_f64>(); });
                            break;
                    }
                            break;
                }
                case op_basic::local_set:
                case op_basic::local_tee:
                {
                    if(!details::require_stack_size(state, 1u))
                    {
                        break;
                    }
                    auto const source_type{state.stack.back().type};
                    switch(source_type)
                    {
                        case value_type::i32:
                            details::compile_local_set<value_type::i32>(state, opcode == op_basic::local_tee);
                            break;
                        case value_type::i64:
                            details::compile_local_set<value_type::i64>(state, opcode == op_basic::local_tee);
                            break;
                        case value_type::f32:
                            details::compile_local_set<value_type::f32>(state, opcode == op_basic::local_tee);
                            break;
                        case value_type::f64:
                            details::compile_local_set<value_type::f64>(state, opcode == op_basic::local_tee);
                            break;
                    }
                    break;
                }
                case op_basic::i32_const:
                    details::compile_const<value_type::i32>(state);
                    break;
                case op_basic::i64_const:
                    details::compile_const<value_type::i64>(state);
                    break;
                case op_basic::f32_const:
                    details::compile_const<value_type::f32>(state);
                    break;
                case op_basic::f64_const:
                    details::compile_const<value_type::f64>(state);
                    break;
                case op_basic::select:
                {
                    if(!details::require_stack_size(state, 3u))
                    {
                        break;
                    }
                    auto const select_type{state.stack[state.stack.size() - 2u].type};
                    switch(select_type)
                    {
                        case value_type::i32:
                            details::compile_select<value_type::i32>(state);
                            break;
                        case value_type::i64:
                            details::compile_select<value_type::i64>(state);
                            break;
                        case value_type::f32:
                            details::compile_select<value_type::f32>(state);
                            break;
                        case value_type::f64:
                            details::compile_select<value_type::f64>(state);
                            break;
                    }
                    break;
                }
                case op_basic::i32_eqz:
                    details::compile_unary_numeric<wasm_i32, wasm_i32, optable::numeric_details::equal_to_zero_operation>(state);
                    break;
                case op_basic::i32_eq:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::equal_operation, true>(state);
                    break;
                case op_basic::i32_ne:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::not_equal_operation, true>(state);
                    break;
                case op_basic::i32_lt_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::i32_lt_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::i32_gt_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::i32_gt_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::i32_le_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::i32_le_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::i32_ge_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::i32_ge_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::i64_eqz:
                    details::compile_unary_numeric<wasm_i32, wasm_i64, optable::numeric_details::equal_to_zero_operation>(state);
                    break;
                case op_basic::i64_eq:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::equal_operation, true>(state);
                    break;
                case op_basic::i64_ne:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::not_equal_operation, true>(state);
                    break;
                case op_basic::i64_lt_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::i64_lt_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u64, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::i64_gt_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::i64_gt_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u64, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::i64_le_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::i64_le_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u64, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::i64_ge_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i64, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::i64_ge_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u64, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::f32_eq:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::equal_operation, true>(state);
                    break;
                case op_basic::f32_ne:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::not_equal_operation, true>(state);
                    break;
                case op_basic::f32_lt:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::f32_gt:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::f32_le:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::f32_ge:
                    details::compile_binary_numeric<wasm_i32, wasm_f32, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::f64_eq:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::equal_operation, true>(state);
                    break;
                case op_basic::f64_ne:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::not_equal_operation, true>(state);
                    break;
                case op_basic::f64_lt:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::less_than_operation, false>(state);
                    break;
                case op_basic::f64_gt:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::greater_than_operation, false>(state);
                    break;
                case op_basic::f64_le:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::less_equal_operation, false>(state);
                    break;
                case op_basic::f64_ge:
                    details::compile_binary_numeric<wasm_i32, wasm_f64, optable::numeric_details::greater_equal_operation, false>(state);
                    break;
                case op_basic::i32_clz:
                    details::compile_unary_numeric<wasm_i32, wasm_u32, optable::numeric_details::countl_zero_operation>(state);
                    break;
                case op_basic::i32_ctz:
                    details::compile_unary_numeric<wasm_i32, wasm_u32, optable::numeric_details::countr_zero_operation>(state);
                    break;
                case op_basic::i32_popcnt:
                    details::compile_unary_numeric<wasm_i32, wasm_u32, optable::numeric_details::popcount_operation>(state);
                    break;
                case op_basic::i32_add:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::add_operation, true>(state);
                    break;
                case op_basic::i32_sub:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::subtract_operation, false>(state);
                    break;
                case op_basic::i32_mul:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::multiply_operation, true>(state);
                    break;
                case op_basic::i32_div_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::signed_divide_operation, false>(state);
                    break;
                case op_basic::i32_div_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::unsigned_divide_operation, false>(state);
                    break;
                case op_basic::i32_rem_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::signed_remainder_operation, false>(state);
                    break;
                case op_basic::i32_rem_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::unsigned_remainder_operation, false>(state);
                    break;
                case op_basic::i32_and:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::and_operation, true>(state);
                    break;
                case op_basic::i32_or:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::or_operation, true>(state);
                    break;
                case op_basic::i32_xor:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::xor_operation, true>(state);
                    break;
                case op_basic::i32_shl:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::shift_left_operation, false>(state);
                    break;
                case op_basic::i32_shr_s:
                    details::compile_binary_numeric<wasm_i32, wasm_i32, optable::numeric_details::shift_right_operation, false>(state);
                    break;
                case op_basic::i32_shr_u:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::shift_right_operation, false>(state);
                    break;
                case op_basic::i32_rotl:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::rotate_left_operation, false>(state);
                    break;
                case op_basic::i32_rotr:
                    details::compile_binary_numeric<wasm_i32, wasm_u32, optable::numeric_details::rotate_right_operation, false>(state);
                    break;
                case op_basic::i64_clz:
                    details::compile_unary_numeric<wasm_i64, wasm_u64, optable::numeric_details::countl_zero_operation>(state);
                    break;
                case op_basic::i64_ctz:
                    details::compile_unary_numeric<wasm_i64, wasm_u64, optable::numeric_details::countr_zero_operation>(state);
                    break;
                case op_basic::i64_popcnt:
                    details::compile_unary_numeric<wasm_i64, wasm_u64, optable::numeric_details::popcount_operation>(state);
                    break;
                case op_basic::i64_add:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::add_operation, true>(state);
                    break;
                case op_basic::i64_sub:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::subtract_operation, false>(state);
                    break;
                case op_basic::i64_mul:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::multiply_operation, true>(state);
                    break;
                case op_basic::i64_div_s:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::signed_divide_operation, false>(state);
                    break;
                case op_basic::i64_div_u:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::unsigned_divide_operation, false>(state);
                    break;
                case op_basic::i64_rem_s:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::signed_remainder_operation, false>(state);
                    break;
                case op_basic::i64_rem_u:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::unsigned_remainder_operation, false>(state);
                    break;
                case op_basic::i64_and:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::and_operation, true>(state);
                    break;
                case op_basic::i64_or:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::or_operation, true>(state);
                    break;
                case op_basic::i64_xor:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::xor_operation, true>(state);
                    break;
                case op_basic::i64_shl:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::shift_left_operation, false>(state);
                    break;
                case op_basic::i64_shr_s:
                    details::compile_binary_numeric<wasm_i64, wasm_i64, optable::numeric_details::shift_right_operation, false>(state);
                    break;
                case op_basic::i64_shr_u:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::shift_right_operation, false>(state);
                    break;
                case op_basic::i64_rotl:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::rotate_left_operation, false>(state);
                    break;
                case op_basic::i64_rotr:
                    details::compile_binary_numeric<wasm_i64, wasm_u64, optable::numeric_details::rotate_right_operation, false>(state);
                    break;
                case op_basic::f32_abs:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::abs_operation>(state);
                    break;
                case op_basic::f32_neg:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::negate_operation>(state);
                    break;
                case op_basic::f32_ceil:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::ceil_operation>(state);
                    break;
                case op_basic::f32_floor:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::floor_operation>(state);
                    break;
                case op_basic::f32_trunc:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::trunc_operation>(state);
                    break;
                case op_basic::f32_nearest:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::nearest_operation>(state);
                    break;
                case op_basic::f32_sqrt:
                    details::compile_unary_numeric<wasm_f32, wasm_f32, optable::numeric_details::sqrt_operation>(state);
                    break;
                case op_basic::f32_add:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::add_operation, true>(state);
                    break;
                case op_basic::f32_sub:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::subtract_operation, false>(state);
                    break;
                case op_basic::f32_mul:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::multiply_operation, true>(state);
                    break;
                case op_basic::f32_div:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::float_divide_operation, false>(state);
                    break;
                case op_basic::f32_min:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::min_operation, false>(state);
                    break;
                case op_basic::f32_max:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::max_operation, false>(state);
                    break;
                case op_basic::f32_copysign:
                    details::compile_binary_numeric<wasm_f32, wasm_f32, optable::numeric_details::copysign_operation, false>(state);
                    break;
                case op_basic::f64_abs:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::abs_operation>(state);
                    break;
                case op_basic::f64_neg:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::negate_operation>(state);
                    break;
                case op_basic::f64_ceil:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::ceil_operation>(state);
                    break;
                case op_basic::f64_floor:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::floor_operation>(state);
                    break;
                case op_basic::f64_trunc:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::trunc_operation>(state);
                    break;
                case op_basic::f64_nearest:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::nearest_operation>(state);
                    break;
                case op_basic::f64_sqrt:
                    details::compile_unary_numeric<wasm_f64, wasm_f64, optable::numeric_details::sqrt_operation>(state);
                    break;
                case op_basic::f64_add:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::add_operation, true>(state);
                    break;
                case op_basic::f64_sub:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::subtract_operation, false>(state);
                    break;
                case op_basic::f64_mul:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::multiply_operation, true>(state);
                    break;
                case op_basic::f64_div:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::float_divide_operation, false>(state);
                    break;
                case op_basic::f64_min:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::min_operation, false>(state);
                    break;
                case op_basic::f64_max:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::max_operation, false>(state);
                    break;
                case op_basic::f64_copysign:
                    details::compile_binary_numeric<wasm_f64, wasm_f64, optable::numeric_details::copysign_operation, false>(state);
                    break;
                case op_basic::i32_wrap_i64:
                    details::compile_unary_transform<wasm_i32, wasm_i64, optable::convert_details::wrap_i64_operation>(state);
                    break;
                case op_basic::i32_trunc_f32_s:
                    details::compile_unary_transform<wasm_i32, wasm_f32, optable::convert_details::trunc_s_i32_operation>(state);
                    break;
                case op_basic::i32_trunc_f32_u:
                    details::compile_unary_transform<wasm_i32, wasm_f32, optable::convert_details::trunc_u32_operation>(state);
                    break;
                case op_basic::i32_trunc_f64_s:
                    details::compile_unary_transform<wasm_i32, wasm_f64, optable::convert_details::trunc_s_i32_operation>(state);
                    break;
                case op_basic::i32_trunc_f64_u:
                    details::compile_unary_transform<wasm_i32, wasm_f64, optable::convert_details::trunc_u32_operation>(state);
                    break;
                case op_basic::i64_extend_i32_s:
                    details::compile_unary_transform<wasm_i64, wasm_i32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::i64_extend_i32_u:
                    details::compile_unary_transform<wasm_i64, wasm_u32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::i64_trunc_f32_s:
                    details::compile_unary_transform<wasm_i64, wasm_f32, optable::convert_details::trunc_s_i64_operation>(state);
                    break;
                case op_basic::i64_trunc_f32_u:
                    details::compile_unary_transform<wasm_i64, wasm_f32, optable::convert_details::trunc_u64_operation>(state);
                    break;
                case op_basic::i64_trunc_f64_s:
                    details::compile_unary_transform<wasm_i64, wasm_f64, optable::convert_details::trunc_s_i64_operation>(state);
                    break;
                case op_basic::i64_trunc_f64_u:
                    details::compile_unary_transform<wasm_i64, wasm_f64, optable::convert_details::trunc_u64_operation>(state);
                    break;
                case op_basic::f32_convert_i32_s:
                    details::compile_unary_transform<wasm_f32, wasm_i32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f32_convert_i32_u:
                    details::compile_unary_transform<wasm_f32, wasm_u32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f32_convert_i64_s:
                    details::compile_unary_transform<wasm_f32, wasm_i64, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f32_convert_i64_u:
                    details::compile_unary_transform<wasm_f32, wasm_u64, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f32_demote_f64:
                    details::compile_unary_transform<wasm_f32, wasm_f64, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f64_convert_i32_s:
                    details::compile_unary_transform<wasm_f64, wasm_i32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f64_convert_i32_u:
                    details::compile_unary_transform<wasm_f64, wasm_u32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f64_convert_i64_s:
                    details::compile_unary_transform<wasm_f64, wasm_i64, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f64_convert_i64_u:
                    details::compile_unary_transform<wasm_f64, wasm_u64, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::f64_promote_f32:
                    details::compile_unary_transform<wasm_f64, wasm_f32, optable::convert_details::static_cast_operation>(state);
                    break;
                case op_basic::i32_reinterpret_f32:
                    details::compile_unary_transform<wasm_i32, wasm_f32, optable::convert_details::reinterpret_cast_operation>(state);
                    break;
                case op_basic::i64_reinterpret_f64:
                    details::compile_unary_transform<wasm_i64, wasm_f64, optable::convert_details::reinterpret_cast_operation>(state);
                    break;
                case op_basic::f32_reinterpret_i32:
                    details::compile_unary_transform<wasm_f32, wasm_i32, optable::convert_details::reinterpret_cast_operation>(state);
                    break;
                case op_basic::f64_reinterpret_i64:
                    details::compile_unary_transform<wasm_f64, wasm_i64, optable::convert_details::reinterpret_cast_operation>(state);
                    break;
                case op_basic::block:
                    details::compile_enter_block(state, details::control_frame_kind_t::block);
                    break;
                case op_basic::loop:
                    details::compile_enter_block(state, details::control_frame_kind_t::loop);
                    break;
                case op_basic::if_:
                    details::compile_if_block(state);
                    break;
                case op_basic::else_:
                    details::compile_else_block(state);
                    break;
                case op_basic::br:
                    details::compile_branch(state, false);
                    break;
                case op_basic::br_if:
                    details::compile_branch(state, true);
                    break;
                case op_basic::br_table:
                    details::compile_branch_table(state);
                    break;
                case op_basic::call:
                    details::compile_call(state);
                    break;
                case op_basic::call_indirect:
                    details::compile_call_indirect(state);
                    break;
                default:
                    details::set_error(state, compile_error_t::unsupported_opcode);
                    break;
            }
        }

        if(state.error == compile_error_t::none && !state.terminated)
        {
            state.error = compile_error_t::missing_end;
        }

        result.function.bytecode = ::std::move(state.bytecode);
        if(state.error == compile_error_t::none)
        {
            if(!state.function_end_offset_ready)
            {
                state.error = compile_error_t::missing_end;
            }
            else
            {
                for(auto const patch : state.control_patches)
                {
                    auto const target{reinterpret_cast<optable::m3_code_t>(result.function.bytecode.data() + static_cast<::std::ptrdiff_t>(patch.target_offset))};
                    ::std::memcpy(result.function.bytecode.data() + static_cast<::std::ptrdiff_t>(patch.immediate_offset),
                                  ::std::addressof(target),
                                  sizeof(target));
                }
                for(auto const patch : state.function_end_patches)
                {
                    auto const target{reinterpret_cast<optable::m3_code_t>(result.function.bytecode.data() + static_cast<::std::ptrdiff_t>(state.function_end_offset))};
                    ::std::memcpy(result.function.bytecode.data() + static_cast<::std::ptrdiff_t>(patch), ::std::addressof(target), sizeof(target));
                }
            }
        }
        result.function.parameter_count = descriptor.parameters.size();
        result.function.local_count = details::total_local_count(descriptor);
        result.function.stack_bytes = state.max_slot_bytes;
        if(descriptor.results.size() == 1u)
        {
            result.function.result_type = descriptor.results.front();
        }
        result.error = state.error;
        result.expression_offset = state.error_offset;
        return result;
    }

    namespace details
    {
        inline void interpret_compiled_function(compiled_function_t const& function,
                                                optable::m3_stack_t sp,
                                                optable::m3_int_register_t& r0,
                                                optable::m3_fp_register_t& fp0) noexcept
        {
            auto ip{static_cast<optable::m3_code_t>(function.bytecode.data())};
            while(optable::control_state.signal == optable::m3_control_signal_t::none)
            {
                optable::m3_interpreter_opfunc_t op{};
                ::std::memcpy(::std::addressof(op), ip, sizeof(op));
                ip += sizeof(op);
                op(ip, sp, r0, fp0);
            }
        }

        [[nodiscard]] inline module_function_t const* find_module_function_by_pc(optable::m3_code_t pc) noexcept
        {
            if(current_execution_runtime_context == nullptr)
            {
                return nullptr;
            }
            for(auto const& function : current_execution_runtime_context->module_functions)
            {
                if(!function.compile_attempted || !function.compiled || function.compiled.function.bytecode.empty())
                {
                    continue;
                }
                if(static_cast<optable::m3_code_t>(function.compiled.function.bytecode.data()) == pc)
                {
                    return ::std::addressof(function);
                }
            }
            return nullptr;
        }

        [[nodiscard]] inline module_function_t const* module_function_from_handle(void* handle) noexcept
        {
            return static_cast<module_function_t const*>(handle);
        }

        [[nodiscard]] inline optable::m3_code_t straight_line_compile_hook(void* handle) noexcept
        {
            auto const* const function{module_function_from_handle(handle)};
            if(function == nullptr)
            {
                return nullptr;
            }
            if(!function->compile_attempted)
            {
                function->compiled = compile_function(function->descriptor);
                function->compile_attempted = true;
            }
            if(!function->compiled || function->compiled.function.bytecode.empty())
            {
                return nullptr;
            }
            return static_cast<optable::m3_code_t>(function->compiled.function.bytecode.data());
        }

        inline void invoke_module_function(module_function_t const& function,
                                           optable::m3_stack_t sp,
                                           optable::m3_int_register_t& r0,
                                           optable::m3_fp_register_t& fp0) noexcept
        {
            auto const compiled_pc{straight_line_compile_hook(const_cast<module_function_t*>(::std::addressof(function)))};
            if(compiled_pc == nullptr)
            {
                ::std::terminate();
            }

            auto const& compiled{function.compiled.function};
            ::uwvm2::utils::container::vector<::std::byte> stack{};
            stack.resize(compiled.stack_bytes);
            ::std::memset(stack.data(), 0, stack.size());
            for(::std::size_t i{}; i != compiled.parameter_count; ++i)
            {
                ::std::memcpy(stack.data() + static_cast<::std::ptrdiff_t>(i * slot_size),
                              sp + static_cast<::std::ptrdiff_t>(i * slot_size),
                              sizeof(wasm_u64));
            }

            optable::control_state = {};
            interpret_compiled_function(compiled, stack.data(), r0, fp0);
            if(optable::control_state.signal == optable::m3_control_signal_t::returned)
            {
                optable::control_state = {};
            }
        }

        inline void straight_line_call_hook(optable::m3_code_t call_pc,
                                            optable::m3_stack_t sp,
                                            optable::m3_int_register_t& r0,
                                            optable::m3_fp_register_t& fp0) noexcept
        {
            auto const* const function{find_module_function_by_pc(call_pc)};
            if(function == nullptr)
            {
                ::std::terminate();
            }
            invoke_module_function(*function, sp, r0, fp0);
        }

        inline void straight_line_indirect_call_hook(optable::wasm_u32 selector,
                                                     void* table_handle,
                                                     void* type_handle,
                                                     optable::m3_stack_t sp,
                                                     optable::m3_int_register_t& r0,
                                                     optable::m3_fp_register_t& fp0) noexcept
        {
            auto const* const table{static_cast<table_reference_t const*>(table_handle)};
            auto const* const expected_type{static_cast<indirect_type_reference_t const*>(type_handle)};
            if(table == nullptr || expected_type == nullptr)
            {
                ::std::terminate();
            }
            if(selector >= table->elements.size())
            {
                ::std::terminate();
            }

            auto* const handle{table->elements[selector]};
            auto const* const function{module_function_from_handle(handle)};
            if(function == nullptr)
            {
                ::std::terminate();
            }
            if(!value_type_span_equal(function->descriptor.parameters, expected_type->parameters) ||
               !value_type_span_equal(function->descriptor.results, expected_type->results))
            {
                ::std::terminate();
            }
            invoke_module_function(*function, sp, r0, fp0);
        }
    }

    inline bool execute_function(compiled_function_t const& function,
                                 ::std::span<wasm_u64 const> parameters,
                                 execution_result_t& out,
                                 ::std::span<module_function_t const> module_functions) noexcept
    {
        if(parameters.size() != function.parameter_count)
        {
            return false;
        }

        out = {};
        out.stack.resize(function.stack_bytes);
        ::std::memset(out.stack.data(), 0, out.stack.size());
        for(::std::size_t i{}; i != parameters.size(); ++i)
        {
            ::std::memcpy(out.stack.data() + static_cast<::std::ptrdiff_t>(i * details::slot_size), ::std::addressof(parameters[i]), sizeof(parameters[i]));
        }

        auto const previous_call_func{optable::call_func};
        auto const previous_compile_func{optable::compile_func};
        auto const previous_indirect_call_func{optable::indirect_call_func};
        auto const previous_runtime_context{details::current_execution_runtime_context};
        details::execution_runtime_context_t runtime_context{.module_functions = module_functions};
        details::current_execution_runtime_context = ::std::addressof(runtime_context);
        optable::call_func = details::straight_line_call_hook;
        optable::compile_func = details::straight_line_compile_hook;
        optable::indirect_call_func = details::straight_line_indirect_call_hook;

        optable::control_state = {};
        details::interpret_compiled_function(function, out.stack.data(), out.r0, out.fp0);
        out.control = optable::control_state;

        optable::call_func = previous_call_func;
        optable::compile_func = previous_compile_func;
        optable::indirect_call_func = previous_indirect_call_func;
        details::current_execution_runtime_context = previous_runtime_context;
        return true;
    }

    inline bool execute_function(compiled_function_t const& function, ::std::span<wasm_u64 const> parameters, execution_result_t& out) noexcept
    {
        return execute_function(function, parameters, out, {});
    }

    template <typename T>
    [[nodiscard]] inline T get_result(execution_result_t const& result) noexcept
    {
        if constexpr(::std::same_as<T, wasm_i32>)
        {
            return static_cast<wasm_i32>(result.r0);
        }
        else if constexpr(::std::same_as<T, wasm_u32>)
        {
            return ::std::bit_cast<wasm_u32>(static_cast<wasm_i32>(result.r0));
        }
        else if constexpr(::std::same_as<T, wasm_i64>)
        {
            return static_cast<wasm_i64>(result.r0);
        }
        else if constexpr(::std::same_as<T, wasm_u64>)
        {
            return ::std::bit_cast<wasm_u64>(static_cast<wasm_i64>(result.r0));
        }
        else if constexpr(::std::same_as<T, wasm_f32>)
        {
            return static_cast<wasm_f32>(result.fp0);
        }
        else
        {
            return static_cast<wasm_f64>(result.fp0);
        }
    }
}
