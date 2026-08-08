#include <m03gf09la5rvbh6kk4vvt1qawv_module_shell/module_shell.h>
#include <m03gagbhsp2drqq3gkop8pzfrm_workspace_graph/workspace_graph.h>
#include <m03gagbhst621faiop1rztfkqp_builder_cli/builder_cli.h>
#include <m03gm33dj5xo77vegpbspger4r_cli/api.h>
#include <m03gm491bquimk7j45lpvis1yq_cli_shell/api.h>

#include <exception>
#include <format>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace m03gf09la5rvbh6kk4vvt1qawv_module_shell {

namespace {

namespace cli = m03gm33dj5xo77vegpbspger4r_cli;
namespace cli_shell = m03gm491bquimk7j45lpvis1yq_cli_shell;
namespace workspace_graph = m03gagbhsp2drqq3gkop8pzfrm_workspace_graph;
namespace builder_cli = m03gagbhst621faiop1rztfkqp_builder_cli;

using module_name_t = workspace_graph::module_name_t;
using module_names_by_friendly_name_t = std::multimap<std::string, module_name_t>;

std::vector<std::string> complete_friendly_names(const module_names_by_friendly_name_t& module_names, std::string_view partial) {
    std::set<std::string> candidates;
    for (auto iterator = module_names.begin(); iterator != module_names.end(); ++iterator) {
        const std::string& friendly_name = iterator->first;
        if (friendly_name.starts_with(partial)) {
            candidates.insert(friendly_name);
        }
    }
    return {candidates.begin(), candidates.end()};
}

std::vector<std::string> translate_arguments(
    std::span<const std::string> arguments,
    const module_names_by_friendly_name_t& module_names_by_friendly_name
) {
    std::vector<std::string> result;
    result.reserve(arguments.size());
    for (const std::string& argument : arguments) {
        const auto iterator = module_names_by_friendly_name.find(argument);
        if (iterator != module_names_by_friendly_name.end()) {
            result.emplace_back(iterator->second.unique_name());
        } else {
            result.emplace_back(argument);
        }
    }
    return result;
}

cli::argument_t module_argument(const module_names_by_friendly_name_t& module_names_by_friendly_name) {
    return cli::argument_t::custom("argument", [&module_names_by_friendly_name](std::span<const std::string>, std::string_view partial) {
        return complete_friendly_names(module_names_by_friendly_name, partial);
    }).optional().variadic();
}

void run_module(
    cli::context_t& context,
    const module_name_t& module_name,
    const module_names_by_friendly_name_t& module_names_by_friendly_name
) {
    try {
        builder_cli::create_and_wait_checked(
            module_name,
            translate_arguments(context.arguments.remaining(), module_names_by_friendly_name)
        );
    } catch (const std::exception& exception) {
        context.err << std::format("Error: {}\n", exception.what());
    }
}

void add_module_command(
    cli::application_t& app,
    std::string command_name,
    module_name_t module_name,
    std::vector<std::vector<std::string>> aliases,
    bool hidden,
    const module_names_by_friendly_name_t& module_names_by_friendly_name
) {
    cli::command_t command(
        {std::move(command_name)},
        "Run module CLI.",
        {module_argument(module_names_by_friendly_name)},
        [module_name = std::move(module_name), &module_names_by_friendly_name](cli::context_t& context) {
            run_module(context, module_name, module_names_by_friendly_name);
        }
    );
    command.aliases = std::move(aliases);
    command.hidden = hidden;
    app.add(std::move(command));
}

std::set<module_name_t> load_module_names(module_names_by_friendly_name_t& module_names_by_friendly_name) {
    const auto context = workspace_graph::invocation_context();
    workspace_graph::workspace_graph_t workspace_graph(context.workspace_root, context.artifact_root);
    std::set<module_name_t> module_names = workspace_graph.module_names();
    for (const module_name_t& module_name : module_names) {
        module_names_by_friendly_name.emplace(module_name.friendly_name(), module_name);
    }
    return module_names;
}

} // namespace

void run() {
    module_names_by_friendly_name_t module_names_by_friendly_name;
    const std::set<module_name_t> module_names = load_module_names(module_names_by_friendly_name);

    cli::application_t app;
    app.add({
        {"help"},
        "Show shell commands.",
        {module_argument(module_names_by_friendly_name)},
        [](cli::context_t& context) {
            context.out << "Commands:\n";
            context.out << "  help [topic...]\n";
            context.out << "  ls\n";
            context.out << "  <module> [argument...]\n";
        }
    });
    app.add({
        {"ls"},
        "List modules.",
        [&module_names_by_friendly_name](cli::context_t& context) {
            for (auto iterator = module_names_by_friendly_name.begin(); iterator != module_names_by_friendly_name.end(); ++iterator) {
                context.out << std::format("{}\n", iterator->first);
            }
        }
    });

    std::set<std::string> reserved_names{"help", "ls"};
    for (const module_name_t& module_name : module_names) {
        reserved_names.insert(module_name.unique_name());
    }
    for (const module_name_t& module_name : module_names) {
        const std::string friendly_name = module_name.friendly_name();
        const bool has_visible_name = module_names_by_friendly_name.count(friendly_name) == 1 && !reserved_names.contains(friendly_name);
        if (has_visible_name) {
            add_module_command(app, friendly_name, module_name, {{module_name.unique_name()}}, false, module_names_by_friendly_name);
        } else {
            add_module_command(app, module_name.unique_name(), module_name, {}, true, module_names_by_friendly_name);
        }
    }

    cli_shell::shell_t shell(app, "module_shell> ");
    shell.run();
}

} // namespace m03gf09la5rvbh6kk4vvt1qawv_module_shell
