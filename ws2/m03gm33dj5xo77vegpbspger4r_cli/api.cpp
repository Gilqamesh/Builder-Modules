#include "api.h"

#include <charconv>
#include <cmath>
#include <concepts>
#include <filesystem>
#include <limits>
#include <optional>
#include <ostream>
#include <set>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace m03gm33dj5xo77vegpbspger4r_cli {

namespace {

enum class quote_t {
    none,
    single,
    double_quote
};

struct tokenization_t {
    tokenization_t();

    std::vector<std::string> tokens;
    bool trailing_separator;
    std::optional<std::string> error;
};

struct completion_line_t {
    std::vector<std::string> prefix;
    std::string partial;
};

tokenization_t::tokenization_t():
    tokens(),
    trailing_separator(false),
    error()
{
}

template <std::integral T>
T parse_integer(std::string_view text, std::string_view name) {
    T value{};
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto [position, error] = std::from_chars(begin, end, value);
    if (error != std::errc{} || position != end) {
        throw std::invalid_argument(std::format("{} must be an integer, got '{}'", name, text));
    }
    return value;
}

template <std::floating_point T>
T parse_floating(std::string_view text, std::string_view name) {
    T value{};
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto [position, error] = std::from_chars(begin, end, value);
    if (error != std::errc{} || position != end || !std::isfinite(value)) {
        throw std::invalid_argument(std::format("{} must be a finite number, got '{}'", name, text));
    }
    return value;
}

bool parse_bool(std::string_view text, std::string_view name) {
    if (text == "true" || text == "on" || text == "yes" || text == "1") {
        return true;
    }
    if (text == "false" || text == "off" || text == "no" || text == "0") {
        return false;
    }
    throw std::invalid_argument(std::format("{} must be true/false, on/off, yes/no or 1/0, got '{}'", name, text));
}

std::vector<std::string> own_values(std::initializer_list<std::string_view> values) {
    std::vector<std::string> result;
    result.reserve(values.size());
    for (const std::string_view value : values) {
        result.emplace_back(value);
    }
    return result;
}

std::string quote_token(std::string_view token) {
    const bool needs_quotes = token.empty() || std::ranges::any_of(token, [](char character) {
        return std::isspace(static_cast<unsigned char>(character)) != 0 || character == '\'' || character == '"' || character == '\\' || character == '#';
    });

    if (!needs_quotes) {
        return std::string(token);
    }

    std::string result;
    result.reserve(token.size() + 2);
    result.push_back('"');
    for (const char character : token) {
        if (character == '"' || character == '\\') {
            result.push_back('\\');
        }
        result.push_back(character);
    }
    result.push_back('"');
    return result;
}

std::string render_tokens(std::span<const std::string> tokens) {
    std::string result;
    for (std::size_t index = 0; index < tokens.size(); ++index) {
        if (index != 0) {
            result.push_back(' ');
        }
        result += quote_token(tokens[index]);
    }
    return result;
}

std::string render_choice_list(std::span<const std::string> values) {
    std::string result;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            result += ", ";
        }
        result += quote_token(values[index]);
    }
    return result;
}

std::string render_choice_usage(std::span<const std::string> values) {
    std::string result;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            result += "|";
        }
        result += quote_token(values[index]);
    }
    return result;
}

tokenization_t tokenize_impl(std::string_view line, bool allow_incomplete) {
    tokenization_t result;
    std::string token;
    quote_t quote = quote_t::none;
    bool token_started = false;
    bool escaped = false;

    const auto start_token = [&] {
        token_started = true;
    };

    const auto finish_token = [&] {
        result.tokens.push_back(std::move(token));
        token.clear();
        token_started = false;
        result.trailing_separator = true;
    };

    for (const char character : line) {
        result.trailing_separator = false;
        if (quote == quote_t::none && character == '#' && !token_started) {
            result.trailing_separator = true;
            break;
        }

        if (escaped) {
            start_token();
            token.push_back(character);
            escaped = false;
            continue;
        }

        if (character == '\\') {
            start_token();
            escaped = true;
            continue;
        }

        if (quote == quote_t::none) {
            if (character == '\'') {
                start_token();
                quote = quote_t::single;
                continue;
            }
            if (character == '"') {
                start_token();
                quote = quote_t::double_quote;
                continue;
            }
            if (std::isspace(static_cast<unsigned char>(character)) != 0) {
                if (token_started) {
                    finish_token();
                } else {
                    result.trailing_separator = true;
                }
                continue;
            }
        } else if ((quote == quote_t::single && character == '\'') || (quote == quote_t::double_quote && character == '"')) {
            quote = quote_t::none;
            continue;
        }

        start_token();
        token.push_back(character);
    }

    if (escaped && !allow_incomplete) {
        result.error = "line ends with an incomplete escape";
        return result;
    }
    if (quote != quote_t::none && !allow_incomplete) {
        result.error = "unterminated quoted string";
        return result;
    }
    if (token_started) {
        result.tokens.push_back(std::move(token));
        result.trailing_separator = false;
    }
    return result;
}

std::vector<std::string> tokenize(std::string_view line) {
    tokenization_t result = tokenize_impl(line, false);
    if (result.error) {
        throw std::invalid_argument(*result.error);
    }
    return std::move(result.tokens);
}

completion_line_t parse_completion_line(std::string_view line, std::size_t cursor) {
    if (cursor == std::string_view::npos || cursor > line.size()) {
        cursor = line.size();
    }

    tokenization_t result = tokenize_impl(line.substr(0, cursor), true);
    completion_line_t completion;
    if (!result.tokens.empty() && !result.trailing_separator) {
        completion.prefix.assign(result.tokens.begin(), result.tokens.end() - 1);
        completion.partial = std::move(result.tokens.back());
    } else {
        completion.prefix = std::move(result.tokens);
    }
    return completion;
}

std::vector<std::string> complete_file(std::string_view partial) {
    const std::filesystem::path input(partial);
    const std::filesystem::path parent = input.parent_path();
    const std::filesystem::path directory = parent.empty() ? std::filesystem::path(".") : parent;
    const std::string filename = input.filename().string();
    std::error_code error;
    std::filesystem::directory_iterator entries(directory, error);
    std::set<std::string> candidates;

    if (error) {
        return {};
    }

    for (const std::filesystem::directory_entry& entry : entries) {
        const std::string entry_filename = entry.path().filename().string();
        if (!entry_filename.starts_with(filename)) {
            continue;
        }

        std::filesystem::path candidate = parent.empty() ? std::filesystem::path(entry_filename) : parent / entry_filename;
        std::error_code status_error;
        if (entry.is_directory(status_error) && !status_error) {
            candidate += std::filesystem::path::preferred_separator;
        }
        candidates.insert(candidate.string());
    }

    return {candidates.begin(), candidates.end()};
}

std::vector<std::string> complete_values(std::span<const std::string> values, std::string_view partial) {
    std::set<std::string> candidates;
    for (const std::string& value : values) {
        if (value.starts_with(partial)) {
            candidates.insert(value);
        }
    }
    return {candidates.begin(), candidates.end()};
}

std::function<std::vector<std::string>(std::span<const std::string>, std::string_view)> complete_value_set(std::vector<std::string> values) {
    return [values = std::move(values)](std::span<const std::string>, std::string_view partial) {
        return complete_values(values, partial);
    };
}

std::string render_usage(const command_t& command) {
    return std::format("{}", command);
}

bool starts_with(std::span<const std::string> path, std::span<const std::string> prefix) {
    if (path.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (path[index] != prefix[index]) {
            return false;
        }
    }
    return true;
}

void sort_commands(std::vector<const command_t*>& commands) {
    std::ranges::sort(commands, {}, [](const command_t* command) {
        return render_usage(*command);
    });
}

void print_commands(std::ostream& out, std::span<const command_t* const> commands) {
    constexpr std::size_t maximum_usage_width = 64;

    std::size_t usage_width = 0;
    for (const command_t* command : commands) {
        const std::string usage = render_usage(*command);
        usage_width = std::max(usage_width, std::min(usage.size(), maximum_usage_width));
    }

    for (const command_t* command : commands) {
        const std::string usage = render_usage(*command);
        if (usage.size() <= maximum_usage_width) {
            out << std::format("  {:<{}}  {}\n", usage, usage_width, command->description);
        } else {
            out << "  " << usage << '\n' << "      " << command->description << '\n';
        }
    }
}

} // namespace

arguments_t::arguments_t(std::span<const std::string> values):
    m_values(values),
    m_index(0)
{
}

bool arguments_t::empty() const {
    return m_index == m_values.size();
}

std::size_t arguments_t::size() const {
    return m_values.size() - m_index;
}

std::span<const std::string> arguments_t::remaining() const {
    return m_values.subspan(m_index);
}

std::string_view arguments_t::pop(std::string_view name) {
    if (empty()) {
        throw std::invalid_argument(std::format("missing {}", name));
    }
    return m_values[m_index++];
}

int arguments_t::pop_int(std::string_view name) {
    return parse_integer<int>(pop(name), name);
}

long long arguments_t::pop_long_long(std::string_view name) {
    return parse_integer<long long>(pop(name), name);
}

std::size_t arguments_t::pop_size(std::string_view name) {
    const auto value = parse_integer<unsigned long long>(pop(name), name);
    if (value > std::numeric_limits<std::size_t>::max()) {
        throw std::invalid_argument(std::format("{} is too large", name));
    }
    return static_cast<std::size_t>(value);
}

float arguments_t::pop_float(std::string_view name) {
    return parse_floating<float>(pop(name), name);
}

double arguments_t::pop_double(std::string_view name) {
    return parse_floating<double>(pop(name), name);
}

bool arguments_t::pop_bool(std::string_view name) {
    return parse_bool(pop(name), name);
}

argument_t::argument_t():
    m_name(),
    m_usage(),
    m_complete(),
    m_validate(),
    m_is_optional(false),
    m_is_variadic(false)
{
}

argument_t argument_t::optional() && {
    m_is_optional = true;
    return std::move(*this);
}

argument_t argument_t::variadic() && {
    m_is_variadic = true;
    return std::move(*this);
}

std::string_view argument_t::name() const {
    return m_name;
}

std::string_view argument_t::usage() const {
    return m_usage;
}

bool argument_t::is_optional() const {
    return m_is_optional;
}

bool argument_t::is_variadic() const {
    return m_is_variadic;
}

std::vector<std::string> argument_t::complete(std::span<const std::string> arguments, std::string_view partial) const {
    if (!m_complete) {
        return {};
    }
    return m_complete(arguments, partial);
}

void argument_t::validate(std::string_view value) const {
    if (m_validate) {
        m_validate(value, m_name);
    }
}

argument_t argument_t::token(std::string name) {
    argument_t result;
    result.m_name = std::move(name);
    return result;
}

argument_t argument_t::integer(std::string name) {
    argument_t result = token(std::move(name));
    result.m_validate = [](std::string_view value, std::string_view value_name) {
        static_cast<void>(parse_integer<long long>(value, value_name));
    };
    return result;
}

argument_t argument_t::unsigned_integer(std::string name) {
    argument_t result = token(std::move(name));
    result.m_validate = [](std::string_view value, std::string_view value_name) {
        static_cast<void>(parse_integer<unsigned long long>(value, value_name));
    };
    return result;
}

argument_t argument_t::number(std::string name) {
    argument_t result = token(std::move(name));
    result.m_validate = [](std::string_view value, std::string_view value_name) {
        static_cast<void>(parse_floating<double>(value, value_name));
    };
    return result;
}

argument_t argument_t::boolean(std::string name) {
    argument_t result = token(std::move(name));
    result.m_usage = "true|false";
    result.m_complete = complete_value_set(own_values({"true", "false", "on", "off", "yes", "no", "1", "0"}));
    result.m_validate = [](std::string_view value, std::string_view value_name) {
        static_cast<void>(parse_bool(value, value_name));
    };
    return result;
}

argument_t argument_t::file(std::string name) {
    argument_t result = token(std::move(name));
    result.m_complete = [](std::span<const std::string>, std::string_view partial) {
        return complete_file(partial);
    };
    return result;
}

argument_t argument_t::choice(std::string name, std::initializer_list<std::string_view> values) {
    return choice(std::move(name), own_values(values));
}

argument_t argument_t::choice(std::string name, std::vector<std::string> values) {
    argument_t result = token(std::move(name));
    result.m_usage = render_choice_usage(values);
    result.m_complete = complete_value_set(values);
    result.m_validate = [values = std::move(values)](std::string_view value, std::string_view value_name) {
        if (std::ranges::find(values, value) == values.end()) {
            throw std::invalid_argument(std::format("{} must be one of {}, got '{}'", value_name, render_choice_list(values), value));
        }
    };
    return result;
}

argument_t argument_t::custom(
    std::string name,
    std::function<std::vector<std::string>(std::span<const std::string>, std::string_view)> complete,
    std::function<void(std::string_view, std::string_view)> validate
) {
    argument_t result = token(std::move(name));
    result.m_complete = std::move(complete);
    result.m_validate = std::move(validate);
    return result;
}

context_t::context_t(arguments_t arguments, std::ostream& out, std::ostream& err):
    arguments(arguments),
    out(out),
    err(err),
    m_stop_requested(false)
{
}

void context_t::stop() {
    m_stop_requested = true;
}

bool context_t::stop_requested() const {
    return m_stop_requested;
}

command_t::command_t():
    path(),
    description(),
    arguments(),
    handler(),
    usage()
{
}

command_t::command_t(std::vector<std::string> path, std::string description, std::function<void(context_t&)> handler):
    command_t(std::move(path), {}, std::move(description), {}, std::move(handler))
{
}

command_t::command_t(std::vector<std::string> path, std::string description, std::vector<argument_t> arguments, std::function<void(context_t&)> handler):
    command_t(std::move(path), {}, std::move(description), std::move(arguments), std::move(handler))
{
}

command_t::command_t(std::vector<std::string> path, std::string usage, std::string description, std::function<void(context_t&)> handler):
    command_t(std::move(path), std::move(usage), std::move(description), {}, std::move(handler))
{
}

command_t::command_t(std::vector<std::string> path, std::string usage, std::string description, std::vector<argument_t> arguments, std::function<void(context_t&)> handler):
    path(std::move(path)),
    description(std::move(description)),
    arguments(std::move(arguments)),
    handler(std::move(handler)),
    usage(std::move(usage))
{
}

application_t::application_t():
    m_running(true),
    m_commands(),
    m_index_by_path(),
    m_max_path_size(0)
{
}

void application_t::add(command_t command) {
    if (command.path.empty()) {
        throw std::logic_error("command path must not be empty");
    }
    if (!command.handler) {
        throw std::logic_error("command handler must not be empty");
    }

    for (const std::string& component : command.path) {
        if (component.empty()) {
            throw std::logic_error("command path component must not be empty");
        }
    }

    bool saw_optional = false;
    for (std::size_t index = 0; index < command.arguments.size(); ++index) {
        const argument_t& argument = command.arguments[index];
        if (argument.name().empty()) {
            throw std::logic_error("argument name must not be empty");
        }
        if (argument.is_variadic() && index + 1 != command.arguments.size()) {
            throw std::logic_error("variadic argument must be last");
        }
        if (saw_optional && !argument.is_optional()) {
            throw std::logic_error("required argument must not follow optional argument");
        }
        saw_optional = saw_optional || argument.is_optional();
    }

    const auto [iterator, inserted] = m_index_by_path.emplace(command.path, m_commands.size());
    if (!inserted) {
        static_cast<void>(iterator);
        throw std::logic_error(std::format("duplicate command path '{}'", render_tokens(command.path)));
    }

    m_max_path_size = std::max(m_max_path_size, command.path.size());
    m_commands.push_back(std::move(command));
}

void application_t::install_help_command() {
    add({
        {"help"},
        "Show commands or commands below a topic.",
        {argument_t::custom("topic", [this](std::span<const std::string> arguments, std::string_view partial) {
            return complete_topic(arguments, partial);
        }).optional().variadic()},
        [this](context_t& context) {
            help(context.out, context.arguments.remaining());
        }
    });
}

bool application_t::running() const {
    return m_running;
}

void application_t::stop() {
    m_running = false;
}

bool application_t::run_command(std::string_view command, std::ostream& out, std::ostream& err) {
    const std::vector<std::string> tokens = tokenize(command);
    return run_arguments(tokens, out, err);
}

bool application_t::run_arguments(std::span<const std::string> arguments, std::ostream& out, std::ostream& err) {
    if (!arguments.empty()) {
        run_tokens(arguments, out, err);
    }
    return m_running;
}

std::vector<std::string> application_t::complete_line(std::string_view line, std::size_t cursor) const {
    const completion_line_t completion = parse_completion_line(line, cursor);
    return complete(completion.prefix, completion.partial);
}

const command_t& application_t::find(std::span<const std::string> tokens) const {
    const std::size_t longest = std::min(tokens.size(), m_max_path_size);
    for (std::size_t size = longest; size > 0; --size) {
        std::vector<std::string> key(tokens.begin(), tokens.begin() + static_cast<std::ptrdiff_t>(size));
        const auto iterator = m_index_by_path.find(key);
        if (iterator != m_index_by_path.end()) {
            return m_commands[iterator->second];
        }
    }

    if (!tokens.empty()) {
        const std::size_t displayed_count = std::min<std::size_t>(tokens.size(), 3);
        std::string attempted;
        for (std::size_t index = 0; index < displayed_count; ++index) {
            if (!attempted.empty()) {
                attempted.push_back(' ');
            }
            attempted += tokens[index];
        }
        throw std::invalid_argument(std::format("unknown command '{}'; use 'help {}'", attempted, tokens.front()));
    }
    throw std::invalid_argument("empty command");
}

const command_t* application_t::find_completion_command(std::span<const std::string> prefix) const {
    const std::size_t longest = std::min(prefix.size(), m_max_path_size);
    for (std::size_t size = longest; size > 0; --size) {
        std::vector<std::string> key(prefix.begin(), prefix.begin() + static_cast<std::ptrdiff_t>(size));
        const auto iterator = m_index_by_path.find(key);
        if (iterator != m_index_by_path.end()) {
            return &m_commands[iterator->second];
        }
    }
    return nullptr;
}

void application_t::validate(const command_t& command, std::span<const std::string> arguments) const {
    const bool variadic = !command.arguments.empty() && command.arguments.back().is_variadic();
    std::size_t required_count = 0;
    for (const argument_t& argument : command.arguments) {
        if (!argument.is_optional()) {
            ++required_count;
        }
    }

    if (arguments.size() < required_count || (!variadic && arguments.size() > command.arguments.size())) {
        throw std::invalid_argument(std::format("usage: {}", render_usage(command)));
    }

    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const argument_t* argument = nullptr;
        if (index < command.arguments.size()) {
            argument = &command.arguments[index];
        } else if (variadic) {
            argument = &command.arguments.back();
        }
        if (!argument) {
            throw std::invalid_argument(std::format("usage: {}", render_usage(command)));
        }
        argument->validate(arguments[index]);
    }
}

std::vector<std::string> application_t::complete(std::span<const std::string> prefix, std::string_view partial) const {
    if (auto candidates = complete_topic(prefix, partial); !candidates.empty()) {
        return candidates;
    }

    const command_t* command = find_completion_command(prefix);
    if (!command) {
        return {};
    }

    const auto arguments = prefix.subspan(command->path.size());
    const argument_t* argument = nullptr;
    if (arguments.size() < command->arguments.size()) {
        argument = &command->arguments[arguments.size()];
    } else if (!command->arguments.empty() && command->arguments.back().is_variadic()) {
        argument = &command->arguments.back();
    }

    if (!argument) {
        return {};
    }
    return argument->complete(arguments, partial);
}

std::vector<std::string> application_t::complete_topic(std::span<const std::string> prefix, std::string_view partial) const {
    std::set<std::string> candidates;
    for (const command_t& command : m_commands) {
        if (command.path.size() <= prefix.size() || !starts_with(command.path, prefix)) {
            continue;
        }

        const std::string& candidate = command.path[prefix.size()];
        if (candidate.starts_with(partial)) {
            candidates.insert(candidate);
        }
    }
    return {candidates.begin(), candidates.end()};
}

void application_t::help(std::ostream& out, std::span<const std::string> prefix) const {
    if (prefix.empty()) {
        std::vector<const command_t*> commands;
        std::set<std::string> topics;
        for (const command_t& command : m_commands) {
            if (command.path.size() == 1) {
                commands.push_back(&command);
            } else {
                topics.insert(command.path.front());
            }
        }

        sort_commands(commands);
        out << "Commands:\n";
        print_commands(out, commands);

        if (!topics.empty()) {
            out << "\nTopics:\n";
            for (const std::string& topic : topics) {
                out << std::format("  {:<16}  Use 'help {}'.\n", topic, topic);
            }
        }
        return;
    }

    std::vector<const command_t*> exact_commands;
    std::vector<const command_t*> direct_commands;
    std::set<std::string> deeper_topics;
    for (const command_t& command : m_commands) {
        if (!starts_with(command.path, prefix)) {
            continue;
        }
        if (command.path.size() == prefix.size()) {
            exact_commands.push_back(&command);
        } else if (command.path.size() == prefix.size() + 1) {
            direct_commands.push_back(&command);
        } else {
            deeper_topics.insert(command.path[prefix.size()]);
        }
    }

    if (exact_commands.empty() && direct_commands.empty() && deeper_topics.empty()) {
        throw std::invalid_argument(std::format("no commands match help topic '{}'", render_tokens(prefix)));
    }

    sort_commands(exact_commands);
    sort_commands(direct_commands);

    bool wrote_section = false;
    if (!exact_commands.empty()) {
        out << (exact_commands.size() == 1 ? "Command:\n" : "Commands:\n");
        print_commands(out, exact_commands);
        wrote_section = true;
    }
    if (!direct_commands.empty()) {
        if (wrote_section) {
            out << '\n';
        }
        out << std::format("{} commands:\n", render_tokens(prefix));
        print_commands(out, direct_commands);
        wrote_section = true;
    }
    if (!deeper_topics.empty()) {
        if (wrote_section) {
            out << '\n';
        }
        out << "Further help:\n";
        for (const std::string& component : deeper_topics) {
            std::string topic = render_tokens(prefix);
            if (!topic.empty()) {
                topic.push_back(' ');
            }
            topic += component;
            out << std::format("  {:<24}  Use 'help {}'.\n", topic, topic);
        }
    }
}

bool application_t::run_tokens(std::span<const std::string> tokens, std::ostream& out, std::ostream& err) {
    const command_t& command = find(tokens);
    const auto argument_values = tokens.subspan(command.path.size());
    validate(command, argument_values);

    arguments_t arguments(argument_values);
    context_t context(arguments, out, err);
    command.handler(context);

    if (context.stop_requested()) {
        stop();
    }
    return m_running;
}

} // namespace m03gm33dj5xo77vegpbspger4r_cli
