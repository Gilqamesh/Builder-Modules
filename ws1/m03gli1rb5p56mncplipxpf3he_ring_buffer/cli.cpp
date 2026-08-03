#include "api.h"

#include <algorithm>
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
#include <vector>

namespace {

using m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t;

constexpr std::size_t default_capacity = 4;
constexpr std::size_t default_test_count = 16;

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

void print_command_help() {
    std::cout
        << "commands:\n"
        << "  help\n"
        << "      Show this help text.\n"
        << "  new <capacity>\n"
        << "  reset <capacity>\n"
        << "      Construct a new ring_buffer_t<int64_t>.\n"
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
        << "  const-history <offset>\n"
        << "      Print history(offset) through a const ring buffer reference.\n"
        << "  size\n"
        << "      Print size().\n"
        << "  capacity\n"
        << "      Print capacity().\n"
        << "  show\n"
        << "  format\n"
        << "      Print std::format(\"{}\", ring_buffer).\n"
        << "  demo [capacity] [values...]\n"
        << "      Reset the buffer and publish values. Defaults: capacity=" << default_capacity << ", values=1 2 3 4 5 6.\n"
        << "  test [capacity] [count]\n"
        << "      Verify wraparound history order. Defaults: capacity=" << default_capacity << ", count=" << default_test_count << ".\n"
        << "  quit\n"
        << "  exit\n"
        << "      Leave the REPL.\n";
}

void print_usage(std::string_view program) {
    std::cout
        << "usage: " << program << " [command [args...]]\n"
        << "\n"
        << "With no arguments, starts an interactive REPL. With arguments, runs one command.\n"
        << "Use `repl [capacity]` to start a REPL with an optional initial buffer.\n"
        << "\n";
    print_command_help();
}

void print_history(const ring_buffer_t<std::int64_t>& ring_buffer) {
    std::cout << "history newest-to-oldest:";
    if (ring_buffer.size() == 0) {
        std::cout << " empty\n";
        return;
    }

    for (std::size_t offset = 0; offset < ring_buffer.size(); ++offset) {
        std::cout << ' ' << offset << '=' << ring_buffer.history(offset);
    }

    std::cout << '\n';
}

int run_history_order_test(std::size_t capacity, std::size_t count) {
    ring_buffer_t<std::size_t> ring_buffer(capacity);
    for (std::size_t value = 0; value < count; ++value) {
        ring_buffer.stage() = value;
        ring_buffer.commit();
    }

    const auto expected_size = std::min(capacity, count);
    if (ring_buffer.capacity() != capacity) {
        std::cerr << "FAIL: capacity is " << ring_buffer.capacity() << ", expected " << capacity << '\n';
        return 1;
    }
    if (ring_buffer.size() != expected_size) {
        std::cerr << "FAIL: size is " << ring_buffer.size() << ", expected " << expected_size << '\n';
        return 1;
    }

    for (std::size_t offset = 0; offset < expected_size; ++offset) {
        const auto expected_value = count - 1 - offset;
        const auto actual_value = ring_buffer.history(offset);
        if (actual_value != expected_value) {
            std::cerr
                << "FAIL: history(" << offset << ") is " << actual_value
                << ", expected " << expected_value << '\n';
            return 1;
        }
    }

    try {
        (void)ring_buffer.history(expected_size);
        std::cerr << "FAIL: history(" << expected_size << ") did not throw\n";
        return 1;
    } catch (const std::out_of_range&) {
    }

    std::cout
        << "PASS: published " << count << " values into capacity " << capacity
        << "; retained " << expected_size << " newest values in order\n";
    return 0;
}

class repl_t {
public:
    repl_t() = default;

    explicit repl_t(std::size_t capacity) {
        reset(capacity);
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
            require_arg_count(args, 2);
            reset(parse_size(args[1], "capacity"));
            return true;
        }

        if (command == "stage") {
            auto& ring_buffer = require_ring_buffer();
            if (args.size() == 1) {
                std::cout << ring_buffer.stage() << '\n';
                return true;
            }

            require_arg_count(args, 2);
            ring_buffer.stage() = parse_i64(args[1], "value");
            std::cout << "stage = " << ring_buffer.stage() << '\n';
            return true;
        }

        if (command == "commit") {
            require_arg_count(args, 1);
            auto& ring_buffer = require_ring_buffer();
            ring_buffer.commit();
            std::cout << "committed: size=" << ring_buffer.size() << ", capacity=" << ring_buffer.capacity() << '\n';
            return true;
        }

        if (command == "publish") {
            require_arg_count(args, 2);
            auto& ring_buffer = require_ring_buffer();
            ring_buffer.stage() = parse_i64(args[1], "value");
            ring_buffer.commit();
            std::cout << "published " << ring_buffer.history(0) << '\n';
            return true;
        }

        if (command == "history") {
            auto& ring_buffer = require_ring_buffer();
            if (args.size() == 2) {
                const auto offset = parse_size(args[1], "offset");
                std::cout << ring_buffer.history(offset) << '\n';
                return true;
            }

            require_arg_count(args, 3);
            const auto offset = parse_size(args[1], "offset");
            ring_buffer.history(offset) = parse_i64(args[2], "value");
            std::cout << "history(" << offset << ") = " << ring_buffer.history(offset) << '\n';
            return true;
        }

        if (command == "const-history") {
            require_arg_count(args, 2);
            const auto& ring_buffer = require_const_ring_buffer();
            const auto offset = parse_size(args[1], "offset");
            std::cout << ring_buffer.history(offset) << '\n';
            return true;
        }

        if (command == "size") {
            require_arg_count(args, 1);
            std::cout << require_const_ring_buffer().size() << '\n';
            return true;
        }

        if (command == "capacity") {
            require_arg_count(args, 1);
            std::cout << require_const_ring_buffer().capacity() << '\n';
            return true;
        }

        if (command == "show" || command == "format") {
            require_arg_count(args, 1);
            const auto& ring_buffer = require_const_ring_buffer();
            std::cout << std::format("{}", ring_buffer) << '\n';
            print_history(ring_buffer);
            return true;
        }

        if (command == "demo") {
            run_demo(args);
            return true;
        }

        if (command == "test") {
            run_test(args);
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

    ring_buffer_t<std::int64_t>& require_ring_buffer() {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <capacity>` first");
        }

        return *m_ring_buffer;
    }

    const ring_buffer_t<std::int64_t>& require_const_ring_buffer() const {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <capacity>` first");
        }

        return *m_ring_buffer;
    }

    void reset(std::size_t capacity) {
        m_ring_buffer.emplace(capacity);
        std::cout << "created ring_buffer_t<int64_t> with capacity " << m_ring_buffer->capacity() << '\n';
    }

    void run_demo(const std::vector<std::string>& args) {
        auto capacity = default_capacity;
        auto first_value_arg = std::size_t{1};
        if (args.size() > 1) {
            capacity = parse_size(args[1], "capacity");
            first_value_arg = 2;
        }

        std::vector<std::int64_t> values;
        if (first_value_arg < args.size()) {
            for (auto i = first_value_arg; i < args.size(); ++i) {
                values.push_back(parse_i64(args[i], "value"));
            }
        } else {
            values = {1, 2, 3, 4, 5, 6};
        }

        reset(capacity);
        auto& ring_buffer = require_ring_buffer();
        for (const auto value : values) {
            ring_buffer.stage() = value;
            ring_buffer.commit();
            std::cout << "published " << value << " -> size " << ring_buffer.size() << '/' << ring_buffer.capacity() << '\n';
            print_history(ring_buffer);
        }
    }

    void run_test(const std::vector<std::string>& args) {
        auto capacity = default_capacity;
        auto count = default_test_count;

        if (args.size() > 1) {
            capacity = parse_size(args[1], "capacity");
        }
        if (args.size() > 2) {
            count = parse_size(args[2], "count");
        }
        if (args.size() > 3) {
            throw std::invalid_argument("test accepts at most two arguments: capacity and count");
        }

        const auto exit_code = run_history_order_test(capacity, count);
        if (exit_code != 0) {
            throw std::runtime_error("test failed");
        }
    }

private:
    std::optional<ring_buffer_t<std::int64_t>> m_ring_buffer;
};

int run_repl(repl_t repl) {
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
            return run_repl(repl_t(default_capacity));
        }

        const std::string_view command = argv[1];
        if (command == "help" || command == "--help" || command == "-h") {
            print_usage(argv[0]);
            return 0;
        }

        if (command == "repl") {
            if (argc == 2) {
                return run_repl(repl_t(default_capacity));
            }

            if (argc == 3) {
                return run_repl(repl_t(parse_size(argv[2], "capacity")));
            }

            throw std::invalid_argument("repl accepts at most one argument: capacity");
        }

        return run_one_command(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << argv[0] << ": " << e.what() << '\n';
        return 1;
    }
}
