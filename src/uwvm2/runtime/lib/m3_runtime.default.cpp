/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
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

// std
#include <cstddef>
#include <span>
// macro
#include <uwvm2/utils/macro/push_macros.h>

#ifndef UWVM_MODULE
# include <fast_io.h>
# include <uwvm2/runtime/compiler/m3_int/straight_line.h>
# include <uwvm2/uwvm/io/impl.h>
# include <uwvm2/uwvm/runtime/storage/impl.h>
# include <uwvm2/runtime/lib/m3_runtime.h>
#endif

namespace uwvm2::runtime::m3_int
{
    namespace
    {
        using value_type = ::uwvm2::runtime::compiler::m3_int::straight_line::value_type;
        using function_descriptor_t = ::uwvm2::runtime::compiler::m3_int::straight_line::function_descriptor_t;
        using local_entry_t = ::uwvm2::runtime::compiler::m3_int::straight_line::local_entry_t;
        using compile_error_t = ::uwvm2::runtime::compiler::m3_int::straight_line::compile_error_t;
        using execution_result_t = ::uwvm2::runtime::compiler::m3_int::straight_line::execution_result_t;
        using module_function_t = ::uwvm2::runtime::compiler::m3_int::straight_line::module_function_t;
        using function_reference_t = ::uwvm2::runtime::compiler::m3_int::straight_line::function_reference_t;
        using indirect_type_reference_t = ::uwvm2::runtime::compiler::m3_int::straight_line::indirect_type_reference_t;
        using table_reference_t = ::uwvm2::runtime::compiler::m3_int::straight_line::table_reference_t;
        using wasm_module_storage_t = ::uwvm2::uwvm::runtime::storage::wasm_module_storage_t;
        using local_defined_function_storage_t = ::uwvm2::uwvm::runtime::storage::local_defined_function_storage_t;
        using imported_function_storage_t = ::uwvm2::uwvm::runtime::storage::imported_function_storage_t;
        using imported_function_link_kind = ::uwvm2::uwvm::runtime::storage::imported_function_link_kind;
        using local_defined_table_storage_t = ::uwvm2::uwvm::runtime::storage::local_defined_table_storage_t;
        using imported_table_storage_t = ::uwvm2::uwvm::runtime::storage::imported_table_storage_t;
        using local_defined_table_elem_storage_type_t = ::uwvm2::uwvm::runtime::storage::local_defined_table_elem_storage_type_t;
        using wasm_u64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_u64;

        [[nodiscard]] inline constexpr ::uwvm2::utils::container::u8string_view compile_error_name(compile_error_t err) noexcept
        {
            switch(err)
            {
                case compile_error_t::none:
                    return u8"none";
                case compile_error_t::missing_end:
                    return u8"missing_end";
                case compile_error_t::invalid_immediate:
                    return u8"invalid_immediate";
                case compile_error_t::invalid_local_index:
                    return u8"invalid_local_index";
                case compile_error_t::stack_underflow:
                    return u8"stack_underflow";
                case compile_error_t::type_mismatch:
                    return u8"type_mismatch";
                case compile_error_t::invalid_result_arity:
                    return u8"invalid_result_arity";
                case compile_error_t::invalid_final_stack:
                    return u8"invalid_final_stack";
                case compile_error_t::unsupported_opcode:
                    return u8"unsupported_opcode";
                case compile_error_t::unsupported_control_flow:
                    return u8"unsupported_control_flow";
            }
            return u8"unknown";
        }

        [[noreturn]] inline void fatal_runtime(::uwvm2::utils::container::u8string_view message) noexcept
        {
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                u8"uwvm: [fatal] ",
                                message,
                                u8" (m3int-runtime)\n\n");
            ::fast_io::fast_terminate();
        }

        [[noreturn]] inline void fatal_compile_error(::uwvm2::utils::container::u8string_view module_name,
                                                     ::std::size_t function_index,
                                                     compile_error_t err,
                                                     ::std::size_t offset) noexcept
        {
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                u8"uwvm: [fatal] m3int straight-line translation failed for module \"",
                                module_name,
                                u8"\", function #",
                                function_index,
                                u8": ",
                                compile_error_name(err),
                                u8" at expression offset ",
                                offset,
                                u8". (m3int-runtime)\n\n");
            ::fast_io::fast_terminate();
        }

        [[nodiscard]] inline imported_function_storage_t const* resolve_import_leaf(imported_function_storage_t const* imp) noexcept
        {
            constexpr ::std::size_t max_chain{4096uz};
            for(::std::size_t steps{}; steps != max_chain; ++steps)
            {
                if(imp == nullptr) [[unlikely]]
                {
                    return nullptr;
                }
                if(imp->link_kind != imported_function_link_kind::imported)
                {
                    return imp;
                }
                imp = imp->target.imported_ptr;
            }
            return nullptr;
        }

        [[nodiscard]] inline local_defined_function_storage_t const*
            resolve_entry_function(wasm_module_storage_t const& runtime_module, ::std::size_t function_index) noexcept
        {
            auto const import_n{runtime_module.imported_function_vec_storage.size()};
            auto const local_n{runtime_module.local_defined_function_vec_storage.size()};
            if(function_index >= import_n + local_n)
            {
                return nullptr;
            }

            if(function_index < import_n)
            {
                auto const* const imported{::std::addressof(runtime_module.imported_function_vec_storage.index_unchecked(function_index))};
                auto const* const leaf{resolve_import_leaf(imported)};
                if(leaf == nullptr || leaf->link_kind != imported_function_link_kind::defined)
                {
                    return nullptr;
                }
                return leaf->target.defined_ptr;
            }

            return ::std::addressof(runtime_module.local_defined_function_vec_storage.index_unchecked(function_index - import_n));
        }

        [[nodiscard]] inline bool is_void_to_void(local_defined_function_storage_t const& func) noexcept
        {
            auto const* const ft{func.function_type_ptr};
            return ft != nullptr && ft->parameter.begin == ft->parameter.end && ft->result.begin == ft->result.end;
        }

        [[nodiscard]] inline constexpr local_defined_table_storage_t const* resolve_table(wasm_module_storage_t const& module,
                                                                                          ::std::size_t table_index) noexcept
        {
            auto const import_n{module.imported_table_vec_storage.size()};
            if(table_index < import_n)
            {
                auto table{::std::addressof(module.imported_table_vec_storage.index_unchecked(table_index))};
                for(;;)
                {
                    if(table == nullptr) [[unlikely]]
                    {
                        return nullptr;
                    }
                    using link_kind = imported_table_storage_t::imported_table_link_kind;
                    switch(table->link_kind)
                    {
                        case link_kind::defined:
                            return table->target.defined_ptr;
                        case link_kind::imported:
                            table = table->target.imported_ptr;
                            continue;
                        case link_kind::unresolved:
                        default:
                            return nullptr;
                    }
                }
            }

            auto const local_index{table_index - import_n};
            if(local_index >= module.local_defined_table_vec_storage.size()) [[unlikely]]
            {
                return nullptr;
            }
            return ::std::addressof(module.local_defined_table_vec_storage.index_unchecked(local_index));
        }

        struct runtime_function_storage_t
        {
            ::uwvm2::utils::container::vector<value_type> parameters{};
            ::uwvm2::utils::container::vector<value_type> results{};
            ::uwvm2::utils::container::vector<local_entry_t> locals{};
            bool available{};
        };

        struct runtime_type_storage_t
        {
            ::uwvm2::utils::container::vector<value_type> parameters{};
            ::uwvm2::utils::container::vector<value_type> results{};
        };

        struct runtime_table_storage_t
        {
            ::uwvm2::utils::container::vector<void*> elements{};
            bool available{};
        };

        inline void build_and_run_straight_line(::uwvm2::utils::container::u8string_view module_name,
                                                ::std::size_t function_index,
                                                wasm_module_storage_t const& runtime_module) noexcept
        {
            auto const import_n{runtime_module.imported_function_vec_storage.size()};
            auto const local_n{runtime_module.local_defined_function_vec_storage.size()};
            auto const function_count{import_n + local_n};
            if(function_index >= function_count) [[unlikely]]
            {
                fatal_runtime(u8"m3int runtime bridge entry index is out of range");
            }

            ::uwvm2::utils::container::vector<module_function_t> module_functions{};
            ::uwvm2::utils::container::vector<function_reference_t> function_refs{};
            ::uwvm2::utils::container::vector<runtime_function_storage_t> function_storage{};
            module_functions.resize(function_count);
            function_refs.resize(function_count);
            function_storage.resize(function_count);

            auto const* const type_begin{runtime_module.type_section_storage.type_section_begin};
            auto const* const type_end{runtime_module.type_section_storage.type_section_end};
            if(type_begin == nullptr || type_end == nullptr) [[unlikely]]
            {
                fatal_runtime(u8"m3int runtime bridge encountered a null type section");
            }
            auto const type_count{static_cast<::std::size_t>(type_end - type_begin)};
            ::uwvm2::utils::container::vector<runtime_type_storage_t> type_storage{};
            ::uwvm2::utils::container::vector<indirect_type_reference_t> type_refs{};
            type_storage.resize(type_count);
            type_refs.resize(type_count);
            for(::std::size_t i{}; i != type_count; ++i)
            {
                auto const& runtime_type{type_begin[i]};
                auto& storage{type_storage.index_unchecked(i)};
                auto& ref{type_refs.index_unchecked(i)};
                storage.parameters.reserve(static_cast<::std::size_t>(runtime_type.parameter.end - runtime_type.parameter.begin));
                for(auto it{runtime_type.parameter.begin}; it != runtime_type.parameter.end; ++it)
                {
                    storage.parameters.push_back(static_cast<value_type>(*it));
                }
                storage.results.reserve(static_cast<::std::size_t>(runtime_type.result.end - runtime_type.result.begin));
                for(auto it{runtime_type.result.begin}; it != runtime_type.result.end; ++it)
                {
                    storage.results.push_back(static_cast<value_type>(*it));
                }
                ref = indirect_type_reference_t{.parameters = {storage.parameters.data(), storage.parameters.size()},
                                                .results = {storage.results.data(), storage.results.size()}};
            }

            for(::std::size_t i{}; i != function_count; ++i)
            {
                auto const* const resolved{resolve_entry_function(runtime_module, i)};
                if(resolved == nullptr)
                {
                    continue;
                }

                auto const* const resolved_ft{resolved->function_type_ptr};
                auto const* const resolved_code{resolved->wasm_code_ptr};
                if(resolved_ft == nullptr || resolved_code == nullptr) [[unlikely]]
                {
                    fatal_runtime(u8"m3int runtime bridge encountered a null function descriptor");
                }

                auto& storage{function_storage.index_unchecked(i)};
                auto& module_function{module_functions.index_unchecked(i)};
                auto& function_ref{function_refs.index_unchecked(i)};

                auto const parameter_count{static_cast<::std::size_t>(resolved_ft->parameter.end - resolved_ft->parameter.begin)};
                auto const result_count{static_cast<::std::size_t>(resolved_ft->result.end - resolved_ft->result.begin)};
                auto const expression_size{static_cast<::std::size_t>(resolved_code->body.code_end - resolved_code->body.expr_begin)};

                storage.parameters.reserve(parameter_count);
                for(auto it{resolved_ft->parameter.begin}; it != resolved_ft->parameter.end; ++it)
                {
                    storage.parameters.push_back(static_cast<value_type>(*it));
                }
                storage.results.reserve(result_count);
                for(auto it{resolved_ft->result.begin}; it != resolved_ft->result.end; ++it)
                {
                    storage.results.push_back(static_cast<value_type>(*it));
                }
                storage.locals.reserve(resolved_code->locals.size());
                for(auto const& local : resolved_code->locals)
                {
                    storage.locals.push_back(local_entry_t{.count = local.count, .type = static_cast<value_type>(local.type)});
                }
                storage.available = true;

                module_function.descriptor = function_descriptor_t{.parameters = {storage.parameters.data(), storage.parameters.size()},
                                                                   .results = {storage.results.data(), storage.results.size()},
                                                                   .locals = {storage.locals.data(), storage.locals.size()},
                                                                   .expression = {resolved_code->body.expr_begin, expression_size},
                                                                   .functions = {}};
                function_ref = function_reference_t{.parameters = {storage.parameters.data(), storage.parameters.size()},
                                                    .results = {storage.results.data(), storage.results.size()},
                                                    .handle = ::std::addressof(module_function)};
            }

            auto const import_function_base{runtime_module.imported_function_vec_storage.data()};
            auto const local_function_base{runtime_module.local_defined_function_vec_storage.data()};
            auto const table_count{runtime_module.imported_table_vec_storage.size() + runtime_module.local_defined_table_vec_storage.size()};
            ::uwvm2::utils::container::vector<runtime_table_storage_t> table_storage{};
            ::uwvm2::utils::container::vector<table_reference_t> table_refs{};
            table_storage.resize(table_count);
            table_refs.resize(table_count);

            auto const resolve_function_handle = [&](imported_function_storage_t const* imported,
                                                    local_defined_function_storage_t const* defined) noexcept -> void* {
                if(imported != nullptr)
                {
                    auto const import_n_local{runtime_module.imported_function_vec_storage.size()};
                    if(import_function_base != nullptr && imported >= import_function_base &&
                       imported < import_function_base + static_cast<::std::ptrdiff_t>(import_n_local))
                    {
                        auto const index{static_cast<::std::size_t>(imported - import_function_base)};
                        if(function_storage.index_unchecked(index).available)
                        {
                            return ::std::addressof(module_functions.index_unchecked(index));
                        }
                    }

                    auto const* const leaf{resolve_import_leaf(imported)};
                    if(leaf != nullptr && leaf->link_kind == imported_function_link_kind::defined)
                    {
                        defined = leaf->target.defined_ptr;
                    }
                }

                if(defined != nullptr)
                {
                    auto const import_n_local{runtime_module.imported_function_vec_storage.size()};
                    auto const local_n_local{runtime_module.local_defined_function_vec_storage.size()};
                    if(local_function_base != nullptr && defined >= local_function_base &&
                       defined < local_function_base + static_cast<::std::ptrdiff_t>(local_n_local))
                    {
                        auto const local_index{static_cast<::std::size_t>(defined - local_function_base)};
                        return ::std::addressof(module_functions.index_unchecked(import_n_local + local_index));
                    }
                }
                return nullptr;
            };

            for(::std::size_t i{}; i != table_count; ++i)
            {
                auto const* const table{resolve_table(runtime_module, i)};
                if(table == nullptr)
                {
                    continue;
                }

                auto& storage{table_storage.index_unchecked(i)};
                auto& table_ref{table_refs.index_unchecked(i)};
                storage.elements.reserve(table->elems.size());
                for(auto const& elem : table->elems)
                {
                    void* handle{};
                    switch(elem.type)
                    {
                        case local_defined_table_elem_storage_type_t::func_ref_imported:
                            handle = resolve_function_handle(elem.storage.imported_ptr, nullptr);
                            break;
                        case local_defined_table_elem_storage_type_t::func_ref_defined:
                            handle = resolve_function_handle(nullptr, elem.storage.defined_ptr);
                            break;
                    }
                    storage.elements.push_back(handle);
                }
                storage.available = true;
                table_ref = table_reference_t{.elements = {storage.elements.data(), storage.elements.size()}};
            }

            for(::std::size_t i{}; i != function_count; ++i)
            {
                if(!function_storage.index_unchecked(i).available)
                {
                    continue;
                }
                auto& descriptor{module_functions.index_unchecked(i).descriptor};
                descriptor.functions = {function_refs.data(), function_refs.size()};
                descriptor.types = {type_refs.data(), type_refs.size()};
                descriptor.tables = {table_refs.data(), table_refs.size()};
            }

            auto const& entry_function{module_functions.index_unchecked(function_index)};
            auto const compiled{::uwvm2::runtime::compiler::m3_int::straight_line::compile_function(entry_function.descriptor)};
            if(!compiled)
            {
                fatal_compile_error(module_name, function_index, compiled.error, compiled.expression_offset);
            }

            execution_result_t execution{};
            ::std::span<wasm_u64 const> args{};
            if(!::uwvm2::runtime::compiler::m3_int::straight_line::execute_function(compiled.function, args, execution, module_functions))
            {
                fatal_runtime(u8"m3int runtime bridge failed to execute compiled entry function");
            }
        }
    }

    void full_compile_and_run_main_module(::uwvm2::utils::container::u8string_view main_module_name, full_compile_run_config cfg) noexcept
    {
        auto const it{::uwvm2::uwvm::runtime::storage::wasm_module_runtime_storage.find(main_module_name)};
        if(it == ::uwvm2::uwvm::runtime::storage::wasm_module_runtime_storage.end()) [[unlikely]]
        {
            fatal_runtime(u8"m3int runtime bridge could not find runtime storage for the main module");
        }

        auto const& runtime_module{it->second};
        auto const* const entry{resolve_entry_function(runtime_module, cfg.entry_function_index)};
        if(entry == nullptr) [[unlikely]]
        {
            fatal_runtime(u8"m3int runtime bridge could not resolve the requested entry function");
        }
        if(!is_void_to_void(*entry)) [[unlikely]]
        {
            fatal_runtime(u8"m3int runtime bridge currently requires a () -> () entry function");
        }

        build_and_run_straight_line(main_module_name, cfg.entry_function_index, runtime_module);
    }
}  // namespace uwvm2::runtime::m3_int
