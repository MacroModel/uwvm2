#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
#include <cstring>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

namespace
{
    optable::m3_code_t last_call_pc{};
    optable::m3_stack_t last_call_sp{};
    void* last_compile_handle{};
    int loop_iterations{};
    void const* loop_id_to_repeat{};
    void const* propagated_loop_id{};

    template <typename T>
    inline T load(optable::m3_stack_t base, ::std::ptrdiff_t offset = 0) noexcept
    {
        T out{};
        ::std::memcpy(::std::addressof(out), base + offset, sizeof(T));
        return out;
    }

    void call_stub(optable::m3_code_t call_pc, optable::m3_stack_t sp, optable::m3_int_register_t& r0, optable::m3_fp_register_t&) noexcept
    {
        last_call_pc = call_pc;
        last_call_sp = sp;
        if(sp != nullptr)
        {
            r0 = static_cast<optable::m3_int_register_t>(static_cast<optable::wasm_i32>(load<optable::wasm_i32>(sp)) + 1);
        }
        else
        {
            r0 = static_cast<optable::m3_int_register_t>(optable::wasm_i32{77});
        }
    }

    optable::m3_code_t compile_stub(void* handle) noexcept
    {
        last_compile_handle = handle;
        return static_cast<optable::m3_code_t>(handle);
    }

    void repeat_loop_stub(optable::m3_code_t&, optable::m3_stack_t, optable::m3_int_register_t& r0, optable::m3_fp_register_t&) noexcept
    {
        ++loop_iterations;
        if(loop_iterations != 3)
        {
            optable::control_state.signal = optable::m3_control_signal_t::continue_loop;
            optable::control_state.payload = loop_id_to_repeat;
            return;
        }
        r0 = static_cast<optable::m3_int_register_t>(optable::wasm_i32{33});
    }

    void propagate_loop_stub(optable::m3_code_t&, optable::m3_stack_t, optable::m3_int_register_t&, optable::m3_fp_register_t&) noexcept
    {
        ++loop_iterations;
        optable::control_state.signal = optable::m3_control_signal_t::continue_loop;
        optable::control_state.payload = propagated_loop_id;
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

static_assert(optable::translate::get_Call_fptr() != nullptr);
static_assert(optable::translate::get_Compile_fptr() != nullptr);
static_assert(optable::translate::get_Loop_fptr() != nullptr);
static_assert(optable::translate::get_Return_fptr() != nullptr);
static_assert(optable::translate::get_End_fptr() != nullptr);
static_assert(optable::translate::get_ContinueLoop_fptr() != nullptr);
static_assert(optable::translate::get_ContinueLoopIf_fptr() != nullptr);
static_assert(optable::translate::get_Select_i64_fptr<optable::select_shape_t::sss>() != nullptr);
static_assert(optable::translate::get_Select_f64_fptr<optable::select_shape_t::rsr>() != nullptr);

int main()
{
    using wasm_i32 = optable::wasm_i32;

    auto const reset_runtime = []() noexcept {
        optable::call_func = nullptr;
        optable::compile_func = nullptr;
        optable::loop_func = nullptr;
        optable::control_state = {};
        last_call_pc = nullptr;
        last_call_sp = nullptr;
        last_compile_handle = nullptr;
        loop_iterations = 0;
        loop_id_to_repeat = nullptr;
        propagated_loop_id = nullptr;
    };

    {
        reset_runtime();
        optable::call_func = call_stub;

        alignas(16) ::std::byte target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<optable::m3_code_t>(target));
        emit_offset(code_it, 8);

        alignas(16) ::std::byte stack[32]{};
        store(stack, 8, wasm_i32{41});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Call(ip, stack, r0, fp0);

        if(last_call_pc != target) { return 1; }
        if(last_call_sp != stack + 8) { return 2; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{42}) { return 3; }
        if(ip != code_buf + sizeof(optable::m3_code_t) + sizeof(optable::m3_slot_offset_t)) { return 4; }
    }

    {
        reset_runtime();
        optable::call_func = call_stub;

        alignas(16) ::std::byte target[4]{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<optable::m3_code_t>(target));
        emit_offset(code_it, 0);

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Call(ip, nullptr, r0, fp0);

        if(last_call_sp != nullptr) { return 5; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{77}) { return 6; }
    }

    {
        reset_runtime();
        optable::call_func = call_stub;
        optable::compile_func = compile_stub;

        alignas(16) ::std::byte compiled_target[4]{};
        auto* function_handle{static_cast<void*>(compiled_target)};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, function_handle);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 4, wasm_i32{19});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Compile(ip, stack, r0, fp0);

        if(last_compile_handle != function_handle) { return 7; }
        if(last_call_pc != compiled_target) { return 8; }
        if(last_call_sp != stack + 4) { return 9; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{20}) { return 10; }
    }

    {
        reset_runtime();
        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Return(ip, nullptr, r0, fp0);

        if(optable::control_state.signal != optable::m3_control_signal_t::returned) { return 11; }
        if(optable::control_state.payload != nullptr) { return 12; }
    }

    {
        reset_runtime();
        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::End(ip, nullptr, r0, fp0);

        if(optable::control_state.signal != optable::m3_control_signal_t::returned) { return 13; }
    }

    {
        reset_runtime();
        int loop_token{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<void const*>(&loop_token));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::ContinueLoop(ip, nullptr, r0, fp0);

        if(optable::control_state.signal != optable::m3_control_signal_t::continue_loop) { return 14; }
        if(optable::control_state.payload != &loop_token) { return 15; }
    }

    {
        reset_runtime();
        int loop_token{};
        alignas(16) ::std::byte code_buf[32]{};
        ::std::byte* code_it{code_buf};
        emit_immediate(code_it, static_cast<void const*>(&loop_token));

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{0}};
        optable::m3_fp_register_t fp0{};
        optable::ContinueLoopIf(ip, nullptr, r0, fp0);

        if(optable::control_state.signal != optable::m3_control_signal_t::none) { return 16; }

        ip = code_buf;
        r0 = static_cast<optable::m3_int_register_t>(wasm_i32{1});
        optable::ContinueLoopIf(ip, nullptr, r0, fp0);

        if(optable::control_state.signal != optable::m3_control_signal_t::continue_loop) { return 17; }
        if(optable::control_state.payload != &loop_token) { return 18; }
    }

    {
        reset_runtime();
        optable::loop_func = repeat_loop_stub;

        alignas(16) ::std::byte code_buf[4]{};
        loop_id_to_repeat = code_buf;

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Loop(ip, nullptr, r0, fp0);

        if(loop_iterations != 3) { return 19; }
        if(static_cast<wasm_i32>(r0) != wasm_i32{33}) { return 20; }
        if(optable::control_state.signal != optable::m3_control_signal_t::none) { return 21; }
    }

    {
        reset_runtime();
        optable::loop_func = propagate_loop_stub;

        int outer_loop_token{};
        propagated_loop_id = &outer_loop_token;

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::Loop(ip, nullptr, r0, fp0);

        if(loop_iterations != 1) { return 22; }
        if(optable::control_state.signal != optable::m3_control_signal_t::continue_loop) { return 23; }
        if(optable::control_state.payload != &outer_loop_token) { return 24; }
    }

    reset_runtime();
    return 0;
}
