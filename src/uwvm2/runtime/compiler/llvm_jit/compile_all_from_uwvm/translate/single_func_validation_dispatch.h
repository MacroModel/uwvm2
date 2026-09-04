// This header is intentionally included inside `validate_runtime_local_func`, where the validation state, bytecode cursor,
// operand stack, control stack, and optional LLVM JIT emit state are all local variables.  Keeping the dispatch body here
// avoids threading a very large state object through every opcode family while still keeping the opcode groups split into
// readable include files.
//
// The switch currently dispatches WebAssembly 1.0/MVP primary opcodes (`wasm1_code`).  When later WebAssembly proposals
// add opcode spaces that are not representable by this one-byte MVP enum, extend this dispatch layer and the included
// opcode-family files together so validation and optional LLVM emission stay in lockstep.
//
// Keep a monolithic opcode switch in the LLVM JIT translator so the host compiler can still lower it into a jump table or
// other efficient dispatch structure, while the per-opcode-family logic lives in smaller headers.

// [before_section ... ] | opbase opextent
// [        safe       ] | unsafe (could be the section_end)
//                         ^^ code_curr

// A WebAssembly function with type `() -> ()` can have no meaningful runtime
// work, but its bytecode stream still must contain a valid terminating `end`.
for(;;)
{
    auto const instruction_begin{code_curr};

    if(code_curr == code_end) [[unlikely]]
    {
        // [... ] | (end)
        // [safe] | unsafe (could be the section_end)
        //          ^^ code_curr

        // Validation completes only after consuming `end`. Reaching the raw end
        // of the bytecode buffer here therefore means the function body is missing
        // its terminating instruction.
        err.err_curr = code_curr;
        err.err_code = ::uwvm2::validation::error::code_validation_error_code::missing_end;
        ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
    }

    // opbase ...
    // [safe] unsafe (could be the section_end)
    // ^^ code_curr

    // The bytecode pointer may be unaligned.  Use memcpy instead of dereferencing a wasm1_code pointer so the dispatch is
    // well-defined on strict-alignment targets.
    wasm1_code curr_opbase;  // no initialization necessary
    ::std::memcpy(::std::addressof(curr_opbase), code_curr, sizeof(wasm1_code));
    // "Inline" here means that this opcode arm emits IR directly while it still owns validation-local immediates.
    // It is not LLVM function inlining: every generated Wasm/public/raw function receives the NoInline attribute.
    bool llvm_jit_instruction_emitted_inline{};
    auto const disable_inline_llvm_jit_emission{[&]() constexpr noexcept
                                                {
                                                    // Validation continues even if native LLVM emission is no longer
                                                    // possible. Clearing the output storage makes full AOT compilation
                                                    // fail instead of publishing a partially emitted module.
                                                    emit_llvm_jit_active = false;
                                                    if(emitted_llvm_jit_ir_storage != nullptr) { *emitted_llvm_jit_ir_storage = {}; }
                                                }};

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wswitch"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wswitch"
#endif
    switch(curr_opbase)
    {
#include "opcode/control_flow_cases.h"
#include "opcode/branch_cases.h"
#include "opcode/call_cases.h"
#include "opcode/variable_cases.h"
#include "opcode/memory_cases.h"
#include "opcode/const_compare_cases.h"
#include "opcode/int_numeric_cases.h"
#include "opcode/float_numeric_convert_cases.h"
#include "opcode/wasm1p1_cases.h"
        [[unlikely]] default:
        {
            // The standard wasm1p1 validator has already accepted this function. Reaching an opcode outside the
            // native LLVM dispatch therefore means the AOT backend has no lowering for it, not that the Wasm is
            // malformed. In an emitting pass, invalidate the whole LLVM module and let materialization report the
            // backend failure; never route the function through the interpreter.
            if(capability_failure != nullptr)
            {
                auto const primary_opcode{static_cast<::std::uint_least32_t>(static_cast<::std::uint_least8_t>(curr_opbase))};
                ::uwvm2::utils::container::u8string_view reason{u8"instruction has no LLVM lowering"};
                ::std::uint_least32_t extended_opcode{};
                bool has_extended_opcode{};

                switch(static_cast<wasm1p1_code>(curr_opbase))
                {
                    case wasm1p1_code::table_get: reason = u8"table.get has no LLVM lowering"; break;
                    case wasm1p1_code::table_set: reason = u8"table.set has no LLVM lowering"; break;
                    case wasm1p1_code::ref_null: reason = u8"ref.null has no LLVM lowering"; break;
                    case wasm1p1_code::ref_is_null: reason = u8"ref.is_null has no LLVM lowering"; break;
                    case wasm1p1_code::ref_func: reason = u8"ref.func has no LLVM lowering"; break;
                    case wasm1p1_code::simd_prefix:
                    {
                        reason = u8"SIMD instruction has no LLVM lowering";

                        // simd_prefix extended_opcode ...
                        // [  safe   ] unsafe (could be the section_end)
                        // ^^ code_curr

                        // The dispatcher has already proved the one-byte prefix at code_curr safe. Therefore code_curr + 1
                        // is inside the closed range ending at code_end (and may equal code_end); read_leb128 performs the
                        // remaining bounds check without advancing the main instruction cursor.
                        auto extended_curr{code_curr + 1u};

                        // simd_prefix extended_opcode ...
                        // [  safe   ] unsafe (could be the section_end)
                        // ^^ code_curr
                        //             ^^ extended_curr

                        extended_opcode = read_leb128.template operator()<validation_module_traits_t::wasm_u32>(
                            extended_curr, code_end, instruction_begin, u8"simd_prefix");

                        // simd_prefix extended_opcode ...
                        // [          safe           ] unsafe (could be the section_end)
                        // ^^ code_curr
                        //                             ^^ extended_curr
                        // read_leb128 commits only extended_curr after the complete ULEB128. Failure leaves it at the
                        // entry position above; success advances only extended_curr, so the main code_curr stays on the prefix.

                        has_extended_opcode = true;
                        break;
                    }
                    default: break;
                }

                report_capability_failure(llvm_jit_capability_failure_kind::instruction,
                                          reason,
                                          instruction_begin,
                                          primary_opcode,
                                          true,
                                          extended_opcode,
                                          has_extended_opcode);
                return;
            }
            if(emitted_llvm_jit_ir_storage != nullptr)
            {
                disable_inline_llvm_jit_emission();
                return;
            }

            err.err_curr = code_curr;
            err.err_selectable.u8 = static_cast<::std::uint_least8_t>(curr_opbase);
            err.err_code = ::uwvm2::validation::error::code_validation_error_code::illegal_opbase;
            ::uwvm2::parser::wasm::base::throw_wasm_parse_code(::fast_io::parse_code::invalid);
            break;
        }
    }
#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__)
# pragma GCC diagnostic pop
#endif

    if(emit_llvm_jit_active && !llvm_jit_instruction_emitted_inline)
    {
        // Most opcode cases validate only and leave IR emission to the single-instruction emitter.  Cases that need
        // validation-local data may emit inline and set `llvm_jit_instruction_emitted_inline` themselves.
        if(!try_emit_runtime_local_func_llvm_jit_instruction(llvm_jit_emit_state, instruction_begin, code_curr)) [[unlikely]]
        {
            disable_inline_llvm_jit_emission();
        }
    }
}
