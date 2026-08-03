#ifndef M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H
# define M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H

# include <algorithm>
# include <cctype>
# include <cstddef>
# include <format>
# include <functional>
# include <initializer_list>
# include <iosfwd>
# include <map>
# include <span>
# include <string>
# include <string_view>
# include <vector>

namespace m03gm33dj5xo77vegpbspger4r_cli {

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
     * @brief Pops the next argument.
     */
    std::string_view pop(std::string_view name);

    /**
     * @brief Pops the next argument as int.
     */
    int pop_int(std::string_view name);

    /**
     * @brief Pops the next argument as long long.
     */
    long long pop_long_long(std::string_view name);

    /**
     * @brief Pops the next argument as size_t.
     */
    std::size_t pop_size(std::string_view name);

    /**
     * @brief Pops the next argument as float.
     */
    float pop_float(std::string_view name);

    /**
     * @brief Pops the next argument as double.
     */
    double pop_double(std::string_view name);

    /**
     * @brief Pops the next argument as bool.
     */
    bool pop_bool(std::string_view name);

private:
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
     * @brief Completes a command line at cursor.
     */
    std::vector<std::string> complete_line(std::string_view line, std::size_t cursor = std::string_view::npos) const;

private:
    const command_t& find(std::span<const std::string> tokens) const;
    const command_t* find_completion_command(std::span<const std::string> prefix) const;
    void validate(const command_t& command, std::span<const std::string> arguments) const;
    std::vector<std::string> complete(std::span<const std::string> prefix, std::string_view partial) const;
    std::vector<std::string> complete_topic(std::span<const std::string> prefix, std::string_view partial) const;
    void help(std::ostream& out, std::span<const std::string> prefix = {}) const;
    bool run_tokens(std::span<const std::string> tokens, std::ostream& out, std::ostream& err);

private:
    bool m_running;
    std::vector<command_t> m_commands;
    std::map<std::vector<std::string>, std::size_t> m_index_by_path;
    std::size_t m_max_path_size;
};

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
            out = format_token(component, out);
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

private:
    template <typename OutputIterator>
    static OutputIterator format_token(std::string_view token, OutputIterator out) {
        const bool needs_quotes = token.empty() || std::ranges::any_of(token, [](char character) {
            return std::isspace(static_cast<unsigned char>(character)) != 0 || character == '\'' || character == '"' || character == '\\' || character == '#';
        });

        if (!needs_quotes) {
            return format_to(out, "{}", token);
        }

        out = format_to(out, "\"");
        for (const char character : token) {
            if (character == '"' || character == '\\') {
                out = format_to(out, "\\");
            }
            out = format_to(out, "{}", character);
        }
        return format_to(out, "\"");
    }
};

} // namespace std

#endif // M03GM33DJ5XO77VEGPBSPGER4R_CLI_API_H
