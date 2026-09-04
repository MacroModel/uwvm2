/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

#pragma once

#include <type_traits>

namespace uwvm2::runtime::lib::details
{
    // Runtime-module call_indirect views borrow storage owned by the runtime registry.  Detach every view before the owning
    // records are cleared so otherwise-live module storage cannot retain a dangling native target array.
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

    // Tiered builds also publish an interpreter-side refresh hook.  Disable it before detaching views: after reset begins,
    // a stale table-write notification must not rebuild snapshots from records that are about to be destroyed.
    template <typename RuntimeModuleRecords, typename RefreshHook>
    inline constexpr void detach_borrowed_llvm_jit_call_indirect_table_views(RuntimeModuleRecords const& records,
                                                                              RefreshHook& refresh_hook) noexcept
    {
        refresh_hook = nullptr;
        clear_borrowed_llvm_jit_call_indirect_table_views(records);
    }
}
