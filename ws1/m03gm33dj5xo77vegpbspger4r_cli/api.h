#ifndef M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H
# define M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H

# include <algorithm>
# include <charconv>
# include <cctype>
# include <cmath>
# include <concepts>
# include <cstddef>
# include <filesystem>
# include <format>
# include <functional>
# include <initializer_list>
# include <iosfwd>
# include <map>
# include <span>
# include <stdexcept>
# include <string>
# include <string_view>
# include <system_error>
# include <type_traits>
# include <utility>
# include <vector>

namespace m03gm33dj5xo77vegpbspger4r_cli {

template <typename T>
struct argument_parser_t;

template <>
struct argument_parser_t<std::string_view>;

template <>
struct argument_parser_t<std::string>;

template <>
struct argument_parser_t<std::filesystem::path>;

template <>
struct argument_parser_t<bool>;

template <std::integral T>
requires (!std::same_as<T, bool>)
struct argument_parser_t<T>;

template <std::floating_point T>
struct argument_parser_t<T>;

/**
 * @brief Maintains a cursor over command arguments.
 */
class arguments_t {
public:
    /**
     * @brief Constructs an argument cursor over values.
     */
    explicit arguments_t(std::span<const std::string> values);

    /**
     * @brief Returns true if no arguments remain.
     */
    bool empty() const;

    /**
     * @brief Returns the number of remaining arguments.
     */
    std::size_t size() const;

    /**
     * @brief Returns all remaining arguments.
     */
    std::span<const std::string> remaining() const;

    /**
     * @brief Pops the next argument as T.
     */
    template <typename T = std::string_view>
    T pop(std::string_view name);

    /**
     * @brief Throws if arguments remain.
     */
    void expect_end(std::string_view usage) const;

private:
    std::string_view pop_token(std::string_view name);

    std::span<const std::string> m_values;
    std::size_t m_index;
};

/**
 * @brief Describes one command argument.
 */
class argument_t {
public:
    /**
     * @brief Constructs a plain required token argument.
     */
    argument_t();

    /**
     * @brief Marks the argument optional.
     */
    argument_t optional() &&;

    /**
     * @brief Marks the argument variadic.
     */
    argument_t variadic() &&;

    /**
     * @brief Returns the argument name.
     */
    std::string_view name() const;

    /**
     * @brief Returns the argument usage override.
     */
    std::string_view usage() const;

    /**
     * @brief Returns true if the argument is optional.
     */
    bool is_optional() const;

    /**
     * @brief Returns true if the argument is variadic.
     */
    bool is_variadic() const;

    /**
     * @brief Completes a partial value.
     */
    std::vector<std::string> complete(std::span<const std::string> arguments, std::string_view partial) const;

    /**
     * @brief Validates a value.
     */
    void validate(std::string_view value) const;

    /**
     * @brief Creates an unconstrained token argument.
     */
    static argument_t token(std::string name);

    /**
     * @brief Creates a signed integer argument.
     */
    static argument_t integer(std::string name);

    /**
     * @brief Creates an unsigned integer argument.
     */
    static argument_t unsigned_integer(std::string name);

    /**
     * @brief Creates a finite number argument.
     */
    static argument_t number(std::string name);

    /**
     * @brief Creates a boolean argument.
     */
    static argument_t boolean(std::string name);

    /**
     * @brief Creates a filesystem path argument.
     */
    static argument_t file(std::string name);

    /**
     * @brief Creates an argument constrained to a fixed set of values.
     */
    static argument_t choice(std::string name, std::initializer_list<std::string_view> values);

    /**
     * @brief Creates an argument constrained to a fixed set of values.
     */
    static argument_t choice(std::string name, std::vector<std::string> values);

    /**
     * @brief Creates an argument from custom completion and validation callbacks.
     */
    static argument_t custom(
        std::string name,
        std::function<std::vector<std::string>(std::span<const std::string>, std::string_view)> complete = {},
        std::function<void(std::string_view, std::string_view)> validate = {}
    );

private:
    std::string m_name;
    std::string m_usage;
    std::function<std::vector<std::string>(std::span<const std::string>, std::string_view)> m_complete;
    std::function<void(std::string_view, std::string_view)> m_validate;
    bool m_is_optional;
    bool m_is_variadic;
};

/**
 * @brief Provides command handlers with arguments and output streams.
 */
class context_t {
public:
    /**
     * @brief Constructs a command context.
     */
    context_t(arguments_t arguments, std::ostream& out, std::ostream& err);

    /**
     * @brief The command arguments.
     */
    arguments_t arguments;

    /**
     * @brief The standard output stream.
     */
    std::ostream& out;

    /**
     * @brief The standard error stream.
     */
    std::ostream& err;

    /**
     * @brief Requests application shutdown after the command returns.
     */
    void stop();

    /**
     * @brief Returns true if the command requested shutdown.
     */
    bool stop_requested() const;

private:
    bool m_stop_requested;
};

/**
 * @brief Describes one command table entry.
 */
struct command_t {
    /**
     * @brief Constructs an empty command.
     */
    command_t();

    /**
     * @brief Constructs a command with generated usage.
     */
    command_t(std::vector<std::string> path, std::string description, std::function<void(context_t&)> handler);

    /**
     * @brief Constructs a command with generated usage.
     */
    command_t(std::vector<std::string> path, std::string description, std::vector<argument_t> arguments, std::function<void(context_t&)> handler);

    /**
     * @brief Constructs a command with explicit usage.
     */
    command_t(std::vector<std::string> path, std::string usage, std::string description, std::function<void(context_t&)> handler);

    /**
     * @brief Constructs a command with explicit usage.
     */
    command_t(std::vector<std::string> path, std::string usage, std::string description, std::vector<argument_t> arguments, std::function<void(context_t&)> handler);

    /**
     * @brief The token path used for dispatch.
     */
    std::vector<std::string> path;

    /**
     * @brief Additional dispatch paths hidden from help and completion.
     */
    std::vector<std::vector<std::string>> aliases;

    /**
     * @brief Excludes the command from help and command completion.
     */
    bool hidden;

    /**
     * @brief Short help text.
     */
    std::string description;

    /**
     * @brief Positional arguments consumed after the path.
     */
    std::vector<argument_t> arguments;

    /**
     * @brief Handler called after lookup and validation.
     */
    std::function<void(context_t&)> handler;

    /**
     * @brief Optional full usage string overriding generated usage.
     */
    std::string usage;
};

/**
 * @brief Stores a command table and dispatches tokenized input.
 */
class application_t {
public:
    /**
     * @brief Constructs an empty command application.
     */
    application_t();

    /**
     * @brief Adds a command table entry.
     */
    void add(command_t command);

    /**
     * @brief Installs the built-in help command.
     */
    void install_help_command();

    /**
     * @brief Sets a handler for unknown command lines.
     *
     * The handler receives the original tokens as arguments and should return
     * true only when it handled the line.
     */
    void fallback(std::function<bool(context_t&)> handler);

    /**
     * @brief Returns true while the application should keep running.
     */
    bool running() const;

    /**
     * @brief Stops the application.
     */
    void stop();

    /**
     * @brief Tokenizes and runs one command line.
     */
    bool run_command(std::string_view command, std::ostream& out, std::ostream& err);

    /**
     * @brief Runs already-tokenized command arguments.
     */
    bool run_arguments(std::span<const std::string> arguments, std::ostream& out, std::ostream& err);

    /**
     * @brief Runs commands from a script file.
     */
    bool run_script(const std::filesystem::path& path, std::ostream& out, std::ostream& err, bool echo_commands = true);

    /**
     * @brief Completes a command line at cursor.
     */
    std::vector<std::string> complete_line(std::string_view line, std::size_t cursor = std::string_view::npos) const;

private:
    std::pair<const command_t*, std::size_t> find(std::span<const std::string> tokens) const;
    std::pair<const command_t*, std::size_t> find_completion_command(std::span<const std::string> prefix) const;
    bool has_help_topic(std::span<const std::string> prefix) const;
    std::vector<std::string> postfix_help_topic(std::span<const std::string> prefix) const;
    void validate(const command_t& command, std::span<const std::string> arguments) const;
    std::vector<std::string> complete(std::span<const std::string> prefix, std::string_view partial) const;
    std::vector<std::string> complete_topic(std::span<const std::string> prefix, std::string_view partial) const;
    void help(std::ostream& out, std::span<const std::string> prefix = {}) const;
    bool run_fallback(std::span<const std::string> tokens, std::ostream& out, std::ostream& err);
    bool run_tokens(std::span<const std::string> tokens, std::ostream& out, std::ostream& err);

private:
    bool m_running;
    std::vector<command_t> m_commands;
    std::map<std::vector<std::string>, std::size_t> m_index_by_path;
    std::size_t m_max_path_size;
    std::function<bool(context_t&)> m_fallback;
};

} // namespace m03gm33dj5xo77vegpbspger4r_cli

namespace std {

template <>
struct formatter<m03gm33dj5xo77vegpbspger4r_cli::argument_t>;

template <>
struct formatter<m03gm33dj5xo77vegpbspger4r_cli::command_t>;

} // namespace std

namespace m03gm33dj5xo77vegpbspger4r_cli {

template <>
struct argument_parser_t<std::string_view> {
    static std::string_view parse(std::string_view value, std::string_view) {
        return value;
    }
};

template <>
struct argument_parser_t<std::string> {
    static std::string parse(std::string_view value, std::string_view) {
        return std::string(value);
    }
};

template <>
struct argument_parser_t<std::filesystem::path> {
    static std::filesystem::path parse(std::string_view value, std::string_view) {
        return std::filesystem::path(std::string(value));
    }
};

template <>
struct argument_parser_t<bool> {
    static bool parse(std::string_view value, std::string_view name) {
        if (value == "true" || value == "on" || value == "yes" || value == "1") {
            return true;
        }
        if (value == "false" || value == "off" || value == "no" || value == "0") {
            return false;
        }
        throw std::invalid_argument(std::format("{} must be true/false, on/off, yes/no or 1/0, got '{}'", name, value));
    }
};

template <std::integral T>
requires (!std::same_as<T, bool>)
struct argument_parser_t<T> {
    static T parse(std::string_view value, std::string_view name) {
        T result{};
        const char* const begin = value.data();
        const char* const end = begin + value.size();
        const auto [position, error] = std::from_chars(begin, end, result);
        if (error != std::errc{} || position != end) {
            throw std::invalid_argument(std::format("{} must be an integer, got '{}'", name, value));
        }
        return result;
    }
};

template <std::floating_point T>
struct argument_parser_t<T> {
    static T parse(std::string_view value, std::string_view name) {
        T result{};
        const char* const begin = value.data();
        const char* const end = begin + value.size();
        const auto [position, error] = std::from_chars(begin, end, result);
        if (error != std::errc{} || position != end || !std::isfinite(result)) {
            throw std::invalid_argument(std::format("{} must be a finite number, got '{}'", name, value));
        }
        return result;
    }
};

template <typename T>
T arguments_t::pop(std::string_view name) {
    static_assert(std::same_as<T, std::remove_cvref_t<T>>, "arguments_t::pop<T>: T must not be cv-qualified or a reference.");
    return argument_parser_t<T>::parse(pop_token(name), name);
}

} // namespace m03gm33dj5xo77vegpbspger4r_cli

namespace std {

template <>
struct formatter<m03gm33dj5xo77vegpbspger4r_cli::argument_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gm33dj5xo77vegpbspger4r_cli::argument_t& argument, auto& ctx) const {
        auto out = ctx.out();

        if (argument.is_optional()) {
            out = format_to(out, "[");
        }

        if (!argument.usage().empty()) {
            out = format_to(out, "{}", argument.usage());
        } else {
            out = format_to(out, "<{}>", argument.name());
        }

        if (argument.is_variadic()) {
            out = format_to(out, "...");
        }
        
        if (argument.is_optional()) {
            out = format_to(out, "]");
        }

        return out;
    }
};

template <>
struct formatter<m03gm33dj5xo77vegpbspger4r_cli::command_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gm33dj5xo77vegpbspger4r_cli::command_t& command, auto& ctx) const {
        auto out = ctx.out();

        if (!command.usage.empty()) {
            return format_to(out, "{}", command.usage);
        }

        bool first = true;
        for (const std::string& component : command.path) {
            if (!first) {
                out = format_to(out, " ");
            }
            first = false;
            out = format_to(out, "{}", component);
        }

        for (const m03gm33dj5xo77vegpbspger4r_cli::argument_t& argument : command.arguments) {
            if (!first) {
                out = format_to(out, " ");
            }
            first = false;
            out = format_to(out, "{}", argument);
        }

        return out;
    }
};

} // namespace std

#endif // M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H
