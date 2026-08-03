#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_ARGUMENTS_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_ARGUMENTS_H

# include <algorithm>
# include <charconv>
# include <cmath>
# include <concepts>
# include <cstddef>
# include <cstdint>
# include <cctype>
# include <format>
# include <limits>
# include <optional>
# include <ranges>
# include <span>
# include <stdexcept>
# include <string>
# include <string_view>
# include <system_error>
# include <utility>
# include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

using id_t = std::uint64_t;

class command_error_t final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

[[noreturn]] inline void command_error(std::string message) {
    throw command_error_t(std::move(message));
}

template <std::integral T>
T parse_integer(std::string_view text, std::string_view name) {
    T value{};
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto [position, error] = std::from_chars(begin, end, value);

    if (error != std::errc{} || position != end) {
        command_error(std::format("{} must be an integer, got '{}'", name, text));
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
        command_error(std::format("{} must be a finite number, got '{}'", name, text));
    }

    return value;
}

inline bool parse_bool(std::string_view text, std::string_view name) {
    if (text == "true" || text == "on" || text == "yes" || text == "1") {
        return true;
    }
    if (text == "false" || text == "off" || text == "no" || text == "0") {
        return false;
    }

    command_error(std::format(
        "{} must be true/false, on/off, yes/no or 1/0, got '{}'",
        name,
        text
    ));
}

inline std::optional<int> parse_optional_non_negative_integer(
    std::string_view text,
    std::string_view name
) {
    if (
        text == "none" ||
        text == "any" ||
        text == "no-limit" ||
        text == "no-preference" ||
        text == "dont-care"
    ) {
        return std::nullopt;
    }

    const int value = parse_integer<int>(text, name);
    if (value < 0) {
        command_error(std::format("{} must be non-negative or none, got {}", name, value));
    }

    return value;
}

inline std::string quote_token(std::string_view token) {
    const bool needs_quotes = token.empty() || std::ranges::any_of(token, [](unsigned char character) {
        return std::isspace(character) != 0 || character == '\'' || character == '"' || character == '\\' || character == '#';
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

inline std::string render_tokens(std::span<const std::string> tokens) {
    std::string result;
    for (std::size_t index = 0; index < tokens.size(); ++index) {
        if (index != 0) {
            result.push_back(' ');
        }
        result += quote_token(tokens[index]);
    }
    return result;
}

inline std::vector<std::string> tokenize(std::string_view line) {
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
            if (index + 1 >= line.size()) {
                command_error("line ends with an incomplete escape");
            }
            token.push_back(line[++index]);
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

    if (quote != quote_t::none) {
        command_error("unterminated quoted string");
    }

    if (token_started) {
        tokens.push_back(std::move(token));
    }

    return tokens;
}

class arguments_t {
public:
    explicit arguments_t(std::span<const std::string> values):
        m_values(values)
    {
    }

    bool empty() const {
        return m_index == m_values.size();
    }

    std::size_t size() const {
        return m_values.size() - m_index;
    }

    std::span<const std::string> remaining() const {
        return m_values.subspan(m_index);
    }

    std::string_view pop(std::string_view name) {
        if (empty()) {
            command_error(std::format("missing {}", name));
        }
        return m_values[m_index++];
    }

    id_t pop_id(std::string_view name) {
        return parse_integer<id_t>(pop(name), name);
    }

    int pop_int(std::string_view name) {
        return parse_integer<int>(pop(name), name);
    }

    long long pop_long_long(std::string_view name) {
        return parse_integer<long long>(pop(name), name);
    }

    std::size_t pop_size(std::string_view name) {
        const auto value = parse_integer<unsigned long long>(pop(name), name);
        if (value > std::numeric_limits<std::size_t>::max()) {
            command_error(std::format("{} is too large", name));
        }
        return static_cast<std::size_t>(value);
    }

    float pop_float(std::string_view name) {
        return parse_floating<float>(pop(name), name);
    }

    double pop_double(std::string_view name) {
        return parse_floating<double>(pop(name), name);
    }

    bool pop_bool(std::string_view name) {
        return parse_bool(pop(name), name);
    }

    void expect_end(std::string_view usage) const {
        if (!empty()) {
            command_error(std::format("usage: {}", usage));
        }
    }

private:
    std::span<const std::string> m_values;
    std::size_t m_index = 0;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_ARGUMENTS_H
