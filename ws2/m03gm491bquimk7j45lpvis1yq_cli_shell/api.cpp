#include "api.h"

#include <algorithm>
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

#include <readline/history.h>
#include <readline/readline.h>

namespace m03gm491bquimk7j45lpvis1yq_cli_shell {

namespace {

using application_t = m03gm33dj5xo77vegpbspger4r_cli::application_t;

struct readline_state_t {
    readline_state_t(application_t& application);

    application_t& application;
    std::vector<std::string> completion_candidates;
    std::size_t completion_index;
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

readline_state_t* active_readline_state = nullptr;
char word_break_characters[] = " \t\n";
char quote_characters[] = "\"'";

readline_state_t::readline_state_t(application_t& application):
    application(application),
    completion_candidates(),
    completion_index(0)
{
}

void line_deleter_t::operator()(char* line) const {
    std::free(line);
}

std::string error_message(int error) {
    return std::strerror(error);
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

} // namespace

shell_t::shell_t(m03gm33dj5xo77vegpbspger4r_cli::application_t& application, std::string prompt):
    m_application(application),
    m_prompt(std::move(prompt)),
    m_history_size(),
    m_history_file()
{
}

void shell_t::history_size(std::size_t size) {
    m_history_size = size;
}

void shell_t::history_file(std::filesystem::path path) {
    m_history_file = std::move(path);
}

int shell_t::run() {
    return run(std::cout, std::cerr);
}

int shell_t::run(std::ostream& out, std::ostream& err) {
    apply_history_size(m_history_size);
    read_history_file(m_history_file);
    readline_state_t readline_state(m_application);
    readline_scope_t readline_scope(readline_state);

    while (m_application.running()) {
        std::unique_ptr<char, line_deleter_t> line(readline(m_prompt.c_str()));
        if (!line) {
            out << '\n';
            break;
        }

        const std::string command(line.get());
        if (command.empty()) {
            continue;
        }

        add_history(command.c_str());
        try {
            m_application.run_command(command, out, err);
        } catch (const std::invalid_argument& exception) {
            err << std::format("error: {}\n", exception.what());
        } catch (const std::exception& exception) {
            err << std::format("exception: {}\n", exception.what());
        }
    }

    write_history_file(m_history_file);
    return 0;
}

} // namespace m03gm491bquimk7j45lpvis1yq_cli_shell
