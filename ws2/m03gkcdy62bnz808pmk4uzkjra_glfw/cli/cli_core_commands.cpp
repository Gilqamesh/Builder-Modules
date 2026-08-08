#include "cli_application.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

void initialize_input_state(glfw_api::input_state_t& state) {
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::button_t::_button_count); ++index) {
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

    const glfw_api::input_state_change_t input_change(previous_input, current_input);
    const bool input_change_valid =
        input_change.press_delta(glfw_api::button_t::button_a) == 1 &&
        input_change.release_delta(glfw_api::button_t::button_a) == 0 &&
        input_change.cursor_position_delta()[0] == 3.0 &&
        input_change.cursor_position_delta()[1] == 4.0 &&
        input_change.scroll_offset_delta()[0] == 1.0 &&
        input_change.scroll_offset_delta()[1] == -2.0;
    if (!input_change_valid) {
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

    const glfw_api::joystick_state_change_t joystick_change(previous_joystick, current_joystick);
    const bool joystick_change_valid =
        joystick_change.axis_delta(0) == 0.5f &&
        joystick_change.was_pressed(0) &&
        joystick_change.hat_delta(0)[0] == 1 &&
        joystick_change.hat_delta(0)[1] == 0;
    if (!joystick_change_valid) {
        throw std::logic_error("joystick_state_change_t smoke test failed");
    }

    glfw_api::gamepad_state_t previous_gamepad;
    glfw_api::gamepad_state_t current_gamepad;
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::gamepad_button_t::_button_count); ++index) {
        const auto button = static_cast<glfw_api::gamepad_button_t>(index);
        previous_gamepad.button_state(button) = false;
        current_gamepad.button_state(button) = false;
    }
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::gamepad_axis_t::_axis_count); ++index) {
        const auto axis = static_cast<glfw_api::gamepad_axis_t>(index);
        previous_gamepad.axis_state(axis) = 0.0f;
        current_gamepad.axis_state(axis) = 0.0f;
    }

    current_gamepad.button_state(glfw_api::gamepad_button_t::button_a) = true;
    current_gamepad.axis_state(glfw_api::gamepad_axis_t::axis_left_x) = 0.5f;

    const glfw_api::gamepad_state_change_t gamepad_change(previous_gamepad, current_gamepad);
    const bool gamepad_change_valid =
        gamepad_change.was_pressed(glfw_api::gamepad_button_t::button_a) &&
        gamepad_change.axis_delta(glfw_api::gamepad_axis_t::axis_left_x) == 0.5f;
    if (!gamepad_change_valid) {
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

void validate_event_pump_interval(std::chrono::milliseconds interval) {
    if (interval.count() <= 0) {
        command_error("milliseconds must be positive");
    }
    if (interval.count() > std::numeric_limits<int>::max()) {
        command_error(std::format("milliseconds must be no greater than {}", std::numeric_limits<int>::max()));
    }
}

} // namespace

void application_t::register_core_commands() {
    const auto add_exit_command = [this](std::string name) {
        add_command(
            {name},
            name,
            "Exit the test CLI.",
            [this, name](arguments_t& arguments) {
                arguments.expect_end(name);
                m_commands.stop();
                std::cout << "Exiting.\n";
            },
            false
        );
    };

    add_exit_command("exit");
    add_exit_command("quit");

    add_command(
        {"poll"},
        "poll [count]",
        "Poll events and commit window, joystick and gamepad snapshots.",
        [this](arguments_t& arguments) {
            const std::size_t count = arguments.empty()
                ? 1
                : arguments.pop<std::size_t>("count");
            arguments.expect_end("poll [count]");

            for (std::size_t index = 0; index < count; ++index) {
                poll_once();
            }

            std::cout << std::format("Polled events {} time(s).\n", count);
        },
        {
            argument("count")
        },
        false
    );

    add_command(
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

    add_command(
        {"wait-timeout"},
        "wait-timeout <seconds>",
        "Wait for an event up to a non-negative timeout in seconds.",
        [this](arguments_t& arguments) {
            const double timeout = arguments.pop<double>("seconds");
            arguments.expect_end("wait-timeout <seconds>");
            if (timeout < 0.0) {
                command_error("seconds must be non-negative");
            }

            wait_timeout_once(timeout);
            std::cout << std::format("Wait completed after at most {} second(s).\n", timeout);
        },
        {
            argument("seconds")
        },
        false
    );

    add_command(
        {"post-empty-event"},
        "post-empty-event",
        "Post an empty event and process it in the automatic poll.",
        [](arguments_t& arguments) {
            arguments.expect_end("post-empty-event");
            glfw_api::post_empty_event();
            std::cout << "Posted an empty event.\n";
        }
    );

    add_command(
        {"event-pump", "show"},
        "event-pump show",
        "Show the automatic non-blocking event-pump interval.",
        [this](arguments_t& arguments) {
            arguments.expect_end("event-pump show");
            std::cout << std::format(
                "event-pump interval_ms={}\n",
                m_event_pump_interval.count()
            );
        },
        false
    );

    add_command(
        {"event-pump", "interval"},
        "event-pump interval <milliseconds>",
        "Set the automatic non-blocking event-pump interval.",
        [this](arguments_t& arguments) {
            const auto interval = std::chrono::milliseconds(arguments.pop<long long>("milliseconds"));
            arguments.expect_end("event-pump interval <milliseconds>");
            validate_event_pump_interval(interval);

            m_event_pump_interval = interval;
            m_next_idle_time = std::chrono::steady_clock::now() + m_event_pump_interval;
            std::cout << std::format(
                "event-pump interval_ms={}.\n",
                m_event_pump_interval.count()
            );
        },
        {
            argument("milliseconds")
        },
        false
    );

    add_command(
        {"watch", "start", "window"},
        "watch start window <window-id> <milliseconds>",
        "Start a non-blocking window input watch on each event-pump cycle.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("window-id");
            const auto duration = std::chrono::milliseconds(arguments.pop<long long>("milliseconds"));
            arguments.expect_end("watch start window <window-id> <milliseconds>");

            const id_t watch_id = start_watch(watch_target_t::window_input, id, duration, std::chrono::milliseconds(0));
            std::cout << std::format(
                "Started watch {} for window {} for {} ms on each event-pump cycle.\n",
                watch_id,
                id,
                duration.count()
            );
        },
        {
            window_id_argument(),
            argument("milliseconds")
        }
    );

    const auto add_interval_watch_start = [this](std::string target_name, watch_target_t target, argument_spec_t id_argument) {
        const std::string id_name = target_name + "-id";
        const std::string usage = std::format("watch start {} <{}> <milliseconds> [interval-ms]", target_name, id_name);

        add_command(
            {"watch", "start", std::string_view(target_name)},
            usage,
            "Start a non-blocking device watch that prints retained state history.",
            [this, target_name, target, id_name, usage](arguments_t& arguments) {
                const id_t id = arguments.pop<id_t>(id_name);
                const auto duration = std::chrono::milliseconds(arguments.pop<long long>("milliseconds"));
                const auto interval = std::chrono::milliseconds(
                    arguments.empty()
                        ? m_event_pump_interval.count()
                        : arguments.pop<long long>("interval-ms")
                );
                arguments.expect_end(usage);

                const id_t watch_id = start_watch(target, id, duration, interval);
                std::cout << std::format(
                    "Started watch {} for {} {} for {} ms at {} ms intervals.\n",
                    watch_id,
                    target_name,
                    id,
                    duration.count(),
                    interval.count()
                );
            },
            {
                std::move(id_argument),
                argument("milliseconds"),
                argument("interval-ms")
            }
        );
    };

    add_interval_watch_start("joystick", watch_target_t::joystick, connected_joystick_id_argument("joystick-id"));
    add_interval_watch_start("gamepad", watch_target_t::gamepad, connected_gamepad_id_argument("gamepad-id"));

    add_command(
        {"watch", "list"},
        "watch list",
        "List active non-blocking watches.",
        [this](arguments_t& arguments) {
            arguments.expect_end("watch list");

            if (m_watches.empty()) {
                std::cout << "No active watches.\n";
                return;
            }

            const auto now = std::chrono::steady_clock::now();
            for (const auto& [id, watch] : m_watches) {
                const auto remaining = watch.deadline > now
                    ? std::chrono::duration_cast<std::chrono::milliseconds>(watch.deadline - now)
                    : std::chrono::milliseconds(0);
                const auto next_poll = watch.next_poll_time > now
                    ? std::chrono::duration_cast<std::chrono::milliseconds>(watch.next_poll_time - now)
                    : std::chrono::milliseconds(0);
                const std::string interval = watch_target_uses_interval(watch.target)
                    ? std::format("{}ms", watch.interval.count())
                    : std::format("event-pump({}ms)", m_event_pump_interval.count());

                std::cout << std::format(
                    "{}: target={}, object_id={}, polls={}, interval={}, "
                    "remaining_ms={}, next_poll_ms={}\n",
                    id,
                    watch.target,
                    watch.object_id,
                    watch.poll_count,
                    interval,
                    remaining.count(),
                    next_poll.count()
                );
            }
        },
        false
    );

    add_command(
        {"watch", "set-interval"},
        "watch set-interval <watch-id> <interval-ms>",
        "Set the interval for an active interval-based watch.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("watch-id");
            const auto interval = std::chrono::milliseconds(arguments.pop<long long>("interval-ms"));
            arguments.expect_end("watch set-interval <watch-id> <interval-ms>");

            if (interval.count() <= 0) {
                command_error("interval-ms must be positive");
            }

            auto iterator = m_watches.find(id);
            if (iterator == m_watches.end()) {
                command_error(std::format("no active watch with ID {}", id));
            }

            auto& watch = iterator->second;
            if (!watch_target_uses_interval(watch.target)) {
                command_error(
                    "window watches follow the event-pump interval; "
                    "use 'event-pump interval <milliseconds>'"
                );
            }

            watch.interval = interval;
            watch.next_poll_time = std::chrono::steady_clock::now() + interval;
            std::cout << std::format(
                "watch {} interval_ms={}.\n",
                id,
                interval.count()
            );
        },
        {
            argument(
                "watch-id",
                [this](std::span<const std::string>, std::string_view partial) {
                    return complete_watch_ids(partial, false);
                }
            ),
            argument("interval-ms")
        },
        false
    );

    add_command(
        {"watch", "stop"},
        "watch stop <watch-id>|all",
        "Stop one or every active non-blocking watch.",
        [this](arguments_t& arguments) {
            const std::string_view target = arguments.pop("watch-id or all");
            arguments.expect_end("watch stop <watch-id>|all");

            if (target == "all") {
                const std::size_t count = m_watches.size();
                m_watches.clear();
                std::cout << std::format("Stopped {} watch(es).\n", count);
                return;
            }

            const id_t id = parse_integer<id_t>(target, "watch-id");
            if (m_watches.erase(id) == 0) {
                command_error(std::format("no active watch with ID {}", id));
            }

            std::cout << std::format("Stopped watch {}.\n", id);
        },
        {
            watch_id_or_all_argument()
        },
        false
    );

    add_command(
        {"pump"},
        "pump <milliseconds> [interval-ms]",
        "Poll repeatedly for a bounded period.",
        [this](arguments_t& arguments) {
            const auto duration = std::chrono::milliseconds(arguments.pop<long long>("milliseconds"));
            const auto interval = std::chrono::milliseconds(
                arguments.empty()
                    ? m_event_pump_interval.count()
                    : arguments.pop<long long>("interval-ms")
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
        {
            argument("milliseconds"),
            argument("interval-ms")
        },
        false
    );

    add_command(
        {"source"},
        "source <file>",
        "Execute a repeatable command script.",
        [this](arguments_t& arguments) {
            const std::filesystem::path path = arguments.pop<std::filesystem::path>("file");
            arguments.expect_end("source <file>");
            run_script(path);
        },
        {
            files_argument("file")
        },
        false
    );

    add_command(
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
