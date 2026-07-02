#include <uwvm2/runtime/compiler/uwvm_int/optable/numeric.h>

#include <cstddef>
#include <cstring>

namespace optable = ::uwvm2::runtime::compiler::uwvm_int::optable;

using wasm_i32 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i32;
using wasm_i64 = ::uwvm2::parser::wasm::standard::wasm1::type::wasm_i64;

template <typename T>
inline void write_slot(::std::byte* p, T const& v) noexcept
{ ::std::memcpy(p, ::std::addressof(v), sizeof(T)); }

template <typename T>
inline void push_operand(::std::byte*& sp, T v) noexcept
{
    ::std::memcpy(sp, ::std::addressof(v), sizeof(T));
    sp += sizeof(T);
}

template <typename T>
inline T read_slot(::std::byte const* p) noexcept
{
    T out;  // no init
    ::std::memcpy(::std::addressof(out), p, sizeof(T));
    return out;
}

inline int g_hit{};
inline ::std::byte const* g_ip{};
inline ::std::byte* g_sp{};
inline ::std::byte* g_local_base{};
inline wasm_i32 g_value0{};
inline wasm_i32 g_value1{};
inline wasm_i32 g_value2{};
inline wasm_i64 g_i64_value0{};
inline wasm_i64 g_i64_value1{};

static void reset_state() noexcept
{
    g_hit = 0;
    g_ip = nullptr;
    g_sp = nullptr;
    g_local_base = nullptr;
    g_value0 = 0;
    g_value1 = 0;
    g_value2 = 0;
    g_i64_value0 = 0;
    g_i64_value1 = 0;
}

static void capture_frame(::std::byte* slot_base) noexcept
{
    auto const& frame_slot{*reinterpret_cast<optable::uwvm_interpreter_frame_slot_t const*>(slot_base)};
    g_sp = frame_slot.operand_stack_top;
    g_local_base = frame_slot.local_base;
}

static ::std::byte* slot_base_for(optable::uwvm_interpreter_frame_slot_t& frame_slot,
                                  ::std::byte* sp,
                                  ::std::byte* local_base,
                                  ::std::byte* operand_base) noexcept
{
    frame_slot = optable::uwvm_interpreter_frame_slot_t{.operand_stack_top = sp, .local_base = local_base, .operand_base = operand_base};
    return reinterpret_cast<::std::byte*>(::std::addressof(frame_slot));
}

[[gnu::sysv_abi]] static void end0(::std::byte const* ip, ::std::byte* slot_base) noexcept
{
    g_hit = 10;
    g_ip = ip;
    capture_frame(slot_base);
}

[[gnu::sysv_abi]] static void end1(::std::byte const* ip, ::std::byte* slot_base, wasm_i32 p0) noexcept
{
    g_hit = 11;
    g_ip = ip;
    capture_frame(slot_base);
    g_value0 = p0;
}

[[gnu::sysv_abi]] static void end2(::std::byte const* ip, ::std::byte* slot_base, wasm_i32 p0, wasm_i32 p1) noexcept
{
    g_hit = 12;
    g_ip = ip;
    capture_frame(slot_base);
    g_value0 = p0;
    g_value1 = p1;
}

[[gnu::sysv_abi]] static void end3_i32(::std::byte const* ip, ::std::byte* slot_base, wasm_i32 p0, wasm_i32 p1, wasm_i32 p2) noexcept
{
    g_hit = 13;
    g_ip = ip;
    capture_frame(slot_base);
    g_value0 = p0;
    g_value1 = p1;
    g_value2 = p2;
}

[[gnu::sysv_abi]] static void end2_i64(::std::byte const* ip, ::std::byte* slot_base, wasm_i64 p0, wasm_i64 p1) noexcept
{
    g_hit = 22;
    g_ip = ip;
    capture_frame(slot_base);
    g_i64_value0 = p0;
    g_i64_value1 = p1;
}

inline constexpr optable::uwvm_interpreter_translate_option_t opt_tail{.is_tail_call = true};

static_assert(optable::uwvm_interpreter_anonymous_stacktop_param_begin == 2uz);
static_assert(optable::uwvm_interpreter_anonymous_stacktop_param_max == 999uz);

int main()
{
    using T0 = ::std::byte const*;
    using T1 = ::std::byte*;
    using I0 = wasm_i32;
    using I1 = wasm_i32;
    using I2 = wasm_i32;

    using opfunc0_t = optable::uwvm_interpreter_opfunc_t<T0, T1>;
    using opfunc1_t = optable::uwvm_interpreter_opfunc_t<T0, T1, I0>;
    using opfunc2_t = optable::uwvm_interpreter_opfunc_t<T0, T1, I0, I1>;
    using opfunc3_t = optable::uwvm_interpreter_opfunc_t<T0, T1, I0, I1, I2>;

    static_assert(optable::details::anonymous_stacktop_param_capacity<wasm_i32, T0, T1, I0, I1, I2>() == 3uz);
    static_assert(optable::details::anonymous_stacktop_param_capacity<wasm_i64, T0, T1, I0, I1, I2>() == 0uz);

    // The selector is count-driven. With a three-slot ABI it may choose count 0, 1, 2, or 3.
    {
        opfunc3_t selected = optable::translate::get_uwvmint_i32_binop_anonymous_stacktop_fptr<
            opt_tail,
            optable::numeric_details::int_binop::add,
            3uz,
            T0,
            T1,
            I0,
            I1,
            I2>(3uz);

        opfunc3_t expected = &optable::uwvmint_i32_binop_anonymous_stacktop<
            opt_tail,
            optable::numeric_details::int_binop::add,
            3uz,
            T0,
            T1,
            I0,
            I1,
            I2>;

        if(selected != expected) { return 1; }
    }

    // count=3: the top two anonymous params are consumed and the result is written to the new top.
    {
        reset_state();

        alignas(16)::std::byte instr[sizeof(opfunc3_t) + sizeof(opfunc3_t)]{};
        opfunc3_t fn = &optable::uwvmint_i32_binop_anonymous_stacktop<opt_tail, optable::numeric_details::int_binop::add, 3uz, T0, T1, I0, I1, I2>;
        opfunc3_t end_fn = &end3_i32;
        write_slot(instr, fn);
        write_slot(instr + sizeof(opfunc3_t), end_fn);

        alignas(16)::std::byte mem[32]{};
        ::std::byte* sp = mem;
        ::std::byte* local_base = mem;
        optable::uwvm_interpreter_frame_slot_t frame_slot{};

        fn(instr, slot_base_for(frame_slot, sp, local_base, mem), wasm_i32{100}, wasm_i32{40}, wasm_i32{2});

        if(g_hit != 13) { return 12; }
        if(g_sp != mem || g_local_base != mem) { return 13; }
        if(g_value0 != 100 || g_value1 != 42 || g_value2 != 2) { return 14; }
    }

    // count=2: both operands are passed through anonymous stack-top params.
    {
        reset_state();

        alignas(16)::std::byte instr[sizeof(opfunc2_t) + sizeof(opfunc2_t)]{};
        opfunc2_t fn = &optable::uwvmint_i32_binop_anonymous_stacktop<opt_tail, optable::numeric_details::int_binop::add, 2uz, T0, T1, I0, I1>;
        opfunc2_t end_fn = &end2;
        write_slot(instr, fn);
        write_slot(instr + sizeof(opfunc2_t), end_fn);

        alignas(16)::std::byte mem[32]{};
        ::std::byte* sp = mem;
        ::std::byte* local_base = mem;
        optable::uwvm_interpreter_frame_slot_t frame_slot{};

        fn(instr, slot_base_for(frame_slot, sp, local_base, mem), wasm_i32{40}, wasm_i32{2});

        if(g_hit != 12) { return 2; }
        if(g_ip != instr + sizeof(opfunc2_t)) { return 3; }
        if(g_sp != mem || g_local_base != mem) { return 4; }
        if(g_value0 != 42 || g_value1 != 2) { return 5; }
    }

    // count=1: RHS is an anonymous param, LHS is popped from operand stack memory.
    {
        reset_state();

        alignas(16)::std::byte instr[sizeof(opfunc1_t) + sizeof(opfunc1_t)]{};
        opfunc1_t fn = &optable::uwvmint_i32_binop_anonymous_stacktop<opt_tail, optable::numeric_details::int_binop::add, 1uz, T0, T1, I0>;
        opfunc1_t end_fn = &end1;
        write_slot(instr, fn);
        write_slot(instr + sizeof(opfunc1_t), end_fn);

        alignas(16)::std::byte mem[32]{};
        ::std::byte* sp = mem;
        ::std::byte* local_base = mem;
        optable::uwvm_interpreter_frame_slot_t frame_slot{};
        push_operand(sp, wasm_i32{10});

        fn(instr, slot_base_for(frame_slot, sp, local_base, mem), wasm_i32{7});

        if(g_hit != 11) { return 6; }
        if(g_sp != mem || g_local_base != mem) { return 7; }
        if(g_value0 != 17) { return 8; }
    }

    // count=0: both operands are popped from operand stack memory and the result is pushed back there.
    {
        reset_state();

        alignas(16)::std::byte instr[sizeof(opfunc0_t) + sizeof(opfunc0_t)]{};
        opfunc0_t fn = &optable::uwvmint_i32_binop_anonymous_stacktop<opt_tail, optable::numeric_details::int_binop::add, 0uz, T0, T1>;
        opfunc0_t end_fn = &end0;
        write_slot(instr, fn);
        write_slot(instr + sizeof(opfunc0_t), end_fn);

        alignas(16)::std::byte mem[32]{};
        ::std::byte* sp = mem;
        ::std::byte* local_base = mem;
        optable::uwvm_interpreter_frame_slot_t frame_slot{};
        push_operand(sp, wasm_i32{5});
        push_operand(sp, wasm_i32{8});

        fn(instr, slot_base_for(frame_slot, sp, local_base, mem));

        if(g_hit != 10) { return 9; }
        if(g_sp != mem + sizeof(wasm_i32) || g_local_base != mem) { return 10; }
        if(read_slot<wasm_i32>(mem) != 13) { return 11; }
    }

    // i64 uses the same anonymous-count selector, so the integer window is not i32-only.
    {
        using I3 = wasm_i64;
        using I4 = wasm_i64;
        using i64_opfunc2_t = optable::uwvm_interpreter_opfunc_t<T0, T1, I3, I4>;

        static_assert(optable::details::anonymous_stacktop_param_capacity<wasm_i64, T0, T1, I3, I4>() == 2uz);

        i64_opfunc2_t selected = optable::translate::get_uwvmint_i64_binop_anonymous_stacktop_fptr<
            opt_tail,
            optable::numeric_details::int_binop::add,
            2uz,
            T0,
            T1,
            I3,
            I4>(2uz);

        i64_opfunc2_t expected = &optable::uwvmint_i64_binop_anonymous_stacktop<
            opt_tail,
            optable::numeric_details::int_binop::add,
            2uz,
            T0,
            T1,
            I3,
            I4>;

        if(selected != expected) { return 15; }

        reset_state();

        alignas(16)::std::byte instr[sizeof(i64_opfunc2_t) + sizeof(i64_opfunc2_t)]{};
        i64_opfunc2_t fn = expected;
        i64_opfunc2_t end_fn = &end2_i64;
        write_slot(instr, fn);
        write_slot(instr + sizeof(i64_opfunc2_t), end_fn);

        alignas(16)::std::byte mem[32]{};
        ::std::byte* sp = mem;
        ::std::byte* local_base = mem;
        optable::uwvm_interpreter_frame_slot_t frame_slot{};

        fn(instr, slot_base_for(frame_slot, sp, local_base, mem), wasm_i64{5000000000ll}, wasm_i64{7});

        if(g_hit != 22) { return 16; }
        if(g_sp != mem || g_local_base != mem) { return 17; }
        if(g_i64_value0 != wasm_i64{5000000007ll} || g_i64_value1 != wasm_i64{7}) { return 18; }
    }

    return 0;
}
