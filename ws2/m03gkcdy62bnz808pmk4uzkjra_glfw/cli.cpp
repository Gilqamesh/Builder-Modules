#include "cli/cli_application.h"

// binary_phase_t::install_cli currently compiles only cli.cpp. Keep the CLI
// implementation split by responsibility in source control while compiling it
// as one translation unit. builder.cpp excludes these files from the library.
#include "cli/cli_application.cpp"
#include "cli/cli_core_commands.cpp"
#include "cli/cli_monitor_commands.cpp"
#include "cli/cli_device_commands.cpp"
#include "cli/cli_window_commands.cpp"
#include "cli/cli_settings_commands.cpp"
#include "cli/cli_window_callbacks.cpp"

#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;
namespace glfw_cli = m03gkcdy62bnz808pmk4uzkjra_glfw_cli;

namespace {

void print_usage(std::string_view executable) {
    std::cout << std::format(
        "usage:\n"
        "  {}\n"
        "  {} --script <file>\n"
        "  {} --command <command>\n"
        "  {} --help\n",
        executable,
        executable,
        executable,
        executable
    );
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        print_usage(argv[0]);
        return 0;
    }

    const bool valid_arguments =
        argc == 1 ||
        (argc == 3 && std::string_view(argv[1]) == "--script") ||
        (argc == 3 && std::string_view(argv[1]) == "--command");

    if (!valid_arguments) {
        print_usage(argv[0]);
        return 2;
    }

    std::vector<std::shared_ptr<glfw_api::monitor_t>> retained_monitors;

    try {
        {
            glfw_api::glfw_t glfw;
            glfw_cli::application_t cli;

            if (argc == 1) {
                cli.repl();
            } else if (std::string_view(argv[1]) == "--script") {
                cli.run_script(argv[2]);
            } else {
                cli.run_command(argv[2]);
            }

            // application_t and its windows are destroyed before glfw_t.
            retained_monitors = cli.retained_monitors();
        }

        // Exercise monitor_t's cached snapshot after native handles are invalidated.
        std::cout << std::format(
            "After glfw_t destruction, {} retained monitor object(s):\n",
            retained_monitors.size()
        );
        for (std::size_t index = 0; index < retained_monitors.size(); ++index) {
            std::cout << std::format("  [{}] {}\n", index, *retained_monitors[index]);
        }

        return 0;
    } catch (const glfw_cli::command_error_t& exception) {
        std::cerr << std::format("error: {}\n", exception.what());
        return 2;
    } catch (const std::exception& exception) {
        std::cerr << std::format("fatal: {}\n", exception.what());
        return 1;
    }
}
