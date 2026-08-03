#include "cli_application.h"

#include <cerrno>
#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(__unix__) || defined(__APPLE__)
# include <poll.h>
# include <unistd.h>
#endif

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

application_t::application_t():
    m_monitors(
        "monitor",
        [](const glfw_api::monitor_t& monitor) -> std::optional<std::uintptr_t> {
            if (const GLFWmonitor* handle = monitor.handle()) {
                return reinterpret_cast<std::uintptr_t>(handle);
            }
            return std::nullopt;
        }
    ),
    m_joysticks(
        "joystick",
        [](const glfw_api::joystick_t& joystick) {
            return joystick.id();
        }
    ),
    m_gamepads(
        "gamepad",
        [](const glfw_api::gamepad_t& gamepad) {
            return gamepad.id();
        }
    )
{
    refresh_all(false);
    sample_all_devices();
    register_commands();
}

bool application_t::repl() {
    std::cout
        << "GLFW abstraction test CLI\n"
        << "Use 'help' for command groups or 'help window' for one group.\n";

#if defined(__unix__) || defined(__APPLE__)
    std::cout << "glfw-test> " << std::flush;

    while (m_running) {
        poll_once();

        pollfd descriptor{
            .fd = STDIN_FILENO,
            .events = POLLIN,
            .revents = 0
        };

        const int result = ::poll(&descriptor, 1, 16);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::format(
                "poll(stdin) failed: {}",
                std::strerror(errno)
            ));
        }

        if (result == 0 || (descriptor.revents & (POLLIN | POLLHUP | POLLERR)) == 0) {
            continue;
        }

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        execute_safely(line);
        if (m_running) {
            std::cout << "glfw-test> " << std::flush;
        }
    }
#else
    while (m_running) {
        std::cout << "glfw-test> " << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        execute_safely(line);
    }
#endif

    return true;
}

bool application_t::run_command(std::string_view command) {
    execute(command);
    return m_running;
}

bool application_t::run_script(
    const std::filesystem::path& path,
    bool echo_commands
) {
    std::ifstream input(path);
    if (!input) {
        command_error(std::format("cannot open script '{}'", path.string()));
    }

    std::string line;
    std::size_t line_number = 0;
    while (m_running && std::getline(input, line)) {
        ++line_number;

        try {
            const std::vector<std::string> tokens = tokenize(line);
            if (tokens.empty()) {
                continue;
            }

            if (echo_commands) {
                std::cout << std::format(
                    "{}:{}> {}\n",
                    path.string(),
                    line_number,
                    line
                );
            }

            execute_tokens(tokens);
        } catch (const std::exception& exception) {
            command_error(std::format(
                "{}:{}: {}",
                path.string(),
                line_number,
                exception.what()
            ));
        }
    }

    return m_running;
}

std::vector<std::shared_ptr<glfw_api::monitor_t>> application_t::retained_monitors() const {
    return m_monitors.retained_objects();
}

void application_t::execute_safely(std::string_view line) {
    try {
        execute(line);
    } catch (const command_error_t& exception) {
        std::cerr << std::format("error: {}\n", exception.what());
    } catch (const std::exception& exception) {
        std::cerr << std::format("exception: {}\n", exception.what());
    }
}

void application_t::execute(std::string_view line) {
    const std::vector<std::string> tokens = tokenize(line);
    if (!tokens.empty()) {
        execute_tokens(tokens);
    }
}

void application_t::execute_tokens(std::span<const std::string> tokens) {
    const command_table_t::command_t& command = m_commands.find(tokens);
    arguments_t arguments(tokens.subspan(command.path.size()));

    command.handler(arguments);
    if (m_running && command.poll_after) {
        poll_once();
    }
}

void application_t::register_commands() {
    register_core_commands();
    register_monitor_commands();
    register_device_commands();
    register_window_commands();
    register_settings_commands();
}

void application_t::refresh_monitors(bool announce_changes) {
    m_monitors.refresh(glfw_api::monitors(), announce_changes);
}

void application_t::refresh_all(bool announce_changes) {
    refresh_monitors(announce_changes);
    m_joysticks.refresh(glfw_api::joysticks(), announce_changes);
    m_gamepads.refresh(glfw_api::gamepads(), announce_changes);
}

void application_t::poll_once() {
    glfw_api::poll_events();
    after_event_dispatch();
}

void application_t::wait_once() {
    glfw_api::wait_events();
    after_event_dispatch();
}

void application_t::wait_timeout_once(double timeout) {
    glfw_api::wait_events_timeout(timeout);
    after_event_dispatch();
}

void application_t::after_event_dispatch() {
    refresh_all(true);
    sample_all_devices();
    advance_window_inputs();
}

void application_t::sample_all_devices() {
    for (auto& [id, entry] : m_joysticks.entries()) {
        static_cast<void>(id);
        if (entry.connected && entry.object) {
            sample_joystick(*entry.object);
        }
    }

    for (auto& [id, entry] : m_gamepads.entries()) {
        static_cast<void>(id);
        if (entry.connected && entry.object) {
            sample_gamepad(*entry.object);
        }
    }
}

void application_t::sample_joystick(glfw_api::joystick_t& joystick) {
    glfw_api::poll_joystick(joystick);

    auto& history = joystick.joystick_states();
    history.commit();
}

void application_t::sample_gamepad(glfw_api::gamepad_t& gamepad) {
    glfw_api::poll_gamepad(gamepad);

    auto& history = gamepad.gamepad_states();
    history.commit();
}

void application_t::advance_window_inputs() {
    for (auto& [id, window] : m_windows) {
        static_cast<void>(id);

        auto& history = window->input_states();
        history.commit();
        history.stage() = history.history(0);
    }
}

void application_t::require_history_size(
    std::size_t actual_size,
    std::size_t required_size,
    std::string_view owner
) {
    if (actual_size < required_size) {
        command_error(std::format(
            "{} has {} committed sample(s), but {} are required",
            owner,
            actual_size,
            required_size
        ));
    }
}

std::shared_ptr<glfw_api::window_t> application_t::require_window(id_t id) const {
    const auto iterator = m_windows.find(id);
    if (iterator == m_windows.end()) {
        command_error(std::format("no window with ID {}", id));
    }
    return iterator->second;
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
