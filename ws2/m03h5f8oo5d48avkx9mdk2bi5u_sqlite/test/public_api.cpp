#include <m03h5f8oo5d48avkx9mdk2bi5u_sqlite/sqlite.h>
#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace api = m03h5f8oo5d48avkx9mdk2bi5u_sqlite;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

static_assert(!std::is_copy_constructible_v<api::database_t>);
static_assert(!std::is_copy_assignable_v<api::database_t>);
static_assert(!std::is_move_constructible_v<api::database_t>);
static_assert(!std::is_move_assignable_v<api::database_t>);
static_assert(std::is_nothrow_destructible_v<api::database_t>);
static_assert(!std::is_copy_constructible_v<api::statement_t>);
static_assert(!std::is_copy_assignable_v<api::statement_t>);
static_assert(!std::is_move_constructible_v<api::statement_t>);
static_assert(!std::is_move_assignable_v<api::statement_t>);
static_assert(std::is_nothrow_destructible_v<api::statement_t>);
static_assert(std::is_abstract_v<api::statement_t>);
static_assert(std::is_same_v<decltype(std::declval<api::database_t&>().prepare(std::string_view())), std::unique_ptr<api::statement_t>>);

int main() {
    return test::run([] {
        api::database_t database(":memory:");
        {
            auto version = database.prepare("SELECT sqlite_version()");
            test::expect(std::equal_to<>(), version->step(), true);
            test::expect(std::equal_to<>(), version->column_text(0), std::string("3.54.1"));
            test::expect(std::equal_to<>(), version->step(), false);
        }

        {
            auto statement = database.prepare("SELECT 42");
            test::expect(std::equal_to<>(), statement != nullptr, true);
            auto moved_statement = std::move(statement);
            test::expect(std::equal_to<>(), statement == nullptr, true);
            test::expect(std::equal_to<>(), moved_statement->step(), true);
            test::expect(std::equal_to<>(), moved_statement->column_integer(0), std::int64_t(42));
        }

        database.execute("CREATE TABLE bytes (content BLOB NOT NULL)");
        {
            const std::array<std::byte, 4> bytes { std::byte(0), std::byte(0xff), std::byte(0x7f), std::byte(0x80) };
            auto insert = database.prepare("INSERT INTO bytes VALUES (?1)");
            insert->bind(1, std::span<const std::byte>(bytes));
            test::expect(std::equal_to<>(), insert->step(), false);
            insert->reset();
            insert->bind(1, std::span<const std::byte>());
            test::expect(std::equal_to<>(), insert->step(), false);

            auto select = database.prepare("SELECT content, typeof(content) FROM bytes ORDER BY rowid");
            test::expect(std::equal_to<>(), select->step(), true);
            const auto stored_bytes = select->column_blob(0);
            test::expect(std::equal_to<>(), std::equal(stored_bytes.begin(), stored_bytes.end(), bytes.begin(), bytes.end()), true);
            test::expect(std::equal_to<>(), select->column_text(1), std::string("blob"));
            test::expect(std::equal_to<>(), select->step(), true);
            test::expect(std::equal_to<>(), select->column_blob(0).empty(), true);
            test::expect(std::equal_to<>(), select->column_null(0), false);
            test::expect(std::equal_to<>(), select->column_text(1), std::string("blob"));
            test::expect(std::equal_to<>(), select->step(), false);
        }

        test::expect(std::equal_to<>(), std::format("{}", database), std::string("database"));
        database.execute("CREATE TABLE numbers (number INTEGER UNIQUE); INSERT INTO numbers VALUES (7)");
        test::expect_throws<std::runtime_error>([&] { database.execute("invalid SQL"); });
        test::expect_throws<std::invalid_argument>([&] { database.execute(std::string_view("SELECT 1\0; SELECT 2", 19)); });
        test::expect_throws<std::invalid_argument>([] { api::database_t invalid_database(std::string_view(":memory:\0ignored", 16)); });
        test::expect_throws<std::runtime_error>([] { api::database_t invalid_database("/dev/null/unopenable.sqlite3"); });
        test::expect_throws<std::invalid_argument>([&] { auto empty_statement = database.prepare(""); });
        test::expect_throws<std::invalid_argument>([&] { auto comment_statement = database.prepare("-- only a comment"); });
        test::expect_throws<std::invalid_argument>([&] { auto invalid_statement = database.prepare(std::string_view("SELECT 1\0; SELECT 2", 19)); });
        test::expect_throws<std::runtime_error>([&] { auto invalid_statement = database.prepare("SELECT FROM"); });

        {
            auto insert_number = database.prepare("INSERT INTO numbers VALUES (?1)");
            test::expect(std::equal_to<>(), std::format("{}", *insert_number), std::string("statement"));
            test::expect(std::equal_to<>(), insert_number->column_count(), std::size_t(0));
            insert_number->bind(1, 42);
            test::expect(std::equal_to<>(), insert_number->step(), false);
            insert_number->reset();
            insert_number->bind(1, std::uint64_t(43));
            test::expect(std::equal_to<>(), insert_number->step(), false);
            insert_number->reset();
            test::expect_throws<std::out_of_range>([&] { insert_number->bind(1, std::numeric_limits<std::uint64_t>::max()); });
            test::expect_throws<std::runtime_error>([&] { insert_number->bind(0, 5); });
            test::expect_throws<std::runtime_error>([&] { insert_number->bind(2, 5); });
            insert_number->bind(1, 7);
            try {
                insert_number->step();
                test::fail();
            } catch (const std::runtime_error& error) {
                const std::string_view message(error.what());
                test::expect(std::equal_to<>(), message.contains("step statement"), true);
                test::expect(std::equal_to<>(), message.contains("code 19"), true);
                test::expect(std::equal_to<>(), message.contains("UNIQUE constraint failed"), true);
            }
            test::expect_throws<std::runtime_error>([&] { insert_number->reset(); });
            insert_number->bind(1, 44);
            test::expect(std::equal_to<>(), insert_number->step(), false);
        }

        test::expect_throws<std::logic_error>([&] {
            auto unwound_statement = database.prepare("SELECT number FROM numbers");
            test::expect(std::equal_to<>(), unwound_statement->step(), true);
            throw std::logic_error("unwind statement");
        });

        std::string stored_text;
        std::vector<std::byte> stored_bytes;
        const std::array<std::byte, 4> expected_bytes { std::byte(0), std::byte(0xff), std::byte(0x7f), std::byte(0x80) };
        {
            auto values = database.prepare("SELECT ?1, ?2, ?3, ?4, ?5");
            test::expect(std::equal_to<>(), values->column_count(), std::size_t(5));
            test::expect_throws<std::logic_error>([&] { values->column_text(0); });
            test::expect_throws<std::out_of_range>([&] { values->column_null(-1); });
            test::expect_throws<std::out_of_range>([&] { values->column_integer(5); });
            values->bind(1, nullptr);
            values->bind(2, std::numeric_limits<std::int64_t>::min());
            values->bind(3, 1.25);
            {
                std::string text("a\0b", 3);
                auto bound_bytes = expected_bytes;
                values->bind(4, std::string_view(text));
                values->bind(5, std::span<const std::byte>(bound_bytes));
                text.assign("changed");
                bound_bytes.fill(std::byte(1));
            }
            test::expect(std::equal_to<>(), values->step(), true);
            test::expect(std::equal_to<>(), values->column_null(0), true);
            test::expect(std::equal_to<>(), values->column_integer(0), std::int64_t(0));
            test::expect(std::equal_to<>(), values->column_real(0), 0.0);
            test::expect(std::equal_to<>(), values->column_text(0), std::string());
            test::expect(std::equal_to<>(), values->column_blob(0).empty(), true);
            test::expect(std::equal_to<>(), values->column_null(1), false);
            test::expect(std::equal_to<>(), values->column_integer(1), std::numeric_limits<std::int64_t>::min());
            test::expect(std::equal_to<>(), values->column_real(2), 1.25);
            stored_text = values->column_text(3);
            stored_bytes = values->column_blob(4);
            test::expect(std::equal_to<>(), stored_text, std::string("a\0b", 3));
            test::expect(std::equal_to<>(), std::equal(stored_bytes.begin(), stored_bytes.end(), expected_bytes.begin(), expected_bytes.end()), true);
            test::expect_throws<std::out_of_range>([&] { values->column_text(-1); });
            test::expect_throws<std::out_of_range>([&] { values->column_real(5); });
            test::expect_throws<std::out_of_range>([&] { values->column_blob(5); });
            test::expect(std::equal_to<>(), values->step(), false);
            test::expect_throws<std::logic_error>([&] { values->column_null(0); });
            test::expect_throws<std::logic_error>([&] { values->column_integer(1); });
            test::expect_throws<std::logic_error>([&] { values->column_real(2); });
            test::expect_throws<std::logic_error>([&] { values->column_blob(4); });
            values->reset();
            test::expect_throws<std::logic_error>([&] { values->column_text(0); });
            // Reset preserves bindings; clearing them is a separate operation.
            test::expect(std::equal_to<>(), values->step(), true);
            test::expect(std::equal_to<>(), values->column_text(3), stored_text);
            values->reset();
            values->clear_bindings();
            test::expect(std::equal_to<>(), values->step(), true);
            for (int column = 0; column < 5; ++column) {
                test::expect(std::equal_to<>(), values->column_null(column), true);
            }
        }
        test::expect(std::equal_to<>(), stored_text, std::string("a\0b", 3));
        test::expect(std::equal_to<>(), std::equal(stored_bytes.begin(), stored_bytes.end(), expected_bytes.begin(), expected_bytes.end()), true);

        {
            auto empty_values = database.prepare("SELECT ?1, ?2, ?3, typeof(?1), typeof(?2), typeof(?3)");
            empty_values->bind(1, std::string_view());
            empty_values->bind(2, std::span<const std::byte>());
            empty_values->bind(3, nullptr);
            test::expect(std::equal_to<>(), empty_values->step(), true);
            test::expect(std::equal_to<>(), empty_values->column_null(0), false);
            test::expect(std::equal_to<>(), empty_values->column_null(1), false);
            test::expect(std::equal_to<>(), empty_values->column_null(2), true);
            test::expect(std::equal_to<>(), empty_values->column_text(3), std::string("text"));
            test::expect(std::equal_to<>(), empty_values->column_text(4), std::string("blob"));
            test::expect(std::equal_to<>(), empty_values->column_text(5), std::string("null"));
            test::expect(std::equal_to<>(), empty_values->column_text(0).empty(), true);
            test::expect(std::equal_to<>(), empty_values->column_blob(1).empty(), true);
        }

        {
            auto conversions = database.prepare("SELECT '123', 4.5, 67, X'6162'");
            test::expect(std::equal_to<>(), conversions->step(), true);
            test::expect(std::equal_to<>(), conversions->column_integer(0), std::int64_t(123));
            test::expect(std::equal_to<>(), conversions->column_real(0), 123.0);
            test::expect(std::equal_to<>(), conversions->column_integer(1), std::int64_t(4));
            test::expect(std::equal_to<>(), conversions->column_text(2), std::string("67"));
            test::expect(std::equal_to<>(), conversions->column_text(3), std::string("ab"));
        }

        {
            auto select_numbers = database.prepare("SELECT number FROM numbers ORDER BY number");
            std::vector<std::int64_t> numbers;
            while (select_numbers->step()) {
                numbers.push_back(select_numbers->column_integer(0));
            }
            test::expect(std::equal_to<>(), (numbers == std::vector<std::int64_t>{7, 42, 43, 44}), true);
            // Stepping after completion restarts the query.
            test::expect(std::equal_to<>(), select_numbers->step(), true);
            test::expect(std::equal_to<>(), select_numbers->column_integer(0), std::int64_t(7));
            test::expect_throws<std::format_error>([&] { static_cast<void>(std::vformat("{:x}", std::make_format_args(*select_numbers))); });
        }
        test::expect_throws<std::format_error>([&] { static_cast<void>(std::vformat("{:x}", std::make_format_args(database))); });

        {
            auto first_statement = database.prepare("SELECT 9; DROP TABLE numbers");
            test::expect(std::equal_to<>(), first_statement->step(), true);
            test::expect(std::equal_to<>(), first_statement->column_integer(0), std::int64_t(9));
            test::expect(std::equal_to<>(), first_statement->step(), false);
            database.execute("SELECT number FROM numbers");
        }
        database.execute("");
        {
            // Views need not be NUL-terminated at their logical end.
            const std::string filename(":memory:ignored");
            api::database_t view_database(std::string_view(filename).substr(0, 8));
            const std::string sql("SELECT 81 ignored suffix");
            auto view_statement = view_database.prepare(std::string_view(sql).substr(0, 9));
            test::expect(std::equal_to<>(), view_statement->step(), true);
            test::expect(std::equal_to<>(), view_statement->column_integer(0), std::int64_t(81));
        }

        api::database_t temporary_database("");
        temporary_database.execute("CREATE TABLE temporary_values (number INTEGER)");

        const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto directory = std::filesystem::temp_directory_path() / std::format("builder-sqlite-{}", suffix);
        test::expect(std::equal_to<>(), std::filesystem::create_directory(directory), true);
        try {
            const auto filename = (directory / "persistent.sqlite3").string();
            {
                api::database_t persistent_database(filename);
                persistent_database.execute("CREATE TABLE entries (number INTEGER); INSERT INTO entries VALUES (1)");
            }
            test::expect_throws<std::logic_error>([&] {
                api::database_t unwound_database(filename);
                unwound_database.execute("BEGIN IMMEDIATE; INSERT INTO entries VALUES (2)");
                throw std::logic_error("unwind database");
            });
            {
                api::database_t reopened_database(filename);
                auto persisted_rows = reopened_database.prepare("SELECT count(*) FROM entries");
                test::expect(std::equal_to<>(), persisted_rows->step(), true);
                test::expect(std::equal_to<>(), persisted_rows->column_integer(0), std::int64_t(1));
                test::expect(std::equal_to<>(), persisted_rows->step(), false);
                // A second connection can write after the failed owner's destruction.
                reopened_database.execute("INSERT INTO entries VALUES (3)");

                api::database_t reader_database(filename);
                {
                    auto active_read = reader_database.prepare("SELECT number FROM entries");
                    test::expect(std::equal_to<>(), active_read->step(), true);
                    test::expect_throws<std::runtime_error>([&] { reopened_database.execute("BEGIN EXCLUSIVE"); });
                }
                // Finalizing an active statement releases its read lock.
                reopened_database.execute("BEGIN EXCLUSIVE; ROLLBACK");
                test::expect_throws<std::logic_error>([&] {
                    auto unwound_read = reader_database.prepare("SELECT number FROM entries");
                    test::expect(std::equal_to<>(), unwound_read->step(), true);
                    test::expect_throws<std::runtime_error>([&] { reopened_database.execute("BEGIN EXCLUSIVE"); });
                    throw std::logic_error("unwind active read");
                });
                // The lock is released during exception unwinding as well.
                reopened_database.execute("BEGIN EXCLUSIVE; ROLLBACK");
            }
        } catch (...) {
            std::filesystem::remove_all(directory);
            throw;
        }
        std::filesystem::remove_all(directory);
    });
}
