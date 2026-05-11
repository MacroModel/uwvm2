/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @copyright   APL-2.0 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

#pragma once

#ifndef UWVM_MODULE
// std
# include <cstddef>
# include <cstdint>
# include <cstring>
# include <limits>
# include <memory>
# include <concepts>
// macro
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/runtime/compiler/uwvm_int/macro/push_macros.h>
# include <uwvm2/uwvm/runtime/macro/push_macros.h>
// import
# include <fast_io.h>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/parser/wasm/standard/wasm1/impl.h>
# include <uwvm2/object/impl.h>
# include "define.h"
# include "storage.h"
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

#if defined(UWVM_RUNTIME_UWVM_INTERPRETER)
#if !(__cpp_pack_indexing >= 202311L)
# error "UWVM requires at least C++26 standard compiler. See https://en.cppreference.com/w/cpp/feature_test#cpp_pack_indexing"
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::compiler::uwvm_int::optable
{
    namespace details
    {
        /// @brief Runtime tiered probe bridge.
        /// @details The probe is intentionally tiny: interpreter bytecode owns the placement policy, while the runtime owns the hotness and scheduling policy.
        UWVM_ALWAYS_INLINE inline constexpr void
            tiered_probe(::std::size_t curr_module_id, ::std::size_t function_index, ::std::uint_least32_t probe_kind) noexcept
        {
            auto const probe_func{::uwvm2::runtime::compiler::uwvm_int::optable::tiered_probe_func};
            if(probe_func == nullptr) [[unlikely]] { return; }
            probe_func(curr_module_id, function_index, probe_kind);
        }
    }  // namespace details

    /// @brief Tiered compilation probe (tail-call): records a function/block/loop hotness event and continues at the next op.
    /// @details
    /// - Stack-top optimization: not applicable; no operand values are read or written.
    /// - `type[0]` layout: `[opfunc_ptr][curr_module_id][function_index][probe_kind:u32][next_opfunc_ptr]`.
    template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
              ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... Type>
        requires (CompileOption.is_tail_call)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline constexpr void uwvmint_tiered_probe(Type... type) UWVM_THROWS
    {
        static_assert(sizeof...(Type) >= 1uz);
        static_assert(::std::same_as<Type...[0u], ::std::byte const*>);

        type...[0] += sizeof(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_opfunc_t<Type...>);

        ::std::size_t curr_module_id;  // no init
        ::std::memcpy(::std::addressof(curr_module_id), type...[0], sizeof(curr_module_id));
        type...[0] += sizeof(curr_module_id);

        ::std::size_t function_index;  // no init
        ::std::memcpy(::std::addressof(function_index), type...[0], sizeof(function_index));
        type...[0] += sizeof(function_index);

        ::std::uint_least32_t probe_kind;  // no init
        ::std::memcpy(::std::addressof(probe_kind), type...[0], sizeof(probe_kind));
        type...[0] += sizeof(probe_kind);

        details::tiered_probe(curr_module_id, function_index, probe_kind);

        ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_opfunc_t<Type...> next_interpreter;  // no init
        ::std::memcpy(::std::addressof(next_interpreter), type...[0], sizeof(next_interpreter));

        UWVM_MUSTTAIL return next_interpreter(type...);
    }

    /// @brief Tiered compilation probe (non-tail-call/byref): records a hotness event and returns to the dispatcher.
    template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
              ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... TypeRef>
        requires (!CompileOption.is_tail_call)
    UWVM_INTERPRETER_OPFUNC_HOT_MACRO inline constexpr void uwvmint_tiered_probe(TypeRef & ... typeref) UWVM_THROWS
    {
        static_assert(sizeof...(TypeRef) >= 1uz);
        static_assert(::std::same_as<TypeRef...[0u], ::std::byte const*>);
        static_assert(CompileOption.i32_stack_top_begin_pos == SIZE_MAX && CompileOption.i32_stack_top_end_pos == SIZE_MAX);
        static_assert(CompileOption.i64_stack_top_begin_pos == SIZE_MAX && CompileOption.i64_stack_top_end_pos == SIZE_MAX);
        static_assert(CompileOption.f32_stack_top_begin_pos == SIZE_MAX && CompileOption.f32_stack_top_end_pos == SIZE_MAX);
        static_assert(CompileOption.f64_stack_top_begin_pos == SIZE_MAX && CompileOption.f64_stack_top_end_pos == SIZE_MAX);
        static_assert(CompileOption.v128_stack_top_begin_pos == SIZE_MAX && CompileOption.v128_stack_top_end_pos == SIZE_MAX);

        typeref...[0] += sizeof(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_opfunc_byref_t<TypeRef...>);

        ::std::size_t curr_module_id;  // no init
        ::std::memcpy(::std::addressof(curr_module_id), typeref...[0], sizeof(curr_module_id));
        typeref...[0] += sizeof(curr_module_id);

        ::std::size_t function_index;  // no init
        ::std::memcpy(::std::addressof(function_index), typeref...[0], sizeof(function_index));
        typeref...[0] += sizeof(function_index);

        ::std::uint_least32_t probe_kind;  // no init
        ::std::memcpy(::std::addressof(probe_kind), typeref...[0], sizeof(probe_kind));
        typeref...[0] += sizeof(probe_kind);

        details::tiered_probe(curr_module_id, function_index, probe_kind);
    }

    namespace translate
    {
        /// @brief Translator: returns the interpreter function pointer for the tiered probe (tail-call).
        template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
                  ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... Type>
            requires (CompileOption.is_tail_call)
        inline constexpr uwvm_interpreter_opfunc_t<Type...>
            get_uwvmint_tiered_probe_fptr(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_currpos_t const&) noexcept
        { return uwvmint_tiered_probe<CompileOption, Type...>; }

        template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
                  ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... TypeInTuple>
            requires (CompileOption.is_tail_call)
        inline constexpr auto
            get_uwvmint_tiered_probe_fptr_from_tuple(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_currpos_t const& curr_stacktop,
                                                     ::uwvm2::utils::container::tuple<TypeInTuple...> const&) noexcept
        { return get_uwvmint_tiered_probe_fptr<CompileOption, TypeInTuple...>(curr_stacktop); }

        /// @brief Translator: returns the interpreter function pointer for the tiered probe (non-tail-call/byref).
        template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
                  ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... Type>
            requires (!CompileOption.is_tail_call)
        inline constexpr uwvm_interpreter_opfunc_byref_t<Type...>
            get_uwvmint_tiered_probe_fptr(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_currpos_t const&) noexcept
        { return uwvmint_tiered_probe<CompileOption, Type...>; }

        template <::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_translate_option_t CompileOption,
                  ::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_int_stack_top_type... TypeInTuple>
            requires (!CompileOption.is_tail_call)
        inline constexpr auto
            get_uwvmint_tiered_probe_fptr_from_tuple(::uwvm2::runtime::compiler::uwvm_int::optable::uwvm_interpreter_stacktop_currpos_t const& curr_stacktop,
                                                     ::uwvm2::utils::container::tuple<TypeInTuple...> const&) noexcept
        { return get_uwvmint_tiered_probe_fptr<CompileOption, TypeInTuple...>(curr_stacktop); }
    }  // namespace translate
}
#endif

#ifndef UWVM_MODULE
// macro
# include <uwvm2/uwvm/runtime/macro/pop_macros.h>
# include <uwvm2/runtime/compiler/uwvm_int/macro/pop_macros.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif
