#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9sciyp8y22mzr4nme82tm_module_dependency_ir/module_dependency_ir.h>

#include <functional>
#include <string>
#include <vector>

namespace api = m03ge9sciyp8y22mzr4nme82tm_module_dependency_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        const api::module_t empty_module;
        test::expect(std::identity(), empty_module.name.empty());
        test::expect(std::identity(), empty_module.module_dependencies.empty());
        test::expect(std::identity(), empty_module.builder_dependencies.empty());

        const api::workspace_t empty_workspace;
        test::expect(std::identity(), empty_workspace.name.empty());
        test::expect(std::identity(), empty_workspace.modules.empty());

        const api::module_dependency_ir_t empty_ir;
        test::expect(std::identity(), empty_ir.workspaces.empty());

        api::module_dependency_ir_t ir {
            .workspaces = {
                api::workspace_t {
                    .name = "ws0",
                    .modules = {
                        api::module_t {
                            .name = "module_a",
                            .module_dependencies = { "module_b", "module_c" },
                            .builder_dependencies = { "module_d" }
                        },
                        api::module_t {
                            .name = "module_b",
                            .module_dependencies = {},
                            .builder_dependencies = {}
                        }
                    }
                },
                api::workspace_t {
                    .name = "ws1",
                    .modules = {
                        api::module_t {
                            .name = "module_c",
                            .module_dependencies = { "module_b" },
                            .builder_dependencies = { "module_a", "module_d" }
                        }
                    }
                }
            }
        };

        test::expect(std::equal_to<>(), ir.workspaces.size(), std::size_t(2));
        test::expect(std::equal_to<>(), ir.workspaces[0].name, std::string("ws0"));
        test::expect(std::equal_to<>(), ir.workspaces[0].modules.size(), std::size_t(2));
        test::expect(std::equal_to<>(), ir.workspaces[0].modules[0].name, std::string("module_a"));
        test::expect(std::identity(), ir.workspaces[0].modules[0].module_dependencies == std::vector<std::string>({ "module_b", "module_c" })
        );
        test::expect(std::identity(), ir.workspaces[0].modules[0].builder_dependencies == std::vector<std::string>({ "module_d" })
        );
        test::expect(std::equal_to<>(), ir.workspaces[1].name, std::string("ws1"));
        test::expect(std::equal_to<>(), ir.workspaces[1].modules[0].name, std::string("module_c"));

        ir.workspaces[1].modules[0].module_dependencies.push_back("module_e");
        test::expect(std::equal_to<>(), ir.workspaces[1].modules[0].module_dependencies.back(),
            std::string("module_e")
        );
    });
}
