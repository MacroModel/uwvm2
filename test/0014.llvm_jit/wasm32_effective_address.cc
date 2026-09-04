#include <uwvm2/runtime/compiler/llvm_jit/compile_all_from_uwvm/impl.h>
#include <uwvm2/runtime/compiler/uwvm_int/optable/memory.h>

#include <atomic>
#include <bit>
#include <cstdint>

namespace
{
    namespace llvm_details = ::uwvm2::runtime::compiler::llvm_jit::compile_all_from_uwvm::details;
    namespace int_details = ::uwvm2::runtime::compiler::uwvm_int::optable::details;

    using runtime_i32 = llvm_details::runtime_wasm_i32;

    [[nodiscard]] consteval runtime_i32 i32_bits(::std::uint32_t bits) noexcept
    { return ::std::bit_cast<runtime_i32>(bits); }

    consteval bool check_pure_effective_address_helpers() noexcept
    {
        auto const check_one{[](::std::uint32_t address_bits,
                                ::std::uint32_t static_offset,
                                ::std::uint_least64_t expected_offset,
                                bool expected_overflow) constexpr noexcept
                             {
                                 auto const address{i32_bits(address_bits)};
                                 auto const llvm_result{llvm_details::llvm_jit_compute_wasm32_effective_offset(address, static_offset)};
                                 auto const int_result{int_details::wasm32_effective_offset(address, static_offset)};
                                 return llvm_result.offset == expected_offset && llvm_result.offset_65_bit == expected_overflow &&
                                        int_result.offset == expected_offset && int_result.offset_65_bit == expected_overflow;
                             }};

        return check_one(0x80000000u, 0u, 0x80000000ull, false) &&
               check_one(0xffffffffu, 0u, 0xffffffffull, false) &&
               check_one(0xffffffffu, 1u, 0x100000000ull, true) &&
               check_one(0x80000000u, 0xffffffffu, 0x17fffffffull, true);
    }

    static_assert(check_pure_effective_address_helpers());

#if defined(UWVM_SUPPORT_MMAP)
    struct wasm32_full_protection_memory_mock
    {
        inline static constexpr bool can_mmap{true};
        ::std::atomic_size_t* memory_length_p{};
        ::uwvm2::object::memory::linear::mmap_memory_status_t status{
            ::uwvm2::object::memory::linear::mmap_memory_status_t::wasm32};

        [[nodiscard]] constexpr bool require_dynamic_determination_memory_size() const noexcept { return false; }
    };

    consteval bool check_full_protection_overflow_gate() noexcept
    {
        wasm32_full_protection_memory_mock memory{};
        return int_details::should_trap_oob_unlocked(memory, int_details::wasm32_effective_offset(i32_bits(0xffffffffu), 1u), 1u) &&
               !int_details::should_trap_oob_unlocked(memory, int_details::wasm32_effective_offset(i32_bits(0xffffffffu), 0u), 1u);
    }

    static_assert(check_full_protection_overflow_gate());
#endif

    [[nodiscard]] int check_direct_llvm_ir() noexcept
    {
        ::llvm::LLVMContext context{};
        ::llvm::Module module{"wasm32-effective-address", context};
        auto i32_type{::llvm::Type::getInt32Ty(context)};
        auto function_type{::llvm::FunctionType::get(::llvm::Type::getVoidTy(context), {i32_type}, false)};
        auto function{::llvm::Function::Create(function_type, ::llvm::Function::ExternalLinkage, "probe", module)};
        auto block{::llvm::BasicBlock::Create(context, "entry", function)};
        ::llvm::IRBuilder<> builder{block};

        auto effective_offset{llvm_details::emit_llvm_wasm32_effective_offset(builder, function->getArg(0), 1u)};
        auto add{::llvm::dyn_cast_or_null<::llvm::BinaryOperator>(effective_offset)};
        if(add == nullptr || add->getOpcode() != ::llvm::Instruction::Add) { return 1; }
        if(!::llvm::isa<::llvm::ZExtInst>(add->getOperand(0)) || ::llvm::isa<::llvm::SExtInst>(add->getOperand(0))) { return 2; }

        auto out_of_range{llvm_details::emit_llvm_wasm32_effective_offset_out_of_range(builder, effective_offset)};
        auto range_compare{::llvm::dyn_cast_or_null<::llvm::ICmpInst>(out_of_range)};
        if(range_compare == nullptr || range_compare->getPredicate() != ::llvm::ICmpInst::ICMP_UGT) { return 3; }
        auto range_limit{::llvm::dyn_cast<::llvm::ConstantInt>(range_compare->getOperand(1))};
        if(range_limit == nullptr || range_limit->getZExtValue() != 0xffffffffull) { return 4; }

        builder.CreateRetVoid();
        return ::llvm::verifyModule(module, ::std::addressof(::llvm::errs())) ? 5 : 0;
    }
}

int main()
{
    return check_direct_llvm_ir();
}
