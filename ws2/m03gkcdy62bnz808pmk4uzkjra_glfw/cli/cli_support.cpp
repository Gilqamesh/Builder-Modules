#include "cli_support.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

[[noreturn]] void command_error(std::string message) {
    throw std::invalid_argument(std::move(message));
}

std::optional<int> parse_optional_non_negative_integer(std::string_view text, std::string_view name) {
    if (text == "none" || text == "any" || text == "no-limit" || text == "no-preference" || text == "dont-care") {
        return std::nullopt;
    }

    const int value = parse_integer<int>(text, name);
    if (value < 0) {
        command_error(std::format("{} must be non-negative or none, got {}", name, value));
    }
    return value;
}

std::string quote_token(std::string_view token) {
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

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
