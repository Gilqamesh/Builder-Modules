#include "cli_application.h"

#include <format>
#include <iostream>
#include <optional>
#include <string>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

void application_t::register_monitor_commands() {
    add_command(
        {"monitor", "refresh"},
        "monitor refresh",
        "Refresh stable monitor IDs and report connection changes.",
        [this](arguments_t& arguments) {
            arguments.expect_end("monitor refresh");
            refresh_monitors(true);
            std::cout << "Monitor registry refreshed.\n";
        },
        false
    );

    add_command(
        {"monitor", "list"},
        "monitor list",
        "List connected monitors and retained disconnected snapshots.",
        [this](arguments_t& arguments) {
            arguments.expect_end("monitor list");
            refresh_monitors(false);

            if (m_monitors.entries().empty()) {
                std::cout << "No monitor objects have been observed.\n";
                return;
            }

            std::optional<id_t> primary_id;
            if (const auto primary = glfw_api::primary_monitor()) {
                primary_id = m_monitors.observe(primary);
            }

            for (const auto& [id, entry] : m_monitors.entries()) {
                std::cout << std::format(
                    "monitor {}{} [{}]: {}\n",
                    id,
                    primary_id == id ? " (primary)" : "",
                    entry.connected ? "connected" : "disconnected snapshot",
                    *entry.object
                );
            }
        }
    );

    add_command(
        {"monitor", "primary"},
        "monitor primary",
        "Show the current primary monitor and its stable CLI ID.",
        [this](arguments_t& arguments) {
            arguments.expect_end("monitor primary");
            refresh_monitors(false);

            const auto primary = glfw_api::primary_monitor();
            if (!primary) {
                std::cout << "No primary monitor is currently available.\n";
                return;
            }

            const id_t id = m_monitors.observe(primary);
            std::cout << std::format("primary monitor {}: {}\n", id, *primary);
        }
    );

    add_command(
        {"monitor", "show"},
        "monitor show <monitor-id>",
        "Show every cached or live property of one monitor.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("monitor-id");
            arguments.expect_end("monitor show <monitor-id>");

            const auto& entry = m_monitors.require(id, false);
            std::cout << std::format(
                "monitor {} [{}]: {}\n",
                id,
                entry.connected ? "connected" : "disconnected snapshot",
                *entry.object
            );
        },
        {
            monitor_id_argument()
        }
    );

    add_command(
        {"monitor", "modes"},
        "monitor modes <monitor-id>",
        "List all currently available video modes for a connected monitor.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("monitor-id");
            arguments.expect_end("monitor modes <monitor-id>");

            const auto& entry = m_monitors.require(id, true);
            const auto video_modes = entry.object->video_modes();
            std::cout << std::format("monitor {} video modes ({}):\n", id, video_modes.size());
            for (std::size_t index = 0; index < video_modes.size(); ++index) {
                std::cout << std::format("  [{}] {}\n", index, video_modes[index]);
            }
        },
        {
            connected_monitor_id_argument()
        }
    );

    add_command(
        {"monitor", "handle"},
        "monitor handle <monitor-id>",
        "Show the current native monitor handle, including null after disconnect.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop<id_t>("monitor-id");
            arguments.expect_end("monitor handle <monitor-id>");

            const auto& entry = m_monitors.require(id, false);
            std::cout << std::format(
                "monitor {} handle: {}\n",
                id,
                static_cast<void*>(entry.object->handle())
            );
        },
        {
            monitor_id_argument()
        }
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
