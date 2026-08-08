#include "api.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <poll.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>

namespace m03gm491bquimk7j45lpvis1yq_cli_shell {

namespace {

using application_t = m03gm33dj5xo77vegpbspger4r_cli::application_t;

struct readline_state_t {
    readline_state_t(application_t& application, std::ostream& out, std::ostream& err);

    application_t& application;
    std::ostream& out;
    std::ostream& err;
    std::vector<std::string> completion_candidates;
    std::size_t completion_index;
    bool eof;
};

struct line_deleter_t {
    void operator()(char* line) const;
};

struct readline_scope_t {
    explicit readline_scope_t(readline_state_t& state);
    ~readline_scope_t();

private:
    decltype(rl_attempted_completion_function) m_previous_completion_function;
    decltype(rl_basic_word_break_characters) m_previous_word_break_characters;
    decltype(rl_completer_quote_characters) m_previous_quote_characters;
};

struct callback_readline_scope_t {
    callback_readline_scope_t(readline_state_t& state, const std::string& prompt);
    ~callback_readline_scope_t();

private:
    readline_scope_t m_readline_scope;
};

readline_state_t* active_readline_state = nullptr;
char word_break_characters[] = " \t\n";
char quote_characters[] = "\"'";

readline_state_t::readline_state_t(application_t& application, std::ostream& out, std::ostream& err):
    application(application),
    out(out),
    err(err),
    completion_candidates(),
    completion_index(0),
    eof(false)
{
}

void line_deleter_t::operator()(char* line) const {
    std::free(line);
}

std::string error_message(int error) {
    return std::strerror(error);
}

int poll_timeout(std::chrono::milliseconds interval) {
    if (interval.count() < 0) {
        throw std::invalid_argument("idle interval must not be negative");
    }
    if (interval.count() > std::numeric_limits<int>::max()) {
        throw std::invalid_argument("idle interval is too large");
    }
    return static_cast<int>(interval.count());
}

bool stdin_ready(std::chrono::milliseconds interval) {
    pollfd descriptor{
        .fd = STDIN_FILENO,
        .events = POLLIN,
        .revents = 0
    };

    while (true) {
        const int result = ::poll(&descriptor, 1, poll_timeout(interval));
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::format("poll(stdin) failed: {}", error_message(errno)));
        }
        return result != 0 && (descriptor.revents & (POLLIN | POLLHUP | POLLERR)) != 0;
    }
}

void execute_command(readline_state_t& state, const std::string& command) {
    if (command.empty()) {
        return;
    }

    add_history(command.c_str());
    try {
        state.application.run_command(command, state.out, state.err);
    } catch (const std::invalid_argument& exception) {
        state.err << std::format("error: {}\n", exception.what());
    } catch (const std::exception& exception) {
        state.err << std::format("exception: {}\n", exception.what());
    }
}

char* completion_generator(const char*, int state) {
    if (state == 0) {
        active_readline_state->completion_index = 0;
    }
    if (active_readline_state->completion_candidates.size() <= active_readline_state->completion_index) {
        return nullptr;
    }
    return ::strdup(active_readline_state->completion_candidates[active_readline_state->completion_index++].c_str());
}

char** attempted_completion_function(const char* text, int, int end) {
    rl_attempted_completion_over = 1;
    if (!active_readline_state) {
        return nullptr;
    }

    const std::string line = rl_line_buffer ? rl_line_buffer : "";
    const std::size_t cursor = end < 0 ? line.size() : std::min(static_cast<std::size_t>(end), line.size());

    active_readline_state->completion_candidates = active_readline_state->application.complete_line(line, cursor);
    active_readline_state->completion_index = 0;
    if (active_readline_state->completion_candidates.empty()) {
        return nullptr;
    }
    return rl_completion_matches(text, completion_generator);
}

void line_handler(char* line) {
    if (!active_readline_state) {
        std::free(line);
        return;
    }

    if (!line) {
        active_readline_state->out << '\n';
        active_readline_state->eof = true;
        return;
    }

    std::unique_ptr<char, line_deleter_t> owned_line(line);
    execute_command(*active_readline_state, owned_line.get());
}

readline_scope_t::readline_scope_t(readline_state_t& state):
    m_previous_completion_function(rl_attempted_completion_function),
    m_previous_word_break_characters(rl_basic_word_break_characters),
    m_previous_quote_characters(rl_completer_quote_characters)
{
    if (active_readline_state) {
        throw std::logic_error("cli_shell: only one active shell is supported");
    }

    active_readline_state = &state;
    rl_attempted_completion_function = attempted_completion_function;
    rl_basic_word_break_characters = word_break_characters;
    rl_completer_quote_characters = quote_characters;
}

readline_scope_t::~readline_scope_t() {
    rl_attempted_completion_function = m_previous_completion_function;
    rl_basic_word_break_characters = m_previous_word_break_characters;
    rl_completer_quote_characters = m_previous_quote_characters;
    active_readline_state = nullptr;
}

callback_readline_scope_t::callback_readline_scope_t(readline_state_t& state, const std::string& prompt):
    m_readline_scope(state)
{
    rl_callback_handler_install(prompt.c_str(), line_handler);
}

callback_readline_scope_t::~callback_readline_scope_t() {
    rl_callback_handler_remove();
}

void apply_history_size(std::optional<std::size_t> size) {
    if (!size) {
        return;
    }
    if (*size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("history size is too large");
    }
    stifle_history(static_cast<int>(*size));
}

void read_history_file(const std::optional<std::filesystem::path>& path) {
    if (!path || !std::filesystem::exists(*path)) {
        return;
    }

    const std::string history_path = path->string();
    const int error = ::read_history(history_path.c_str());
    if (error != 0) {
        throw std::runtime_error(std::format("failed to read history '{}': {}", history_path, error_message(error)));
    }
}

void write_history_file(const std::optional<std::filesystem::path>& path) {
    if (!path) {
        return;
    }

    const std::filesystem::path parent = path->parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    const std::string history_path = path->string();
    const int error = ::write_history(history_path.c_str());
    if (error != 0) {
        throw std::runtime_error(std::format("failed to write history '{}': {}", history_path, error_message(error)));
    }
}

void run_blocking(readline_state_t& readline_state, const std::string& prompt) {
    readline_scope_t readline_scope(readline_state);

    while (readline_state.application.running()) {
        std::unique_ptr<char, line_deleter_t> line(readline(prompt.c_str()));
        if (!line) {
            readline_state.out << '\n';
            break;
        }

        execute_command(readline_state, line.get());
    }
}

void run_with_idle(
    readline_state_t& readline_state,
    const std::string& prompt,
    std::chrono::milliseconds interval,
    const std::function<void()>& callback
) {
    callback_readline_scope_t readline_scope(readline_state, prompt);

    while (readline_state.application.running() && !readline_state.eof) {
        if (stdin_ready(interval)) {
            rl_callback_read_char();
        } else {
            callback();
        }
    }
}

} // namespace

shell_t::shell_t(m03gm33dj5xo77vegpbspger4r_cli::application_t& application, std::string prompt):
    m_application(application),
    m_prompt(std::move(prompt)),
    m_history_size(),
    m_history_file(),
    m_idle_interval(),
    m_idle_callback()
{
}

void shell_t::history_size(std::size_t size) {
    m_history_size = size;
}

void shell_t::history_file(std::filesystem::path path) {
    m_history_file = std::move(path);
}

void shell_t::idle(std::chrono::milliseconds interval, std::function<void()> callback) {
    if (!callback) {
        throw std::invalid_argument("idle callback must not be empty");
    }
    static_cast<void>(poll_timeout(interval));
    m_idle_interval = interval;
    m_idle_callback = std::move(callback);
}

int shell_t::run() {
    return run(std::cout, std::cerr);
}

int shell_t::run(std::ostream& out, std::ostream& err) {
    apply_history_size(m_history_size);
    read_history_file(m_history_file);
    readline_state_t readline_state(m_application, out, err);

    if (m_idle_callback) {
        run_with_idle(readline_state, m_prompt, *m_idle_interval, m_idle_callback);
    } else {
        run_blocking(readline_state, m_prompt);
    }

    write_history_file(m_history_file);
    return 0;
}

} // namespace m03gm491bquimk7j45lpvis1yq_cli_shell
