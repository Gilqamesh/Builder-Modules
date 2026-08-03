#include "api.h"

#include <atomic>
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
#include <thread>
#include <vector>

namespace {

using m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer::spsc_ring_buffer_t;

constexpr std::size_t default_demo_capacity_power_of_two = 2;
constexpr std::size_t default_repl_capacity_power_of_two = 2;
constexpr std::size_t default_test_capacity_power_of_two = 10;
constexpr std::size_t default_test_count = 1'000'000;

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
        << "  new <capacity_power_of_two>\n"
        << "  reset <capacity_power_of_two>\n"
        << "      Construct a new spsc_ring_buffer_t<int64_t> with capacity 1 << exponent.\n"
        << "  try-write <value>\n"
        << "  write <value>\n"
        << "      Call try_write(const int64_t& value).\n"
        << "  try-write-move <value>\n"
        << "  write-move <value>\n"
        << "      Call try_write(int64_t&& value).\n"
        << "  try-read\n"
        << "  read\n"
        << "      Call try_read(value).\n"
        << "  buffer\n"
        << "      Print buffer() with indexes.\n"
        << "  buffer <index>\n"
        << "      Print buffer()[index].\n"
        << "  head\n"
        << "      Print head().\n"
        << "  tail\n"
        << "      Print tail().\n"
        << "  capacity\n"
        << "      Print buffer().size().\n"
        << "  show\n"
        << "  format\n"
        << "      Print std::format(\"{}\", ring_buffer).\n"
        << "  demo [capacity_power_of_two] [values...]\n"
        << "      Reset, write values until full, then drain. Defaults: capacity_power_of_two="
        << default_demo_capacity_power_of_two << " (capacity=4), values=1 2 3 4 5 6.\n"
        << "  test [capacity_power_of_two] [count]\n"
        << "      Run the two-thread producer/consumer correctness test. Defaults: capacity_power_of_two="
        << default_test_capacity_power_of_two << " (capacity=1024), count=" << default_test_count << ".\n"
        << "  quit\n"
        << "  exit\n"
        << "      Leave the REPL.\n";
}

void print_usage(std::string_view program) {
    std::cout
        << "usage: " << program << " [command [args...]]\n"
        << "\n"
        << "With no arguments, starts an interactive REPL. With arguments, runs one command.\n"
        << "Use `repl [capacity_power_of_two]` to start a REPL with an optional initial buffer.\n"
        << "\n";
    print_command_help();
}

void print_buffer_values(const spsc_ring_buffer_t<std::int64_t>& ring_buffer) {
    const auto& values = ring_buffer.buffer();
    std::cout << "buffer:";
    if (values.empty()) {
        std::cout << " empty\n";
        return;
    }

    for (std::size_t i = 0; i < values.size(); ++i) {
        std::cout << ' ' << i << '=' << values[i];
    }

    std::cout << '\n';
}

struct thread_test_result_t {
    bool passed = true;
    std::string error;
    std::size_t produced = 0;
    std::size_t consumed = 0;
    std::size_t capacity = 0;
    std::size_t producer_yields = 0;
    std::size_t consumer_yields = 0;
};

thread_test_result_t run_thread_test(std::size_t capacity_power_of_two, std::size_t count) {
    spsc_ring_buffer_t<std::size_t> ring_buffer(capacity_power_of_two);
    thread_test_result_t result;
    result.capacity = ring_buffer.buffer().size();
    std::atomic<bool> failed(false);
    std::atomic<bool> producer_done(false);

    std::thread producer([&] {
        for (std::size_t value = 0; value < count; ++value) {
            while (!failed.load(std::memory_order_acquire) && !ring_buffer.try_write(value)) {
                ++result.producer_yields;
                std::this_thread::yield();
            }

            if (failed.load(std::memory_order_acquire)) {
                break;
            }

            result.produced = value + 1;
        }

        producer_done.store(true, std::memory_order_release);
    });

    std::thread consumer([&] {
        for (std::size_t expected = 0; expected < count;) {
            std::size_t value = 0;
            if (!ring_buffer.try_read(value)) {
                ++result.consumer_yields;
                if (producer_done.load(std::memory_order_acquire)) {
                    result.passed = false;
                    result.error = std::format("buffer became empty after {} reads; expected {}", expected, count);
                    failed.store(true, std::memory_order_release);
                    return;
                }

                std::this_thread::yield();
                continue;
            }

            if (value != expected) {
                result.passed = false;
                result.error = std::format("expected {}, read {}", expected, value);
                failed.store(true, std::memory_order_release);
                return;
            }

            ++expected;
            result.consumed = expected;
        }
    });

    producer.join();
    consumer.join();

    if (result.passed && result.produced != count) {
        result.passed = false;
        result.error = std::format("producer wrote {} values; expected {}", result.produced, count);
    }

    if (result.passed && result.consumed != count) {
        result.passed = false;
        result.error = std::format("consumer read {} values; expected {}", result.consumed, count);
    }

    if (result.passed && (ring_buffer.head() != count || ring_buffer.tail() != count)) {
        result.passed = false;
        result.error = std::format("final positions were head={}, tail={}; expected {}", ring_buffer.head(), ring_buffer.tail(), count);
    }

    return result;
}

int print_thread_test_result(std::size_t capacity_power_of_two, std::size_t count) {
    const auto result = run_thread_test(capacity_power_of_two, count);
    if (!result.passed) {
        std::cerr << "FAIL: " << result.error << '\n';
        std::cerr << "produced=" << result.produced << ", consumed=" << result.consumed << '\n';
        return 1;
    }

    std::cout
        << "PASS: transferred " << result.consumed << " values through capacity "
        << result.capacity << " with one producer thread and one consumer thread\n"
        << "producer_yields=" << result.producer_yields
        << ", consumer_yields=" << result.consumer_yields << '\n';
    return 0;
}

class repl_t {
public:
    repl_t() = default;

    explicit repl_t(std::size_t capacity_power_of_two) {
        reset(capacity_power_of_two);
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
            reset(parse_size(args[1], "capacity_power_of_two"));
            return true;
        }

        if (command == "try-write" || command == "write") {
            require_arg_count(args, 2);
            auto& ring_buffer = require_ring_buffer();
            const auto value = parse_i64(args[1], "value");
            std::cout << (ring_buffer.try_write(value) ? "true" : "false") << '\n';
            return true;
        }

        if (command == "try-write-move" || command == "write-move") {
            require_arg_count(args, 2);
            auto& ring_buffer = require_ring_buffer();
            auto value = parse_i64(args[1], "value");
            std::cout << (ring_buffer.try_write(std::move(value)) ? "true" : "false") << '\n';
            return true;
        }

        if (command == "try-read" || command == "read") {
            require_arg_count(args, 1);
            auto& ring_buffer = require_ring_buffer();
            std::int64_t value = 0;
            const auto read = ring_buffer.try_read(value);
            std::cout << (read ? "true" : "false");
            if (read) {
                std::cout << ' ' << value;
            }
            std::cout << '\n';
            return true;
        }

        if (command == "buffer") {
            const auto& ring_buffer = require_const_ring_buffer();
            if (args.size() == 1) {
                print_buffer_values(ring_buffer);
                return true;
            }

            require_arg_count(args, 2);
            const auto index = parse_size(args[1], "index");
            std::cout << ring_buffer.buffer().at(index) << '\n';
            return true;
        }

        if (command == "head") {
            require_arg_count(args, 1);
            std::cout << require_const_ring_buffer().head() << '\n';
            return true;
        }

        if (command == "tail") {
            require_arg_count(args, 1);
            std::cout << require_const_ring_buffer().tail() << '\n';
            return true;
        }

        if (command == "capacity") {
            require_arg_count(args, 1);
            std::cout << require_const_ring_buffer().buffer().size() << '\n';
            return true;
        }

        if (command == "show" || command == "format") {
            require_arg_count(args, 1);
            const auto& ring_buffer = require_const_ring_buffer();
            std::cout << std::format("{}", ring_buffer) << '\n';
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

    spsc_ring_buffer_t<std::int64_t>& require_ring_buffer() {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <capacity_power_of_two>` first");
        }

        return *m_ring_buffer;
    }

    const spsc_ring_buffer_t<std::int64_t>& require_const_ring_buffer() const {
        if (!m_ring_buffer.has_value()) {
            throw std::logic_error("no ring buffer: run `new <capacity_power_of_two>` first");
        }

        return *m_ring_buffer;
    }

    void reset(std::size_t capacity_power_of_two) {
        m_ring_buffer.emplace(capacity_power_of_two);
        std::cout
            << "created spsc_ring_buffer_t<int64_t> with capacity_power_of_two "
            << capacity_power_of_two << " and capacity " << m_ring_buffer->buffer().size() << '\n';
    }

    void run_demo(const std::vector<std::string>& args) {
        auto capacity_power_of_two = default_demo_capacity_power_of_two;
        auto first_value_arg = std::size_t{1};
        if (args.size() > 1) {
            capacity_power_of_two = parse_size(args[1], "capacity_power_of_two");
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

        reset(capacity_power_of_two);
        auto& ring_buffer = require_ring_buffer();

        for (const auto value : values) {
            const auto wrote = ring_buffer.try_write(value);
            std::cout << "write " << value << " -> " << (wrote ? "ok" : "full") << '\n';
        }

        std::cout << "after writes " << std::format("{}", ring_buffer) << '\n';

        std::int64_t value = 0;
        while (ring_buffer.try_read(value)) {
            std::cout << "read -> " << value << '\n';
        }

        std::cout << "read -> empty\n";
        std::cout << "final " << std::format("{}", ring_buffer) << '\n';
    }

    void run_test(const std::vector<std::string>& args) const {
        auto capacity_power_of_two = default_test_capacity_power_of_two;
        auto count = default_test_count;

        if (args.size() > 1) {
            capacity_power_of_two = parse_size(args[1], "capacity_power_of_two");
        }
        if (args.size() > 2) {
            count = parse_size(args[2], "count");
        }
        if (args.size() > 3) {
            throw std::invalid_argument("test accepts at most two arguments: capacity_power_of_two and count");
        }

        const auto exit_code = print_thread_test_result(capacity_power_of_two, count);
        if (exit_code != 0) {
            throw std::runtime_error("test failed");
        }
    }

private:
    std::optional<spsc_ring_buffer_t<std::int64_t>> m_ring_buffer;
};

int run_repl(repl_t repl) {
    print_command_help();

    for (std::string line;;) {
        std::cout << "spsc-ring-buffer> ";
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
            return run_repl(repl_t(default_repl_capacity_power_of_two));
        }

        const std::string_view command = argv[1];
        if (command == "help" || command == "--help" || command == "-h") {
            print_usage(argv[0]);
            return 0;
        }

        if (command == "repl") {
            if (argc == 2) {
                return run_repl(repl_t(default_repl_capacity_power_of_two));
            }

            if (argc == 3) {
                return run_repl(repl_t(parse_size(argv[2], "capacity_power_of_two")));
            }

            throw std::invalid_argument("repl accepts at most one argument: capacity_power_of_two");
        }

        return run_one_command(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << argv[0] << ": " << e.what() << '\n';
        return 1;
    }
}
