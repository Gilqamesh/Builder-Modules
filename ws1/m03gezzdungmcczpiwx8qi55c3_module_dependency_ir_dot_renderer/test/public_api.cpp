#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gezzdungmcczpiwx8qi55c3_module_dependency_ir_dot_renderer/module_dependency_ir_dot_renderer.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace api = m03gezzdungmcczpiwx8qi55c3_module_dependency_ir_dot_renderer;
namespace filesystem_api = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace ir_api = m03ge9sciyp8y22mzr4nme82tm_module_dependency_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        m_path = std::filesystem::temp_directory_path() / std::format(
            "builder-module-ir-dot-public-api-{}-{}",
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

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to read DOT renderer output");
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
                    .name = "ws\"0\nline",
                    .modules = {
                        ir_api::module_t {
                            .name = "module\\a",
                            .module_dependencies = { "module_b" },
                            .builder_dependencies = { "module_c" }
                        },
                        ir_api::module_t {
                            .name = "module_b",
                            .module_dependencies = {},
                            .builder_dependencies = {}
                        }
                    }
                },
                ir_api::workspace_t {
                    .name = "ws1",
                    .modules = {
                        ir_api::module_t {
                            .name = "module_c",
                            .module_dependencies = { "module_b" },
                            .builder_dependencies = {}
                        }
                    }
                }
            }
        };

        const filesystem_api::path_t output(
            temporary_directory.path() / "graphs" / "modules.dot"
        );
        test::expect(std::equal_to<>(), api::render(ir, "module\\a", output), output);
        test::expect(std::identity(), filesystem_api::is_regular_file(output));

        const auto contents = read_file(output.to_native_path());
        test::expect(std::identity(), contents.starts_with("digraph BuilderModuleDependencyGraph {\n"));
        test::expect(std::identity(), contents.ends_with("}\n"));
        test::expect(std::identity(), contents.find("subgraph cluster_workspace_0") != std::string::npos);
        test::expect(std::identity(), contents.find("subgraph cluster_workspace_1") != std::string::npos);
        test::expect(std::identity(), contents.find("label=\"ws\\\"0\\nline\";") != std::string::npos);
        test::expect(std::identity(), contents.find("m0 [label=\"module\\\\a\"") != std::string::npos);
        test::expect(std::identity(), contents.find("m1 [label=\"module_b\"") != std::string::npos);
        test::expect(std::identity(), contents.find("m2 [label=\"module_c\"") != std::string::npos);
        test::expect(std::identity(), contents.find(
                "m0 [label=\"module\\\\a\", penwidth=2.2, color=\"#111827\", fillcolor=\"#E0F2FE\"]"
            ) != std::string::npos
        );
        test::expect(std::identity(), contents.find("m1 -> m0 [label=\"module\"") != std::string::npos
        );
        test::expect(std::identity(), contents.find("m2 -> m0 [label=\"builder\"") != std::string::npos
        );
        test::expect(std::identity(), contents.find("m1 -> m2 [label=\"module\"") != std::string::npos
        );

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(
                ir,
                "module_a",
                filesystem_api::path_t(temporary_directory.path() / "wrong.svg")
            );
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(ir, "module_a", output);
        });

        const ir_api::module_dependency_ir_t duplicate_ir {
            .workspaces = {
                ir_api::workspace_t {
                    .name = "ws0",
                    .modules = {
                        ir_api::module_t { .name = "duplicate" },
                        ir_api::module_t { .name = "duplicate" }
                    }
                }
            }
        };
        const filesystem_api::path_t duplicate_output(
            temporary_directory.path() / "duplicate.dot"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(
                duplicate_ir,
                "duplicate",
                duplicate_output
            );
        });
        test::expect(std::identity(), !filesystem_api::exists(duplicate_output));

        const ir_api::module_dependency_ir_t missing_dependency_ir {
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
        const filesystem_api::path_t missing_dependency_output(
            temporary_directory.path() / "missing-dependency.dot"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::render(
                missing_dependency_ir,
                "owner",
                missing_dependency_output
            );
        });

        const filesystem_api::path_t empty_output(
            temporary_directory.path() / "empty.dot"
        );
        test::expect(std::equal_to<>(), api::render(ir_api::module_dependency_ir_t {}, "", empty_output),
            empty_output
        );
        test::expect(std::identity(), read_file(empty_output.to_native_path()).find(
                "digraph BuilderModuleDependencyGraph"
            ) != std::string::npos
        );
    });
}
