#include "api.h"

#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t;
using m03gli1rb5p56mncplipxpf3he_ring_buffer::commit_policy_t;
using m03gli1rb5p56mncplipxpf3he_ring_buffer::staging_policy_t;

constexpr std::size_t default_history_capacity = 4;

struct buffer_config_t {
    staging_policy_t staging_policy = staging_policy_t::overlapping;
    commit_policy_t commit_policy = commit_policy_t::advance;
};

constexpr buffer_config_t default_buffer_config{
    .staging_policy = staging_policy_t::overlapping,
    .commit_policy = commit_policy_t::advance
};

template <typename T>
using ring_buffer_variant_t = std::variant<
    ring_buffer_t<T, staging_policy_t::overlapping, commit_policy_t::advance>,
    ring_buffer_t<T, staging_policy_t::overlapping, commit_policy_t::copy_with_advance>,
    ring_buffer_t<T, staging_policy_t::dedicated, commit_policy_t::advance>,
    ring_buffer_t<T, staging_policy_t::dedicated, commit_policy_t::copy_with_advance>
>;

std::uint64_t parse_u64(std::string_view text, std::string_view name) {
    if (text.empty() || text.front() == '-') {
        throw std::invalid_argument(std::format("{} must be a non-negative integer", name));
    }

    std::size_t parsed = 0;
    const auto value = std::stoull(std::string(text), &parsed, 10);
    if (parsed != text.size()) {
        throw std::invalid_argument(std::format("{} must be a non-negative integer, got '{}'", name, text));
    }

    return value;
}

std::int64_t parse_i64(std::string_view text, std::string_view name) {
    if (text.empty()) {
        throw std::invalid_argument(std::format("{} must be an integer", name));
    }

    std::size_t parsed = 0;
    const auto value = std::stoll(std::string(text), &parsed, 10);
    if (parsed != text.size()) {
        throw std::invalid_argument(std::format("{} must be an integer, got '{}'", name, text));
    }

    return value;
}

std::size_t parse_size(std::string_view text, std::string_view name) {
    const auto value = parse_u64(text, name);
    if (value > std::numeric_limits<std::size_t>::max()) {
        throw std::out_of_range(std::format("{} is too large for this platform", name));
    }

    return static_cast<std::size_t>(value);
}

std::vector<std::string> split_command_line(const std::string& line) {
    std::istringstream in(line);
    std::vector<std::string> args;

    for (std::string arg; in >> arg;) {
        args.push_back(arg);
    }

    return args;
}

std::string_view staging_policy_name(staging_policy_t policy) {
    switch (policy) {
    case staging_policy_t::overlapping:
        return "overlapping";
    case staging_policy_t::dedicated:
        return "dedicated";
    }

    throw std::logic_error("staging_policy_name: unknown staging policy");
}

std::string_view staging_policy_description(staging_policy_t policy) {
    switch (policy) {
    case staging_policy_t::overlapping:
        return "stage aliases oldest history when full";
    case staging_policy_t::dedicated:
        return "stage never aliases history";
    }

    throw std::logic_error("staging_policy_description: unknown staging policy");
}

std::string_view commit_policy_name(commit_policy_t policy) {
    switch (policy) {
    case commit_policy_t::advance:
        return "advance";
    case commit_policy_t::copy_with_advance:
        return "copy_with_advance";
    }

    throw std::logic_error("commit_policy_name: unknown commit policy");
}

std::string_view commit_policy_description(commit_policy_t policy) {
    switch (policy) {
    case commit_policy_t::advance:
        return "advances to the next staging slot without modifying it";
    case commit_policy_t::copy_with_advance:
        return "copies the staged value into the next staging slot and advances to it";
    }

    throw std::logic_error("commit_policy_description: unknown commit policy");
}

std::optional<staging_policy_t> try_parse_staging_policy(std::string_view text) {
    if (text == "overlapping") {
        return staging_policy_t::overlapping;
    }
    if (text == "dedicated") {
        return staging_policy_t::dedicated;
    }

    return std::nullopt;
}

std::optional<commit_policy_t> try_parse_commit_policy(std::string_view text) {
    if (text == "advance") {
        return commit_policy_t::advance;
    }
    if (text == "copy_with_advance") {
        return commit_policy_t::copy_with_advance;
    }

    return std::nullopt;
}

staging_policy_t parse_staging_policy(std::string_view text, std::string_view name) {
    if (const auto policy = try_parse_staging_policy(text)) {
        return *policy;
    }

    throw std::invalid_argument(std::format("{} has unknown staging policy '{}'", name, text));
}

commit_policy_t parse_commit_policy(std::string_view text, std::string_view name) {
    if (const auto policy = try_parse_commit_policy(text)) {
        return *policy;
    }

    throw std::invalid_argument(std::format("{} has unknown commit policy '{}'", name, text));
}

staging_policy_t prompt_staging_policy(staging_policy_t default_policy) {
    std::cout << "staging_policy_t values:\n";
    for (const staging_policy_t policy : {
        staging_policy_t::overlapping,
        staging_policy_t::dedicated
    }) {
        std::cout << std::format(
            "  {:<15}  {}\n",
            staging_policy_name(policy),
            staging_policy_description(policy)
        );
    }

    for (;;) {
        std::cout << std::format(
            "staging_policy [{}]> ",
            staging_policy_name(default_policy)
        ) << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            return default_policy;
        }

        const auto tokens = split_command_line(line);
        if (tokens.empty()) {
            return default_policy;
        }
        if (tokens.size() != 1) {
            std::cout << "enter one staging policy, or an empty line for the default\n";
            continue;
        }

        if (const auto policy = try_parse_staging_policy(tokens.front())) {
            return *policy;
        }

        std::cout << std::format("unknown staging policy '{}'\n", tokens.front());
    }
}

commit_policy_t prompt_commit_policy(commit_policy_t default_policy) {
    std::cout << "commit_policy_t values:\n";
    for (const commit_policy_t policy : {
        commit_policy_t::advance,
        commit_policy_t::copy_with_advance
    }) {
        std::cout << std::format(
            "  {:<16}  {}\n",
            commit_policy_name(policy),
            commit_policy_description(policy)
        );
    }

    for (;;) {
        std::cout << std::format(
            "commit_policy [{}]> ",
            commit_policy_name(default_policy)
        ) << std::flush;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            return default_policy;
        }

        const auto tokens = split_command_line(line);
        if (tokens.empty()) {
            return default_policy;
        }
        if (tokens.size() != 1) {
            std::cout << "enter one commit policy, or an empty line for the default\n";
            continue;
        }

        if (const auto policy = try_parse_commit_policy(tokens.front())) {
            return *policy;
        }

        std::cout << std::format("unknown commit policy '{}'\n", tokens.front());
    }
}

buffer_config_t prompt_buffer_config(buffer_config_t default_config) {
    return {
        .staging_policy = prompt_staging_policy(default_config.staging_policy),
        .commit_policy = prompt_commit_policy(default_config.commit_policy)
    };
}

template <typename T>
ring_buffer_variant_t<T> make_ring_buffer(buffer_config_t config, std::size_t history_capacity) {
    if (
        config.staging_policy == staging_policy_t::overlapping &&
        config.commit_policy == commit_policy_t::advance
    ) {
        return ring_buffer_t<T, staging_policy_t::overlapping, commit_policy_t::advance>(history_capacity);
    }
    if (
        config.staging_policy == staging_policy_t::overlapping &&
        config.commit_policy == commit_policy_t::copy_with_advance
    ) {
        return ring_buffer_t<T, staging_policy_t::overlapping, commit_policy_t::copy_with_advance>(history_capacity);
    }
    if (
        config.staging_policy == staging_policy_t::dedicated &&
        config.commit_policy == commit_policy_t::advance
    ) {
        return ring_buffer_t<T, staging_policy_t::dedicated, commit_policy_t::advance>(history_capacity);
    }
    if (
        config.staging_policy == staging_policy_t::dedicated &&
        config.commit_policy == commit_policy_t::copy_with_advance
    ) {
        return ring_buffer_t<T, staging_policy_t::dedicated, commit_policy_t::copy_with_advance>(history_capacity);
    }

    throw std::logic_error("make_ring_buffer: unknown buffer configuration");
}

void print_command_help() {
    std::cout
        << "commands:\n"
        << "  help\n"
        << "      Show this help text.\n"
        << "  new <history_capacity> [staging_policy commit_policy]\n"
        << "  reset <history_capacity> [staging_policy commit_policy]\n"
        << "      Construct a new ring_buffer_t<int64_t>. In the REPL, omitted policies are prompted.\n"
        << "  stage\n"
        << "      Print stage().\n"
        << "  stage <value>\n"
        << "      Assign stage() = value.\n"
        << "  commit\n"
        << "      Call commit().\n"
        << "  publish <value>\n"
        << "      Assign stage() = value, then call commit().\n"
        << "  history <offset>\n"
        << "      Print history(offset).\n"
        << "  history <offset> <value>\n"
        << "      Assign history(offset) = value.\n"
        << "  history_size\n"
        << "      Print history_size().\n"
        << "  history_capacity\n"
        << "      Print history_capacity().\n"
        << "  format\n"
        << "      Print std::format(\"{}\", ring_buffer).\n"
        << "  quit\n"
        << "  exit\n"
        << "      Leave the REPL.\n"
        << "\n"
        << "staging_policy values: overlapping, dedicated\n"
        << "commit_policy values: advance, copy_with_advance\n";
}

void print_usage(std::string_view program) {
    std::cout
        << "usage: " << program << " [command [args...]]\n"
        << "\n"
        << "With no arguments, starts an interactive REPL. With arguments, runs one command.\n"
        << "Use `repl [history_capacity]` to start a REPL with an optional initial buffer.\n"
        << "\n";
    print_command_help();
}

template <typename RingBuffer>
void print_ring_buffer_format(const RingBuffer& ring_buffer) {
    std::cout << std::format("{}", ring_buffer) << '\n';
}

template <typename T>
void print_ring_buffer_variant_format(const ring_buffer_variant_t<T>& ring_buffer) {
    std::visit([](const auto& concrete_ring_buffer) {
        print_ring_buffer_format(concrete_ring_buffer);
    }, ring_buffer);
}

class repl_t {
public:
    repl_t() = default;

    explicit repl_t(std::size_t history_capacity) {
        reset(history_capacity, default_buffer_config);
    }

    void prompt_for_reset_config(bool value) {
        m_prompt_for_reset_config = value;
    }

    bool execute(const std::vector<std::string>& args) {
        if (args.empty()) {
            return true;
        }

        const auto& command = args[0];

        if (command == "help" || command == "--help" || command == "-h") {
            print_command_help();
            return true;
        }

        if (command == "quit" || command == "exit") {
            return false;
        }

        if (command == "new" || command == "reset") {
            if (args.size() != 2 && args.size() != 4) {
                throw std::invalid_argument(std::format(
                    "{} expects history_capacity and optional staging and commit policies",
                    command
                ));
            }

            const auto history_capacity = parse_size(args[1], "history_capacity");
            const auto config = args.size() == 4
                ? buffer_config_t{
                    .staging_policy = parse_staging_policy(args[2], "staging_policy"),
                    .commit_policy = parse_commit_policy(args[3], "commit_policy")
                }
                : reset_default_config();
            reset(history_capacity, config);
            return true;
        }

        if (command == "stage") {
            auto& ring_buffer = require_ring_buffer();
            if (args.size() == 1) {
                std::visit([](auto& concrete_ring_buffer) {
                    std::cout << concrete_ring_buffer.stage() << '\n';
                }, ring_buffer);
                return true;
            }

            require_arg_count(args, 2);
            std::visit([&](auto& concrete_ring_buffer) {
                concrete_ring_buffer.stage() = parse_i64(args[1], "value");
                print_ring_buffer_format(concrete_ring_buffer);
            }, ring_buffer);
            return true;
        }

        if (command == "commit") {
            require_arg_count(args, 1);
            auto& ring_buffer = require_ring_buffer();
            std::visit([](auto& concrete_ring_buffer) {
                concrete_ring_buffer.commit();
                print_ring_buffer_format(concrete_ring_buffer);
            }, ring_buffer);
            return true;
        }

        if (command == "publish") {
            require_arg_count(args, 2);
            auto& ring_buffer = require_ring_buffer();
            std::visit([&](auto& concrete_ring_buffer) {
                concrete_ring_buffer.stage() = parse_i64(args[1], "value");
                concrete_ring_buffer.commit();
                print_ring_buffer_format(concrete_ring_buffer);
            }, ring_buffer);
            return true;
        }

        if (command == "history") {
            auto& ring_buffer = require_ring_buffer();
            if (args.size() == 2) {
                const auto offset = parse_size(args[1], "offset");
                std::visit([&](auto& concrete_ring_buffer) {
                    std::cout << concrete_ring_buffer.history(offset) << '\n';
                }, ring_buffer);
                return true;
            }

            require_arg_count(args, 3);
            const auto offset = parse_size(args[1], "offset");
            std::visit([&](auto& concrete_ring_buffer) {
                concrete_ring_buffer.history(offset) = parse_i64(args[2], "value");
                print_ring_buffer_format(concrete_ring_buffer);
            }, ring_buffer);
            return true;
        }

        if (command == "history_size") {
            require_arg_count(args, 1);
            const auto& ring_buffer = require_const_ring_buffer();
            std::visit([](const auto& concrete_ring_buffer) {
                std::cout << concrete_ring_buffer.history_size() << '\n';
            }, ring_buffer);
            return true;
        }

        if (command == "history_capacity") {
            require_arg_count(args, 1);
            const auto& ring_buffer = require_const_ring_buffer();
            std::visit([](const auto& concrete_ring_buffer) {
                std::cout << concrete_ring_buffer.history_capacity() << '\n';
            }, ring_buffer);
            return true;
        }

        if (command == "format") {
            require_arg_count(args, 1);
            print_ring_buffer_variant_format(require_const_ring_buffer());
            return true;
        }

        throw std::invalid_argument(std::format("unknown command: {}", command));
    }

private:
    static void require_arg_count(const std::vector<std::string>& args, std::size_t expected) {
        if (args.size() != expected) {
            throw std::invalid_argument(std::format("{} expects {} argument(s), got {}", args[0], expected - 1, args.size() - 1));
        }
    }

    const ring_buffer_variant_t<std::int64_t>& require_const_ring_buffer() const {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <history_capacity>` first");
        }

        return *m_ring_buffer;
    }

    ring_buffer_variant_t<std::int64_t>& require_ring_buffer() {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <history_capacity>` first");
        }

        return *m_ring_buffer;
    }

    buffer_config_t reset_default_config() const {
        if (m_prompt_for_reset_config) {
            return prompt_buffer_config(default_buffer_config);
        }
        return default_buffer_config;
    }

    void reset(std::size_t history_capacity, buffer_config_t config) {
        m_ring_buffer = make_ring_buffer<std::int64_t>(config, history_capacity);
        print_ring_buffer_variant_format(*m_ring_buffer);
    }

private:
    std::optional<ring_buffer_variant_t<std::int64_t>> m_ring_buffer;
    bool m_prompt_for_reset_config = false;
};

int run_repl(repl_t repl) {
    repl.prompt_for_reset_config(true);
    print_command_help();

    for (std::string line;;) {
        std::cout << "ring-buffer> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            return 0;
        }

        try {
            if (!repl.execute(split_command_line(line))) {
                return 0;
            }
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << '\n';
        }
    }
}

int run_one_command(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    repl_t repl;
    if (!repl.execute(args)) {
        return 0;
    }

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 1) {
            return run_repl(repl_t(default_history_capacity));
        }

        const std::string_view command = argv[1];
        if (command == "help" || command == "--help" || command == "-h") {
            print_usage(argv[0]);
            return 0;
        }

        if (command == "repl") {
            if (argc == 2) {
                return run_repl(repl_t(default_history_capacity));
            }

            if (argc == 3) {
                return run_repl(repl_t(parse_size(argv[2], "history_capacity")));
            }

            throw std::invalid_argument("repl accepts at most one argument: history_capacity");
        }

        return run_one_command(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << argv[0] << ": " << e.what() << '\n';
        return 1;
    }
}
