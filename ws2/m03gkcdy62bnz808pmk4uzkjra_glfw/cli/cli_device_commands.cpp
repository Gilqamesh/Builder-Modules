#include "cli_application.h"

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

void application_t::register_device_commands() {
    const auto print_joystick_delta = [this](id_t id) {
        const auto& history = m_joysticks.require(id, false).object->joystick_states();
        require_history_size(history.history_size(), 2, std::format("joystick {}", id));
        std::cout << std::format(
            "{}\n",
            glfw_api::joystick_state_change_t(history.history(1), history.history(0))
        );
    };

    const auto print_gamepad_delta = [this](id_t id) {
        const auto& history = m_gamepads.require(id, false).object->gamepad_states();
        require_history_size(history.history_size(), 2, std::format("gamepad {}", id));
        std::cout << std::format(
            "{}\n",
            glfw_api::gamepad_state_change_t(history.history(1), history.history(0))
        );
    };

    const auto apply_gamepad_mapping = [this](std::string mapping) {
        if (mapping.empty()) {
            command_error("mapping string must not be empty");
        }

        glfw_api::update_joystick_to_gamepad_mapping(glfw_api::joystick_to_gamepad_mapping_t(mapping));
        refresh_all(true);
        sample_all_devices();
        std::cout << "Gamepad mapping updated.\n";
    };

    add_command(
        {"joystick", "refresh"},
        "joystick refresh",
        "Refresh stable IDs and report connection changes.",
        [this](arguments_t& arguments) {
            arguments.expect_end("joystick refresh");
            m_joysticks.refresh(glfw_api::joysticks(), true);
            std::cout << "joystick registry refreshed.\n";
        },
        false
    );

    add_command(
        {"joystick", "list"},
        "joystick list",
        "List connected devices and retained disconnected snapshots.",
        [this](arguments_t& arguments) {
            arguments.expect_end("joystick list");
            m_joysticks.refresh(glfw_api::joysticks(), false);

            if (m_joysticks.entries().empty()) {
                std::cout << "No joystick objects have been observed.\n";
                return;
            }

            for (const auto& [id, entry] : m_joysticks.entries()) {
                std::cout << std::format(
                    "joystick {} [{}]: {}\n",
                    id,
                    entry.connected ? "connected" : "disconnected snapshot",
                    *entry.object
                );
            }
        }
    );

    add_command(
        {"joystick", "show"},
        "joystick show <joystick-id>",
        "Show the complete device object, including retained history.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick show <joystick-id>");

            const auto& entry = m_joysticks.require(id, false);
            std::cout << std::format(
                "joystick {} [{}]: {}\n",
                id,
                entry.connected ? "connected" : "disconnected snapshot",
                *entry.object
            );
        },
        {
            joystick_id_argument("joystick-id")
        }
    );

    add_command(
        {"joystick", "poll"},
        "joystick poll <joystick-id>",
        "Poll and commit one state snapshot immediately.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick poll <joystick-id>");

            auto& joystick = *m_joysticks.require(id, true).object;
            glfw_api::poll_joystick(joystick);
            auto& history = joystick.joystick_states();
            history.commit();

            std::cout << std::format("joystick {} history[0]: {}\n", id, history.history(0));
        },
        {
            connected_joystick_id_argument("joystick-id")
        },
        false
    );

    add_command(
        {"joystick", "state"},
        "joystick state <joystick-id>",
        "Show the newest committed state.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick state <joystick-id>");

            const auto& history = m_joysticks.require(id, false).object->joystick_states();
            require_history_size(history.history_size(), 1, std::format("joystick {}", id));
            std::cout << std::format("{}\n", history.history(0));
        },
        {
            joystick_id_argument("joystick-id")
        }
    );

    add_command(
        {"joystick", "state-delta"},
        "joystick state-delta <joystick-id>",
        "Show the change from history[1] to history[0].",
        [print_joystick_delta](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick state-delta <joystick-id>");
            print_joystick_delta(id);
        },
        {
            joystick_id_argument("joystick-id")
        }
    );

    add_command(
        {"joystick", "delta"},
        "joystick delta <joystick-id>",
        "Alias for state-delta.",
        [print_joystick_delta](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick delta <joystick-id>");
            print_joystick_delta(id);
        },
        {
            joystick_id_argument("joystick-id")
        }
    );

    add_command(
        {"joystick", "history"},
        "joystick history <joystick-id>",
        "Show every retained state.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            arguments.expect_end("joystick history <joystick-id>");
            std::cout << std::format(
                "joystick {} history: {}\n",
                id,
                m_joysticks.require(id, false).object->joystick_states()
            );
        },
        {
            joystick_id_argument("joystick-id")
        }
    );

    add_command(
        {"joystick", "history-size"},
        "joystick history-size <joystick-id> <sample-count>",
        "Replace retained state history with a new sample capacity.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("joystick-id");
            const std::size_t sample_count = arguments.pop<std::size_t>("sample-count");
            arguments.expect_end("joystick history-size <joystick-id> <sample-count>");

            if (sample_count == 0) {
                command_error("sample-count must be positive");
            }

            auto& joystick = *m_joysticks.require(id, true).object;
            joystick.joystick_states() = glfw_api::joystick_t::joystick_states_t(sample_count);
            glfw_api::poll_joystick(joystick);
            auto& history = joystick.joystick_states();
            history.commit();

            std::cout << std::format(
                "joystick {} history capacity set to {}; samples={}\n",
                id,
                history.history_capacity(),
                history.history_size()
            );
        },
        {
            connected_joystick_id_argument("joystick-id"),
            argument("sample-count")
        },
        false
    );

    add_command(
        {"gamepad", "refresh"},
        "gamepad refresh",
        "Refresh stable IDs and report connection changes.",
        [this](arguments_t& arguments) {
            arguments.expect_end("gamepad refresh");
            m_gamepads.refresh(glfw_api::gamepads(), true);
            std::cout << "gamepad registry refreshed.\n";
        },
        false
    );

    add_command(
        {"gamepad", "list"},
        "gamepad list",
        "List connected devices and retained disconnected snapshots.",
        [this](arguments_t& arguments) {
            arguments.expect_end("gamepad list");
            m_gamepads.refresh(glfw_api::gamepads(), false);

            if (m_gamepads.entries().empty()) {
                std::cout << "No gamepad objects have been observed.\n";
                return;
            }

            for (const auto& [id, entry] : m_gamepads.entries()) {
                std::cout << std::format(
                    "gamepad {} [{}]: {}\n",
                    id,
                    entry.connected ? "connected" : "disconnected snapshot",
                    *entry.object
                );
            }
        }
    );

    add_command(
        {"gamepad", "show"},
        "gamepad show <gamepad-id>",
        "Show the complete device object, including retained history.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad show <gamepad-id>");

            const auto& entry = m_gamepads.require(id, false);
            std::cout << std::format(
                "gamepad {} [{}]: {}\n",
                id,
                entry.connected ? "connected" : "disconnected snapshot",
                *entry.object
            );
        },
        {
            gamepad_id_argument("gamepad-id")
        }
    );

    add_command(
        {"gamepad", "poll"},
        "gamepad poll <gamepad-id>",
        "Poll and commit one state snapshot immediately.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad poll <gamepad-id>");

            auto& gamepad = *m_gamepads.require(id, true).object;
            glfw_api::poll_gamepad(gamepad);
            auto& history = gamepad.gamepad_states();
            history.commit();

            std::cout << std::format("gamepad {} history[0]: {}\n", id, history.history(0));
        },
        {
            connected_gamepad_id_argument("gamepad-id")
        },
        false
    );

    add_command(
        {"gamepad", "state"},
        "gamepad state <gamepad-id>",
        "Show the newest committed state.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad state <gamepad-id>");

            const auto& history = m_gamepads.require(id, false).object->gamepad_states();
            require_history_size(history.history_size(), 1, std::format("gamepad {}", id));
            std::cout << std::format("{}\n", history.history(0));
        },
        {
            gamepad_id_argument("gamepad-id")
        }
    );

    add_command(
        {"gamepad", "state-delta"},
        "gamepad state-delta <gamepad-id>",
        "Show the change from history[1] to history[0].",
        [print_gamepad_delta](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad state-delta <gamepad-id>");
            print_gamepad_delta(id);
        },
        {
            gamepad_id_argument("gamepad-id")
        }
    );

    add_command(
        {"gamepad", "delta"},
        "gamepad delta <gamepad-id>",
        "Alias for state-delta.",
        [print_gamepad_delta](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad delta <gamepad-id>");
            print_gamepad_delta(id);
        },
        {
            gamepad_id_argument("gamepad-id")
        }
    );

    add_command(
        {"gamepad", "history"},
        "gamepad history <gamepad-id>",
        "Show every retained state.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            arguments.expect_end("gamepad history <gamepad-id>");
            std::cout << std::format(
                "gamepad {} history: {}\n",
                id,
                m_gamepads.require(id, false).object->gamepad_states()
            );
        },
        {
            gamepad_id_argument("gamepad-id")
        }
    );

    add_command(
        {"gamepad", "history-size"},
        "gamepad history-size <gamepad-id> <sample-count>",
        "Replace retained state history with a new sample capacity.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("gamepad-id");
            const std::size_t sample_count = arguments.pop<std::size_t>("sample-count");
            arguments.expect_end("gamepad history-size <gamepad-id> <sample-count>");

            if (sample_count == 0) {
                command_error("sample-count must be positive");
            }

            auto& gamepad = *m_gamepads.require(id, true).object;
            gamepad.gamepad_states() = glfw_api::gamepad_t::gamepad_states_t(sample_count);
            glfw_api::poll_gamepad(gamepad);
            auto& history = gamepad.gamepad_states();
            history.commit();

            std::cout << std::format(
                "gamepad {} history capacity set to {}; samples={}\n",
                id,
                history.history_capacity(),
                history.history_size()
            );
        },
        {
            connected_gamepad_id_argument("gamepad-id"),
            argument("sample-count")
        },
        false
    );

    add_command(
        {"gamepad", "mapping", "update"},
        "gamepad mapping update <mapping-string>",
        "Update GLFW gamepad mappings from one mapping string.",
        [apply_gamepad_mapping](arguments_t& arguments) {
            std::string mapping = arguments.pop<std::string>("mapping-string");
            arguments.expect_end("gamepad mapping update <mapping-string>");

            apply_gamepad_mapping(std::move(mapping));
        },
        {
            argument("mapping-string")
        },
        false
    );

    add_command(
        {"gamepad", "mapping", "update-file"},
        "gamepad mapping update-file <file>",
        "Update GLFW gamepad mappings from a mapping file.",
        [apply_gamepad_mapping](arguments_t& arguments) {
            const std::filesystem::path path = arguments.pop<std::filesystem::path>("file");
            arguments.expect_end("gamepad mapping update-file <file>");

            std::ifstream input(path);
            if (!input) {
                command_error(std::format("cannot open gamepad mapping file '{}'", path.string()));
            }

            std::string mapping{
                std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>()
            };
            if (input.bad()) {
                command_error(std::format("failed to read gamepad mapping file '{}'", path.string()));
            }

            apply_gamepad_mapping(std::move(mapping));
        },
        {
            files_argument("file")
        },
        false
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
