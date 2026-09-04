/*************************************************************
 * UlteSoft WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

#pragma once

#include <cfenv>
#include <exception>
#include <memory>

namespace uwvm2::runtime::lib::details
{
    // Generated scalar Wasm FP instructions assume the IEEE default environment: round-to-nearest/ties-to-even,
    // gradual underflow, and masked exceptions.  Embedding threads are allowed to use another environment, so LLVM
    // execution scopes save it and install FE_DFL_ENV.  The caller owns the active marker so builds that intentionally
    // disable C++ thread_local can keep it in the same map-backed per-thread state as the logical call stack.
    [[nodiscard]] inline constexpr bool is_llvm_wasm_fp_environment_active(bool const& active_marker) noexcept
    { return active_marker; }

    class scoped_llvm_wasm_fp_environment
    {
        ::std::fenv_t saved_environment{};
        bool* active_marker{};
        bool previous_active{};
        bool restore_environment{};
        bool ready_state{true};

    public:
        explicit scoped_llvm_wasm_fp_environment(bool& caller_active_marker, bool enable = true) noexcept
            : active_marker{::std::addressof(caller_active_marker)}, previous_active{caller_active_marker}
        {
            if(!enable) { return; }

            ready_state = false;
            if(::std::fegetenv(::std::addressof(saved_environment)) != 0) [[unlikely]] { return; }
            restore_environment = true;
            if(::std::fesetenv(FE_DFL_ENV) != 0) [[unlikely]] { return; }

            *active_marker = true;
            ready_state = true;
        }

        scoped_llvm_wasm_fp_environment(scoped_llvm_wasm_fp_environment const&) = delete;
        scoped_llvm_wasm_fp_environment& operator=(scoped_llvm_wasm_fp_environment const&) = delete;

        ~scoped_llvm_wasm_fp_environment() noexcept
        {
            if(!restore_environment) { return; }
            *active_marker = previous_active;
            if(::std::fesetenv(::std::addressof(saved_environment)) != 0) [[unlikely]] { ::std::terminate(); }
        }

        [[nodiscard]] bool ready() const noexcept { return ready_state; }
    };

    class scoped_llvm_wasm_host_fp_environment_restore
    {
        ::std::fenv_t saved_environment{};
        bool restore_environment{};
        bool ready_state{true};

    public:
        explicit scoped_llvm_wasm_host_fp_environment_restore(bool const& active_marker) noexcept
        {
            if(!active_marker) { return; }

            ready_state = false;
            if(::std::fegetenv(::std::addressof(saved_environment)) != 0) [[unlikely]] { return; }
            restore_environment = true;
            ready_state = true;
        }

        scoped_llvm_wasm_host_fp_environment_restore(scoped_llvm_wasm_host_fp_environment_restore const&) = delete;
        scoped_llvm_wasm_host_fp_environment_restore& operator=(scoped_llvm_wasm_host_fp_environment_restore const&) = delete;

        ~scoped_llvm_wasm_host_fp_environment_restore() noexcept
        {
            if(!restore_environment) { return; }
            if(::std::fesetenv(::std::addressof(saved_environment)) != 0) [[unlikely]] { ::std::terminate(); }
        }

        [[nodiscard]] bool ready() const noexcept { return ready_state; }
    };
}
