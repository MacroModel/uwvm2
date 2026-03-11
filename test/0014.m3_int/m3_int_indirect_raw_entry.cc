#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
#include <cstring>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

namespace
{
    optable::wasm_u32 last_table_index{};
    void* last_module_handle{};
    void* last_type_handle{};
    void* last_raw_target{};
    void* last_function_handle{};
    void* last_user_data{};
    void* last_entry_handle{};
    optable::m3_stack_t last_stack{};
    optable::m3_control_signal_t observed_signal{optable::m3_control_signal_t::none};
    bool entry_saw_zero_regs{};

    template <typename T>
    inline T load(optable::m3_stack_t base, ::std::ptrdiff_t offset = 0) noexcept
    {
        T out{};
        ::std::memcpy(::std::addressof(out), base + offset, sizeof(T));
        return out;
    }

    void indirect_stub(optable::wasm_u32 table_index, void* module_handle, void* type_handle, optable::m3_stack_t sp,
                       optable::m3_int_register_t& r0, optable::m3_fp_register_t&) noexcept
    {
        last_table_index = table_index;
        last_module_handle = module_handle;
        last_type_handle = type_handle;
        last_stack = sp;
        observed_signal = optable::control_state.signal;
        r0 = static_cast<optable::m3_int_register_t>(static_cast<optable::wasm_i32>(load<optable::wasm_i32>(sp)) + 1);
    }

    void raw_stub(void* raw_target, void* function_handle, void* user_data, optable::m3_stack_t sp,
                  optable::m3_int_register_t&, optable::m3_fp_register_t& fp0) noexcept
    {
        last_raw_target = raw_target;
        last_function_handle = function_handle;
        last_user_data = user_data;
        last_stack = sp;
        observed_signal = optable::control_state.signal;
        fp0 = static_cast<optable::m3_fp_register_t>(5.25);
    }

    void entry_stub(void* function_handle, optable::m3_stack_t sp, optable::m3_int_register_t& r0, optable::m3_fp_register_t& fp0) noexcept
    {
        last_entry_handle = function_handle;
        last_stack = sp;
        observed_signal = optable::control_state.signal;
        entry_saw_zero_regs = (r0 == optable::m3_int_register_t{} && fp0 == optable::m3_fp_register_t{});
        r0 = static_cast<optable::m3_int_register_t>(optable::wasm_i32{9});
        fp0 = static_cast<optable::m3_fp_register_t>(1.75);
    }
}

template <typename T>
inline void store(::std::byte* base, ::std::ptrdiff_t offset, T value) noexcept
{
    ::std::memcpy(base + offset, ::std::addressof(value), sizeof(T));
}

template <typename T>
inline void emit_immediate(::std::byte*& code, T const& value) noexcept
{
    ::std::memcpy(code, ::std::addressof(value), sizeof(T));
    code += sizeof(T);
}

inline void emit_offset(::std::byte*& code, optable::m3_slot_offset_t offset) noexcept
{
    ::std::memcpy(code, ::std::addressof(offset), sizeof(offset));
    code += sizeof(offset);
}

static_assert(optable::translate::get_CallIndirect_fptr() != nullptr);
static_assert(optable::translate::get_CallRawFunction_fptr() != nullptr);
static_assert(optable::translate::get_Entry_fptr() != nullptr);

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_f64 = optable::wasm_f64;

    auto const reset_runtime = []() noexcept {
        optable::indirect_call_func = nullptr;
        optable::raw_call_func = nullptr;
        optable::entry_func = nullptr;
        optable::control_state = {};
        last_table_index = 0;
        last_module_handle = nullptr;
        last_type_handle = nullptr;
        last_raw_target = nullptr;
        last_function_handle = nullptr;
        last_user_data = nullptr;
        last_entry_handle = nullptr;
        last_stack = nullptr;
        observed_signal = optable::m3_control_signal_t::none;
        entry_saw_zero_regs = false;
    };

    {
        reset_runtime();
        optable::indirect_call_func = indirect_stub;
        optable::control_state.signal = optable::m3_control_signal_t::continue_loop;

        int module_token{};
        int type_token{};
        alignas(16) ::std::byte code_buf[64]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 4);
        emit_immediate(code_it, static_cast<void*>(&module_token));
        emit_immediate(code_it, static_cast<void*>(&type_token));
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 4, static_cast<optable::wasm_u32>(7));
        store(stack, 8, wasm_i32{35});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::CallIndirect(ip, stack, r0, fp0);

        if(last_table_index != 7u) { return 1; }
        if(last_module_handle != &module_token) { return 2; }
        if(last_type_handle != &type_token) { return 3; }
        if(last_stack != stack + 8) { return 4; }
        if(observed_signal != optable::m3_control_signal_t::none) { return 5; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{36}) { return 6; }
        if(ip != code_buf + sizeof(optable::m3_slot_offset_t) + sizeof(void*) * 2 + sizeof(optable::m3_slot_offset_t)) { return 7; }
    }

    {
        reset_runtime();
        optable::raw_call_func = raw_stub;
        optable::control_state.signal = optable::m3_control_signal_t::returned;

        int raw_token{};
        int function_token{};
        int user_token{};
        alignas(16) ::std::byte code_buf[64]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<void*>(&raw_token));
        emit_immediate(code_it, static_cast<void*>(&function_token));
        emit_immediate(code_it, static_cast<void*>(&user_token));

        alignas(16) ::std::byte stack[16]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::CallRawFunction(ip, stack, r0, fp0);

        if(last_raw_target != &raw_token) { return 8; }
        if(last_function_handle != &function_token) { return 9; }
        if(last_user_data != &user_token) { return 10; }
        if(last_stack != stack) { return 11; }
        if(observed_signal != optable::m3_control_signal_t::none) { return 12; }
        if(static_cast<wasm_f64>(fp0) != static_cast<wasm_f64>(5.25)) { return 13; }
    }

    {
        reset_runtime();
        optable::entry_func = entry_stub;
        optable::control_state.signal = optable::m3_control_signal_t::continue_loop;

        int function_token{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<void*>(&function_token));

        alignas(16) ::std::byte stack[16]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{123}};
        optable::m3_fp_register_t fp0{static_cast<wasm_f64>(9.0)};
        optable::Entry(ip, stack, r0, fp0);

        if(last_entry_handle != &function_token) { return 14; }
        if(last_stack != stack) { return 15; }
        if(observed_signal != optable::m3_control_signal_t::none) { return 16; }
        if(!entry_saw_zero_regs) { return 17; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{9}) { return 18; }
        if(static_cast<wasm_f64>(fp0) != static_cast<wasm_f64>(1.75)) { return 19; }
        if(ip != code_buf + sizeof(void*)) { return 20; }
    }

    reset_runtime();
    return 0;
}
