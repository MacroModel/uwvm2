#include <uwvm2/runtime/lib/m3_runtime.default.cpp>
#include <uwvm2/parser/wasm/standard/wasm1/opcode/mvp.h>

namespace runtime = ::uwvm2::runtime::m3_int;
namespace storage = ::uwvm2::uwvm::runtime::storage;
namespace wasm_type = ::uwvm2::parser::wasm::standard::wasm1::type;
namespace opcode = ::uwvm2::parser::wasm::standard::wasm1::opcode;

namespace
{
    using function_type_t = storage::wasm_binfmt1_final_function_type_t;
    using wasm_code_t = storage::wasm_binfmt1_final_wasm_code_t;

    constexpr wasm_type::wasm_byte end_expr[]{static_cast<wasm_type::wasm_byte>(opcode::op_basic::end)};
    constexpr wasm_type::wasm_byte call_local_zero_expr[]{static_cast<wasm_type::wasm_byte>(opcode::op_basic::call),
                                                          wasm_type::wasm_byte{0u},
                                                          static_cast<wasm_type::wasm_byte>(opcode::op_basic::end)};
    constexpr wasm_type::wasm_byte call_indirect_zero_expr[]{static_cast<wasm_type::wasm_byte>(opcode::op_basic::i32_const),
                                                             wasm_type::wasm_byte{0u},
                                                             static_cast<wasm_type::wasm_byte>(opcode::op_basic::call_indirect),
                                                             wasm_type::wasm_byte{0u},
                                                             wasm_type::wasm_byte{0u},
                                                             static_cast<wasm_type::wasm_byte>(opcode::op_basic::end)};

    function_type_t const empty_void_signature{};

    wasm_code_t make_end_only_code()
    {
        wasm_code_t code{};
        code.body.code_begin = end_expr;
        code.body.expr_begin = end_expr;
        code.body.code_end = end_expr + 1;
        code.all_local_count = 0u;
        return code;
    }

    wasm_code_t make_call_local_zero_code()
    {
        wasm_code_t code{};
        code.body.code_begin = call_local_zero_expr;
        code.body.expr_begin = call_local_zero_expr;
        code.body.code_end = call_local_zero_expr + 3;
        code.all_local_count = 0u;
        return code;
    }

    wasm_code_t make_call_indirect_zero_code()
    {
        wasm_code_t code{};
        code.body.code_begin = call_indirect_zero_expr;
        code.body.expr_begin = call_indirect_zero_expr;
        code.body.code_end = call_indirect_zero_expr + 6;
        code.all_local_count = 0u;
        return code;
    }

    void reset_runtime_storage()
    {
        storage::wasm_module_runtime_storage.clear();
        storage::wasm_module_runtime_storage.reserve(2uz);
    }
}

int main()
{
    reset_runtime_storage();

    {
        auto [it, inserted]{storage::wasm_module_runtime_storage.try_emplace(u8"m3int-local", storage::wasm_module_storage_t{})};
        if(!inserted) { return 1; }

        auto &module{it->second};
        wasm_code_t code{make_end_only_code()};

        module.type_section_storage.type_section_begin = ::std::addressof(empty_void_signature);
        module.type_section_storage.type_section_end = ::std::addressof(empty_void_signature) + 1;
        module.local_defined_function_vec_storage.reserve(1uz);
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(code)});

        runtime::full_compile_run_config cfg{.entry_function_index = 0uz};
        runtime::full_compile_and_run_main_module(u8"m3int-local", cfg);
    }

    reset_runtime_storage();

    {
        auto [it, inserted]{storage::wasm_module_runtime_storage.try_emplace(u8"m3int-import", storage::wasm_module_storage_t{})};
        if(!inserted) { return 2; }

        auto &module{it->second};
        wasm_code_t code{make_end_only_code()};

        module.type_section_storage.type_section_begin = ::std::addressof(empty_void_signature);
        module.type_section_storage.type_section_end = ::std::addressof(empty_void_signature) + 1;
        module.local_defined_function_vec_storage.reserve(1uz);
        module.imported_function_vec_storage.reserve(1uz);
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(code)});

        auto const *local_entry{::std::addressof(module.local_defined_function_vec_storage.index_unchecked(0uz))};

        storage::imported_function_storage_t imported{};
        imported.target.defined_ptr = local_entry;
        imported.link_kind = storage::imported_function_link_kind::defined;
        module.imported_function_vec_storage.push_back(imported);

        runtime::full_compile_run_config imported_cfg{.entry_function_index = 0uz};
        runtime::full_compile_and_run_main_module(u8"m3int-import", imported_cfg);

        runtime::full_compile_run_config local_cfg{.entry_function_index = 1uz};
        runtime::full_compile_and_run_main_module(u8"m3int-import", local_cfg);
    }

    reset_runtime_storage();

    {
        auto [it, inserted]{storage::wasm_module_runtime_storage.try_emplace(u8"m3int-local-call", storage::wasm_module_storage_t{})};
        if(!inserted) { return 3; }

        auto &module{it->second};
        wasm_code_t helper_code{make_end_only_code()};
        wasm_code_t entry_code{make_call_local_zero_code()};

        module.type_section_storage.type_section_begin = ::std::addressof(empty_void_signature);
        module.type_section_storage.type_section_end = ::std::addressof(empty_void_signature) + 1;
        module.local_defined_function_vec_storage.reserve(2uz);
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(helper_code)});
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(entry_code)});

        runtime::full_compile_run_config cfg{.entry_function_index = 1uz};
        runtime::full_compile_and_run_main_module(u8"m3int-local-call", cfg);
    }

    reset_runtime_storage();

    {
        auto [it, inserted]{storage::wasm_module_runtime_storage.try_emplace(u8"m3int-local-call-indirect", storage::wasm_module_storage_t{})};
        if(!inserted) { return 4; }

        auto &module{it->second};
        wasm_code_t helper_code{make_end_only_code()};
        wasm_code_t entry_code{make_call_indirect_zero_code()};

        module.type_section_storage.type_section_begin = ::std::addressof(empty_void_signature);
        module.type_section_storage.type_section_end = ::std::addressof(empty_void_signature) + 1;
        module.local_defined_function_vec_storage.reserve(2uz);
        module.local_defined_table_vec_storage.reserve(1uz);
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(helper_code)});
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(entry_code)});

        storage::local_defined_table_storage_t table{};
        table.elems.reserve(1uz);
        storage::local_defined_table_elem_storage_t elem{};
        elem.type = storage::local_defined_table_elem_storage_type_t::func_ref_defined;
        elem.storage.defined_ptr = ::std::addressof(module.local_defined_function_vec_storage.index_unchecked(0uz));
        table.elems.push_back(elem);
        module.local_defined_table_vec_storage.push_back(::std::move(table));

        runtime::full_compile_run_config cfg{.entry_function_index = 1uz};
        runtime::full_compile_and_run_main_module(u8"m3int-local-call-indirect", cfg);
    }

    reset_runtime_storage();

    {
        auto [it, inserted]{storage::wasm_module_runtime_storage.try_emplace(u8"m3int-imported-table-call-indirect", storage::wasm_module_storage_t{})};
        if(!inserted) { return 5; }

        auto &module{it->second};
        wasm_code_t helper_code{make_end_only_code()};
        wasm_code_t entry_code{make_call_indirect_zero_code()};

        module.type_section_storage.type_section_begin = ::std::addressof(empty_void_signature);
        module.type_section_storage.type_section_end = ::std::addressof(empty_void_signature) + 1;
        module.imported_function_vec_storage.reserve(1uz);
        module.local_defined_function_vec_storage.reserve(2uz);
        module.imported_table_vec_storage.reserve(1uz);
        module.local_defined_table_vec_storage.reserve(1uz);

        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(helper_code)});
        module.local_defined_function_vec_storage.push_back(
            storage::local_defined_function_storage_t{.function_type_ptr = ::std::addressof(empty_void_signature), .wasm_code_ptr = ::std::addressof(entry_code)});

        auto const *helper_local{::std::addressof(module.local_defined_function_vec_storage.index_unchecked(0uz))};

        storage::imported_function_storage_t imported_func{};
        imported_func.target.defined_ptr = helper_local;
        imported_func.link_kind = storage::imported_function_link_kind::defined;
        module.imported_function_vec_storage.push_back(imported_func);

        storage::local_defined_table_storage_t local_table{};
        local_table.elems.reserve(1uz);
        storage::local_defined_table_elem_storage_t elem{};
        elem.type = storage::local_defined_table_elem_storage_type_t::func_ref_imported;
        elem.storage.imported_ptr = ::std::addressof(module.imported_function_vec_storage.index_unchecked(0uz));
        local_table.elems.push_back(elem);
        module.local_defined_table_vec_storage.push_back(::std::move(local_table));

        storage::imported_table_storage_t imported_table{};
        imported_table.target.defined_ptr = ::std::addressof(module.local_defined_table_vec_storage.index_unchecked(0uz));
        imported_table.link_kind = storage::imported_table_storage_t::imported_table_link_kind::defined;
        module.imported_table_vec_storage.push_back(imported_table);

        runtime::full_compile_run_config cfg{.entry_function_index = 2uz};
        runtime::full_compile_and_run_main_module(u8"m3int-imported-table-call-indirect", cfg);
    }

    storage::wasm_module_runtime_storage.clear();
    return 0;
}
