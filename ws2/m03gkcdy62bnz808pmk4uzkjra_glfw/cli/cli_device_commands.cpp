#include "cli_application.h"
#include "cli_history.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

template <typename Mapping>
concept supports_direct_gamepad_mapping_update = requires(const Mapping& mapping) {
    glfw_api::update_joystick_to_gamepad_mapping(mapping);
};

template <typename Mapping>
requires supports_direct_gamepad_mapping_update<Mapping>
void update_gamepad_mapping_impl(const Mapping& mapping) {
    glfw_api::update_joystick_to_gamepad_mapping(mapping);
}

template <typename Mapping>
requires (!supports_direct_gamepad_mapping_update<Mapping>)
void update_gamepad_mapping_impl(const Mapping& mapping) {
    glfw_api::update_joystick_to_gamepad_mapping(
        glfw_api::joystick_to_gamepad_mapping_t(std::string(mapping))
    );
}

void update_gamepad_mapping(std::string mapping) {
    if (mapping.empty()) {
        command_error("mapping string must not be empty");
    }

    update_gamepad_mapping_impl(mapping);
}

std::string read_gamepad_mapping_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        command_error(std::format("cannot open gamepad mapping file '{}'", path.string()));
    }

    std::string contents{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    if (input.bad()) {
        command_error(std::format("failed to read gamepad mapping file '{}'", path.string()));
    }

    return contents;
}

} // namespace

void application_t::register_device_commands() {
    const auto register_group = [this](
        std::string group,
        auto& registry,
        auto enumerate,
        auto poll_device,
        auto history_of,
        auto resize_history,
        auto make_change
    ) {
        auto* const registry_pointer = &registry;

        const auto print_all_changes = [group, history_of, make_change](
            id_t id,
            auto& device
        ) {
            const std::string heading = std::format(
                "[{} {}] retained history",
                group,
                id
            );
            m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history_changes(
                heading,
                history_of(device),
                make_change
            );
        };

        const auto print_complete_history = [group, history_of, make_change](
            id_t id,
            auto& device
        ) {
            const std::string heading = std::format("{} {} history", group, id);
            m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history(
                heading,
                history_of(device),
                make_change
            );
        };

        m_commands.add(
            {group, "refresh"},
            group + " refresh",
            "Refresh stable IDs and report connection changes.",
            [registry_pointer, enumerate, group](arguments_t& arguments) {
                arguments.expect_end(group + " refresh");
                registry_pointer->refresh(enumerate(), true);
                std::cout << std::format("{} registry refreshed.\n", group);
            },
            false
        );

        m_commands.add(
            {group, "list"},
            group + " list",
            "List connected devices and retained disconnected snapshots.",
            [registry_pointer, enumerate, history_of, group](arguments_t& arguments) {
                arguments.expect_end(group + " list");
                registry_pointer->refresh(enumerate(), false);

                if (registry_pointer->entries().empty()) {
                    std::cout << std::format("No {} objects have been observed.\n", group);
                    return;
                }

                for (const auto& [id, entry] : registry_pointer->entries()) {
                    const auto& history = history_of(*entry.object);
                    const std::string native_id = entry.object->id()
                        ? std::to_string(*entry.object->id())
                        : "null";
                    std::cout << std::format(
                        "{}: {}, native_id={}, name={}, samples={}/{}\n",
                        id,
                        entry.connected ? "connected" : "disconnected snapshot",
                        native_id,
                        quote_token(entry.object->name()),
                        history.size(),
                        history.capacity()
                    );
                }
            }
        );

        m_commands.add(
            {group, "show"},
            group + " show <" + group + "-id>",
            "Show the complete device object, including retained history.",
            [registry_pointer, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " show <" + group + "-id>");

                const auto& entry = registry_pointer->require(id, false);
                std::cout << std::format(
                    "{} {} [{}]: {}\n",
                    group,
                    id,
                    entry.connected ? "connected" : "disconnected snapshot",
                    *entry.object
                );
            }
        );

        m_commands.add(
            {group, "poll"},
            group + " poll <" + group + "-id>",
            "Poll and commit one state snapshot immediately.",
            [registry_pointer, poll_device, history_of, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " poll <" + group + "-id>");

                auto& device = *registry_pointer->require(id, true).object;
                poll_device(device);
                auto& history = history_of(device);
                history.commit();

                std::cout << std::format(
                    "{} {} history[0]: {}\n",
                    group,
                    id,
                    history.history(0)
                );
            },
            false
        );

        m_commands.add(
            {group, "state"},
            group + " state <" + group + "-id>",
            "Show the newest committed state.",
            [registry_pointer, history_of, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " state <" + group + "-id>");

                const auto& history = history_of(*registry_pointer->require(id, false).object);
                require_history_size(history.size(), 1, std::format("{} {}", group, id));
                std::cout << std::format("{}\n", history.history(0));
            }
        );

        const auto print_latest_delta = [
            registry_pointer,
            history_of,
            make_change,
            group
        ](id_t id) {
            const auto& history = history_of(*registry_pointer->require(id, false).object);
            require_history_size(history.size(), 2, std::format("{} {}", group, id));
            const auto change = make_change(history.history(1), history.history(0));
            std::cout << std::format("{}\n", change);
        };

        m_commands.add(
            {group, "state-delta"},
            group + " state-delta <" + group + "-id>",
            "Show the change from history[1] to history[0].",
            [print_latest_delta, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " state-delta <" + group + "-id>");
                print_latest_delta(id);
            }
        );

        m_commands.add(
            {group, "delta"},
            group + " delta <" + group + "-id>",
            "Alias for state-delta.",
            [print_latest_delta, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " delta <" + group + "-id>");
                print_latest_delta(id);
            }
        );

        m_commands.add(
            {group, "history"},
            group + " history <" + group + "-id>",
            "Show every retained state and every adjacent change.",
            [registry_pointer, print_complete_history, group](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                arguments.expect_end(group + " history <" + group + "-id>");
                print_complete_history(id, *registry_pointer->require(id, false).object);
            }
        );

        m_commands.add(
            {group, "history-size"},
            group + " history-size <" + group + "-id> <sample-count>",
            "Replace retained state history with a new sample capacity.",
            [
                registry_pointer,
                poll_device,
                history_of,
                resize_history,
                group
            ](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                const std::size_t sample_count = arguments.pop_size("sample-count");
                arguments.expect_end(
                    group + " history-size <" + group + "-id> <sample-count>"
                );

                if (sample_count == 0) {
                    command_error("sample-count must be positive");
                }

                auto& device = *registry_pointer->require(id, true).object;
                resize_history(device, sample_count);

                auto& history = history_of(device);
                poll_device(device);
                history.commit();

                std::cout << std::format(
                    "{} {} history capacity set to {}; samples={}\n",
                    group,
                    id,
                    history.capacity(),
                    history.size()
                );
            },
            false
        );

        m_commands.add(
            {group, "watch"},
            group + " watch <" + group + "-id> <milliseconds> [interval-ms]",
            "Poll repeatedly and print all adjacent changes retained by the current history size.",
            [
                this,
                registry_pointer,
                print_all_changes,
                group
            ](arguments_t& arguments) {
                const id_t id = arguments.pop_id(group + "-id");
                const auto duration = std::chrono::milliseconds(
                    arguments.pop_long_long("milliseconds")
                );
                const auto interval = std::chrono::milliseconds(
                    arguments.empty()
                        ? 16
                        : arguments.pop_long_long("interval-ms")
                );
                arguments.expect_end(
                    group + " watch <" + group + "-id> <milliseconds> [interval-ms]"
                );

                if (duration.count() < 0) {
                    command_error("milliseconds must be non-negative");
                }
                if (interval.count() <= 0) {
                    command_error("interval-ms must be positive");
                }

                registry_pointer->require(id, true);
                const auto deadline = std::chrono::steady_clock::now() + duration;
                std::size_t poll_count = 0;

                do {
                    poll_once();
                    ++poll_count;

                    auto& entry = registry_pointer->require(id, false);
                    if (!entry.connected) {
                        std::cout << std::format("[{} {}] disconnected\n", group, id);
                        break;
                    }

                    std::cout << std::format("watch poll {}:\n", poll_count);
                    print_all_changes(id, *entry.object);

                    const auto now = std::chrono::steady_clock::now();
                    if (now >= deadline) {
                        break;
                    }

                    std::this_thread::sleep_for(std::min(
                        interval,
                        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now)
                    ));
                } while (true);

                std::cout << std::format(
                    "Watched {} {} for {} poll(s).\n",
                    group,
                    id,
                    poll_count
                );
            },
            false
        );
    };

    register_group(
        "joystick",
        m_joysticks,
        [] {
            return glfw_api::joysticks();
        },
        [](glfw_api::joystick_t& joystick) {
            glfw_api::poll_joystick(joystick);
        },
        [](auto& joystick) -> auto& {
            return joystick.joystick_states();
        },
        [](glfw_api::joystick_t& joystick, std::size_t sample_count) {
            joystick.joystick_states() = glfw_api::joystick_t::joystick_states_t(sample_count);
        },
        [](const auto& previous, const auto& current) {
            return glfw_api::joystick_state_change_t(previous, current);
        }
    );

    register_group(
        "gamepad",
        m_gamepads,
        [] {
            return glfw_api::gamepads();
        },
        [](glfw_api::gamepad_t& gamepad) {
            glfw_api::poll_gamepad(gamepad);
        },
        [](auto& gamepad) -> auto& {
            return gamepad.gamepad_states();
        },
        [](glfw_api::gamepad_t& gamepad, std::size_t sample_count) {
            gamepad.gamepad_states() = glfw_api::gamepad_t::gamepad_states_t(sample_count);
        },
        [](const auto& previous, const auto& current) {
            return glfw_api::gamepad_state_change_t(previous, current);
        }
    );

    const auto apply_gamepad_mapping = [this](std::string mapping) {
        update_gamepad_mapping(std::move(mapping));
        refresh_all(true);
        sample_all_devices();
        std::cout << "Gamepad mapping updated.\n";
    };

    m_commands.add(
        {"gamepad", "mapping", "update"},
        "gamepad mapping update <mapping-string>",
        "Update GLFW gamepad mappings from one mapping string.",
        [apply_gamepad_mapping](arguments_t& arguments) {
            std::string mapping(arguments.pop("mapping-string"));
            arguments.expect_end("gamepad mapping update <mapping-string>");

            apply_gamepad_mapping(std::move(mapping));
        },
        false
    );

    m_commands.add(
        {"gamepad", "mapping", "update-file"},
        "gamepad mapping update-file <file>",
        "Update GLFW gamepad mappings from a mapping file.",
        [apply_gamepad_mapping](arguments_t& arguments) {
            const std::filesystem::path path(arguments.pop("file"));
            arguments.expect_end("gamepad mapping update-file <file>");

            apply_gamepad_mapping(read_gamepad_mapping_file(path));
        },
        false
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
