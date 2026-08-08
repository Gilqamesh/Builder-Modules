#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9zyrjajugagmp61034qhi_module_dependency_ir_svg_renderer/module_dependency_ir_svg_renderer.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace api = m03ge9zyrjajugagmp61034qhi_module_dependency_ir_svg_renderer;
namespace filesystem_api = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace ir_api = m03ge9sciyp8y22mzr4nme82tm_module_dependency_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        m_path = std::filesystem::temp_directory_path() / std::format(
            "builder-module-ir-svg-public-api-{}-{}",
            std::chrono::steady_clock::now().time_since_epoch().count(),
            reinterpret_cast<std::uintptr_t>(this)
        );
        std::filesystem::create_directory(m_path);
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

void write_file(const std::filesystem::path& path, std::string_view contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to create dependency SVG fixture");
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to read dependency SVG output");
    }
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
}

} // namespace

int main() {
    return test::run([] {
        temporary_directory_t temporary_directory;
        const ir_api::module_dependency_ir_t ir {
            .workspaces = {
                ir_api::workspace_t {
                    .name = "ws0",
                    .modules = {
                        ir_api::module_t {
                            .name = "module_a",
                            .module_dependencies = { "module_b" },
                            .builder_dependencies = {}
                        },
                        ir_api::module_t {
                            .name = "module_b",
                            .module_dependencies = {},
                            .builder_dependencies = {}
                        }
                    }
                }
            }
        };

        const filesystem_api::path_t output(
            temporary_directory.path() / "graphs" / "modules.svg"
        );
        const auto temporary_dot = output + "_tmp.dot";
        write_file(temporary_dot.to_native_path(), "stale temporary file");

        test::expect_equal(api::render(ir, "module_a", output), output);
        test::expect(filesystem_api::is_regular_file(output));
        test::expect(!filesystem_api::exists(temporary_dot));
        const auto contents = read_file(output.to_native_path());
        test::expect(contents.find("<svg") != std::string::npos);
        test::expect(contents.find("module_a") != std::string::npos);
        test::expect(contents.find("module_b") != std::string::npos);

        const filesystem_api::path_t wrong_extension(
            temporary_directory.path() / "wrong.png"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(
                ir,
                "module_a",
                wrong_extension
            );
        });
        test::expect(!filesystem_api::exists(wrong_extension + "_tmp.dot"));

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(ir, "module_a", output);
        });
        test::expect(!filesystem_api::exists(temporary_dot));

        const ir_api::module_dependency_ir_t invalid_ir {
            .workspaces = {
                ir_api::workspace_t {
                    .name = "ws0",
                    .modules = {
                        ir_api::module_t {
                            .name = "owner",
                            .module_dependencies = { "missing" },
                            .builder_dependencies = {}
                        }
                    }
                }
            }
        };
        const filesystem_api::path_t invalid_output(
            temporary_directory.path() / "invalid.svg"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(
                invalid_ir,
                "owner",
                invalid_output
            );
        });
        test::expect(!filesystem_api::exists(invalid_output));
        test::expect(!filesystem_api::exists(invalid_output + "_tmp.dot"));

        const filesystem_api::path_t empty_output(
            temporary_directory.path() / "empty.svg"
        );
        test::expect_equal(
            api::render(ir_api::module_dependency_ir_t {}, "", empty_output),
            empty_output
        );
        test::expect(read_file(empty_output.to_native_path()).find("<svg") != std::string::npos);
        test::expect(!filesystem_api::exists(empty_output + "_tmp.dot"));
    });
}
