#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gagbhtahg11wzn32idilzte_module_dependency_ir_from_workspace_graph/module_dependency_ir_from_workspace_graph.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace api = m03gagbhtahg11wzn32idilzte_module_dependency_ir_from_workspace_graph;
namespace filesystem_api = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace graph_api = m03gagbhsp2drqq3gkop8pzfrm_workspace_graph;
namespace ir_api = m03ge9sciyp8y22mzr4nme82tm_module_dependency_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        m_path = std::filesystem::temp_directory_path() / std::format(
            "builder-ir-from-graph-public-api-{}-{}",
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
        throw std::runtime_error("failed to create dependency conversion fixture");
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

void build_empty_plugin(const std::filesystem::path& path) {
    const auto source = path.parent_path() / "empty-builder.c";
    write_file(source, "int empty_builder_plugin;\n");
    const auto command = std::format(
        "cc -shared -fPIC -o '{}' '{}'",
        path.string(),
        source.string()
    );
    if (std::system(command.c_str()) != 0) {
        throw std::runtime_error("failed to build empty builder plugin");
    }
}

const ir_api::workspace_t& find_workspace(
    const ir_api::module_dependency_ir_t& ir,
    std::string_view name
) {
    for (const auto& workspace : ir.workspaces) {
        if (workspace.name == name) {
            return workspace;
        }
    }
    throw std::runtime_error("workspace missing from dependency IR");
}

const ir_api::module_t& find_module(
    const ir_api::workspace_t& workspace,
    std::string_view name
) {
    for (const auto& module : workspace.modules) {
        if (module.name == name) {
            return module;
        }
    }
    throw std::runtime_error("module missing from dependency IR");
}

} // namespace

int main() {
    return test::run([] {
        temporary_directory_t temporary_directory;
        const auto workspace_root = temporary_directory.path() / "workspace";
        const auto artifact_root = temporary_directory.path() / "artifacts";

        const graph_api::module_name_t builder_cli(
            "m03gagbhst621faiop1rztfkqp_builder_cli"
        );
        const graph_api::module_name_t owner(
            "m03gn8rf3pe8dkpk1uwsemhhmd_artifact_store"
        );
        const graph_api::module_name_t module_dependency(
            "m03gagbhsnusi43zogoacgj2ez_filesystem"
        );
        const graph_api::module_name_t builder_dependency(
            "m03gagbhsvr0m5w15urj0o291m_process"
        );
        const graph_api::module_name_t undiscovered(
            "m03ge9ij43jyxy821pda20jhwh_typesystem"
        );

        for (const auto& name : { builder_cli, owner, module_dependency, builder_dependency }) {
            std::filesystem::create_directories(
                workspace_root / "ws0" / name.unique_name()
            );
        }
        std::filesystem::create_directories(
            workspace_root / "ws1" / undiscovered.unique_name()
        );

        write_file(
            workspace_root / "ws0" / owner.unique_name() / "api.cpp",
            std::format("#include <{}/api.h>\n", module_dependency.unique_name())
        );
        write_file(
            workspace_root / "ws0" / owner.unique_name() / "builder.cpp",
            std::format("#include <{}/api.h>\n", builder_dependency.unique_name())
        );
        write_file(
            workspace_root / "ws0" / module_dependency.unique_name() / "api.h",
            "#pragma once\n"
        );
        write_file(
            workspace_root / "ws0" / module_dependency.unique_name() / "builder.cpp",
            ""
        );
        write_file(
            workspace_root / "ws0" / builder_dependency.unique_name() / "api.h",
            "#pragma once\n"
        );
        write_file(
            workspace_root / "ws0" / builder_dependency.unique_name() / "builder.cpp",
            ""
        );

        const auto plugin = artifact_root
            / builder_cli.unique_name()
            / "latest"
            / "builder"
            / "install"
            / "builder.so";
        std::filesystem::create_directories(plugin.parent_path());
        build_empty_plugin(plugin);

        graph_api::workspace_graph_t graph {
            filesystem_api::path_t(workspace_root),
            filesystem_api::path_t(artifact_root)
        };

        const auto initially_empty = api::from_workspace_graph(graph);
        test::expect(std::equal_to<>(), initially_empty.workspaces.size(), std::size_t(2));
        test::expect(std::identity(), find_workspace(initially_empty, "ws0").modules.empty());
        test::expect(std::identity(), find_workspace(initially_empty, "ws1").modules.empty());

        graph.discover_module(module_dependency);
        graph.discover_module(owner);
        graph.discover_module(builder_dependency);

        const auto ir = api::from_workspace_graph(graph);
        test::expect(std::equal_to<>(), ir.workspaces.size(), std::size_t(2));
        const auto& ws0 = find_workspace(ir, "ws0");
        const auto& ws1 = find_workspace(ir, "ws1");
        test::expect(std::equal_to<>(), ws0.modules.size(), std::size_t(3));
        test::expect(std::identity(), ws1.modules.empty());

        const auto& owner_ir = find_module(ws0, owner.unique_name());
        test::expect(std::equal_to<>(), owner_ir.name, owner.unique_name());
        test::expect(std::equal_to<>(), owner_ir.module_dependencies.size(), std::size_t(1));
        test::expect(std::equal_to<>(), owner_ir.module_dependencies[0],
            module_dependency.unique_name()
        );
        test::expect(std::equal_to<>(), owner_ir.builder_dependencies.size(), std::size_t(1));
        test::expect(std::equal_to<>(), owner_ir.builder_dependencies[0],
            builder_dependency.unique_name()
        );

        const auto& module_dependency_ir = find_module(
            ws0,
            module_dependency.unique_name()
        );
        const auto& builder_dependency_ir = find_module(
            ws0,
            builder_dependency.unique_name()
        );
        test::expect(std::identity(), module_dependency_ir.module_dependencies.empty());
        test::expect(std::identity(), module_dependency_ir.builder_dependencies.empty());
        test::expect(std::identity(), builder_dependency_ir.module_dependencies.empty());
        test::expect(std::identity(), builder_dependency_ir.builder_dependencies.empty());
    });
}
