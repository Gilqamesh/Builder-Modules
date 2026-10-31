#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct number_row_t { std::int64_t number; };
struct labeled_row_t { std::int64_t number; std::optional<std::string> label; };
struct callback_error_t : std::exception {};
struct decoded_number_t { std::int64_t number; };
std::function<void()> decode_hook;

template <>
struct column_codec<decoded_number_t> {
    using storage_t = std::int64_t;
    static storage_t encode(const decoded_number_t& number) { return number.number; }
    static decoded_number_t decode(storage_t number) {
        if (decode_hook) decode_hook();
        return decoded_number_t{number};
    }
};
struct decoded_row_t { decoded_number_t number; };

template <typename D>
concept has_untyped_drop = requires(D& database) { database.drop_table("numbers"); };

template <typename E, typename F>
void expect_failure(F&& action, std::string_view fragment = {}) {
    bool caught = false;
    try {
        action();
    } catch (const E& error) {
        caught = true;
        if (!fragment.empty()) assert(std::string_view(error.what()).contains(fragment));
    }
    assert(caught);
}

void atomic_groups() {
    static_assert(std::same_as<decltype(&database_t::transaction), void (database_t::*)(std::function_ref<void(database_t&)>)>);
    static_assert(!has_untyped_drop<database_t>);
    static_assert(std::same_as<decltype(&table_t<number_row_t>::drop), void (table_t<number_row_t>::*)() const>);
    database_t database;
    const auto numbers = database.table<number_row_t>("numbers");
    const auto copied = numbers;
    int invocations = 0;
    auto action = [&, token = std::make_unique<int>(7)](database_t& same_database) {
        assert(&same_database == &database && *token == 7);
        ++invocations;
        copied.insert(number_row_t{1});
        same_database.table<number_row_t>("other").insert(number_row_t{2});
        assert(numbers.rows().size() == 1);
        int nested_calls = 0;
        expect_failure<std::logic_error>([&] {
            same_database.transaction([&](database_t&) { ++nested_calls; });
        });
        assert(nested_calls == 0);
    };
    database.transaction(action);
    assert(invocations == 1 && numbers.rows().size() == 1);
    assert(database.table<number_row_t>("other").rows().size() == 1);

    std::optional<table_t<number_row_t>> uncommitted;
    expect_failure<callback_error_t>([&] {
        database.transaction([&](database_t& same_database) {
            numbers.insert(number_row_t{3});
            uncommitted.emplace(same_database.table<number_row_t>("uncommitted"));
            uncommitted->insert(number_row_t{4});
            same_database.table<number_row_t>("other").drop();
            throw callback_error_t{};
        });
    });
    assert(numbers.rows().size() == 1);
    assert(database.table<number_row_t>("other").rows().size() == 1);
    expect_failure<std::runtime_error>([&] { (void)uncommitted->rows(); }, "does not exist");

    database.transaction([&](database_t& same_database) {
        numbers.insert(number_row_t{5});
        expect_failure<std::runtime_error>([&] { (void)same_database.table<labeled_row_t>("numbers"); });
        decode_hook = [] { throw std::runtime_error("decoder sentinel"); };
        expect_failure<std::runtime_error>([&] { (void)same_database.table<decoded_row_t>("numbers").rows(); }, "decoder sentinel");
        decode_hook = {};
        assert(numbers.rows().size() == 2);
        numbers.insert(number_row_t{6});
    });
    assert(numbers.rows().size() == 3);
}

void removal() {
    database_t database;
    const auto numbers = database.table<number_row_t>("numbers");
    numbers.insert(number_row_t{1});
    numbers.drop();
    numbers.drop();
    expect_failure<std::runtime_error>([&] { (void)numbers.rows(); });
    expect_failure<std::runtime_error>([&] { numbers.insert(number_row_t{2}); });
    const auto recreated = database.table<number_row_t>("numbers");
    numbers.insert(number_row_t{3});
    assert(recreated.rows().front().number == 3);
    expect_failure<callback_error_t>([&] {
        database.transaction([&](database_t&) {
            numbers.drop();
            throw callback_error_t{};
        });
    });
    assert(numbers.rows().front().number == 3);
    database.transaction([&](database_t& same_database) {
        numbers.drop();
        const auto replacement = same_database.table<labeled_row_t>("numbers");
        replacement.insert(labeled_row_t{4, "preserved"});
        expect_failure<std::runtime_error>([&] { numbers.drop(); }, "column");
        assert(replacement.rows().front().label == "preserved");
    });
    expect_failure<std::runtime_error>([&] { numbers.drop(); });
    const auto replacement = database.table<labeled_row_t>("numbers");
    assert(replacement.rows().front().label == "preserved");
    replacement.drop();
    numbers.drop();
}

void transaction_snapshots(const std::filesystem::path& directory) {
    const auto filename = directory / "transactions.db";
    database_t reader(filename);
    database_t writer(filename);
    const auto read_numbers = reader.table<number_row_t>("numbers");
    const auto read_others = reader.table<number_row_t>("others");
    const auto write_numbers = writer.table<number_row_t>("numbers");
    const auto write_others = writer.table<number_row_t>("others");
    write_numbers.insert(number_row_t{1});
    write_others.insert(number_row_t{2});
    reader.transaction([&](database_t&) {
        // The snapshot exists before the callback's first read, and spans tables.
        writer.transaction([&](database_t&) {
            write_numbers.insert(number_row_t{3});
            write_others.insert(number_row_t{4});
        });
        assert(read_numbers.rows().size() == 1);
        assert(read_others.rows().size() == 1);
        expect_failure<std::runtime_error>([&] { read_numbers.insert(number_row_t{5}); });
        assert(read_numbers.rows().size() == 1);
    });
    assert(read_numbers.rows().size() == 2 && read_others.rows().size() == 2);

    writer.transaction([&](database_t& same_database) {
        write_numbers.insert(number_row_t{6});
        write_others.insert(number_row_t{7});
        assert(write_numbers.rows().size() == 3 && write_others.rows().size() == 3);
        assert(read_numbers.rows().size() == 2 && read_others.rows().size() == 2);
        // Acquiring another table must not commit the enclosing transaction.
        same_database.table<number_row_t>("created in transaction").insert(number_row_t{8});
        assert(read_numbers.rows().size() == 2);
        expect_failure<std::runtime_error>([&] { read_numbers.insert(number_row_t{9}); });
        expect_failure<std::runtime_error>([&] { read_numbers.drop(); });
        expect_failure<std::runtime_error>([&] { (void)reader.table<number_row_t>("blocked acquisition"); });
    });
    assert(read_numbers.rows().size() == 3 && read_others.rows().size() == 3);
    read_numbers.insert(number_row_t{10});
    assert(write_numbers.rows().size() == 4);
}

void handle_snapshots(const std::filesystem::path& directory) {
    const auto filename = directory / "handles.db";
    database_t reader(filename);
    database_t writer(filename);
    const auto original = reader.table<number_row_t>("numbers");
    const auto writes = writer.table<number_row_t>("numbers");
    original.insert(number_row_t{1});
    original.insert(number_row_t{2});
    const auto decoded = reader.table<decoded_row_t>("numbers");
    bool changed = false;
    decode_hook = [&] {
        if (changed) return;
        changed = true;
        writer.transaction([&](database_t& same_database) {
            writes.drop();
            same_database.table<labeled_row_t>("numbers").insert(labeled_row_t{3, "replacement"});
        });
    };
    const auto snapshot = decoded.rows();
    decode_hook = {};
    assert(changed && snapshot.size() == 2);
    assert(std::ranges::any_of(snapshot, [](const decoded_row_t& row) { return row.number.number == 1; }));
    assert(std::ranges::any_of(snapshot, [](const decoded_row_t& row) { return row.number.number == 2; }));
    expect_failure<std::runtime_error>([&] { original.drop(); });
    expect_failure<std::runtime_error>([&] { original.insert(number_row_t{4}); });
    expect_failure<std::runtime_error>([&] { (void)original.rows(); });
    const auto replacement = reader.table<labeled_row_t>("numbers");
    assert(replacement.rows().front().label == "replacement");

    reader.transaction([&](database_t&) {
        // A replacement committed after snapshot acquisition must also survive drop().
        writer.transaction([&](database_t& same_database) {
            same_database.table<labeled_row_t>("numbers").drop();
            same_database.table<number_row_t>("numbers").insert(number_row_t{9});
        });
        expect_failure<std::runtime_error>([&] { replacement.drop(); });
        assert(replacement.rows().front().label == "replacement");
    });
    assert(original.rows().front().number == 9);
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

int main() {
    using namespace m03h5hm86mnvc18opf1rdttsgh_database;
    atomic_groups();
    removal();
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-transactions-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::create_directories(directory);
    transaction_snapshots(directory);
    handle_snapshots(directory);
    std::filesystem::remove_all(directory);
}
