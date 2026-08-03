#include "cli_application.h"
#include "cli_history.h"

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <span>
#include <utility>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
# include <poll.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <unistd.h>
#endif

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

#if defined(__unix__) || defined(__APPLE__)

application_t* g_readline_application = nullptr;
std::string g_readline_completion_line;
std::size_t g_readline_completion_start = 0;
std::vector<std::string> g_readline_completion_candidates;
std::size_t g_readline_completion_index = 0;
char g_readline_word_break_characters[] = " \t\n";
char g_readline_quote_characters[] = "\"'";

std::vector<std::string> tokenize_completion_prefix(std::string_view line) {
    enum class quote_t {
        none,
        single,
        double_quote
    };

    std::vector<std::string> tokens;
    std::string token;
    quote_t quote = quote_t::none;
    bool token_started = false;

    for (std::size_t index = 0; index < line.size(); ++index) {
        const char character = line[index];

        if (quote == quote_t::none && character == '#' && !token_started) {
            break;
        }

        if (character == '\\') {
            if (index + 1 < line.size()) {
                token.push_back(line[++index]);
            }
            token_started = true;
            continue;
        }

        if (quote == quote_t::none) {
            if (character == '\'') {
                quote = quote_t::single;
                token_started = true;
                continue;
            }
            if (character == '"') {
                quote = quote_t::double_quote;
                token_started = true;
                continue;
            }
            if (std::isspace(static_cast<unsigned char>(character)) != 0) {
                if (token_started) {
                    tokens.push_back(std::move(token));
                    token.clear();
                    token_started = false;
                }
                continue;
            }
        } else if (
            (quote == quote_t::single && character == '\'') ||
            (quote == quote_t::double_quote && character == '"')
        ) {
            quote = quote_t::none;
            continue;
        }

        token.push_back(character);
        token_started = true;
    }

    if (token_started) {
        tokens.push_back(std::move(token));
    }

    return tokens;
}

#endif

command_table_t::completion_result_t complete_ids(
    std::vector<id_t> ids,
    std::string_view partial
) {
    std::vector<std::string> values;
    values.reserve(ids.size());
    for (const id_t id : ids) {
        values.push_back(std::to_string(id));
    }
    return command_table_t::complete_values(values, partial);
}

} // namespace

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
    g_readline_application = this;
    rl_attempted_completion_function = &application_t::readline_completion;
    rl_basic_word_break_characters = g_readline_word_break_characters;
    rl_completer_quote_characters = g_readline_quote_characters;
    rl_callback_handler_install("glfw-test> ", &application_t::readline_line_handler);

    struct readline_cleanup_t {
        ~readline_cleanup_t() {
            rl_callback_handler_remove();
            g_readline_application = nullptr;
            g_readline_completion_candidates.clear();
            g_readline_completion_index = 0;
        }
    } readline_cleanup;

    while (m_running) {
        poll_once();

        pollfd descriptor{
            .fd = STDIN_FILENO,
            .events = POLLIN,
            .revents = 0
        };

        const int result = ::poll(
            &descriptor,
            1,
            static_cast<int>(m_event_pump_interval.count())
        );
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

        rl_callback_read_char();
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

#if defined(__unix__) || defined(__APPLE__)

void application_t::readline_line_handler(char* line) {
    if (!g_readline_application) {
        std::free(line);
        return;
    }

    application_t& application = *g_readline_application;
    if (!line) {
        std::cout << '\n';
        application.m_running = false;
        return;
    }

    std::string command(line);
    std::free(line);

    if (!command.empty()) {
        add_history(command.c_str());
    }

    application.execute_safely(command);
}

char** application_t::readline_completion(const char* text, int start, int end) {
    static_cast<void>(end);

    g_readline_completion_line = rl_line_buffer ? rl_line_buffer : "";
    g_readline_completion_start = start < 0
        ? 0
        : static_cast<std::size_t>(start);

    const auto result = g_readline_application
        ? g_readline_application->complete_command_line(
            g_readline_completion_line,
            g_readline_completion_start,
            text ? text : ""
        )
        : completion_result_t{};
    g_readline_completion_candidates = result.candidates;
    g_readline_completion_index = 0;

    if (result.use_files && g_readline_completion_candidates.empty()) {
        rl_attempted_completion_over = 0;
        return nullptr;
    }

    rl_attempted_completion_over = 1;
    return rl_completion_matches(
        text ? text : "",
        &application_t::readline_completion_generator
    );
}

char* application_t::readline_completion_generator(const char* text, int state) {
    static_cast<void>(text);

    if (state == 0) {
        g_readline_completion_index = 0;
    }

    if (g_readline_completion_candidates.size() <= g_readline_completion_index) {
        return nullptr;
    }

    return ::strdup(g_readline_completion_candidates[g_readline_completion_index++].c_str());
}

command_table_t::completion_result_t application_t::complete_command_line(
    std::string_view line,
    std::size_t word_start,
    std::string_view partial
) const {
    if (line.size() < word_start) {
        word_start = line.size();
    }

    std::vector<std::string> prefix = tokenize_completion_prefix(
        line.substr(0, word_start)
    );
    std::span<const std::string> command_prefix(prefix.data(), prefix.size());

    return m_commands.complete(command_prefix, partial);
}

#endif

application_t::argument_spec_t application_t::window_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_window_ids(context.partial);
        }
    );
}

application_t::argument_spec_t application_t::connected_monitor_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_monitor_ids(context.partial, true);
        }
    );
}

application_t::argument_spec_t application_t::monitor_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_monitor_ids(context.partial, false);
        }
    );
}

application_t::argument_spec_t application_t::connected_joystick_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_joystick_ids(context.partial, true);
        }
    );
}

application_t::argument_spec_t application_t::joystick_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_joystick_ids(context.partial, false);
        }
    );
}

application_t::argument_spec_t application_t::connected_gamepad_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_gamepad_ids(context.partial, true);
        }
    );
}

application_t::argument_spec_t application_t::gamepad_id_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_gamepad_ids(context.partial, false);
        }
    );
}

application_t::argument_spec_t application_t::watch_id_or_all_argument(std::string name) const {
    return command_table_t::argument(
        std::move(name),
        [this](const completion_context_t& context) {
            return complete_watch_ids(context.partial, true);
        }
    );
}

application_t::completion_result_t application_t::complete_window_ids(
    std::string_view partial
) const {
    std::vector<id_t> ids;
    ids.reserve(m_windows.size());
    for (const auto& [id, window] : m_windows) {
        if (window) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

application_t::completion_result_t application_t::complete_monitor_ids(
    std::string_view partial,
    bool require_connected
) const {
    std::vector<id_t> ids;
    ids.reserve(m_monitors.entries().size());
    for (const auto& [id, entry] : m_monitors.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

application_t::completion_result_t application_t::complete_joystick_ids(
    std::string_view partial,
    bool require_connected
) const {
    std::vector<id_t> ids;
    ids.reserve(m_joysticks.entries().size());
    for (const auto& [id, entry] : m_joysticks.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

application_t::completion_result_t application_t::complete_gamepad_ids(
    std::string_view partial,
    bool require_connected
) const {
    std::vector<id_t> ids;
    ids.reserve(m_gamepads.entries().size());
    for (const auto& [id, entry] : m_gamepads.entries()) {
        if (entry.object && (!require_connected || entry.connected)) {
            ids.push_back(id);
        }
    }
    return complete_ids(std::move(ids), partial);
}

application_t::completion_result_t application_t::complete_watch_ids(
    std::string_view partial,
    bool include_all
) const {
    std::vector<std::string> values;
    values.reserve(m_watches.size() + (include_all ? 1 : 0));
    if (include_all) {
        values.emplace_back("all");
    }
    for (const auto& [id, watch] : m_watches) {
        static_cast<void>(watch);
        values.push_back(std::to_string(id));
    }
    return command_table_t::complete_values(values, partial);
}

application_t::completion_result_t application_t::complete_monitor_video_mode_indices(
    id_t monitor_id,
    std::string_view partial
) const {
    const auto iterator = m_monitors.entries().find(monitor_id);
    if (
        iterator == m_monitors.entries().end() ||
        !iterator->second.object ||
        !iterator->second.connected
    ) {
        return {};
    }

    const auto modes = iterator->second.object->video_modes();
    std::vector<std::string> values;
    values.reserve(modes.size());
    for (std::size_t index = 0; index < modes.size(); ++index) {
        values.push_back(std::to_string(index));
    }
    return command_table_t::complete_values(values, partial);
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
    service_watches();
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

id_t application_t::start_watch(
    watch_target_t target,
    id_t object_id,
    std::chrono::milliseconds duration,
    std::chrono::milliseconds interval
) {
    if (duration.count() < 0) {
        command_error("milliseconds must be non-negative");
    }
    if (watch_target_uses_interval(target) && interval.count() <= 0) {
        command_error("interval-ms must be positive");
    }
    if (!watch_target_uses_interval(target) && interval.count() != 0) {
        command_error(std::format(
            "{} watches follow the event-pump interval",
            watch_target_label(target)
        ));
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
    m_watches.emplace(watch_id, watch_t{
        .id = watch_id,
        .target = target,
        .object_id = object_id,
        .deadline = now + duration,
        .next_poll_time = now,
        .interval = interval,
        .poll_count = 0
    });
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

        const auto make_input_change = [](
            const glfw_api::input_state_t& previous,
            const glfw_api::input_state_t& current
        ) {
            return glfw_api::input_state_change_t(previous, current);
        };

        std::cout << std::format(
            "[watch {}] window {} input poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history_changes(
            std::format("[window {}] retained input history", watch.object_id),
            iterator->second->input_states(),
            make_input_change
        );
        return true;
    }

    case watch_target_t::joystick: {
        const auto iterator = m_joysticks.entries().find(watch.object_id);
        if (
            iterator == m_joysticks.entries().end() ||
            !iterator->second.object ||
            !iterator->second.connected
        ) {
            std::cout << std::format(
                "[watch {}] joystick {} disconnected; stopping.\n",
                watch.id,
                watch.object_id
            );
            return false;
        }

        const auto make_joystick_change = [](
            const glfw_api::joystick_state_t& previous,
            const glfw_api::joystick_state_t& current
        ) {
            return glfw_api::joystick_state_change_t(previous, current);
        };

        std::cout << std::format(
            "[watch {}] joystick {} poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history_changes(
            std::format("[joystick {}] retained history", watch.object_id),
            iterator->second.object->joystick_states(),
            make_joystick_change
        );
        return true;
    }

    case watch_target_t::gamepad: {
        const auto iterator = m_gamepads.entries().find(watch.object_id);
        if (
            iterator == m_gamepads.entries().end() ||
            !iterator->second.object ||
            !iterator->second.connected
        ) {
            std::cout << std::format(
                "[watch {}] gamepad {} disconnected; stopping.\n",
                watch.id,
                watch.object_id
            );
            return false;
        }

        const auto make_gamepad_change = [](
            const glfw_api::gamepad_state_t& previous,
            const glfw_api::gamepad_state_t& current
        ) {
            return glfw_api::gamepad_state_change_t(previous, current);
        };

        std::cout << std::format(
            "[watch {}] gamepad {} poll {}:\n",
            watch.id,
            watch.object_id,
            watch.poll_count
        );
        m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history_changes(
            std::format("[gamepad {}] retained history", watch.object_id),
            iterator->second.object->gamepad_states(),
            make_gamepad_change
        );
        return true;
    }
    }

    throw std::logic_error("application_t::print_watch_sample: unknown watch target");
}

std::string_view application_t::watch_target_label(watch_target_t target) {
    switch (target) {
    case watch_target_t::window_input:
        return "window";
    case watch_target_t::joystick:
        return "joystick";
    case watch_target_t::gamepad:
        return "gamepad";
    }

    throw std::logic_error("application_t::watch_target_label: unknown watch target");
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
