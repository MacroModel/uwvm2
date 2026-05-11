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
# include <atomic>
# include <algorithm>
# include <cstddef>
# include <cstdint>
# include <limits>
# include <type_traits>
// macro
# include <uwvm2/utils/macro/push_macros.h>
# include <uwvm2/uwvm/runtime/macro/push_macros.h>
// import
# include <fast_io.h>
# include <uwvm2/uwvm/io/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::runtime::lazy_validator
{
    enum class tiered_probe_kind_t : unsigned
    {
        function_entry,
        block_entry,
        loop_backedge
    };

    struct tiered_opcode_profile_t
    {
        ::std::size_t code_size{};
        ::std::size_t block_count{};
        ::std::size_t loop_count{};
        ::std::size_t if_count{};
    };

    [[nodiscard]] inline constexpr ::std::size_t tiered_opcode_profile_density(tiered_opcode_profile_t const& profile) noexcept
    {
        return profile.block_count + (profile.loop_count * 2uz) + profile.if_count;
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_function_entry_threshold(tiered_opcode_profile_t const& profile) noexcept
    {
        auto threshold{16uz + (profile.code_size / 16384uz)};
        threshold += (profile.block_count + profile.if_count) / 4uz;
        if(profile.loop_count != 0uz) { threshold += 2uz; }
        return threshold == 0uz ? 1uz : threshold;
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_loop_backedge_threshold(tiered_opcode_profile_t const& profile) noexcept
    {
        auto threshold{2uz + (profile.code_size / 32768uz)};
        if(profile.loop_count != 0uz && threshold > 1uz) { --threshold; }
        return threshold == 0uz ? 1uz : threshold;
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_block_entry_threshold(tiered_opcode_profile_t const& profile) noexcept
    {
        auto threshold{24uz + (profile.code_size / 8192uz)};
        threshold += profile.block_count + (profile.if_count / 2uz);
        if(profile.loop_count != 0uz) { threshold += 2uz; }
        return threshold == 0uz ? 1uz : threshold;
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_probe_threshold(tiered_opcode_profile_t const& profile,
                                                                        tiered_probe_kind_t kind) noexcept
    {
        switch(kind)
        {
            case tiered_probe_kind_t::function_entry: return tiered_function_entry_threshold(profile);
            case tiered_probe_kind_t::block_entry: return tiered_block_entry_threshold(profile);
            case tiered_probe_kind_t::loop_backedge: return tiered_loop_backedge_threshold(profile);
            [[unlikely]] default: return tiered_function_entry_threshold(profile);
        }
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_prewarm_budget(tiered_opcode_profile_t const& profile) noexcept
    {
        auto budget{1uz + (profile.code_size / 32768uz)};
        budget += (profile.block_count + profile.if_count) / 8uz;
        budget += profile.loop_count * 2uz;
        return budget > 12uz ? 12uz : budget;
    }

    [[nodiscard]] inline constexpr ::std::size_t tiered_direct_entry_threshold(tiered_opcode_profile_t const& profile) noexcept
    {
        auto threshold{8uz + (profile.code_size / 32768uz)};
        threshold += (profile.block_count + profile.if_count) / 8uz;
        if(profile.loop_count != 0uz) { threshold += 2uz; }
        return threshold == 0uz ? 1uz : threshold;
    }

    [[nodiscard]] inline constexpr bool tiered_profile_is_hot(tiered_opcode_profile_t const& profile) noexcept
    {
        return profile.loop_count != 0uz || profile.code_size >= 16384uz || tiered_opcode_profile_density(profile) >= 12uz;
    }

    [[nodiscard]] inline constexpr ::fast_io::u8string_view tiered_probe_kind_name(tiered_probe_kind_t kind) noexcept
    {
        switch(kind)
        {
            case tiered_probe_kind_t::function_entry: return u8"function_entry";
            case tiered_probe_kind_t::block_entry: return u8"block_entry";
            case tiered_probe_kind_t::loop_backedge: return u8"loop_backedge";
            [[unlikely]] default: return u8"unknown";
        }
    }

    [[nodiscard]] inline constexpr ::std::uint_least32_t tiered_probe_kind_to_raw(tiered_probe_kind_t kind) noexcept
    { return static_cast<::std::uint_least32_t>(static_cast<unsigned>(kind)); }

    [[nodiscard]] inline constexpr tiered_probe_kind_t tiered_probe_kind_from_raw(::std::uint_least32_t raw) noexcept
    {
        switch(static_cast<tiered_probe_kind_t>(raw))
        {
            case tiered_probe_kind_t::function_entry:
            case tiered_probe_kind_t::block_entry:
            case tiered_probe_kind_t::loop_backedge: return static_cast<tiered_probe_kind_t>(raw);
            [[unlikely]] default: return tiered_probe_kind_t::function_entry;
        }
    }

    template <typename T>
    [[nodiscard]] inline T atomic_fetch_add_relaxed(T& value, T delta = 1) noexcept
    {
        return ::std::atomic_ref<T>{value}.fetch_add(delta, ::std::memory_order_relaxed);
    }

    template <typename T>
    [[nodiscard]] inline bool atomic_compare_exchange_acq_rel(T& value, T& expected, T desired) noexcept
    {
        return ::std::atomic_ref<T>{value}.compare_exchange_strong(expected,
                                                                   desired,
                                                                   ::std::memory_order_acq_rel,
                                                                   ::std::memory_order_acquire);
    }

    namespace tiered_runtime_log
    {
        [[nodiscard]] inline bool enabled() noexcept
        {
#ifdef UWVM
            return ::uwvm2::uwvm::io::enable_runtime_log;
#else
            return false;
#endif
        }

        template <typename... Args>
        inline void line(Args&&... args) noexcept
        {
#ifdef UWVM
            if(!enabled()) { return; }

            auto u8runtime_log_output_osr{::fast_io::operations::output_stream_ref(::uwvm2::uwvm::io::u8runtime_log_output)};
            ::fast_io::operations::decay::stream_ref_decay_lock_guard u8runtime_log_output_lg{
                ::fast_io::operations::decay::output_stream_mutex_ref_decay(u8runtime_log_output_osr)};
            auto u8runtime_log_output_ul{::fast_io::operations::decay::output_stream_unlocked_ref_decay(u8runtime_log_output_osr)};

            ::fast_io::io::perrln(u8runtime_log_output_ul, u8"[tiered-lazy] ", ::std::forward<Args>(args)...);
#else
            ((void)args, ...);
#endif
        }
    }  // namespace tiered_runtime_log
}  // namespace uwvm2::runtime::lazy_validator

#ifndef UWVM_MODULE
// macro
# include <uwvm2/uwvm/runtime/macro/pop_macros.h>
# include <uwvm2/utils/macro/pop_macros.h>
#endif
