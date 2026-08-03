#include "api.h"

#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cli = m03gm33dj5xo77vegpbspger4r_cli;

int main(int argc, char** argv) {
    cli::application_t app;
    app.install_help_command();

    const auto exit = [](cli::context_t& context) {
        context.stop();
        context.out << "Exiting.\n";
    };

    std::vector<cli::command_t> commands{
        {{"exit"}, "Exit the CLI.", exit},
        {{"quit"}, "Exit the CLI.", exit},
        {{"echo"}, "Print the provided tokens after shell-style tokenization.", {cli::argument_t::token("value").optional().variadic()}, [](cli::context_t& context) {
            const std::span<const std::string> tokens = context.arguments.remaining();
            for (std::size_t index = 0; index < tokens.size(); ++index) {
                if (index != 0) {
                    context.out << ' ';
                }
                context.out << tokens[index];
            }
            context.out << '\n';
        }},
        {{"math", "add"}, "Add two integers.", {cli::argument_t::integer("left"), cli::argument_t::integer("right")}, [](cli::context_t& context) {
            auto& arguments = context.arguments;
            const long long left = arguments.pop_long_long("left");
            const long long right = arguments.pop_long_long("right");
            context.out << std::format("{}\n", left + right);
        }},
        {{"set", "mode"}, "Exercise choice validation and completion.", {cli::argument_t::choice("mode", {"fast", "safe", "verbose"})}, [](cli::context_t& context) {
            context.out << std::format("mode={}\n", context.arguments.pop("mode"));
        }}
    };
    for (cli::command_t& command : commands) {
        app.add(std::move(command));
    }

    cli::application_t options;
    options.install_help_command();

    std::string executable(argv[0]);
    std::vector<cli::command_t> option_commands{
        {{"--help"}, "Show executable usage.", [&options, executable = std::move(executable)](cli::context_t& context) {
            context.out << std::format("usage:\n  {}\n  {} <option> [arguments]\n\n", executable, executable);
            options.run_command("help", context.out, context.err);
        }},
        {{"--command"}, "Run one command line.", {cli::argument_t::token("command")}, [&app](cli::context_t& context) {
            app.run_command(context.arguments.pop("command"), context.out, context.err);
        }},
        {{"--script"}, "Run commands from a script file.", {cli::argument_t::file("file")}, [&app](cli::context_t& context) {
            const std::filesystem::path path(std::string(context.arguments.pop("file")));
            std::ifstream input(path);
            if (!input) {
                throw std::invalid_argument(std::format("cannot open script '{}'", path.string()));
            }

            std::string line;
            std::size_t line_number = 0;
            while (app.running() && std::getline(input, line)) {
                ++line_number;
                try {
                    context.out << std::format("{}:{}> {}\n", path.string(), line_number, line);
                    app.run_command(line, context.out, context.err);
                } catch (const std::exception& exception) {
                    throw std::invalid_argument(std::format("{}:{}: {}", path.string(), line_number, exception.what()));
                }
            }
        }},
        {{"--complete"}, "Complete a command line.", {cli::argument_t::token("line"), cli::argument_t::unsigned_integer("cursor").optional()}, [&app](cli::context_t& context) {
            auto& arguments = context.arguments;
            const std::string_view line = arguments.pop("line");
            const std::size_t cursor = arguments.empty() ? std::string_view::npos : arguments.pop_size("cursor");
            for (const std::string& candidate : app.complete_line(line, cursor)) {
                context.out << candidate << '\n';
            }
        }}
    };
    for (cli::command_t& command : option_commands) {
        options.add(std::move(command));
    }

    try {
        if (argc == 1) {
            while (app.running()) {
                std::cout << "cli-demo> " << std::flush;

                std::string line;
                if (!std::getline(std::cin, line)) {
                    std::cout << '\n';
                    break;
                }

                try {
                    app.run_command(line, std::cout, std::cerr);
                } catch (const std::invalid_argument& exception) {
                    std::cerr << std::format("error: {}\n", exception.what());
                } catch (const std::exception& exception) {
                    std::cerr << std::format("exception: {}\n", exception.what());
                }
            }
            return 0;
        }

        const std::vector<std::string> arguments(argv + 1, argv + argc);
        options.run_arguments(arguments, std::cout, std::cerr);
        return 0;
    } catch (const std::invalid_argument& exception) {
        std::cerr << std::format("error: {}\n", exception.what());
        return 2;
    } catch (const std::exception& exception) {
        std::cerr << std::format("fatal: {}\n", exception.what());
        return 1;
    }
}
