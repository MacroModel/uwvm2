#include <uwvm2/runtime/compiler/m3_int/optable/impl.h>

#include <cstddef>
#include <cstring>

namespace optable = ::uwvm2::runtime::compiler::m3_int::optable;

namespace
{
    alignas(16) ::std::byte grow_memory_1[65536]{};
    alignas(16) ::std::byte grow_memory_2[131072]{};
    bool allow_growth{};

    optable::wasm_i32 grow_pages(optable::wasm_i32 pages_to_grow) noexcept
    {
        if(!allow_growth || pages_to_grow != optable::wasm_i32{1} || optable::memory_view.page_count != optable::wasm_u32{1})
        {
            return optable::wasm_i32{-1};
        }

        ::std::memcpy(grow_memory_2, grow_memory_1, sizeof(grow_memory_1));
        optable::memory_view.data = grow_memory_2;
        optable::memory_view.size = sizeof(grow_memory_2);
        optable::memory_view.page_count = optable::wasm_u32{2};
        return optable::wasm_i32{1};
    }
}

template <typename T>
inline void store(::std::byte* base, ::std::ptrdiff_t offset, T value) noexcept
{
    ::std::memcpy(base + offset, ::std::addressof(value), sizeof(T));
}

template <typename T>
inline T load(::std::byte const* base, ::std::ptrdiff_t offset = 0) noexcept
{
    T out{};
    ::std::memcpy(::std::addressof(out), base + offset, sizeof(T));
    return out;
}

inline void emit_offset(::std::byte*& code, optable::m3_slot_offset_t offset) noexcept
{
    ::std::memcpy(code, ::std::addressof(offset), sizeof(offset));
    code += sizeof(offset);
}

static_assert(optable::translate::get_MemSize_fptr() != nullptr);
static_assert(optable::translate::get_MemGrow_fptr() != nullptr);
static_assert(optable::translate::get_MemCopy_fptr() != nullptr);
static_assert(optable::translate::get_MemFill_fptr() != nullptr);

int main()
{
    using wasm_i32 = optable::wasm_i32;
    using wasm_u32 = optable::wasm_u32;

    auto const reset_memory = []() noexcept {
        optable::memory_view = {};
        optable::memory_grow_func = nullptr;
        allow_growth = false;
    };

    {
        alignas(16) ::std::byte memory[64]{};
        optable::memory_view.data = memory;
        optable::memory_view.size = sizeof(memory);
        optable::memory_view.page_size = wasm_u32{16};
        optable::memory_view.page_count = wasm_u32{4};

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{};
        optable::m3_fp_register_t fp0{};
        optable::MemSize(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{4}) { return 1; }
    }

    {
        alignas(16) ::std::byte memory[16]{};
        for(int index{}; index != 6; ++index)
        {
            memory[index] = static_cast<::std::byte>('a' + index);
        }

        optable::memory_view.data = memory;
        optable::memory_view.size = sizeof(memory);
        optable::memory_view.page_size = wasm_u32{16};
        optable::memory_view.page_count = wasm_u32{1};

        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0});
        store(stack, 4, wasm_u32{2});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{4}};
        optable::m3_fp_register_t fp0{};
        optable::MemCopy(ip, stack, r0, fp0);

        if(memory[0] != static_cast<::std::byte>('a')) { return 2; }
        if(memory[1] != static_cast<::std::byte>('b')) { return 3; }
        if(memory[2] != static_cast<::std::byte>('a')) { return 4; }
        if(memory[3] != static_cast<::std::byte>('b')) { return 5; }
        if(memory[4] != static_cast<::std::byte>('c')) { return 6; }
        if(memory[5] != static_cast<::std::byte>('d')) { return 7; }
    }

    {
        alignas(16) ::std::byte memory[16]{};
        optable::memory_view.data = memory;
        optable::memory_view.size = sizeof(memory);
        optable::memory_view.page_size = wasm_u32{16};
        optable::memory_view.page_count = wasm_u32{1};

        alignas(16) ::std::byte code_buf[16]{};
        ::std::byte* code_it{code_buf};
        emit_offset(code_it, 0);
        emit_offset(code_it, 4);

        alignas(16) ::std::byte stack[16]{};
        store(stack, 0, wasm_u32{0x7f});
        store(stack, 4, wasm_u32{5});

        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{3}};
        optable::m3_fp_register_t fp0{};
        optable::MemFill(ip, stack, r0, fp0);

        if(load<::std::uint_least8_t>(memory, 5) != ::std::uint_least8_t{0x7f}) { return 8; }
        if(load<::std::uint_least8_t>(memory, 6) != ::std::uint_least8_t{0x7f}) { return 9; }
        if(load<::std::uint_least8_t>(memory, 7) != ::std::uint_least8_t{0x7f}) { return 10; }
    }

    {
        ::std::memset(grow_memory_1, 0, sizeof(grow_memory_1));
        grow_memory_1[0] = static_cast<::std::byte>(0x5a);
        optable::memory_view.data = grow_memory_1;
        optable::memory_view.size = sizeof(grow_memory_1);
        optable::memory_view.page_size = wasm_u32{65536};
        optable::memory_view.page_count = wasm_u32{1};
        optable::memory_grow_func = grow_pages;
        allow_growth = true;

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{1}};
        optable::m3_fp_register_t fp0{};
        optable::MemGrow(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 11; }
        if(optable::memory_view.page_count != wasm_u32{2}) { return 12; }
        if(optable::memory_view.data != grow_memory_2) { return 13; }
        if(grow_memory_2[0] != static_cast<::std::byte>(0x5a)) { return 14; }

        optable::MemSize(ip, nullptr, r0, fp0);
        if(static_cast<wasm_i32>(r0) != wasm_i32{2}) { return 15; }
    }

    {
        optable::memory_view.data = grow_memory_1;
        optable::memory_view.size = sizeof(grow_memory_1);
        optable::memory_view.page_size = wasm_u32{65536};
        optable::memory_view.page_count = wasm_u32{1};
        optable::memory_grow_func = grow_pages;
        allow_growth = false;

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{1}};
        optable::m3_fp_register_t fp0{};
        optable::MemGrow(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{-1}) { return 16; }
    }

    {
        optable::memory_view.data = grow_memory_1;
        optable::memory_view.size = sizeof(grow_memory_1);
        optable::memory_view.page_size = wasm_u32{65536};
        optable::memory_view.page_count = wasm_u32{1};

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{0}};
        optable::m3_fp_register_t fp0{};
        optable::MemGrow(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{1}) { return 17; }
    }

    {
        optable::memory_view.data = grow_memory_1;
        optable::memory_view.size = sizeof(grow_memory_1);
        optable::memory_view.page_size = wasm_u32{65536};
        optable::memory_view.page_count = wasm_u32{1};

        alignas(16) ::std::byte code_buf[4]{};
        optable::m3_code_t ip{code_buf};
        optable::m3_int_register_t r0{wasm_i32{-1}};
        optable::m3_fp_register_t fp0{};
        optable::MemGrow(ip, nullptr, r0, fp0);

        if(static_cast<wasm_i32>(r0) != wasm_i32{-1}) { return 18; }
    }

    reset_memory();
    return 0;
}
