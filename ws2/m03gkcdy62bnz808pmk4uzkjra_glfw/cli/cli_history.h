#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_HISTORY_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_HISTORY_H

# include <cstddef>
# include <format>
# include <iostream>
# include <string_view>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

template <typename History, typename MakeChange>
void print_history_changes(
    std::string_view heading,
    const History& history,
    const MakeChange& make_change
) {
    std::cout << std::format(
        "{}: {} sample(s); history[0] is newest\n",
        heading,
        history.size()
    );

    if (history.size() == 0) {
        std::cout << "  no committed samples\n";
        return;
    }

    if (history.size() == 1) {
        std::cout << std::format("  history[0]: {}\n", history.history(0));
        return;
    }

    for (
        std::size_t previous_index = history.size() - 1;
        previous_index > 0;
        --previous_index
    ) {
        const std::size_t current_index = previous_index - 1;
        const auto change = make_change(
            history.history(previous_index),
            history.history(current_index)
        );
        std::cout << std::format(
            "  history[{}] -> history[{}]: {}\n",
            previous_index,
            current_index,
            change
        );
    }
}

template <typename History, typename MakeChange>
void print_history(
    std::string_view heading,
    const History& history,
    const MakeChange& make_change
) {
    std::cout << std::format(
        "{}: {} sample(s); history[0] is newest\n",
        heading,
        history.size()
    );

    if (history.size() == 0) {
        std::cout << "  no committed samples\n";
        return;
    }

    for (std::size_t index = history.size(); index > 0; --index) {
        const std::size_t history_index = index - 1;
        std::cout << std::format(
            "  history[{}]: {}\n",
            history_index,
            history.history(history_index)
        );
    }

    if (history.size() < 2) {
        return;
    }

    std::cout << "  adjacent changes:\n";
    for (
        std::size_t previous_index = history.size() - 1;
        previous_index > 0;
        --previous_index
    ) {
        const std::size_t current_index = previous_index - 1;
        const auto change = make_change(
            history.history(previous_index),
            history.history(current_index)
        );
        std::cout << std::format(
            "    history[{}] -> history[{}]: {}\n",
            previous_index,
            current_index,
            change
        );
    }
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_HISTORY_H
