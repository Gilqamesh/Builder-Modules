#include "api.h"

#include <m03gm33dj5xo77vegpbspger4r_cli/api.h>

#include <exception>
#include <format>
#include <iostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

int main() {
    namespace cli = m03gm33dj5xo77vegpbspger4r_cli;
    namespace cli_shell = m03gm491bquimk7j45lpvis1yq_cli_shell;

    try {
        cli::application_t app;
        app.install_help_command();

        const auto exit = [](cli::context_t& context) {
            context.stop();
            context.out << "Exiting.\n";
        };

        std::vector<cli::command_t> commands{
            {{"exit"}, "Exit the shell.", exit},
            {{"quit"}, "Exit the shell.", exit},
            {{"echo"}, "Print the provided tokens.", {cli::argument_t::token("value").optional().variadic()}, [](cli::context_t& context) {
                const std::span<const std::string> values = context.arguments.remaining();
                for (std::size_t index = 0; index < values.size(); ++index) {
                    if (index != 0) {
                        context.out << ' ';
                    }
                    context.out << values[index];
            }
            context.out << '\n';
        }},
        {{"math", "add"}, "Add two integers.", {cli::argument_t::integer("left"), cli::argument_t::integer("right")}, [](cli::context_t& context) {
            auto& arguments = context.arguments;
            const long long left = arguments.pop<long long>("left");
            const long long right = arguments.pop<long long>("right");
            context.out << std::format("{}\n", left + right);
        }},
            {{"set", "mode"}, "Exercise choice completion.", {cli::argument_t::choice("mode", {"fast", "safe", "verbose"})}, [](cli::context_t& context) {
                context.out << std::format("mode={}\n", context.arguments.pop("mode"));
            }}
        };
        for (cli::command_t& command : commands) {
            app.add(std::move(command));
        }

        cli_shell::shell_t shell(app, "cli-shell> ");
        return shell.run();
    } catch (const std::exception& exception) {
        std::cerr << std::format("fatal: {}\n", exception.what());
        return 1;
    }
}
