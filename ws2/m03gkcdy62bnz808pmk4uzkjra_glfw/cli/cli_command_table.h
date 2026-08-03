#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_COMMAND_TABLE_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_COMMAND_TABLE_H

# include "cli_arguments.h"

# include <algorithm>
# include <cstddef>
# include <format>
# include <functional>
# include <initializer_list>
# include <iostream>
# include <ranges>
# include <set>
# include <span>
# include <string>
# include <string_view>
# include <utility>
# include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

class command_table_t {
public:
    using handler_t = std::function<void(arguments_t&)>;

    struct completion_context_t {
        std::span<const std::string> arguments;
        std::string_view partial;
    };

    struct completion_result_t {
        std::vector<std::string> candidates;
        bool use_files = false;
    };

    using completion_handler_t = std::function<completion_result_t(const completion_context_t&)>;

    struct argument_spec_t {
        std::string name;
        completion_handler_t complete;
    };

    struct command_t {
        std::vector<std::string> path;
        std::string usage;
        std::string description;
        handler_t handler;
        std::vector<argument_spec_t> arguments;
        completion_handler_t complete_arguments;
        bool poll_after = true;
    };

    static argument_spec_t argument(std::string name) {
        return argument(std::move(name), completion_handler_t{});
    }

    static argument_spec_t argument(
        std::string name,
        completion_handler_t complete
    ) {
        return argument_spec_t{
            .name = std::move(name),
            .complete = std::move(complete)
        };
    }

    static argument_spec_t values_argument(
        std::string name,
        std::initializer_list<std::string_view> values
    ) {
        return argument(
            std::move(name),
            values_completion(values)
        );
    }

    static argument_spec_t files_argument(std::string name) {
        return argument(
            std::move(name),
            [](const completion_context_t&) {
                return completion_result_t{.use_files = true};
            }
        );
    }

    static completion_handler_t values_completion(
        std::initializer_list<std::string_view> values
    ) {
        std::vector<std::string> owned_values;
        owned_values.reserve(values.size());
        for (const std::string_view value : values) {
            owned_values.emplace_back(value);
        }

        return [values = std::move(owned_values)](const completion_context_t& context) {
            return complete_values(values, context.partial);
        };
    }

    static completion_result_t complete_values(
        std::span<const std::string> values,
        std::string_view partial
    ) {
        std::set<std::string> candidates;
        for (const std::string& value : values) {
            if (value.starts_with(partial)) {
                candidates.insert(value);
            }
        }

        return completion_result_t{
            .candidates = {candidates.begin(), candidates.end()}
        };
    }

    void add(
        std::initializer_list<std::string_view> path,
        std::string usage,
        std::string description,
        handler_t handler,
        bool poll_after = true
    ) {
        add_impl(
            path,
            std::move(usage),
            std::move(description),
            std::move(handler),
            {},
            {},
            poll_after
        );
    }

    void add(
        std::initializer_list<std::string_view> path,
        std::string usage,
        std::string description,
        handler_t handler,
        std::vector<argument_spec_t> arguments,
        bool poll_after = true
    ) {
        add_impl(
            path,
            std::move(usage),
            std::move(description),
            std::move(handler),
            std::move(arguments),
            {},
            poll_after
        );
    }

    void add(
        std::initializer_list<std::string_view> path,
        std::string usage,
        std::string description,
        handler_t handler,
        completion_handler_t complete_arguments,
        bool poll_after = true
    ) {
        add_impl(
            path,
            std::move(usage),
            std::move(description),
            std::move(handler),
            {},
            std::move(complete_arguments),
            poll_after
        );
    }

    void add_impl(
        std::initializer_list<std::string_view> path,
        std::string usage,
        std::string description,
        handler_t handler,
        std::vector<argument_spec_t> arguments,
        completion_handler_t complete_arguments,
        bool poll_after
    ) {
        if (path.size() == 0) {
            throw std::logic_error("command path must not be empty");
        }

        std::vector<std::string> owned_path;
        owned_path.reserve(path.size());
        for (const std::string_view component : path) {
            if (component.empty()) {
                throw std::logic_error("command path component must not be empty");
            }
            owned_path.emplace_back(component);
        }

        if (std::ranges::any_of(m_commands, [&](const command_t& command) {
            return command.path == owned_path;
        })) {
            throw std::logic_error(std::format("duplicate command path '{}'", render_path(owned_path)));
        }

        m_commands.push_back(command_t{
            .path = std::move(owned_path),
            .usage = std::move(usage),
            .description = std::move(description),
            .handler = std::move(handler),
            .arguments = std::move(arguments),
            .complete_arguments = std::move(complete_arguments),
            .poll_after = poll_after
        });
    }

    const command_t& find(std::span<const std::string> tokens) const {
        const command_t* best_match = nullptr;

        for (const command_t& command : m_commands) {
            if (command.path.size() > tokens.size()) {
                continue;
            }

            bool matches = true;
            for (std::size_t index = 0; index < command.path.size(); ++index) {
                if (command.path[index] != tokens[index]) {
                    matches = false;
                    break;
                }
            }

            if (matches && (!best_match || best_match->path.size() < command.path.size())) {
                best_match = &command;
            }
        }

        if (best_match) {
            return *best_match;
        }

        const std::size_t displayed_token_count = std::min<std::size_t>(tokens.size(), 3);
        std::string attempted;
        for (std::size_t index = 0; index < displayed_token_count; ++index) {
            if (!attempted.empty()) {
                attempted.push_back(' ');
            }
            attempted += tokens[index];
        }

        if (!tokens.empty()) {
            command_error(std::format(
                "unknown command '{}'; use 'help {}'",
                attempted,
                tokens.front()
            ));
        }
        command_error("empty command");
    }

    std::vector<std::string> complete_next_component(
        std::span<const std::string> prefix,
        std::string_view partial
    ) const {
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

    completion_result_t complete(
        std::span<const std::string> prefix,
        std::string_view partial
    ) const {
        if (auto candidates = complete_next_component(prefix, partial); !candidates.empty()) {
            return completion_result_t{
                .candidates = std::move(candidates)
            };
        }

        const command_t* command = find_completion_command(prefix);
        if (!command) {
            return {};
        }

        const auto argument_prefix = prefix.subspan(command->path.size());
        const completion_context_t context{
            .arguments = argument_prefix,
            .partial = partial
        };

        if (command->complete_arguments) {
            return command->complete_arguments(context);
        }

        if (argument_prefix.size() >= command->arguments.size()) {
            return {};
        }

        const argument_spec_t& argument = command->arguments[argument_prefix.size()];
        if (!argument.complete) {
            return {};
        }
        return argument.complete(context);
    }

    void print_help(std::span<const std::string> prefix = {}) const {
        if (prefix.empty()) {
            std::vector<const command_t*> core_commands;
            std::set<std::string> topics;

            for (const command_t& command : m_commands) {
                if (command.path.size() == 1) {
                    core_commands.push_back(&command);
                } else {
                    topics.insert(command.path.front());
                }
            }

            sort_commands(core_commands);

            std::cout << "Commands:\n";
            print_commands(core_commands);

            if (!topics.empty()) {
                std::cout << "\nTopics:\n";
                for (const std::string& topic : topics) {
                    std::cout << std::format(
                        "  {:<12}  Use 'help {}'.\n",
                        topic,
                        topic
                    );
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
            command_error(std::format(
                "no commands match help topic '{}'",
                render_tokens(prefix)
            ));
        }

        sort_commands(exact_commands);
        sort_commands(direct_commands);

        bool wrote_section = false;
        if (!exact_commands.empty()) {
            std::cout << (exact_commands.size() == 1 ? "Command:\n" : "Commands:\n");
            print_commands(exact_commands);
            wrote_section = true;
        }

        if (!direct_commands.empty()) {
            if (wrote_section) {
                std::cout << '\n';
            }
            std::cout << std::format("{} commands:\n", render_tokens(prefix));
            print_commands(direct_commands);
            wrote_section = true;
        }

        if (!deeper_topics.empty()) {
            if (wrote_section) {
                std::cout << '\n';
            }
            std::cout << "Further help:\n";
            for (const std::string& component : deeper_topics) {
                std::string topic = render_tokens(prefix);
                if (!topic.empty()) {
                    topic.push_back(' ');
                }
                topic += component;
                std::cout << std::format(
                    "  {:<20}  Use 'help {}'.\n",
                    topic,
                    topic
                );
            }
        }
    }

private:
    static bool starts_with(
        std::span<const std::string> path,
        std::span<const std::string> prefix
    ) {
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

    static void sort_commands(std::vector<const command_t*>& commands) {
        std::ranges::sort(commands, {}, [](const command_t* command) {
            return command->usage;
        });
    }

    const command_t* find_completion_command(
        std::span<const std::string> prefix
    ) const {
        const command_t* best_match = nullptr;

        for (const command_t& command : m_commands) {
            if (prefix.size() < command.path.size()) {
                continue;
            }
            if (!starts_with(prefix, command.path)) {
                continue;
            }
            if (!best_match || best_match->path.size() < command.path.size()) {
                best_match = &command;
            }
        }

        return best_match;
    }

    static std::string render_path(std::span<const std::string> path) {
        std::string result;
        for (std::size_t index = 0; index < path.size(); ++index) {
            if (index != 0) {
                result.push_back(' ');
            }
            result += path[index];
        }
        return result;
    }

    static void print_commands(std::span<const command_t* const> commands) {
        constexpr std::size_t maximum_usage_width = 56;

        std::size_t usage_width = 0;
        for (const command_t* command : commands) {
            usage_width = std::max(
                usage_width,
                std::min(command->usage.size(), maximum_usage_width)
            );
        }

        for (const command_t* command : commands) {
            if (command->usage.size() <= maximum_usage_width) {
                std::cout << std::format(
                    "  {:<{}}  {}\n",
                    command->usage,
                    usage_width,
                    command->description
                );
            } else {
                std::cout
                    << "  " << command->usage << '\n'
                    << "      " << command->description << '\n';
            }
        }
    }

private:
    std::vector<command_t> m_commands;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_COMMAND_TABLE_H
