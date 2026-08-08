#include "cli_application.h"

#include <m03gm491bquimk7j45lpvis1yq_cli_shell/api.h>

#include <chrono>
#include <exception>
#include <format>
#include <iostream>
#include <initializer_list>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

std::vector<std::string> complete_strings(std::span<const std::string> values, std::string_view partial) {
    std::set<std::string> candidates;
    for (const std::string& value : values) {
        if (value.starts_with(partial)) {
            candidates.insert(value);
        }
    }
    return {candidates.begin(), candidates.end()};
}

std::vector<std::string> complete_ids(std::vector<id_t> ids, std::string_view partial) {
    std::vector<std::string> values;
    values.reserve(ids.size());
    for (const id_t id : ids) {
        values.push_back(std::to_string(id));
    }
    return complete_strings(values, partial);
}

std::vector<std::string> own_path(std::initializer_list<std::string_view> path) {
    std::vector<std::string> result;
    result.reserve(path.size());
    for (const std::string_view component : path) {
        result.emplace_back(component);
    }
    return result;
}

} // namespace

application_t::watch_t::watch_t():
    id(0),
    target(watch_target_t::window_input),
    object_id(0),
    deadline(),
    next_poll_time(),
    interval(0),
    poll_count(0)
{
}

application_t::watch_t::watch_t(id_t id, watch_target_t target, id_t object_id, std::chrono::steady_clock::time_point deadline, std::chrono::steady_clock::time_point next_poll_time, std::chrono::milliseconds interval, std::size_t poll_count):
    id(id),
    target(target),
    object_id(object_id),
    deadline(deadline),
    next_poll_time(next_poll_time),
    interval(interval),
    poll_count(poll_count)
{
}

application_t::application_t():
    m_commands(),
    m_event_pump_interval(16),
    m_next_idle_time(std::chrono::steady_clock::now()),
    m_creation_settings(),
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
    ),
    m_windows(),
    m_next_window_id(1),
    m_watches(),
    m_next_watch_id(1)
{
    m_commands.install_help_command();
    refresh_all(false);
    sample_all_devices();
    register_commands();
}

bool application_t::repl() {
    std::cout
        << "GLFW abstraction test CLI\n"
        << "Use 'help' for command groups or 'window help' for one group.\n";

    m03gm491bquimk7j45lpvis1yq_cli_shell::shell_t shell(m_commands, "glfw-test> ");
    shell.idle(std::chrono::milliseconds(1), [this] {
        idle_once();
    });
    shell.run();

    return true;
}

application_t::argument_spec_t application_t::window_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_window_ids(partial);
    }).optional();
}

application_t::argument_spec_t application_t::connected_monitor_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_monitor_ids(partial, true);
    }).optional();
}

application_t::argument_spec_t application_t::monitor_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_monitor_ids(partial, false);
    }).optional();
}

application_t::argument_spec_t application_t::connected_joystick_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_joystick_ids(partial, true);
    }).optional();
}

application_t::argument_spec_t application_t::joystick_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_joystick_ids(partial, false);
    }).optional();
}

application_t::argument_spec_t application_t::connected_gamepad_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_gamepad_ids(partial, true);
    }).optional();
}

application_t::argument_spec_t application_t::gamepad_id_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_gamepad_ids(partial, false);
    }).optional();
}

application_t::argument_spec_t application_t::watch_id_or_all_argument(std::string name) const {
    return cli::argument_t::custom(std::move(name), [this](std::span<const std::string>, std::string_view partial) {
        return complete_watch_ids(partial, true);
    }).optional();
}

std::vector<std::string> application_t::complete_window_ids(std::string_view partial) const {
    std::vector<id_t> ids;
    ids.reserve(m_windows.size());
    for (const auto& [id, window] : m_windows) {
        if (window) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

std::vector<std::string> application_t::complete_monitor_ids(std::string_view partial, bool require_connected) const {
    std::vector<id_t> ids;
    ids.reserve(m_monitors.entries().size());
    for (const auto& [id, entry] : m_monitors.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

std::vector<std::string> application_t::complete_joystick_ids(std::string_view partial, bool require_connected) const {
    std::vector<id_t> ids;
    ids.reserve(m_joysticks.entries().size());
    for (const auto& [id, entry] : m_joysticks.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

std::vector<std::string> application_t::complete_gamepad_ids(std::string_view partial, bool require_connected) const {
    std::vector<id_t> ids;
    ids.reserve(m_gamepads.entries().size());
    for (const auto& [id, entry] : m_gamepads.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

std::vector<std::string> application_t::complete_watch_ids(std::string_view partial, bool include_all) const {
    std::vector<std::string> values;
    values.reserve(m_watches.size() + (include_all ? 1 : 0));
    if (include_all) {
        values.emplace_back("all");
    }
    for (const auto& [id, watch] : m_watches) {
        static_cast<void>(watch);
        values.push_back(std::to_string(id));
    }
    return complete_strings(values, partial);
}

std::vector<std::string> application_t::complete_monitor_video_mode_indices(id_t monitor_id, std::string_view partial) const {
    const auto iterator = m_monitors.entries().find(monitor_id);
    if (iterator == m_monitors.entries().end() || !iterator->second.object || !iterator->second.connected) {
        return {};
    }

    const auto modes = iterator->second.object->video_modes();
    std::vector<std::string> values;
    values.reserve(modes.size());
    for (std::size_t index = 0; index < modes.size(); ++index) {
        values.push_back(std::to_string(index));
    }
    return complete_strings(values, partial);
}

application_t::argument_spec_t application_t::argument(std::string name) {
    return cli::argument_t::token(std::move(name)).optional();
}

application_t::argument_spec_t application_t::argument(std::string name, completion_handler_t complete) {
    return cli::argument_t::custom(std::move(name), std::move(complete)).optional();
}

application_t::argument_spec_t application_t::choice_argument(std::string name, std::initializer_list<std::string_view> values) {
    return cli::argument_t::choice(std::move(name), values).optional();
}

application_t::argument_spec_t application_t::suggested_values_argument(std::string name, std::initializer_list<std::string_view> values) {
    std::vector<std::string> owned_values;
    owned_values.reserve(values.size());
    for (const std::string_view value : values) {
        owned_values.emplace_back(value);
    }
    return cli::argument_t::custom(std::move(name), [values = std::move(owned_values)](std::span<const std::string>, std::string_view partial) {
        return complete_strings(values, partial);
    }).optional();
}

application_t::argument_spec_t application_t::files_argument(std::string name) {
    return cli::argument_t::file(std::move(name)).optional();
}

void application_t::add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, bool poll_after) {
    add_command(
        path,
        std::move(usage),
        std::move(description),
        std::move(handler),
        std::vector<argument_spec_t>{},
        poll_after
    );
}

void application_t::add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, std::vector<argument_spec_t> arguments, bool poll_after) {
    cli::command_t command(
        own_path(path),
        std::move(usage),
        std::move(description),
        std::move(arguments),
        [this, handler = std::move(handler), poll_after](cli::context_t& context) {
            handler(context.arguments);
            if (m_commands.running() && poll_after) {
                poll_once();
            }
        }
    );
    m_commands.add(std::move(command));
}

void application_t::add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, completion_handler_t complete_arguments, bool poll_after) {
    add_command(
        path,
        std::move(usage),
        std::move(description),
        std::move(handler),
        {
            cli::argument_t::custom("argument", [complete_arguments = std::move(complete_arguments)](std::span<const std::string> arguments, std::string_view partial) {
                return complete_arguments(arguments, partial);
            }).optional().variadic()
        },
        poll_after
    );
}

bool application_t::run_command(std::string_view command) {
    return m_commands.run_command(command, std::cout, std::cerr);
}

bool application_t::run_script(const std::filesystem::path& path, bool echo_commands) {
    return m_commands.run_script(path, std::cout, std::cerr, echo_commands);
}

std::vector<std::shared_ptr<glfw_api::monitor_t>> application_t::retained_monitors() const {
    return m_monitors.retained_objects();
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
    service_watches();
}

void application_t::idle_once() {
    const auto now = std::chrono::steady_clock::now();
    if (now < m_next_idle_time) {
        return;
    }

    poll_once();
    m_next_idle_time = now + m_event_pump_interval;
}

void application_t::sample_all_devices() {
    for (auto& [id, entry] : m_joysticks.entries()) {
        static_cast<void>(id);
        if (entry.connected && entry.object) {
            glfw_api::poll_joystick(*entry.object);
            entry.object->joystick_states().commit();
        }
    }

    for (auto& [id, entry] : m_gamepads.entries()) {
        static_cast<void>(id);
        if (entry.connected && entry.object) {
            glfw_api::poll_gamepad(*entry.object);
            entry.object->gamepad_states().commit();
        }
    }
}

void application_t::advance_window_inputs() {
    for (auto& [id, window] : m_windows) {
        static_cast<void>(id);

        auto& history = window->input_states();
        history.commit();
        history.stage() = history.history(0);
    }
}

id_t application_t::start_watch(watch_target_t target, id_t object_id, std::chrono::milliseconds duration, std::chrono::milliseconds interval) {
    if (duration.count() < 0) {
        command_error("milliseconds must be non-negative");
    }
    if (watch_target_uses_interval(target) && interval.count() <= 0) {
        command_error("interval-ms must be positive");
    }
    if (!watch_target_uses_interval(target) && interval.count() != 0) {
        command_error(std::format("{} watches follow the event-pump interval", target));
    }

    switch (target) {
    case watch_target_t::window_input:
        require_window(object_id);
        break;
    case watch_target_t::joystick:
        m_joysticks.require(object_id, true);
        break;
    case watch_target_t::gamepad:
        m_gamepads.require(object_id, true);
        break;
    }

    const auto now = std::chrono::steady_clock::now();
    const id_t watch_id = m_next_watch_id++;
    m_watches.emplace(watch_id, watch_t(watch_id, target, object_id, now + duration, now, interval, 0));
    return watch_id;
}

void application_t::service_watches() {
    if (m_watches.empty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    std::vector<id_t> finished;

    for (auto& [id, watch] : m_watches) {
        if (watch_target_uses_interval(watch.target) && now < watch.next_poll_time) {
            continue;
        }

        bool keep_watching = false;
        try {
            keep_watching = print_watch_sample(watch);
        } catch (const std::exception& exception) {
            std::cerr << std::format(
                "[watch {}] stopped after exception: {}\n",
                id,
                exception.what()
            );
            finished.push_back(id);
            continue;
        }

        if (!keep_watching) {
            finished.push_back(id);
            continue;
        }

        if (now >= watch.deadline) {
            std::cout << std::format(
                "[watch {}] completed after {} poll(s).\n",
                id,
                watch.poll_count
            );
            finished.push_back(id);
            continue;
        }

        watch.next_poll_time = watch_target_uses_interval(watch.target)
            ? now + watch.interval
            : now;
    }

    for (const id_t id : finished) {
        m_watches.erase(id);
    }
}

bool application_t::print_watch_sample(watch_t& watch) {
    ++watch.poll_count;

    switch (watch.target) {
    case watch_target_t::window_input: {
        const auto iterator = m_windows.find(watch.object_id);
        if (iterator == m_windows.end() || !iterator->second) {
            std::cout << std::format(
                "[watch {}] window {} no longer exists; stopping.\n",
                watch.id,
                watch.object_id
            );
            return false;
        }

        std::cout << std::format(
            "[watch {}] window {} input poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        std::cout << std::format(
            "[window {}] retained input history: {}\n",
            watch.object_id,
            iterator->second->input_states()
        );
        return true;
    }

    case watch_target_t::joystick: {
        const auto iterator = m_joysticks.entries().find(watch.object_id);
        if (iterator == m_joysticks.entries().end() || !iterator->second.object || !iterator->second.connected) {
            std::cout << std::format(
                "[watch {}] joystick {} disconnected; stopping.\n",
                watch.id,
                watch.object_id
            );
            return false;
        }

        std::cout << std::format(
            "[watch {}] joystick {} poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        std::cout << std::format(
            "[joystick {}] retained history: {}\n",
            watch.object_id,
            iterator->second.object->joystick_states()
        );
        return true;
    }

    case watch_target_t::gamepad: {
        const auto iterator = m_gamepads.entries().find(watch.object_id);
        if (iterator == m_gamepads.entries().end() || !iterator->second.object || !iterator->second.connected) {
            std::cout << std::format(
                "[watch {}] gamepad {} disconnected; stopping.\n",
                watch.id,
                watch.object_id
            );
            return false;
        }

        std::cout << std::format(
            "[watch {}] gamepad {} poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        std::cout << std::format(
            "[gamepad {}] retained history: {}\n",
            watch.object_id,
            iterator->second.object->gamepad_states()
        );
        return true;
    }
    }

    throw std::logic_error("application_t::print_watch_sample: unknown watch target");
}

bool application_t::watch_target_uses_interval(watch_target_t target) {
    switch (target) {
    case watch_target_t::window_input:
        return false;
    case watch_target_t::joystick:
    case watch_target_t::gamepad:
        return true;
    }

    throw std::logic_error("application_t::watch_target_uses_interval: unknown watch target");
}

void application_t::require_history_size(std::size_t actual_size, std::size_t required_size, std::string_view owner) {
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
