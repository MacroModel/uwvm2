/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @date        2026-03-16
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

// std
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// macro
#include <uwvm2/utils/macro/push_macros.h>
#include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>

#ifndef UWVM_MODULE
// import
# include <fast_io.h>
# include <uwvm2/runtime/compiler/uwvm_int/optable/impl.h>
# include <uwvm2/runtime/lib/uwvm_runtime.h>
# include <uwvm2/runtime/lib/llvm_jit_runtime.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/utils/mutex/impl.h>
# include <uwvm2/uwvm/io/impl.h>
# include <uwvm2/uwvm/runtime/storage/impl.h>
# include <uwvm2/uwvm/utils/ansies/impl.h>
#endif

#if defined(_MSC_VER) && defined(__clang__)
# define UWVM_LLVM_PUSH_WARNINGS _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wswitch-enum\"")
# define UWVM_LLVM_POP_WARNINGS _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
# define UWVM_LLVM_PUSH_WARNINGS __pragma(warning(push, 0))
# define UWVM_LLVM_POP_WARNINGS __pragma(warning(pop))
#else
# define UWVM_LLVM_PUSH_WARNINGS
# define UWVM_LLVM_POP_WARNINGS
#endif

UWVM_LLVM_PUSH_WARNINGS
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
UWVM_LLVM_POP_WARNINGS

namespace uwvm2::runtime::llvm_jit
{
    extern "C" void uwvm2_llvm_jit_fallback_invoke(::std::size_t module_id, ::std::size_t function_index, ::std::byte** stack_top_ptr) noexcept;
    extern "C" void
        uwvm2_llvm_jit_fallback_call_indirect_invoke(::std::size_t module_id,
                                                     ::std::size_t type_index,
                                                     ::std::size_t table_index,
                                                     ::std::byte** stack_top_ptr) noexcept;
    extern "C" void
        uwvm2_llvm_jit_fallback_memory_grow_invoke(::std::size_t module_id, ::std::size_t memory_index, ::std::byte** stack_top_ptr) noexcept;
    extern "C" void uwvm2_llvm_jit_fallback_memory_i32_load_invoke(::std::size_t module_id,
                                                                    ::std::size_t memory_index,
                                                                    ::std::size_t static_offset,
                                                                    ::std::byte** stack_top_ptr) noexcept;
    extern "C" void uwvm2_llvm_jit_fallback_memory_i32_store_invoke(::std::size_t module_id,
                                                                     ::std::size_t memory_index,
                                                                     ::std::size_t static_offset,
                                                                     ::std::byte** stack_top_ptr) noexcept;

    namespace
    {
        using runtime_module_storage_t = ::uwvm2::uwvm::runtime::storage::wasm_module_storage_t;
        using wasm_code_t = ::uwvm2::uwvm::runtime::storage::wasm_binfmt1_final_wasm_code_t;
        using function_type_t = ::uwvm2::uwvm::runtime::storage::wasm_binfmt1_final_function_type_t;
        using compiled_defined_call_info_t = ::uwvm2::runtime::compiler::uwvm_int::optable::compiled_defined_call_info;
        using trivial_defined_call_kind = ::uwvm2::runtime::compiler::uwvm_int::optable::trivial_defined_call_kind;
        using wasm1_code = ::uwvm2::runtime::compiler::uwvm_int::optable::wasm1_code;
        using wasm_value_type = ::uwvm2::parser::wasm::standard::wasm1::type::value_type;
        using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
        using native_entry_t = void (*)(::std::byte**);

        inline constexpr ::std::size_t invalid_module_id{::std::numeric_limits<::std::size_t>::max()};
        inline constexpr char fallback_symbol_name[]{"uwvm2_llvm_jit_fallback_invoke"};
        inline constexpr char fallback_call_indirect_symbol_name[]{"uwvm2_llvm_jit_fallback_call_indirect_invoke"};
        inline constexpr char fallback_memory_grow_symbol_name[]{"uwvm2_llvm_jit_fallback_memory_grow_invoke"};
        inline constexpr char fallback_memory_i32_load_symbol_name[]{"uwvm2_llvm_jit_fallback_memory_i32_load_invoke"};
        inline constexpr char fallback_memory_i32_store_symbol_name[]{"uwvm2_llvm_jit_fallback_memory_i32_store_invoke"};

        struct native_mutex_guard
        {
            ::uwvm2::utils::mutex::mutex_t* mutex{};

            explicit native_mutex_guard(::uwvm2::utils::mutex::mutex_t& m) noexcept : mutex{::std::addressof(m)} { mutex->lock(); }
            native_mutex_guard(native_mutex_guard const&) = delete;
            native_mutex_guard& operator= (native_mutex_guard const&) = delete;
            ~native_mutex_guard()
            {
                if(mutex != nullptr) [[likely]] { mutex->unlock(); }
            }
        };

        struct jit_module_record
        {
            ::uwvm2::utils::container::u8string_view module_name{};
            runtime_module_storage_t const* runtime_module{};
            ::std::size_t module_id{invalid_module_id};
            ::std::size_t imported_function_count{};
            ::std::size_t local_function_count{};
            bool compiled{};
            ::uwvm2::utils::mutex::mutex_t compile_mutex{};
            ::std::unique_ptr<llvm::LLVMContext> llvm_context{};
            ::std::unique_ptr<llvm::ExecutionEngine> execution_engine{};
            ::std::vector<native_entry_t> local_entries{};
        };

        struct runtime_state
        {
            ::std::atomic_bool prepared{};
            ::uwvm2::utils::mutex::mutex_t prepare_mutex{};
            bool jit_bridges_installed{};
            ::std::vector<::std::unique_ptr<jit_module_record>> modules{};
        };

        // Keep the JIT runtime state alive until process teardown without running destructors.
        // MCJIT and some LLVM globals have fragile cross-DSO destruction ordering on macOS.
        inline runtime_state& g_runtime{
            []() noexcept -> runtime_state&
            {
                static runtime_state* state{new runtime_state{}};
                return *state;
            }()};  // [global]

        template <typename... Args>
        [[noreturn]] inline void fatal_runtime_error(Args&&... args) noexcept
        {
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_RED),
                                u8"[fatal] ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                ::std::forward<Args>(args)...,
                                u8"\n\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
            ::fast_io::fast_terminate();
        }

        inline void initialize_llvm_once() noexcept
        {
            static bool initialized{
                []
                {
                    llvm::InitializeNativeTarget();
                    llvm::InitializeNativeTargetAsmPrinter();
                    llvm::InitializeNativeTargetAsmParser();
                    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
                    llvm::sys::DynamicLibrary::AddSymbol(fallback_symbol_name, reinterpret_cast<void*>(&uwvm2_llvm_jit_fallback_invoke));
                    llvm::sys::DynamicLibrary::AddSymbol(fallback_call_indirect_symbol_name,
                                                         reinterpret_cast<void*>(&uwvm2_llvm_jit_fallback_call_indirect_invoke));
                    llvm::sys::DynamicLibrary::AddSymbol(fallback_memory_grow_symbol_name,
                                                         reinterpret_cast<void*>(&uwvm2_llvm_jit_fallback_memory_grow_invoke));
                    llvm::sys::DynamicLibrary::AddSymbol(fallback_memory_i32_load_symbol_name,
                                                         reinterpret_cast<void*>(&uwvm2_llvm_jit_fallback_memory_i32_load_invoke));
                    llvm::sys::DynamicLibrary::AddSymbol(fallback_memory_i32_store_symbol_name,
                                                         reinterpret_cast<void*>(&uwvm2_llvm_jit_fallback_memory_i32_store_invoke));
                    return true;
                }()};
            static_cast<void>(initialized);
        }

        [[nodiscard]] inline jit_module_record* module_record_from_id(::std::size_t module_id) noexcept
        {
            if(module_id >= g_runtime.modules.size()) [[unlikely]] { return nullptr; }
            return g_runtime.modules[module_id].get();
        }

        enum class simple_call_inline_kind : ::std::uint_least8_t
        {
            none,
            passthrough,
            append_const_i32_passthrough,
            add_const_i32,
            xor_const_i32,
            mul_const_i32
        };

        enum class simple_i32_post_kind : ::std::uint_least8_t
        {
            none,
            add_const_i32,
            xor_const_i32,
            mul_const_i32
        };

        enum class simple_call_indirect_selector_kind : ::std::uint_least8_t
        {
            none,
            last_param,
            const_i32
        };

        struct simple_call_inline_match
        {
            simple_call_inline_kind kind{};
            ::std::size_t function_index{};
            wasm_i32 imm{};
        };

        struct simple_call_indirect_inline_match
        {
            simple_call_indirect_selector_kind selector_kind{};
            simple_i32_post_kind post_kind{};
            ::std::size_t type_index{};
            ::std::size_t table_index{};
            wasm_i32 selector_imm{};
            wasm_i32 post_imm{};
        };

        struct simple_memory_roundtrip_match
        {
            bool matched{};
            bool has_grow{};
            ::std::size_t memory_index{};
            wasm_i32 grow_delta{};
            wasm_i32 address_imm{};
            wasm_i32 value_imm{};
            ::std::size_t store_offset{};
            ::std::size_t load_offset{};
            simple_i32_post_kind post_kind{};
            wasm_i32 post_imm{};
        };

        [[nodiscard]] inline constexpr ::std::size_t wasm_value_abi_bytes(wasm_value_type t) noexcept
        {
            switch(t)
            {
                case wasm_value_type::i32:
                case wasm_value_type::f32: return 4uz;
                case wasm_value_type::i64:
                case wasm_value_type::f64: return 8uz;
                [[unlikely]] default:
                    return 0uz;
            }
        }

        [[nodiscard]] inline constexpr ::std::size_t function_param_count(function_type_t const& ft) noexcept
        { return static_cast<::std::size_t>(ft.parameter.end - ft.parameter.begin); }

        [[nodiscard]] inline constexpr ::std::size_t function_result_count(function_type_t const& ft) noexcept
        { return static_cast<::std::size_t>(ft.result.end - ft.result.begin); }

        [[nodiscard]] inline constexpr ::std::size_t function_param_bytes(function_type_t const& ft) noexcept
        {
            ::std::size_t total{};
            for(auto it{ft.parameter.begin}; it != ft.parameter.end; ++it)
            {
                auto const add{wasm_value_abi_bytes(*it)};
                if(add == 0uz) [[unlikely]] { return 0uz; }
                total += add;
            }
            return total;
        }

        [[nodiscard]] inline constexpr ::std::size_t function_result_bytes(function_type_t const& ft) noexcept
        {
            ::std::size_t total{};
            for(auto it{ft.result.begin}; it != ft.result.end; ++it)
            {
                auto const add{wasm_value_abi_bytes(*it)};
                if(add == 0uz) [[unlikely]] { return 0uz; }
                total += add;
            }
            return total;
        }

        [[nodiscard]] inline constexpr bool function_params_equal(function_type_t const& lhs, function_type_t const& rhs) noexcept
        {
            auto const lhs_n{function_param_count(lhs)};
            if(lhs_n != function_param_count(rhs)) { return false; }
            for(::std::size_t i{}; i != lhs_n; ++i)
            {
                if(lhs.parameter.begin[i] != rhs.parameter.begin[i]) { return false; }
            }
            return true;
        }

        [[nodiscard]] inline constexpr bool
            function_params_equal_prefix(function_type_t const& lhs, function_type_t const& rhs, ::std::size_t count) noexcept
        {
            if(function_param_count(lhs) < count || function_param_count(rhs) < count) { return false; }
            for(::std::size_t i{}; i != count; ++i)
            {
                if(lhs.parameter.begin[i] != rhs.parameter.begin[i]) { return false; }
            }
            return true;
        }

        [[nodiscard]] inline constexpr bool function_results_equal(function_type_t const& lhs, function_type_t const& rhs) noexcept
        {
            auto const lhs_n{function_result_count(lhs)};
            if(lhs_n != function_result_count(rhs)) { return false; }
            for(::std::size_t i{}; i != lhs_n; ++i)
            {
                if(lhs.result.begin[i] != rhs.result.begin[i]) { return false; }
            }
            return true;
        }

        [[nodiscard]] inline constexpr bool function_is_single_i32_result(function_type_t const& ft) noexcept
        { return function_result_count(ft) == 1uz && ft.result.begin[0] == wasm_value_type::i32; }

        [[nodiscard]] inline function_type_t const* resolve_function_type(runtime_module_storage_t const& module, ::std::size_t function_index) noexcept
        {
            auto const import_n{module.imported_function_vec_storage.size()};
            auto const local_n{module.local_defined_function_vec_storage.size()};
            if(function_index >= import_n + local_n) [[unlikely]] { return nullptr; }

            if(function_index < import_n)
            {
                auto const* const import_type_ptr{module.imported_function_vec_storage.index_unchecked(function_index).import_type_ptr};
                if(import_type_ptr == nullptr) [[unlikely]] { return nullptr; }
                return import_type_ptr->imports.storage.function;
            }

            return module.local_defined_function_vec_storage.index_unchecked(function_index - import_n).function_type_ptr;
        }

        [[nodiscard]] inline function_type_t const* resolve_type_function(runtime_module_storage_t const& module, ::std::size_t type_index) noexcept
        {
            auto const begin{module.type_section_storage.type_section_begin};
            auto const end{module.type_section_storage.type_section_end};
            if(begin == nullptr || end == nullptr) [[unlikely]] { return nullptr; }

            auto const total{static_cast<::std::size_t>(end - begin)};
            if(type_index >= total) [[unlikely]] { return nullptr; }

            return begin + type_index;
        }

        [[nodiscard]] inline bool supports_native_memory_bridge(runtime_module_storage_t const& module, ::std::size_t memory_index) noexcept
        {
            using imported_memory_storage_t = ::uwvm2::uwvm::runtime::storage::imported_memory_storage_t;
            using memory_link_kind = imported_memory_storage_t::imported_memory_link_kind;

            auto const imported_count{module.imported_memory_vec_storage.size()};
            if(memory_index < imported_count)
            {
                auto const* curr{::std::addressof(module.imported_memory_vec_storage.index_unchecked(memory_index))};
                for(;;)
                {
                    if(curr == nullptr) [[unlikely]] { return false; }
                    switch(curr->link_kind)
                    {
                        case memory_link_kind::imported:
                        {
                            curr = curr->target.imported_ptr;
                            continue;
                        }
                        case memory_link_kind::defined:
                        {
                            return curr->target.defined_ptr != nullptr;
                        }
                        case memory_link_kind::local_imported: [[fallthrough]];
                        case memory_link_kind::unresolved: [[fallthrough]];
                        default:
                        {
                            return false;
                        }
                    }
                }
            }

            auto const local_index{memory_index - imported_count};
            return local_index < module.local_defined_memory_vec_storage.size() &&
                   module.local_defined_memory_vec_storage.index_unchecked(local_index).memory_type_ptr != nullptr;
        }

        [[nodiscard]] inline constexpr simple_i32_post_kind post_kind_from_simple_call_kind(simple_call_inline_kind kind) noexcept
        {
            switch(kind)
            {
                case simple_call_inline_kind::add_const_i32:
                    return simple_i32_post_kind::add_const_i32;
                case simple_call_inline_kind::xor_const_i32:
                    return simple_i32_post_kind::xor_const_i32;
                case simple_call_inline_kind::mul_const_i32:
                    return simple_i32_post_kind::mul_const_i32;
                [[unlikely]] default:
                    return simple_i32_post_kind::none;
            }
        }

        [[nodiscard]] inline simple_call_inline_match match_simple_call_inline_body(wasm_code_t const* code_ptr, ::std::size_t caller_param_count) noexcept
        {
            simple_call_inline_match res{};
            if(code_ptr == nullptr) { return res; }

            auto const* curr{reinterpret_cast<::std::byte const*>(code_ptr->body.expr_begin)};
            auto const* const end{reinterpret_cast<::std::byte const*>(code_ptr->body.code_end)};

            auto const read_op{[&](wasm1_code& out) noexcept -> bool
                               {
                                   if(curr == end) { return false; }
                                   ::std::memcpy(::std::addressof(out), curr, sizeof(out));
                                   ++curr;
                                   return true;
                               }};

            auto const read_u32_leb{[&](::std::uint32_t& out) noexcept -> bool
                                    {
                                        ::std::uint32_t v{};
                                        ::std::uint32_t shift{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            auto const byte{::std::to_integer<::std::uint8_t>(*curr)};
                                            ++curr;
                                            v |= (static_cast<::std::uint32_t>(byte & 0x7fu) << shift);
                                            if((byte & 0x80u) == 0u)
                                            {
                                                out = v;
                                                return true;
                                            }
                                            shift += 7u;
                                        }
                                        return false;
                                    }};

            auto const read_i32_leb{[&](wasm_i32& out) noexcept -> bool
                                    {
                                        ::std::int32_t v{};
                                        ::std::uint32_t shift{};
                                        ::std::uint8_t byte{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            byte = ::std::to_integer<::std::uint8_t>(*curr);
                                            ++curr;
                                            v |= (static_cast<::std::int32_t>(byte & 0x7fu) << shift);
                                            shift += 7u;
                                            if((byte & 0x80u) == 0u)
                                            {
                                                if(shift < 32u && (byte & 0x40u)) { v |= (-1) << shift; }
                                                out = static_cast<wasm_i32>(v);
                                                return true;
                                            }
                                        }
                                        return false;
                                    }};

            wasm1_code op_call{};
            for(::std::size_t i{}; i != caller_param_count; ++i)
            {
                wasm1_code op{};
                if(!read_op(op) || op != wasm1_code::local_get) { return res; }

                ::std::uint32_t idx{};
                if(!read_u32_leb(idx) || idx != i) { return res; }
            }

            if(!read_op(op_call)) { return res; }

            if(op_call == wasm1_code::i32_const)
            {
                wasm_i32 imm{};
                if(!read_i32_leb(imm)) { return res; }

                if(!read_op(op_call) || op_call != wasm1_code::call) { return res; }

                ::std::uint32_t function_index{};
                if(!read_u32_leb(function_index)) { return res; }

                wasm1_code op_end{};
                if(!read_op(op_end) || op_end != wasm1_code::end || curr != end) { return res; }

                res.kind = simple_call_inline_kind::append_const_i32_passthrough;
                res.function_index = function_index;
                res.imm = imm;
                return res;
            }

            if(op_call != wasm1_code::call) { return res; }

            ::std::uint32_t function_index{};
            if(!read_u32_leb(function_index)) { return res; }

            wasm1_code op_next{};
            if(!read_op(op_next)) { return res; }

            if(op_next == wasm1_code::end && curr == end)
            {
                res.kind = simple_call_inline_kind::passthrough;
                res.function_index = function_index;
                return res;
            }

            if(op_next != wasm1_code::i32_const) { return res; }

            wasm_i32 imm{};
            if(!read_i32_leb(imm)) { return res; }

            wasm1_code op_post{};
            wasm1_code op_end{};
            if(!read_op(op_post) || !read_op(op_end) || op_end != wasm1_code::end || curr != end) { return res; }

            switch(op_post)
            {
                case wasm1_code::i32_add:
                    res.kind = simple_call_inline_kind::add_const_i32;
                    break;
                case wasm1_code::i32_xor:
                    res.kind = simple_call_inline_kind::xor_const_i32;
                    break;
                case wasm1_code::i32_mul:
                    res.kind = simple_call_inline_kind::mul_const_i32;
                    break;
                [[unlikely]] default:
                    return {};
            }

            res.function_index = function_index;
            res.imm = imm;
            return res;
        }

        [[nodiscard]] inline simple_call_indirect_inline_match
            match_simple_call_indirect_body(wasm_code_t const* code_ptr, ::std::size_t caller_param_count) noexcept
        {
            simple_call_indirect_inline_match res{};
            if(code_ptr == nullptr) { return res; }

            auto const* curr{reinterpret_cast<::std::byte const*>(code_ptr->body.expr_begin)};
            auto const* const end{reinterpret_cast<::std::byte const*>(code_ptr->body.code_end)};

            auto const read_op{[&](wasm1_code& out) noexcept -> bool
                               {
                                   if(curr == end) { return false; }
                                   ::std::memcpy(::std::addressof(out), curr, sizeof(out));
                                   ++curr;
                                   return true;
                               }};

            auto const read_u32_leb{[&](::std::uint32_t& out) noexcept -> bool
                                    {
                                        ::std::uint32_t v{};
                                        ::std::uint32_t shift{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            auto const byte{::std::to_integer<::std::uint8_t>(*curr)};
                                            ++curr;
                                            v |= (static_cast<::std::uint32_t>(byte & 0x7fu) << shift);
                                            if((byte & 0x80u) == 0u)
                                            {
                                                out = v;
                                                return true;
                                            }
                                            shift += 7u;
                                        }
                                        return false;
                                    }};

            auto const read_i32_leb{[&](wasm_i32& out) noexcept -> bool
                                    {
                                        ::std::int32_t v{};
                                        ::std::uint32_t shift{};
                                        ::std::uint8_t byte{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            byte = ::std::to_integer<::std::uint8_t>(*curr);
                                            ++curr;
                                            v |= (static_cast<::std::int32_t>(byte & 0x7fu) << shift);
                                            shift += 7u;
                                            if((byte & 0x80u) == 0u)
                                            {
                                                if(shift < 32u && (byte & 0x40u)) { v |= (-1) << shift; }
                                                out = static_cast<wasm_i32>(v);
                                                return true;
                                            }
                                        }
                                        return false;
                                    }};

            ::std::size_t loaded_local_count{};
            for(;;)
            {
                auto const* const saved_curr{curr};
                wasm1_code op{};
                if(!read_op(op)) { return {}; }
                if(op != wasm1_code::local_get)
                {
                    curr = saved_curr;
                    break;
                }

                ::std::uint32_t idx{};
                if(!read_u32_leb(idx) || idx != loaded_local_count) { return {}; }
                ++loaded_local_count;
            }

            wasm1_code op{};
            if(!read_op(op)) { return {}; }

            if(op == wasm1_code::call_indirect)
            {
                if(loaded_local_count != caller_param_count || loaded_local_count == 0uz) { return {}; }
                res.selector_kind = simple_call_indirect_selector_kind::last_param;
            }
            else if(op == wasm1_code::i32_const)
            {
                if(loaded_local_count != caller_param_count) { return {}; }
                if(!read_i32_leb(res.selector_imm)) { return {}; }
                if(!read_op(op) || op != wasm1_code::call_indirect) { return {}; }
                res.selector_kind = simple_call_indirect_selector_kind::const_i32;
            }
            else
            {
                return {};
            }

            ::std::uint32_t type_index{};
            ::std::uint32_t table_index{};
            if(!read_u32_leb(type_index) || !read_u32_leb(table_index)) { return {}; }
            res.type_index = type_index;
            res.table_index = table_index;

            wasm1_code op_next{};
            if(!read_op(op_next)) { return {}; }

            if(op_next == wasm1_code::end && curr == end) { return res; }
            if(op_next != wasm1_code::i32_const) { return {}; }
            if(!read_i32_leb(res.post_imm)) { return {}; }

            wasm1_code op_post{};
            wasm1_code op_end{};
            if(!read_op(op_post) || !read_op(op_end) || op_end != wasm1_code::end || curr != end) { return {}; }

            switch(op_post)
            {
                case wasm1_code::i32_add:
                    res.post_kind = simple_i32_post_kind::add_const_i32;
                    break;
                case wasm1_code::i32_xor:
                    res.post_kind = simple_i32_post_kind::xor_const_i32;
                    break;
                case wasm1_code::i32_mul:
                    res.post_kind = simple_i32_post_kind::mul_const_i32;
                    break;
                [[unlikely]] default:
                    return {};
            }

            return res;
        }

        [[nodiscard]] inline simple_memory_roundtrip_match match_simple_memory_roundtrip_body(wasm_code_t const* code_ptr) noexcept
        {
            simple_memory_roundtrip_match res{};
            if(code_ptr == nullptr) { return res; }

            auto const* curr{reinterpret_cast<::std::byte const*>(code_ptr->body.expr_begin)};
            auto const* const end{reinterpret_cast<::std::byte const*>(code_ptr->body.code_end)};

            auto const read_op{[&](wasm1_code& out) noexcept -> bool
                               {
                                   if(curr == end) { return false; }
                                   ::std::memcpy(::std::addressof(out), curr, sizeof(out));
                                   ++curr;
                                   return true;
                               }};

            auto const read_u32_leb{[&](::std::uint32_t& out) noexcept -> bool
                                    {
                                        ::std::uint32_t v{};
                                        ::std::uint32_t shift{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            auto const byte{::std::to_integer<::std::uint8_t>(*curr)};
                                            ++curr;
                                            v |= (static_cast<::std::uint32_t>(byte & 0x7fu) << shift);
                                            if((byte & 0x80u) == 0u)
                                            {
                                                out = v;
                                                return true;
                                            }
                                            shift += 7u;
                                        }
                                        return false;
                                    }};

            auto const read_i32_leb{[&](wasm_i32& out) noexcept -> bool
                                    {
                                        ::std::int32_t v{};
                                        ::std::uint32_t shift{};
                                        ::std::uint8_t byte{};
                                        for(::std::size_t i{}; i != 5uz; ++i)
                                        {
                                            if(curr == end) { return false; }
                                            byte = ::std::to_integer<::std::uint8_t>(*curr);
                                            ++curr;
                                            v |= (static_cast<::std::int32_t>(byte & 0x7fu) << shift);
                                            shift += 7u;
                                            if((byte & 0x80u) == 0u)
                                            {
                                                if(shift < 32u && (byte & 0x40u)) { v |= (-1) << shift; }
                                                out = static_cast<wasm_i32>(v);
                                                return true;
                                            }
                                        }
                                        return false;
                                    }};

            auto const* const saved_before_prefix{curr};
            wasm1_code op{};
            if(!read_op(op)) { return {}; }
            if(op == wasm1_code::i32_const)
            {
                wasm_i32 grow_delta{};
                if(!read_i32_leb(grow_delta)) { return {}; }

                wasm1_code op_after_const{};
                if(!read_op(op_after_const)) { return {}; }
                if(op_after_const == wasm1_code::memory_grow)
                {
                    ::std::uint32_t memory_index{};
                    if(!read_u32_leb(memory_index) || memory_index != 0u) { return {}; }

                    wasm1_code op_drop{};
                    if(!read_op(op_drop) || op_drop != wasm1_code::drop) { return {}; }

                    res.has_grow = true;
                    res.memory_index = memory_index;
                    res.grow_delta = grow_delta;
                }
                else
                {
                    curr = saved_before_prefix;
                }
            }
            else
            {
                curr = saved_before_prefix;
            }

            wasm1_code op_addr{};
            wasm1_code op_value{};
            wasm1_code op_store{};
            wasm1_code op_load_addr{};
            wasm1_code op_load{};
            if(!read_op(op_addr) || op_addr != wasm1_code::i32_const) { return {}; }
            if(!read_i32_leb(res.address_imm)) { return {}; }
            if(!read_op(op_value) || op_value != wasm1_code::i32_const) { return {}; }
            if(!read_i32_leb(res.value_imm)) { return {}; }
            if(!read_op(op_store) || op_store != wasm1_code::i32_store) { return {}; }

            ::std::uint32_t store_align{};
            ::std::uint32_t store_offset{};
            if(!read_u32_leb(store_align) || !read_u32_leb(store_offset)) { return {}; }
            static_cast<void>(store_align);
            res.store_offset = store_offset;

            wasm_i32 load_addr_imm{};
            if(!read_op(op_load_addr) || op_load_addr != wasm1_code::i32_const) { return {}; }
            if(!read_i32_leb(load_addr_imm) || load_addr_imm != res.address_imm) { return {}; }
            if(!read_op(op_load) || op_load != wasm1_code::i32_load) { return {}; }

            ::std::uint32_t load_align{};
            ::std::uint32_t load_offset{};
            if(!read_u32_leb(load_align) || !read_u32_leb(load_offset)) { return {}; }
            static_cast<void>(load_align);
            res.load_offset = load_offset;

            wasm1_code op_next{};
            if(!read_op(op_next)) { return {}; }
            if(op_next == wasm1_code::end && curr == end)
            {
                res.matched = true;
                return res;
            }

            if(op_next != wasm1_code::i32_const) { return {}; }
            if(!read_i32_leb(res.post_imm)) { return {}; }

            wasm1_code op_post{};
            wasm1_code op_end{};
            if(!read_op(op_post) || !read_op(op_end) || op_end != wasm1_code::end || curr != end) { return {}; }

            switch(op_post)
            {
                case wasm1_code::i32_add:
                    res.post_kind = simple_i32_post_kind::add_const_i32;
                    break;
                case wasm1_code::i32_xor:
                    res.post_kind = simple_i32_post_kind::xor_const_i32;
                    break;
                case wasm1_code::i32_mul:
                    res.post_kind = simple_i32_post_kind::mul_const_i32;
                    break;
                [[unlikely]] default:
                    return {};
            }

            res.matched = true;
            return res;
        }

        [[nodiscard]] inline llvm::Value* make_i8_ptr_offset(llvm::IRBuilder<>& ir_builder, llvm::Value* base_ptr, ::std::int_least64_t offset) noexcept
        {
            return ir_builder.CreateInBoundsGEP(ir_builder.getInt8Ty(), base_ptr, ir_builder.getInt64(offset));
        }

        [[nodiscard]] inline llvm::Value* load_i32_unaligned(llvm::IRBuilder<>& ir_builder, llvm::Value* base_ptr, ::std::size_t offset) noexcept
        {
            auto* const ptr{make_i8_ptr_offset(ir_builder, base_ptr, static_cast<::std::int_least64_t>(offset))};
            auto* const load{ir_builder.CreateLoad(ir_builder.getInt32Ty(), ptr)};
            load->setAlignment(llvm::Align(1u));
            return load;
        }

        inline void store_i32_unaligned(llvm::IRBuilder<>& ir_builder, llvm::Value* base_ptr, ::std::size_t offset, llvm::Value* value) noexcept
        {
            auto* const ptr{make_i8_ptr_offset(ir_builder, base_ptr, static_cast<::std::int_least64_t>(offset))};
            auto* const store{ir_builder.CreateStore(value, ptr)};
            store->setAlignment(llvm::Align(1u));
        }

        inline void emit_fallback_call(llvm::Module& module,
                                       llvm::IRBuilder<>& ir_builder,
                                       llvm::Value* stack_top_ptr_ptr,
                                       ::std::size_t module_id,
                                       ::std::size_t function_index) noexcept
        {
            auto* const size_ty{llvm::IntegerType::get(module.getContext(), static_cast<unsigned>(sizeof(::std::size_t) * 8u))};
            auto* const opaque_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto callee{
                module.getOrInsertFunction(fallback_symbol_name,
                                           llvm::FunctionType::get(ir_builder.getVoidTy(), {size_ty, size_ty, opaque_ptr_ty}, false))};
            ir_builder.CreateCall(callee,
                                  {llvm::ConstantInt::get(size_ty, module_id), llvm::ConstantInt::get(size_ty, function_index), stack_top_ptr_ptr});
        }

        inline void emit_fallback_call_indirect(llvm::Module& module,
                                                llvm::IRBuilder<>& ir_builder,
                                                llvm::Value* stack_top_ptr_ptr,
                                                ::std::size_t module_id,
                                                ::std::size_t type_index,
                                                ::std::size_t table_index) noexcept
        {
            auto* const size_ty{llvm::IntegerType::get(module.getContext(), static_cast<unsigned>(sizeof(::std::size_t) * 8u))};
            auto* const opaque_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto callee{
                module.getOrInsertFunction(fallback_call_indirect_symbol_name,
                                           llvm::FunctionType::get(ir_builder.getVoidTy(), {size_ty, size_ty, size_ty, opaque_ptr_ty}, false))};
            ir_builder.CreateCall(callee,
                                  {llvm::ConstantInt::get(size_ty, module_id),
                                   llvm::ConstantInt::get(size_ty, type_index),
                                   llvm::ConstantInt::get(size_ty, table_index),
                                   stack_top_ptr_ptr});
        }

        inline void emit_fallback_memory_grow(llvm::Module& module,
                                              llvm::IRBuilder<>& ir_builder,
                                              llvm::Value* stack_top_ptr_ptr,
                                              ::std::size_t module_id,
                                              ::std::size_t memory_index) noexcept
        {
            auto* const size_ty{llvm::IntegerType::get(module.getContext(), static_cast<unsigned>(sizeof(::std::size_t) * 8u))};
            auto* const opaque_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto callee{
                module.getOrInsertFunction(fallback_memory_grow_symbol_name,
                                           llvm::FunctionType::get(ir_builder.getVoidTy(), {size_ty, size_ty, opaque_ptr_ty}, false))};
            ir_builder.CreateCall(callee,
                                  {llvm::ConstantInt::get(size_ty, module_id), llvm::ConstantInt::get(size_ty, memory_index), stack_top_ptr_ptr});
        }

        inline void emit_fallback_memory_i32_load(llvm::Module& module,
                                                  llvm::IRBuilder<>& ir_builder,
                                                  llvm::Value* stack_top_ptr_ptr,
                                                  ::std::size_t module_id,
                                                  ::std::size_t memory_index,
                                                  ::std::size_t static_offset) noexcept
        {
            auto* const size_ty{llvm::IntegerType::get(module.getContext(), static_cast<unsigned>(sizeof(::std::size_t) * 8u))};
            auto* const opaque_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto callee{
                module.getOrInsertFunction(fallback_memory_i32_load_symbol_name,
                                           llvm::FunctionType::get(ir_builder.getVoidTy(), {size_ty, size_ty, size_ty, opaque_ptr_ty}, false))};
            ir_builder.CreateCall(callee,
                                  {llvm::ConstantInt::get(size_ty, module_id),
                                   llvm::ConstantInt::get(size_ty, memory_index),
                                   llvm::ConstantInt::get(size_ty, static_offset),
                                   stack_top_ptr_ptr});
        }

        inline void emit_fallback_memory_i32_store(llvm::Module& module,
                                                   llvm::IRBuilder<>& ir_builder,
                                                   llvm::Value* stack_top_ptr_ptr,
                                                   ::std::size_t module_id,
                                                   ::std::size_t memory_index,
                                                   ::std::size_t static_offset) noexcept
        {
            auto* const size_ty{llvm::IntegerType::get(module.getContext(), static_cast<unsigned>(sizeof(::std::size_t) * 8u))};
            auto* const opaque_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto callee{
                module.getOrInsertFunction(fallback_memory_i32_store_symbol_name,
                                           llvm::FunctionType::get(ir_builder.getVoidTy(), {size_ty, size_ty, size_ty, opaque_ptr_ty}, false))};
            ir_builder.CreateCall(callee,
                                  {llvm::ConstantInt::get(size_ty, module_id),
                                   llvm::ConstantInt::get(size_ty, memory_index),
                                   llvm::ConstantInt::get(size_ty, static_offset),
                                   stack_top_ptr_ptr});
        }

        inline void push_i32_constant(llvm::Module& module, llvm::IRBuilder<>& ir_builder, llvm::Value* stack_top_ptr_ptr, wasm_i32 imm) noexcept
        {
            auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto* const i32_ty{ir_builder.getInt32Ty()};
            auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
            auto* const new_top{make_i8_ptr_offset(ir_builder, stack_top, 4)};
            store_i32_unaligned(ir_builder, stack_top, 0uz, llvm::ConstantInt::get(i32_ty, ::std::bit_cast<::std::uint_least32_t>(imm)));
            ir_builder.CreateStore(new_top, stack_top_ptr_ptr);
        }

        inline void drop_top_i32(llvm::Module& module, llvm::IRBuilder<>& ir_builder, llvm::Value* stack_top_ptr_ptr) noexcept
        {
            auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
            ir_builder.CreateStore(make_i8_ptr_offset(ir_builder, stack_top, -4), stack_top_ptr_ptr);
        }

        inline void emit_fallback_body(llvm::Module& module,
                                       llvm::IRBuilder<>& ir_builder,
                                       llvm::Value* stack_top_ptr_ptr,
                                       ::std::size_t module_id,
                                       ::std::size_t function_index) noexcept
        {
            emit_fallback_call(module, ir_builder, stack_top_ptr_ptr, module_id, function_index);
            ir_builder.CreateRetVoid();
        }

        [[nodiscard]] inline bool emit_simple_i32_postprocess(llvm::Module& module,
                                                              llvm::IRBuilder<>& ir_builder,
                                                              llvm::Value* stack_top_ptr_ptr,
                                                              simple_i32_post_kind post_kind,
                                                              wasm_i32 imm) noexcept
        {
            if(post_kind == simple_i32_post_kind::none) { return true; }

            auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto* const i32_ty{ir_builder.getInt32Ty()};
            auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
            auto* const result_ptr{make_i8_ptr_offset(ir_builder, stack_top, -4)};
            auto* const old_value{load_i32_unaligned(ir_builder, result_ptr, 0uz)};
            auto* const imm_const{llvm::ConstantInt::get(i32_ty, ::std::bit_cast<::std::uint_least32_t>(imm))};

            llvm::Value* new_value{};
            switch(post_kind)
            {
                case simple_i32_post_kind::add_const_i32:
                    new_value = ir_builder.CreateAdd(old_value, imm_const);
                    break;
                case simple_i32_post_kind::xor_const_i32:
                    new_value = ir_builder.CreateXor(old_value, imm_const);
                    break;
                case simple_i32_post_kind::mul_const_i32:
                    new_value = ir_builder.CreateMul(old_value, imm_const);
                    break;
                [[unlikely]] default:
                    return false;
            }

            store_i32_unaligned(ir_builder, result_ptr, 0uz, new_value);
            return true;
        }

        [[nodiscard]] inline bool emit_trivial_body(llvm::Module& module,
                                                    llvm::IRBuilder<>& ir_builder,
                                                    llvm::Value* stack_top_ptr_ptr,
                                                    compiled_defined_call_info_t const& info) noexcept
        {
            if(info.trivial_kind == trivial_defined_call_kind::none) { return false; }

            if(info.param_bytes > static_cast<::std::size_t>(::std::numeric_limits<::std::int_least64_t>::max())) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT param_bytes exceeds the supported signed GEP range.");
            }

            auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
            auto* const i32_ty{ir_builder.getInt32Ty()};
            auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
            auto* const args_begin{make_i8_ptr_offset(ir_builder, stack_top, -static_cast<::std::int_least64_t>(info.param_bytes))};

            auto const imm_u32{::std::bit_cast<::std::uint_least32_t>(info.trivial_imm)};
            auto const imm2_u32{::std::bit_cast<::std::uint_least32_t>(info.trivial_imm2)};
            auto* const imm_const{llvm::ConstantInt::get(i32_ty, imm_u32)};
            auto* const imm2_const{llvm::ConstantInt::get(i32_ty, imm2_u32)};

            auto const store_new_stack_top{[&](llvm::Value* new_top) noexcept
                                           {
                                               ir_builder.CreateStore(new_top, stack_top_ptr_ptr);
                                               ir_builder.CreateRetVoid();
                                           }};

            switch(info.trivial_kind)
            {
                case trivial_defined_call_kind::nop_void:
                {
                    if(info.result_bytes != 0uz) [[unlikely]] { return false; }
                    store_new_stack_top(args_begin);
                    return true;
                }
                case trivial_defined_call_kind::const_i32:
                {
                    if(info.result_bytes != 4uz) [[unlikely]] { return false; }
                    store_i32_unaligned(ir_builder, args_begin, 0uz, imm_const);
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::param0_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes < 4uz) [[unlikely]] { return false; }
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::add_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateAdd(a, imm_const));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::xor_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateXor(a, imm_const));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::mul_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateMul(a, imm_const));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::rotr_add_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    auto* const shift_const{llvm::ConstantInt::get(i32_ty, imm2_u32 & 31u)};
                    auto* const rot_decl{llvm::Intrinsic::getOrInsertDeclaration(::std::addressof(module), llvm::Intrinsic::fshr, {i32_ty})};
                    auto* const rotated{ir_builder.CreateCall(rot_decl, {a, a, shift_const})};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateAdd(rotated, imm_const));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::xorshift32_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* x{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    x = ir_builder.CreateXor(x, ir_builder.CreateShl(x, llvm::ConstantInt::get(i32_ty, 13u)));
                    x = ir_builder.CreateXor(x, ir_builder.CreateLShr(x, llvm::ConstantInt::get(i32_ty, 17u)));
                    x = ir_builder.CreateXor(x, ir_builder.CreateShl(x, llvm::ConstantInt::get(i32_ty, 5u)));
                    store_i32_unaligned(ir_builder, args_begin, 0uz, x);
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::mul_add_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 4uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    auto* const value{ir_builder.CreateAdd(ir_builder.CreateMul(a, imm_const), imm2_const)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, value);
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::xor_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 8uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    auto* const b{load_i32_unaligned(ir_builder, args_begin, 4uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateXor(a, b));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::xor_add_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 8uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    auto* const b{load_i32_unaligned(ir_builder, args_begin, 4uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateAdd(a, ir_builder.CreateXor(b, imm_const)));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::sub_or_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 8uz) [[unlikely]] { return false; }
                    auto* const a{load_i32_unaligned(ir_builder, args_begin, 0uz)};
                    auto* const b{load_i32_unaligned(ir_builder, args_begin, 4uz)};
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateSub(a, ir_builder.CreateOr(b, imm_const)));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                case trivial_defined_call_kind::sum8_xor_const_i32:
                {
                    if(info.result_bytes != 4uz || info.param_bytes != 32uz) [[unlikely]] { return false; }
                    llvm::Value* acc{llvm::ConstantInt::get(i32_ty, 0u)};
                    for(::std::size_t i{}; i != 8uz; ++i) { acc = ir_builder.CreateAdd(acc, load_i32_unaligned(ir_builder, args_begin, i * 4uz)); }
                    store_i32_unaligned(ir_builder, args_begin, 0uz, ir_builder.CreateXor(acc, imm_const));
                    store_new_stack_top(make_i8_ptr_offset(ir_builder, args_begin, 4));
                    return true;
                }
                [[unlikely]] default:
                {
                    return false;
                }
            }
        }

        [[nodiscard]] inline bool emit_simple_call_body(llvm::Module& module,
                                                        llvm::IRBuilder<>& ir_builder,
                                                        llvm::Value* stack_top_ptr_ptr,
                                                        jit_module_record const& record,
                                                        ::std::size_t local_index,
                                                        compiled_defined_call_info_t const& info) noexcept
        {
            auto const* const runtime_module{record.runtime_module};
            if(runtime_module == nullptr) [[unlikely]] { return false; }
            if(local_index >= runtime_module->local_defined_function_vec_storage.size()) [[unlikely]] { return false; }

            auto const& caller_storage{runtime_module->local_defined_function_vec_storage.index_unchecked(local_index)};
            auto const* const caller_ft{caller_storage.function_type_ptr};
            if(caller_ft == nullptr) [[unlikely]] { return false; }

            auto const match{match_simple_call_inline_body(caller_storage.wasm_code_ptr, function_param_count(*caller_ft))};
            if(match.kind == simple_call_inline_kind::none) { return false; }

            auto const* const callee_ft{resolve_function_type(*runtime_module, match.function_index)};
            if(callee_ft == nullptr) [[unlikely]] { return false; }

            if(function_param_bytes(*caller_ft) != info.param_bytes) { return false; }

            switch(match.kind)
            {
                case simple_call_inline_kind::passthrough:
                {
                    if(!function_params_equal(*caller_ft, *callee_ft)) { return false; }
                    if(!function_results_equal(*caller_ft, *callee_ft)) { return false; }
                    if(function_result_bytes(*caller_ft) != info.result_bytes) { return false; }

                    emit_fallback_call(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.function_index);
                    ir_builder.CreateRetVoid();
                    return true;
                }
                case simple_call_inline_kind::append_const_i32_passthrough:
                {
                    if(!function_params_equal_prefix(*caller_ft, *callee_ft, function_param_count(*caller_ft))) { return false; }
                    if(function_param_count(*callee_ft) != function_param_count(*caller_ft) + 1uz) { return false; }
                    if(callee_ft->parameter.end[-1] != wasm_value_type::i32) { return false; }
                    if(!function_results_equal(*caller_ft, *callee_ft)) { return false; }
                    if(function_result_bytes(*caller_ft) != info.result_bytes) { return false; }
                    if(function_param_bytes(*callee_ft) != info.param_bytes + 4uz) { return false; }

                    auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
                    auto* const i32_ty{ir_builder.getInt32Ty()};
                    auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
                    auto* const new_top{make_i8_ptr_offset(ir_builder, stack_top, 4)};
                    store_i32_unaligned(ir_builder, stack_top, 0uz, llvm::ConstantInt::get(i32_ty, ::std::bit_cast<::std::uint_least32_t>(match.imm)));
                    ir_builder.CreateStore(new_top, stack_top_ptr_ptr);

                    emit_fallback_call(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.function_index);
                    ir_builder.CreateRetVoid();
                    return true;
                }
                case simple_call_inline_kind::add_const_i32:
                case simple_call_inline_kind::xor_const_i32:
                case simple_call_inline_kind::mul_const_i32:
                {
                    if(!function_params_equal(*caller_ft, *callee_ft)) { return false; }
                    if(!function_is_single_i32_result(*caller_ft) || !function_is_single_i32_result(*callee_ft)) { return false; }
                    if(info.result_bytes != 4uz) { return false; }

                    emit_fallback_call(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.function_index);
                    if(!emit_simple_i32_postprocess(module, ir_builder, stack_top_ptr_ptr, post_kind_from_simple_call_kind(match.kind), match.imm))
                    {
                        return false;
                    }
                    ir_builder.CreateRetVoid();
                    return true;
                }
                [[unlikely]] default:
                {
                    return false;
                }
            }
        }

        [[nodiscard]] inline bool emit_simple_call_indirect_body(llvm::Module& module,
                                                                 llvm::IRBuilder<>& ir_builder,
                                                                 llvm::Value* stack_top_ptr_ptr,
                                                                 jit_module_record const& record,
                                                                 ::std::size_t local_index,
                                                                 compiled_defined_call_info_t const& info) noexcept
        {
            auto const* const runtime_module{record.runtime_module};
            if(runtime_module == nullptr) [[unlikely]] { return false; }
            if(local_index >= runtime_module->local_defined_function_vec_storage.size()) [[unlikely]] { return false; }

            auto const& caller_storage{runtime_module->local_defined_function_vec_storage.index_unchecked(local_index)};
            auto const* const caller_ft{caller_storage.function_type_ptr};
            if(caller_ft == nullptr) [[unlikely]] { return false; }

            auto const match{match_simple_call_indirect_body(caller_storage.wasm_code_ptr, function_param_count(*caller_ft))};
            if(match.selector_kind == simple_call_indirect_selector_kind::none) { return false; }

            auto const* const expected_ft{resolve_type_function(*runtime_module, match.type_index)};
            if(expected_ft == nullptr) [[unlikely]] { return false; }
            if(function_param_bytes(*caller_ft) != info.param_bytes) { return false; }

            switch(match.selector_kind)
            {
                case simple_call_indirect_selector_kind::last_param:
                {
                    auto const expected_param_count{function_param_count(*expected_ft)};
                    if(function_param_count(*caller_ft) != expected_param_count + 1uz) { return false; }
                    if(!function_params_equal_prefix(*caller_ft, *expected_ft, expected_param_count)) { return false; }
                    if(caller_ft->parameter.begin[expected_param_count] != wasm_value_type::i32) { return false; }
                    if(function_param_bytes(*expected_ft) + 4uz != info.param_bytes) { return false; }
                    break;
                }
                case simple_call_indirect_selector_kind::const_i32:
                {
                    if(!function_params_equal(*caller_ft, *expected_ft)) { return false; }
                    if(function_param_bytes(*expected_ft) != info.param_bytes) { return false; }
                    break;
                }
                [[unlikely]] default:
                {
                    return false;
                }
            }

            if(match.post_kind == simple_i32_post_kind::none)
            {
                if(!function_results_equal(*caller_ft, *expected_ft)) { return false; }
                if(function_result_bytes(*caller_ft) != info.result_bytes) { return false; }
            }
            else
            {
                if(!function_is_single_i32_result(*caller_ft) || !function_is_single_i32_result(*expected_ft)) { return false; }
                if(info.result_bytes != 4uz) { return false; }
            }

            if(match.selector_kind == simple_call_indirect_selector_kind::const_i32)
            {
                auto* const i8_ptr_ty{llvm::PointerType::getUnqual(module.getContext())};
                auto* const i32_ty{ir_builder.getInt32Ty()};
                auto* const stack_top{ir_builder.CreateLoad(i8_ptr_ty, stack_top_ptr_ptr)};
                auto* const new_top{make_i8_ptr_offset(ir_builder, stack_top, 4)};
                store_i32_unaligned(ir_builder,
                                    stack_top,
                                    0uz,
                                    llvm::ConstantInt::get(i32_ty, ::std::bit_cast<::std::uint_least32_t>(match.selector_imm)));
                ir_builder.CreateStore(new_top, stack_top_ptr_ptr);
            }

            emit_fallback_call_indirect(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.type_index, match.table_index);
            if(!emit_simple_i32_postprocess(module, ir_builder, stack_top_ptr_ptr, match.post_kind, match.post_imm)) { return false; }
            ir_builder.CreateRetVoid();
            return true;
        }

        [[nodiscard]] inline bool emit_simple_memory_roundtrip_body(llvm::Module& module,
                                                                    llvm::IRBuilder<>& ir_builder,
                                                                    llvm::Value* stack_top_ptr_ptr,
                                                                    jit_module_record const& record,
                                                                    ::std::size_t local_index,
                                                                    compiled_defined_call_info_t const& info) noexcept
        {
            auto const* const runtime_module{record.runtime_module};
            if(runtime_module == nullptr) [[unlikely]] { return false; }
            if(local_index >= runtime_module->local_defined_function_vec_storage.size()) [[unlikely]] { return false; }

            auto const& caller_storage{runtime_module->local_defined_function_vec_storage.index_unchecked(local_index)};
            auto const* const caller_ft{caller_storage.function_type_ptr};
            if(caller_ft == nullptr) [[unlikely]] { return false; }

            auto const match{match_simple_memory_roundtrip_body(caller_storage.wasm_code_ptr)};
            if(!match.matched) { return false; }

            if(function_param_count(*caller_ft) != 0uz || info.param_bytes != 0uz) { return false; }
            if(!function_is_single_i32_result(*caller_ft) || info.result_bytes != 4uz) { return false; }
            if(!supports_native_memory_bridge(*runtime_module, match.memory_index)) { return false; }

            if(match.has_grow)
            {
                push_i32_constant(module, ir_builder, stack_top_ptr_ptr, match.grow_delta);
                emit_fallback_memory_grow(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.memory_index);
                drop_top_i32(module, ir_builder, stack_top_ptr_ptr);
            }

            push_i32_constant(module, ir_builder, stack_top_ptr_ptr, match.address_imm);
            push_i32_constant(module, ir_builder, stack_top_ptr_ptr, match.value_imm);
            emit_fallback_memory_i32_store(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.memory_index, match.store_offset);

            push_i32_constant(module, ir_builder, stack_top_ptr_ptr, match.address_imm);
            emit_fallback_memory_i32_load(module, ir_builder, stack_top_ptr_ptr, record.module_id, match.memory_index, match.load_offset);

            if(!emit_simple_i32_postprocess(module, ir_builder, stack_top_ptr_ptr, match.post_kind, match.post_imm)) { return false; }
            ir_builder.CreateRetVoid();
            return true;
        }

        [[nodiscard]] inline std::unique_ptr<llvm::ExecutionEngine>
            create_execution_engine(::std::unique_ptr<llvm::Module> module, ::std::string const& module_name) noexcept
        {
            initialize_llvm_once();

            module->setTargetTriple(llvm::Triple{llvm::sys::getProcessTriple()});

            ::std::string error_message;
            llvm::EngineBuilder builder{::std::move(module)};
            builder.setErrorStr(::std::addressof(error_message));
            builder.setEngineKind(llvm::EngineKind::JIT);
            builder.setOptLevel(llvm::CodeGenOptLevel::Aggressive);
            builder.setMCJITMemoryManager(::std::make_unique<llvm::SectionMemoryManager>());

            auto engine{::std::unique_ptr<llvm::ExecutionEngine>(builder.create())};
            if(!engine) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT failed to create execution engine for module=\"",
                                    ::fast_io::mnp::code_cvt(::fast_io::mnp::os_c_str(module_name.c_str())),
                                    u8"\": ",
                                    ::fast_io::mnp::code_cvt(::fast_io::mnp::os_c_str(error_message.c_str())));
            }

            engine->finalizeObject();
            return engine;
        }

        inline void compile_module_if_needed(::std::size_t module_id) noexcept;

        inline void jit_call_bridge(::std::size_t wasm_module_id, ::std::size_t func_index, ::std::byte** stack_top_ptr) noexcept
        {
            auto* const record{module_record_from_id(wasm_module_id)};
            if(record == nullptr) [[unlikely]]
            {
                ::uwvm2::runtime::uwvm_int::invoke_function_bridge(wasm_module_id, func_index, stack_top_ptr);
                return;
            }

            if(func_index < record->imported_function_count) [[unlikely]]
            {
                ::uwvm2::runtime::uwvm_int::invoke_function_bridge(wasm_module_id, func_index, stack_top_ptr);
                return;
            }

            auto const local_index{func_index - record->imported_function_count};
            if(local_index >= record->local_function_count) [[unlikely]]
            {
                ::uwvm2::runtime::uwvm_int::invoke_function_bridge(wasm_module_id, func_index, stack_top_ptr);
                return;
            }

            compile_module_if_needed(wasm_module_id);

            if(local_index >= record->local_entries.size() || record->local_entries[local_index] == nullptr) [[unlikely]]
            {
                ::uwvm2::runtime::uwvm_int::invoke_function_bridge(wasm_module_id, func_index, stack_top_ptr);
                return;
            }

            record->local_entries[local_index](stack_top_ptr);
        }

        inline void jit_call_indirect_bridge(::std::size_t wasm_module_id,
                                             ::std::size_t type_index,
                                             ::std::size_t table_index,
                                             ::std::byte** stack_top_ptr) noexcept
        {
            ::uwvm2::runtime::uwvm_int::invoke_function_call_indirect_bridge(wasm_module_id, type_index, table_index, stack_top_ptr);
        }

        inline void prepare_runtime_if_needed() noexcept
        {
            if(g_runtime.prepared.load(::std::memory_order_acquire)) [[likely]] { return; }

            ::uwvm2::runtime::uwvm_int::ensure_runtime_ready();

            native_mutex_guard prepare_guard{g_runtime.prepare_mutex};
            if(g_runtime.prepared.load(::std::memory_order_relaxed)) [[likely]] { return; }

            auto const& rt_map{::uwvm2::uwvm::runtime::storage::wasm_module_runtime_storage};
            ::std::size_t max_module_id{};
            bool has_module{};

            for(auto const& [module_name, module_rt]: rt_map)
            {
                static_cast<void>(module_rt);
                auto const module_id{::uwvm2::runtime::uwvm_int::runtime_module_id_from_name(module_name)};
                if(module_id == invalid_module_id) [[unlikely]]
                {
                    fatal_runtime_error(u8"LLVM JIT cannot resolve uwvm_int runtime module id for module=\"", module_name, u8"\".");
                }
                if(!has_module || module_id > max_module_id)
                {
                    max_module_id = module_id;
                    has_module = true;
                }
            }

            g_runtime.modules.clear();
            if(has_module) { g_runtime.modules.resize(max_module_id + 1uz); }

            for(auto const& [module_name, module_rt]: rt_map)
            {
                auto const module_id{::uwvm2::runtime::uwvm_int::runtime_module_id_from_name(module_name)};
                auto rec{::std::make_unique<jit_module_record>()};
                rec->module_name = module_name;
                rec->runtime_module = ::std::addressof(module_rt);
                rec->module_id = module_id;
                rec->imported_function_count = module_rt.imported_function_vec_storage.size();
                rec->local_function_count = module_rt.local_defined_function_vec_storage.size();
                rec->local_entries.resize(rec->local_function_count);
                g_runtime.modules[module_id] = ::std::move(rec);
            }

            if(!g_runtime.jit_bridges_installed)
            {
                ::uwvm2::runtime::compiler::uwvm_int::optable::call_func = jit_call_bridge;
                ::uwvm2::runtime::compiler::uwvm_int::optable::call_indirect_func = jit_call_indirect_bridge;
                g_runtime.jit_bridges_installed = true;
            }

            g_runtime.prepared.store(true, ::std::memory_order_release);
        }

        inline void compile_module_if_needed(::std::size_t module_id) noexcept
        {
            prepare_runtime_if_needed();

            auto* const record{module_record_from_id(module_id)};
            if(record == nullptr) [[unlikely]] { fatal_runtime_error(u8"LLVM JIT cannot find module record by runtime id."); }
            if(record->compiled) [[likely]] { return; }

            native_mutex_guard compile_guard{record->compile_mutex};
            if(record->compiled) [[likely]] { return; }

            if(record->local_function_count == 0uz)
            {
                record->compiled = true;
                return;
            }

            initialize_llvm_once();

            auto module_name{
                ::std::string{"uwvm2_llvm_jit_module_"} + ::std::to_string(static_cast<unsigned long long>(record->module_id))};
            record->llvm_context = ::std::make_unique<llvm::LLVMContext>();
            auto module{::std::make_unique<llvm::Module>(module_name, *record->llvm_context)};

            auto* const ptr_ty{llvm::PointerType::getUnqual(*record->llvm_context)};
            auto* const wrapper_ty{llvm::FunctionType::get(llvm::Type::getVoidTy(*record->llvm_context), {ptr_ty}, false)};

            ::std::vector<::std::string> function_names{};
            function_names.reserve(record->local_function_count);

            for(::std::size_t local_index{}; local_index != record->local_function_count; ++local_index)
            {
                compiled_defined_call_info_t info{};
                if(!::uwvm2::runtime::uwvm_int::runtime_defined_call_info_at(record->module_id, local_index, ::std::addressof(info))) [[unlikely]]
                {
                    fatal_runtime_error(u8"LLVM JIT cannot query compiled call metadata for module=\"",
                                        record->module_name,
                                        u8"\", local_index=",
                                        local_index,
                                        u8".");
                }

                auto function_name{
                    ::std::string{"uwvm2_llvm_jit_fn_"} + ::std::to_string(static_cast<unsigned long long>(record->module_id)) + "_" +
                    ::std::to_string(static_cast<unsigned long long>(local_index))};
                function_names.push_back(function_name);

                auto* const function{
                    llvm::Function::Create(wrapper_ty, llvm::Function::ExternalLinkage, function_name, *module)};
                auto arg_it{function->arg_begin()};
                auto* const stack_top_ptr_ptr{arg_it++};
                stack_top_ptr_ptr->setName("stack_top_ptr");

                auto* const entry_bb{llvm::BasicBlock::Create(*record->llvm_context, "entry", function)};
                llvm::IRBuilder<> ir_builder{entry_bb};

                if(!emit_trivial_body(*module, ir_builder, stack_top_ptr_ptr, info) &&
                   !emit_simple_call_body(*module, ir_builder, stack_top_ptr_ptr, *record, local_index, info) &&
                   !emit_simple_call_indirect_body(*module, ir_builder, stack_top_ptr_ptr, *record, local_index, info) &&
                   !emit_simple_memory_roundtrip_body(*module, ir_builder, stack_top_ptr_ptr, *record, local_index, info))
                {
                    emit_fallback_body(*module, ir_builder, stack_top_ptr_ptr, record->module_id, info.function_index);
                }

                if(llvm::verifyFunction(*function, &llvm::errs())) [[unlikely]]
                {
                    fatal_runtime_error(u8"LLVM JIT generated an invalid wrapper for module=\"",
                                        record->module_name,
                                        u8"\", local_index=",
                                        local_index,
                                        u8".");
                }
            }

            if(llvm::verifyModule(*module, &llvm::errs())) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT generated an invalid module for module=\"", record->module_name, u8"\".");
            }

            auto engine{create_execution_engine(::std::move(module), module_name)};
            record->execution_engine = ::std::move(engine);

            for(::std::size_t local_index{}; local_index != record->local_function_count; ++local_index)
            {
                auto const address{record->execution_engine->getFunctionAddress(function_names[local_index])};
                if(address == 0u) [[unlikely]]
                {
                    fatal_runtime_error(u8"LLVM JIT failed to materialize wrapper for module=\"",
                                        record->module_name,
                                        u8"\", local_index=",
                                        local_index,
                                        u8".");
                }
                record->local_entries[local_index] = reinterpret_cast<native_entry_t>(static_cast<::std::uintptr_t>(address));
            }

            record->compiled = true;
        }

        inline void compile_all_modules() noexcept
        {
            prepare_runtime_if_needed();
            for(::std::size_t module_id{}; module_id != g_runtime.modules.size(); ++module_id)
            {
                if(g_runtime.modules[module_id]) { compile_module_if_needed(module_id); }
            }
        }

        inline void ensure_void_to_void_local_entry(jit_module_record const& record,
                                                    run_config cfg,
                                                    compiled_defined_call_info_t& out_info) noexcept
        {
            auto const import_n{record.imported_function_count};
            if(cfg.entry_function_index < import_n) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT local-entry path received an imported function index.");
            }

            auto const local_index{cfg.entry_function_index - import_n};
            if(local_index >= record.local_function_count) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT local-entry index is out of range for module=\"", record.module_name, u8"\".");
            }

            if(!::uwvm2::runtime::uwvm_int::runtime_defined_call_info_at(record.module_id, local_index, ::std::addressof(out_info))) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT cannot query entry metadata for module=\"", record.module_name, u8"\".");
            }

            if(out_info.param_bytes != 0uz || out_info.result_bytes != 0uz) [[unlikely]]
            {
                fatal_runtime_error(u8"Entry function signature is not () -> () (module=\"", record.module_name, u8"\").");
            }
        }

        inline void run_main_module_impl(::uwvm2::utils::container::u8string_view main_module_name, run_config cfg, bool eager_compile) noexcept
        {
            prepare_runtime_if_needed();
            if(eager_compile) { compile_all_modules(); }

            auto const module_id{::uwvm2::runtime::uwvm_int::runtime_module_id_from_name(main_module_name)};
            if(module_id == invalid_module_id) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT cannot resolve main module id for module=\"", main_module_name, u8"\".");
            }

            auto* const record{module_record_from_id(module_id)};
            if(record == nullptr || record->runtime_module == nullptr) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT cannot find main module runtime storage for module=\"", main_module_name, u8"\".");
            }

            auto const import_n{record->imported_function_count};
            auto const total_n{import_n + record->local_function_count};
            if(cfg.entry_function_index >= total_n) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT entry function index is out of range for module=\"", main_module_name, u8"\".");
            }

            if(cfg.entry_function_index < import_n)
            {
                // Imported entrypoints may resolve across module boundaries. Reuse the complete uwvm_int bridge for that path.
                ::std::byte* stack_top_ptr{};
                ::uwvm2::runtime::uwvm_int::invoke_function_bridge(module_id, cfg.entry_function_index, ::std::addressof(stack_top_ptr));
                return;
            }

            compile_module_if_needed(module_id);

            auto const local_index{cfg.entry_function_index - import_n};
            if(local_index >= record->local_entries.size() || record->local_entries[local_index] == nullptr) [[unlikely]]
            {
                fatal_runtime_error(u8"LLVM JIT entry wrapper is unavailable for module=\"", main_module_name, u8"\".");
            }

            compiled_defined_call_info_t entry_info{};
            ensure_void_to_void_local_entry(*record, cfg, entry_info);

            ::std::byte* stack_top_ptr{};
            record->local_entries[local_index](::std::addressof(stack_top_ptr));
        }
    }  // namespace

    extern "C" void uwvm2_llvm_jit_fallback_invoke(::std::size_t module_id, ::std::size_t function_index, ::std::byte** stack_top_ptr) noexcept
    {
        ::uwvm2::runtime::uwvm_int::invoke_function_bridge(module_id, function_index, stack_top_ptr);
    }

    extern "C" void
        uwvm2_llvm_jit_fallback_call_indirect_invoke(::std::size_t module_id,
                                                     ::std::size_t type_index,
                                                     ::std::size_t table_index,
                                                     ::std::byte** stack_top_ptr) noexcept
    {
        ::uwvm2::runtime::uwvm_int::invoke_function_call_indirect_bridge(module_id, type_index, table_index, stack_top_ptr);
    }

    extern "C" void
        uwvm2_llvm_jit_fallback_memory_grow_invoke(::std::size_t module_id, ::std::size_t memory_index, ::std::byte** stack_top_ptr) noexcept
    {
        ::uwvm2::runtime::uwvm_int::invoke_memory_grow_bridge(module_id, memory_index, stack_top_ptr);
    }

    extern "C" void uwvm2_llvm_jit_fallback_memory_i32_load_invoke(::std::size_t module_id,
                                                                    ::std::size_t memory_index,
                                                                    ::std::size_t static_offset,
                                                                    ::std::byte** stack_top_ptr) noexcept
    {
        ::uwvm2::runtime::uwvm_int::invoke_memory_i32_load_bridge(module_id, memory_index, static_offset, stack_top_ptr);
    }

    extern "C" void uwvm2_llvm_jit_fallback_memory_i32_store_invoke(::std::size_t module_id,
                                                                     ::std::size_t memory_index,
                                                                     ::std::size_t static_offset,
                                                                     ::std::byte** stack_top_ptr) noexcept
    {
        ::uwvm2::runtime::uwvm_int::invoke_memory_i32_store_bridge(module_id, memory_index, static_offset, stack_top_ptr);
    }

    void lazy_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, run_config cfg) noexcept
    { run_main_module_impl(main_module_name, cfg, false); }

    void full_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, run_config cfg) noexcept
    { run_main_module_impl(main_module_name, cfg, true); }
}  // namespace uwvm2::runtime::llvm_jit

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_pop_macro.h>
#endif
