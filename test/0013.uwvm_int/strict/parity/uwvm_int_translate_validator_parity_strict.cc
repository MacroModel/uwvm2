#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    struct source_sets
    {
        std::set<std::string> mvp{};
        std::set<std::string> basic{};
        std::set<std::string> numeric{};
        std::set<std::string> simd{};
    };

    [[nodiscard]] std::filesystem::path find_repo_root()
    {
        auto curr{std::filesystem::current_path()};
        for(;;)
        {
            auto const marker{curr / "src/uwvm2/validation/standard/wasm1p1/validator.h"};
            if(std::filesystem::exists(marker)) { return curr; }
            if(!curr.has_parent_path() || curr == curr.parent_path()) { break; }
            curr = curr.parent_path();
        }
        return {};
    }

    [[nodiscard]] std::string read_file(std::filesystem::path const& path)
    {
        std::ifstream input(path, std::ios::binary);
        if(!input)
        {
            std::cerr << "failed to open " << path << '\n';
            std::exit(2);
        }
        return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    }

    void collect_with_regex(std::string const& text, std::regex const& pattern, std::set<std::string>& out)
    {
        for(std::sregex_iterator it{text.begin(), text.end(), pattern}, last{}; it != last; ++it)
        {
            auto const& match{*it};
            for(std::size_t i{1}; i != match.size(); ++i)
            {
                if(match[i].matched)
                {
                    out.insert(match[i].str());
                    break;
                }
            }
        }
    }

    [[nodiscard]] source_sets collect_from_files(std::vector<std::filesystem::path> const& files)
    {
        static std::regex const mvp_pattern{
            R"uwvm(case\s+static_cast<wasm_byte>\(wasm1_code::([A-Za-z0-9_]+)\)|case\s+wasm1_code::([A-Za-z0-9_]+))uwvm"};
        static std::regex const basic_pattern{
            R"uwvm(case\s+opcode_byte\(wasm1p1_code::([A-Za-z0-9_]+)\)|case\s+wasm1p1_code::([A-Za-z0-9_]+))uwvm"};
        static std::regex const numeric_pattern{R"uwvm(case\s+wasm1p1_numeric_code::([A-Za-z0-9_]+))uwvm"};
        static std::regex const simd_pattern{R"uwvm(case\s+wasm1p1_simd_code::([A-Za-z0-9_]+))uwvm"};

        source_sets out{};
        for(auto const& file: files)
        {
            auto const text{read_file(file)};
            collect_with_regex(text, mvp_pattern, out.mvp);
            collect_with_regex(text, basic_pattern, out.basic);
            collect_with_regex(text, numeric_pattern, out.numeric);
            collect_with_regex(text, simd_pattern, out.simd);
        }
        return out;
    }

    void print_set(std::set<std::string> const& values)
    {
        for(auto const& value: values) { std::cerr << "  " << value << '\n'; }
    }

    [[nodiscard]] bool compare_one(std::string_view label,
                                   std::set<std::string> const& validator,
                                   std::set<std::string> const& full,
                                   std::set<std::string> const& lazy)
    {
        std::set<std::string> validator_minus_full{};
        std::set<std::string> full_minus_validator{};
        std::set<std::string> validator_minus_lazy{};
        std::set<std::string> lazy_minus_validator{};

        std::ranges::set_difference(validator, full, std::inserter(validator_minus_full, validator_minus_full.end()));
        std::ranges::set_difference(full, validator, std::inserter(full_minus_validator, full_minus_validator.end()));
        std::ranges::set_difference(validator, lazy, std::inserter(validator_minus_lazy, validator_minus_lazy.end()));
        std::ranges::set_difference(lazy, validator, std::inserter(lazy_minus_validator, lazy_minus_validator.end()));

        auto const ok{validator_minus_full.empty() && full_minus_validator.empty() && validator_minus_lazy.empty() &&
                      lazy_minus_validator.empty()};
        std::cerr << label << ": validator=" << validator.size() << " full=" << full.size() << " lazy=" << lazy.size() << '\n';
        if(!ok)
        {
            if(!validator_minus_full.empty())
            {
                std::cerr << label << " missing in full:\n";
                print_set(validator_minus_full);
            }
            if(!full_minus_validator.empty())
            {
                std::cerr << label << " extra in full:\n";
                print_set(full_minus_validator);
            }
            if(!validator_minus_lazy.empty())
            {
                std::cerr << label << " missing in lazy:\n";
                print_set(validator_minus_lazy);
            }
            if(!lazy_minus_validator.empty())
            {
                std::cerr << label << " extra in lazy:\n";
                print_set(lazy_minus_validator);
            }
        }
        return ok;
    }
}  // namespace

int main()
{
    auto const root{find_repo_root()};
    if(root.empty())
    {
        std::cerr << "failed to locate repository root\n";
        return 2;
    }

    std::vector<std::filesystem::path> validator_files{root / "src/uwvm2/validation/standard/wasm1p1/validator.h"};

    std::vector<std::filesystem::path> full_files{root / "src/uwvm2/runtime/compiler/uwvm_int/compile_all_from_uwvm/translate/"
                                                         "single_func_validation_dispatch.h"};
    auto const opcode_dir{root / "src/uwvm2/runtime/compiler/uwvm_int/compile_all_from_uwvm/translate/opcode"};
    for(auto const& entry: std::filesystem::directory_iterator(opcode_dir))
    {
        if(entry.is_regular_file() && entry.path().extension() == ".h") { full_files.push_back(entry.path()); }
    }

    std::vector<std::filesystem::path> lazy_files{root / "src/uwvm2/runtime/compiler/uwvm_int/compile_cu_from_lazy_validator/translate.h"};

    auto const validator{collect_from_files(validator_files)};
    auto const full{collect_from_files(full_files)};
    auto const lazy{collect_from_files(lazy_files)};

    bool ok{true};
    ok = compare_one("mvp", validator.mvp, full.mvp, lazy.mvp) && ok;
    ok = compare_one("basic", validator.basic, full.basic, lazy.basic) && ok;
    ok = compare_one("numeric", validator.numeric, full.numeric, lazy.numeric) && ok;
    ok = compare_one("simd", validator.simd, full.simd, lazy.simd) && ok;
    return ok ? 0 : 1;
}
