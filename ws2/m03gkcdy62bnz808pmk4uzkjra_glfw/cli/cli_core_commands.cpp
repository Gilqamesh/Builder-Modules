#include "cli_application.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <string_view>
#include <thread>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

void initialize_input_state(glfw_api::input_state_t& state) {
    for (
        std::size_t index = 0;
        index < static_cast<std::size_t>(glfw_api::button_t::_button_count);
        ++index
    ) {
        auto& button = state.button_state(static_cast<glfw_api::button_t>(index));
        button.transition_count() = 0;
        button.repeat_count() = 0;
        button.is_down() = false;
    }

    state.cursor_position() = {0.0, 0.0};
    state.scroll_offset() = {0.0, 0.0};
}

void run_interface_self_test() {
    glfw_api::monitor_t monitor;
    monitor.handle(nullptr);
    if (monitor.handle() != nullptr) {
        throw std::logic_error("monitor_t null-handle round trip failed");
    }

    glfw_api::window_creation_settings_t settings;
    settings.reset();

    glfw_api::input_state_t previous_input;
    glfw_api::input_state_t current_input;
    initialize_input_state(previous_input);
    initialize_input_state(current_input);

    auto& current_key = current_input.button_state(glfw_api::button_t::button_a);
    current_key.transition_count() = 1;
    current_key.is_down() = true;
    current_input.cursor_position() = {3.0, 4.0};
    current_input.scroll_offset() = {1.0, -2.0};

    const glfw_api::input_state_change_t input_change(
        previous_input,
        current_input
    );
    if (
        input_change.press_delta(glfw_api::button_t::button_a) != 1 ||
        input_change.release_delta(glfw_api::button_t::button_a) != 0 ||
        input_change.cursor_position_delta()[0] != 3.0 ||
        input_change.cursor_position_delta()[1] != 4.0 ||
        input_change.scroll_offset_delta()[0] != 1.0 ||
        input_change.scroll_offset_delta()[1] != -2.0
    ) {
        throw std::logic_error("input_state_change_t smoke test failed");
    }

    glfw_api::joystick_state_t previous_joystick;
    previous_joystick.axes() = {0.0f};
    previous_joystick.buttons() = {false};
    previous_joystick.hats() = {{0, 0}};

    glfw_api::joystick_state_t current_joystick;
    current_joystick.axes() = {0.5f};
    current_joystick.buttons() = {true};
    current_joystick.hats() = {{1, 0}};

    const glfw_api::joystick_state_change_t joystick_change(
        previous_joystick,
        current_joystick
    );
    if (
        joystick_change.axis_delta(0) != 0.5f ||
        !joystick_change.was_pressed(0) ||
        joystick_change.hat_delta(0)[0] != 1 ||
        joystick_change.hat_delta(0)[1] != 0
    ) {
        throw std::logic_error("joystick_state_change_t smoke test failed");
    }

    glfw_api::gamepad_state_t previous_gamepad;
    glfw_api::gamepad_state_t current_gamepad;
    for (
        std::size_t index = 0;
        index < static_cast<std::size_t>(glfw_api::gamepad_button_t::_button_count);
        ++index
    ) {
        const auto button = static_cast<glfw_api::gamepad_button_t>(index);
        previous_gamepad.button_state(button) = false;
        current_gamepad.button_state(button) = false;
    }
    for (
        std::size_t index = 0;
        index < static_cast<std::size_t>(glfw_api::gamepad_axis_t::_axis_count);
        ++index
    ) {
        const auto axis = static_cast<glfw_api::gamepad_axis_t>(index);
        previous_gamepad.axis_state(axis) = 0.0f;
        current_gamepad.axis_state(axis) = 0.0f;
    }

    current_gamepad.button_state(glfw_api::gamepad_button_t::button_a) = true;
    current_gamepad.axis_state(glfw_api::gamepad_axis_t::axis_left_x) = 0.5f;

    const glfw_api::gamepad_state_change_t gamepad_change(
        previous_gamepad,
        current_gamepad
    );
    if (
        !gamepad_change.was_pressed(glfw_api::gamepad_button_t::button_a) ||
        gamepad_change.axis_delta(glfw_api::gamepad_axis_t::axis_left_x) != 0.5f
    ) {
        throw std::logic_error("gamepad_state_change_t smoke test failed");
    }

    std::cout
        << "self-test passed:\n"
        << "  monitor_t default construction and handle(nullptr)\n"
        << "  window_creation_settings_t construction and reset\n"
        << "  input_state_change_t press, cursor and scroll deltas\n"
        << "  joystick_state_change_t axis, button and hat deltas\n"
        << "  gamepad_state_change_t button and axis deltas\n";

}

} // namespace

void application_t::register_core_commands() {
    m_commands.add(
        {"help"},
        "help [topic ...]",
        "Show core commands or commands below a topic.",
        [this](arguments_t& arguments) {
            m_commands.print_help(arguments.remaining());
        },
        false
    );

    const auto exit_handler = [this](arguments_t& arguments) {
        arguments.expect_end("exit");
        m_running = false;
        std::cout << "Exiting.\n";
    };

    m_commands.add(
        {"exit"},
        "exit",
        "Exit the test CLI.",
        exit_handler,
        false
    );
    m_commands.add(
        {"quit"},
        "quit",
        "Exit the test CLI.",
        exit_handler,
        false
    );

    m_commands.add(
        {"poll"},
        "poll [count]",
        "Poll events and commit window, joystick and gamepad snapshots.",
        [this](arguments_t& arguments) {
            const std::size_t count = arguments.empty()
                ? 1
                : arguments.pop_size("count");
            arguments.expect_end("poll [count]");

            for (std::size_t index = 0; index < count; ++index) {
                poll_once();
            }

            std::cout << std::format("Polled events {} time(s).\n", count);
        },
        false
    );

    m_commands.add(
        {"wait"},
        "wait",
        "Block until an event arrives, then commit all input snapshots.",
        [this](arguments_t& arguments) {
            arguments.expect_end("wait");
            std::cout << "Waiting for one GLFW event...\n";
            wait_once();
            std::cout << "Event received.\n";
        },
        false
    );

    m_commands.add(
        {"wait-timeout"},
        "wait-timeout <seconds>",
        "Wait for an event up to a non-negative timeout in seconds.",
        [this](arguments_t& arguments) {
            const double timeout = arguments.pop_double("seconds");
            arguments.expect_end("wait-timeout <seconds>");
            if (timeout < 0.0) {
                command_error("seconds must be non-negative");
            }

            wait_timeout_once(timeout);
            std::cout << std::format("Wait completed after at most {} second(s).\n", timeout);
        },
        false
    );

    m_commands.add(
        {"post-empty-event"},
        "post-empty-event",
        "Post an empty event and process it in the automatic poll.",
        [](arguments_t& arguments) {
            arguments.expect_end("post-empty-event");
            glfw_api::post_empty_event();
            std::cout << "Posted an empty event.\n";
        }
    );

    m_commands.add(
        {"pump"},
        "pump <milliseconds> [interval-ms]",
        "Poll repeatedly for a bounded period.",
        [this](arguments_t& arguments) {
            const auto duration = std::chrono::milliseconds(
                arguments.pop_long_long("milliseconds")
            );
            const auto interval = std::chrono::milliseconds(
                arguments.empty()
                    ? 16
                    : arguments.pop_long_long("interval-ms")
            );
            arguments.expect_end("pump <milliseconds> [interval-ms]");

            if (duration.count() < 0) {
                command_error("milliseconds must be non-negative");
            }
            if (interval.count() <= 0) {
                command_error("interval-ms must be positive");
            }

            const auto deadline = std::chrono::steady_clock::now() + duration;
            std::size_t poll_count = 0;

            do {
                poll_once();
                ++poll_count;

                const auto now = std::chrono::steady_clock::now();
                if (now >= deadline) {
                    break;
                }

                std::this_thread::sleep_for(std::min(
                    interval,
                    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now)
                ));
            } while (true);

            std::cout << std::format("Pumped events {} time(s).\n", poll_count);
        },
        false
    );

    m_commands.add(
        {"source"},
        "source <file>",
        "Execute a repeatable command script.",
        [this](arguments_t& arguments) {
            const std::filesystem::path path(arguments.pop("file"));
            arguments.expect_end("source <file>");
            run_script(path);
        },
        false
    );

    m_commands.add(
        {"self-test"},
        "self-test",
        "Run safe construction and reset checks that need no window.",
        [](arguments_t& arguments) {
            arguments.expect_end("self-test");
            run_interface_self_test();
        },
        false
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
