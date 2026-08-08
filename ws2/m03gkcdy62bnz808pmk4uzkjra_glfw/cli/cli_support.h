#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_SUPPORT_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_SUPPORT_H

# include <m03gm33dj5xo77vegpbspger4r_cli/api.h>

# include <charconv>
# include <cstdint>
# include <format>
# include <optional>
# include <string>
# include <string_view>
# include <system_error>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace cli = m03gm33dj5xo77vegpbspger4r_cli;

using id_t = std::uint64_t;

[[noreturn]] void command_error(std::string message);

template <typename T>
T parse_integer(std::string_view text, std::string_view name);

std::optional<int> parse_optional_non_negative_integer(std::string_view text, std::string_view name);
std::string quote_token(std::string_view token);

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

template <typename T>
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

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_SUPPORT_H
