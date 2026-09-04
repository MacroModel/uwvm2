/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

#pragma once

#include <type_traits>

namespace uwvm2::runtime::lib::details
{
    // Runtime-module call_indirect views borrow target arrays owned by the compiled runtime records. Detach every view before
    // clearing those records so otherwise-live module storage cannot retain dangling native dispatch pointers.
    template <typename RuntimeModuleRecords>
    inline constexpr void clear_borrowed_llvm_jit_call_indirect_table_views(RuntimeModuleRecords const& records) noexcept
    {
        for(auto const& rec: records)
        {
            auto const runtime_module{rec.runtime_module};
            if(runtime_module == nullptr) { continue; }

            using runtime_module_pointer = ::std::remove_cv_t<decltype(runtime_module)>;
            using mutable_runtime_module = ::std::remove_const_t<::std::remove_pointer_t<runtime_module_pointer>>;
            auto& mutable_module{*const_cast<mutable_runtime_module*>(runtime_module)};
            for(auto& table_view: mutable_module.llvm_jit_call_indirect_table_views) { table_view = {}; }
        }
    }
}
