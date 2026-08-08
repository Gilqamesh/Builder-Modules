#include "lexer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct module_name_t {
    explicit module_name_t(std::string_view value):
        unique_name(value)
    {
        if (unique_name.empty() || unique_name.front() != 'm' || unique_name.find('_') == std::string::npos) {
            throw std::invalid_argument("invalid module name");
        }
        for (const char c : unique_name) {
            if (c != '_' && !std::isalnum(static_cast<unsigned char>(c))) {
                throw std::invalid_argument("invalid module name");
            }
        }
    }

    std::string unique_name;
};

bool operator==(const module_name_t& lhs, const module_name_t& rhs) {
    return lhs.unique_name == rhs.unique_name;
}

bool operator<(const module_name_t& lhs, const module_name_t& rhs) {
    return lhs.unique_name < rhs.unique_name;
}

std::ostream& operator<<(std::ostream& os, const module_name_t& module_name) {
    return os << module_name.unique_name;
}

struct module_t {
    std::string workspace_name;
    std::filesystem::path workspace_root;
    module_name_t name;

    std::filesystem::path root() const {
        return workspace_root / name.unique_name;
    }
};

struct workspace_t {
    std::string name;
    std::filesystem::path root;
    std::vector<module_t> modules;
};

struct registry_t {
    std::vector<workspace_t> workspaces;

    const module_t* find(const module_name_t& name) const {
        for (const auto& workspace : workspaces) {
            for (const auto& module : workspace.modules) {
                if (module.name == name) {
                    return &module;
                }
            }
        }
        return nullptr;
    }
};

registry_t discover_registry() {
    registry_t registry;
    std::set<module_name_t> seen;

    for (const auto& name : std::array<std::string_view, 3> { "ws0", "ws1", "ws2" }) {
        workspace_t workspace {
            .name = std::string(name),
            .root = std::filesystem::current_path() / name,
            .modules = {}
        };

        for (const auto& entry : std::filesystem::directory_iterator(workspace.root)) {
            if (!entry.is_directory()) {
                continue;
            }

            try {
                module_name_t module_name(entry.path().filename().string());
                if (!seen.insert(module_name).second) {
                    throw std::runtime_error("duplicate module name: " + module_name.unique_name);
                }
                workspace.modules.push_back(module_t {
                    .workspace_name = workspace.name,
                    .workspace_root = workspace.root,
                    .name = std::move(module_name)
                });
            } catch (const std::invalid_argument&) {
            }
        }

        std::sort(workspace.modules.begin(), workspace.modules.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.name < rhs.name;
        });
        registry.workspaces.push_back(std::move(workspace));
    }

    return registry;
}

bool is_source_or_header(const std::filesystem::path& path) {
    const auto extension = path.extension().string();
    return extension == ".cpp" || extension == ".c" || extension == ".h" || extension == ".hpp";
}

std::vector<std::filesystem::path> source_and_header_files(const module_t& module) {
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(module.root())) {
        if (entry.is_regular_file() && is_source_or_header(entry.path())) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

int main(int argc, char** argv) {
    const auto registry = discover_registry();
    const module_name_t root(argc < 2 ? "m03gagbhsujjf63n0w3r2w4q6h_build_phases" : argv[1]);
    const auto* module = registry.find(root);
    if (module == nullptr) {
        throw std::runtime_error("module not found: " + root.unique_name);
    }

    for (const auto& file : source_and_header_files(*module)) {
        std::cout << "  " << std::filesystem::relative(file, module->root()).generic_string() << '\n';

        auto source = std::ifstream(file);
        if (!source) {
            throw std::runtime_error("failed to open " + file.string());
        }

        const auto include_paths = includes(source);
        std::vector<std::filesystem::path> module_dependencies;
        std::vector<std::filesystem::path> regular_includes;
        for (const auto& path : include_paths) {
            if (!path.empty()) {
                try {
                    module_name_t module_name(path.begin()->string());
                    if (registry.find(module_name) != nullptr) {
                        module_dependencies.push_back(path);
                        continue;
                    }
                } catch (const std::invalid_argument&) {
                }
            }

            regular_includes.push_back(path);
        }
        std::cout << "    " << module_dependencies.size() << " module dependencies\n";
        for (const auto& path : module_dependencies) {
            std::cout << "    " << path.generic_string() << '\n';
        }
        std::cout << "    " << regular_includes.size() << " regular includes\n";
        for (const auto& path : regular_includes) {
            std::cout << "    " << path.generic_string() << '\n';
        }
    }

    return 0;
}
