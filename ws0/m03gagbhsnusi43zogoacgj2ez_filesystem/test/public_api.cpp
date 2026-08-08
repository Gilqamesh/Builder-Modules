#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace api = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        const auto suffix = std::format(
            "{}-{}",
            std::chrono::steady_clock::now().time_since_epoch().count(),
            reinterpret_cast<std::uintptr_t>(this)
        );
        m_path = std::filesystem::temp_directory_path() / ("builder-filesystem-public-api-" + suffix);

        std::error_code error;
        const bool created = std::filesystem::create_directory(m_path, error);
        if (error || !created) {
            throw std::runtime_error("failed to create temporary test directory");
        }
    }

    ~temporary_directory_t() {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    const std::filesystem::path& path() const {
        return m_path;
    }

private:
    std::filesystem::path m_path;
};

class current_path_guard_t {
public:
    current_path_guard_t():
        m_path(std::filesystem::current_path())
    {
    }

    ~current_path_guard_t() {
        std::error_code error;
        std::filesystem::current_path(m_path, error);
    }

private:
    std::filesystem::path m_path;
};

void write_file(const api::path_t& path, std::string_view contents) {
    std::ofstream output(path.to_native_path(), std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to open test file");
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

std::vector<std::string> relative_strings(const std::vector<api::rooted_path_t>& paths) {
    std::vector<std::string> result;
    result.reserve(paths.size());
    for (const auto& path : paths) {
        result.push_back(path.relative_path().string());
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool contains(const std::vector<std::string>& values, std::string_view value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

} // namespace

int main() {
    return test::run([] {
        temporary_directory_t temporary_directory;
        current_path_guard_t current_path_guard;

        const api::path_t root(temporary_directory.path());

        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const api::relative_path_t invalid(std::filesystem::path("/absolute"));
        });

        api::relative_path_t relative("alpha/./beta/../file.txt");
        test::expect_equal(relative.string(), std::string("alpha/file.txt"));
        test::expect_equal(std::string(relative.c_str()), relative.string());
        test::expect_equal(relative.stem(), std::string("file"));
        test::expect_equal(relative.extension(), std::string(".txt"));
        test::expect_equal(&(relative.extension("hpp")), &relative);
        test::expect_equal(relative.string(), std::string("alpha/file.hpp"));
        test::expect_equal(relative.extension(), std::string(".hpp"));
        test::expect_equal(relative.to_native_path(), std::filesystem::path("alpha/file.hpp"));
        test::expect_equal(relative, api::relative_path_t("alpha/file.hpp"));
        test::expect_equal(
            (relative + ".bak").string(),
            std::string("alpha/file.hpp.bak")
        );
        test::expect_equal(
            (api::relative_path_t("alpha") / api::relative_path_t("beta/file.cpp")).string(),
            std::string("alpha/beta/file.cpp")
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = relative + "/suffix";
        });
        test::expect_equal(std::format("{}", relative), relative.string());
        test::expect_equal(
            std::hash<api::relative_path_t>()(relative),
            std::hash<api::relative_path_t>()(api::relative_path_t("alpha/file.hpp"))
        );

        const auto normalized = api::path_t(root.to_native_path() / "a" / "." / "b" / "..");
        test::expect_equal(normalized, root / api::relative_path_t("a"));
        test::expect_equal(normalized.parent(), root);
        test::expect(root.is_child(normalized));
        test::expect(!normalized.is_child(root));
        test::expect(!root.is_child(root));
        test::expect_equal(root.relative(normalized), api::relative_path_t("a"));
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = normalized.relative(root);
        });
        test::expect(normalized.is_sibling(root / api::relative_path_t("other")));
        test::expect(!normalized.is_sibling(root / api::relative_path_t("nested/other")));
        test::expect_equal(normalized.filename(), std::string("a"));
        test::expect_equal(std::string(normalized.c_str()), normalized.string());
        test::expect_equal(normalized.stem(), std::string("a"));
        test::expect_equal(normalized.extension(), std::string());
        test::expect_equal(normalized.to_native_path(), std::filesystem::path(normalized.string()));
        test::expect_equal(std::format("{}", normalized), normalized.string());
        test::expect_equal(
            std::hash<api::path_t>()(normalized),
            std::hash<api::path_t>()(api::path_t(normalized.string()))
        );
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto invalid = api::path_t("/").parent();
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = root / api::relative_path_t("..");
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = root / api::relative_path_t(".");
        });

        auto extension_path = root / api::relative_path_t("name.old");
        test::expect_equal(extension_path.stem(), std::string("name"));
        test::expect_equal(extension_path.extension(), std::string(".old"));
        test::expect_equal(&(extension_path.extension("new")), &extension_path);
        test::expect_equal(extension_path.filename(), std::string("name.new"));
        test::expect_equal(
            (extension_path + ".backup").filename(),
            std::string("name.new.backup")
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = extension_path + "sub/path";
        });

        const auto source_dir = root / api::relative_path_t("source");
        const auto nested_dir = source_dir / api::relative_path_t("nested");
        const auto deep_dir = nested_dir / api::relative_path_t("deep");
        api::create_directories(deep_dir);
        api::create_directories(deep_dir);
        test::expect(api::exists(source_dir));
        test::expect(api::is_directory(source_dir));
        test::expect(!api::is_regular_file(source_dir));

        const auto cpp_file = source_dir / api::relative_path_t("one.cpp");
        const auto c_file = source_dir / api::relative_path_t("two.c");
        const auto h_file = source_dir / api::relative_path_t("three.h");
        const auto hpp_file = source_dir / api::relative_path_t("four.hpp");
        const auto text_file = source_dir / api::relative_path_t("five.txt");
        const auto nested_cpp = nested_dir / api::relative_path_t("nested.cpp");
        const auto deep_header = deep_dir / api::relative_path_t("deep.h");

        api::touch(cpp_file);
        api::touch(c_file);
        api::touch(h_file);
        api::touch(hpp_file);
        write_file(text_file, "hello");
        api::touch(nested_cpp);
        api::touch(deep_header);

        test::expect(api::exists(cpp_file));
        test::expect(api::is_regular_file(cpp_file));
        test::expect(!api::is_directory(cpp_file));
        test::expect_equal(api::file_size(text_file), std::uintmax_t(5));
        test::expect_no_throw([&] { [[maybe_unused]] const auto value = api::last_write_time(text_file); });
        test::expect_no_throw([&] { api::touch(text_file); });
        test::expect(!api::exists(source_dir / api::relative_path_t("missing")));
        test::expect_throws<std::runtime_error>([&] {
            api::touch(root / api::relative_path_t("missing-parent/file"));
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto size = api::file_size(root / api::relative_path_t("missing"));
        });

        const api::rooted_path_t rooted_file(source_dir, api::relative_path_t("one.cpp"));
        test::expect_equal(rooted_file.root(), source_dir);
        test::expect_equal(rooted_file.relative_path(), api::relative_path_t("one.cpp"));
        test::expect_equal(rooted_file.path(), cpp_file);
        test::expect_equal(std::format("{}", rooted_file), cpp_file.string());
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const api::rooted_path_t invalid(
                source_dir,
                api::relative_path_t("missing")
            );
        });

        test::expect(api::find_include_predicate_t::include_all(cpp_file));
        test::expect(api::find_include_predicate_t::is_dir(source_dir));
        test::expect(!api::find_include_predicate_t::is_dir(cpp_file));
        test::expect(api::find_include_predicate_t::is_regular(cpp_file));
        test::expect(api::find_include_predicate_t::cpp_file(cpp_file));
        test::expect(api::find_include_predicate_t::c_file(c_file));
        test::expect(api::find_include_predicate_t::h_file(h_file));
        test::expect(api::find_include_predicate_t::hpp_file(hpp_file));
        test::expect(api::find_include_predicate_t::filename("one.cpp")(cpp_file));
        test::expect(api::find_include_predicate_t::path(cpp_file)(cpp_file));
        test::expect(!api::find_include_predicate_t::path(cpp_file)(c_file));

        const auto source_predicate =
            api::find_include_predicate_t::is_regular
            && (api::find_include_predicate_t::cpp_file || api::find_include_predicate_t::h_file);
        test::expect(source_predicate(cpp_file));
        test::expect(source_predicate(h_file));
        test::expect(!source_predicate(c_file));
        test::expect((!api::find_include_predicate_t::is_dir)(cpp_file));

        const auto direct_entries = relative_strings(api::find(
            source_dir,
            api::find_include_predicate_t::include_all,
            api::find_descend_predicate_t::descend_none
        ));
        test::expect_equal(direct_entries.size(), std::size_t(6));
        test::expect(contains(direct_entries, "one.cpp"));
        test::expect(contains(direct_entries, "nested"));
        test::expect(!contains(direct_entries, "nested/nested.cpp"));

        const auto all_regular = relative_strings(api::find(
            source_dir,
            api::find_include_predicate_t::is_regular,
            api::find_descend_predicate_t::descend_all
        ));
        test::expect_equal(all_regular.size(), std::size_t(7));
        test::expect(contains(all_regular, "nested/nested.cpp"));
        test::expect(contains(all_regular, "nested/deep/deep.h"));

        const auto cpp_files = relative_strings(api::find(
            source_dir,
            api::find_include_predicate_t::cpp_file,
            api::find_descend_predicate_t::descend_all
        ));
        test::expect_equal(cpp_files, std::vector<std::string>({ "nested/nested.cpp", "one.cpp" }));

        api::find_descend_predicate_t first_level_only(
            [](const api::path_t&, std::size_t depth) { return depth == 0; }
        );
        test::expect(first_level_only(nested_dir, 0));
        test::expect(!first_level_only(deep_dir, 1));
        test::expect(api::find_descend_predicate_t::descend_all(nested_dir, 100));
        test::expect(!api::find_descend_predicate_t::descend_none(nested_dir, 0));
        test::expect((first_level_only || api::find_descend_predicate_t::descend_none)(nested_dir, 0));
        test::expect(!(first_level_only && api::find_descend_predicate_t::descend_none)(nested_dir, 0));
        test::expect((!api::find_descend_predicate_t::descend_none)(nested_dir, 0));

        const auto shallow = relative_strings(api::find(
            source_dir,
            api::find_include_predicate_t::is_regular,
            first_level_only
        ));
        test::expect(contains(shallow, "nested/nested.cpp"));
        test::expect(!contains(shallow, "nested/deep/deep.h"));

        const auto copied_file = root / api::relative_path_t("copy/parents/copied.txt");
        api::copy(text_file, copied_file);
        test::expect(api::is_regular_file(copied_file));
        test::expect_equal(api::file_size(copied_file), std::uintmax_t(5));

        const auto copied_directory = root / api::relative_path_t("directory-copy");
        api::copy(nested_dir, copied_directory);
        test::expect(api::is_regular_file(copied_directory / api::relative_path_t("nested.cpp")));
        test::expect(api::is_regular_file(copied_directory / api::relative_path_t("deep/deep.h")));

        const auto file_link = root / api::relative_path_t("file-link");
        api::create_symlink(text_file, file_link);
        test::expect(api::exists(file_link));
        test::expect(api::is_regular_file(file_link));
        test::expect_equal(api::canonical(file_link), api::canonical(text_file));

        const auto directory_link = root / api::relative_path_t("directory-link");
        api::create_directory_symlink(nested_dir, directory_link);
        test::expect(api::exists(directory_link));
        test::expect(api::is_directory(directory_link));
        test::expect_equal(api::canonical(directory_link), api::canonical(nested_dir));
        test::expect_throws<std::runtime_error>([&] {
            api::create_directory_symlink(nested_dir, directory_link);
        });

        const auto found_through_root = relative_strings(api::find(
            root,
            api::find_include_predicate_t::filename("nested.cpp"),
            api::find_descend_predicate_t::descend_all
        ));
        test::expect(contains(found_through_root, "source/nested/nested.cpp"));
        test::expect(!contains(found_through_root, "directory-link/nested.cpp"));

        const auto strict_from = root / api::relative_path_t("strict-from");
        const auto strict_to = root / api::relative_path_t("strict-to");
        api::touch(strict_from);
        api::rename_strict(strict_from, strict_to);
        test::expect(!api::exists(strict_from));
        test::expect(api::exists(strict_to));

        const auto strict_existing = root / api::relative_path_t("strict-existing");
        api::touch(strict_existing);
        test::expect_throws<std::runtime_error>([&] {
            api::rename_strict(strict_to, strict_existing);
        });
        test::expect(api::exists(strict_to));
        test::expect(api::exists(strict_existing));

        const auto replace_from = root / api::relative_path_t("replace-from");
        const auto replace_to = root / api::relative_path_t("replace-to");
        write_file(replace_from, "new");
        write_file(replace_to, "old-value");
        api::rename_replace(replace_from, replace_to);
        test::expect(!api::exists(replace_from));
        test::expect_equal(api::file_size(replace_to), std::uintmax_t(3));

        const auto removable_file = root / api::relative_path_t("removable-file");
        api::touch(removable_file);
        test::expect(api::remove(removable_file));
        test::expect(!api::remove(removable_file));

        const auto removable_tree = root / api::relative_path_t("removable-tree/a/b");
        api::create_directories(removable_tree);
        api::touch(removable_tree / api::relative_path_t("file"));
        test::expect(0 < api::remove_all(root / api::relative_path_t("removable-tree")));
        test::expect_equal(
            api::remove_all(root / api::relative_path_t("removable-tree")),
            std::uintmax_t(0)
        );

        const auto original_current_path = api::current_path();
        api::current_path(root);
        test::expect_equal(api::current_path(), root);
        const api::pretty_path_t pretty_child(source_dir);
        test::expect_equal(pretty_child.string(), std::string("source"));
        test::expect_equal(std::string(pretty_child.c_str()), pretty_child.string());
        test::expect_equal(std::format("{}", pretty_child), std::string("source"));

        const api::pretty_path_t pretty_parent(root.parent());
        test::expect_equal(pretty_parent.string(), root.parent().string());
        api::current_path(original_current_path);
    });
}
