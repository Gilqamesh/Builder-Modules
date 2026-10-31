#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>
#include <m03h5f8oo5d48avkx9mdk2bi5u_sqlite/sqlite.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

namespace sqlite = m03h5f8oo5d48avkx9mdk2bi5u_sqlite;
namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;

struct integer_row_t { std::int64_t number; };
struct pair_row_t { std::int64_t number; std::optional<std::string> label; };
struct codec_failure_t : std::exception {};
struct failing_number_t { std::int64_t number; };
std::function<void()> failure_hook;
template <>
struct column_codec<failing_number_t> {
    using storage_t = std::int64_t;
    static storage_t encode(const failing_number_t& number) { return number.number; }
    static failing_number_t decode(storage_t number) {
        if (failure_hook) failure_hook();
        return failing_number_t{number};
    }
};
struct failing_row_t { failing_number_t number; };

template <typename F>
void expect_runtime(F&& action, std::string_view fragment) {
    bool caught = false;
    try {
        action();
    } catch (const std::runtime_error& error) {
        caught = true;
        const std::string_view message(error.what());
        assert(message.contains(fragment));
        for (const auto native : { "SQLite ", "sqlite3_", "PRAGMA ", "with rowid", "STRICT table", "code 5", "code 10", ":memory:" }) assert(!message.contains(native));
    }
    assert(caught);
}

std::vector<std::string> contents(sqlite::database_t& backend, std::string_view table) {
    std::vector<std::string> snapshot;
    {
        auto schema = backend.prepare("SELECT type, name, sql FROM sqlite_schema WHERE tbl_name=?1 COLLATE NOCASE ORDER BY type, name");
        schema->bind(1, table);
        while (schema->step()) {
            snapshot.push_back(schema->column_text(0));
            snapshot.push_back(schema->column_text(1));
            snapshot.push_back(schema->column_null(2) ? "NULL" : schema->column_text(2));
        }
    }
    std::string projection;
    {
        auto fields = backend.prepare("SELECT name FROM pragma_table_xinfo(?1, 'main') ORDER BY cid");
        fields->bind(1, table);
        while (fields->step()) {
            if (!projection.empty()) projection += ", ";
            // These fixture identifiers are fixed simple names, not user input.
            projection += std::format("quote(\"{}\")", fields->column_text(0));
        }
    }
    auto rows = backend.prepare(std::format("SELECT {} FROM \"{}\"", projection, table));
    while (rows->step()) {
        for (std::size_t index = 0; index < rows->column_count(); ++index) snapshot.push_back(rows->column_text(static_cast<int>(index)));
    }
    return snapshot;
}

void schemas(const std::filesystem::path& directory) {
    const auto filename = directory / "schemas.db";
    {
        sqlite::database_t backend(filename.string());
        backend.execute(
            "CREATE TABLE reordered(label TEXT, number INTEGER NOT NULL) STRICT;"
            "CREATE INDEX reordered_number ON reordered(number);"
            "INSERT INTO reordered(number,label) VALUES (7,'legacy');"
            "CREATE TABLE not_strict(number INTEGER NOT NULL);"
            "CREATE TABLE wrong_kind(number REAL NOT NULL) STRICT;"
            "CREATE TABLE nullable(number INTEGER) STRICT;"
            "CREATE TABLE extra(number INTEGER NOT NULL, other INTEGER) STRICT;"
            "CREATE TABLE generated(number INTEGER NOT NULL, other INTEGER GENERATED ALWAYS AS(number+1)) STRICT;"
            "CREATE TABLE defaults(number INTEGER NOT NULL DEFAULT 1) STRICT;"
            "CREATE TABLE primary_key(number INTEGER PRIMARY KEY) STRICT;"
            "CREATE TABLE unique_key(number INTEGER NOT NULL UNIQUE) STRICT;"
            "CREATE TABLE parent(number INTEGER PRIMARY KEY) STRICT;"
            "CREATE TABLE foreign_key(number INTEGER NOT NULL REFERENCES parent(number)) STRICT;"
            "CREATE TABLE triggered(number INTEGER NOT NULL) STRICT;"
            "CREATE TRIGGER modify_insert AFTER INSERT ON triggered BEGIN UPDATE triggered SET number=number+1; END;"
            "CREATE VIEW view_table AS SELECT number FROM triggered;"
            "CREATE TABLE without_rowid(number INTEGER NOT NULL PRIMARY KEY) STRICT, WITHOUT ROWID;"
            "CREATE TABLE other_case(Number INTEGER NOT NULL) STRICT;"
        );
    }
    database_t database(filename);
    const auto reordered = database.table<pair_row_t>("reordered");
    assert(reordered.rows().size() == 1 && reordered.rows().front().label == "legacy");
    reordered.insert(pair_row_t{42, "reordered"});
    const auto rows = reordered.rows();
    assert(rows.size() == 2);
    assert(std::ranges::any_of(rows, [](const pair_row_t& row) { return row.number == 42 && row.label == "reordered"; }));
    for (const auto table : { "not_strict", "wrong_kind", "nullable", "extra", "generated", "defaults", "primary_key", "unique_key", "foreign_key", "triggered", "view_table", "without_rowid", "other_case" }) {
        sqlite::database_t backend(filename.string());
        if (std::string_view(table) != "view_table") backend.execute(std::format("INSERT INTO \"{}\"(number) VALUES (11)", table));
        const auto before = contents(backend, table);
        expect_runtime([&] { (void)database.table<integer_row_t>(table); }, "table");
        assert(contents(backend, table) == before);
        reordered.insert(pair_row_t{1, {}});
    }
    database.table<integer_row_t>("after errors");
    database.table<integer_row_t>("after errors").insert(integer_row_t{7});
    assert(database.table<integer_row_t>("after errors").rows().front().number == 7);
}

void stored_kinds(const std::filesystem::path& directory) {
    const auto filename = directory / "stored-kinds.db";
    {
        sqlite::database_t backend(filename.string());
        backend.execute(
            "CREATE TABLE wrong_value(number INTEGER NOT NULL);"
            "INSERT INTO wrong_value VALUES ('123');"
            "UPDATE wrong_value SET number='not an integer';"
            "CREATE TABLE wrong_null(number INTEGER);"
            "INSERT INTO wrong_null VALUES (NULL);"
            "PRAGMA writable_schema=ON;"
            "UPDATE sqlite_schema SET sql='CREATE TABLE wrong_value(number INTEGER NOT NULL) STRICT' WHERE name='wrong_value';"
            "UPDATE sqlite_schema SET sql='CREATE TABLE wrong_null(number INTEGER NOT NULL) STRICT' WHERE name='wrong_null';"
            "PRAGMA writable_schema=OFF;"
        );
    }
    // Deliberately inconsistent fixture: STRICT metadata does not make old values valid.
    database_t database(filename);
    expect_runtime([&] { (void)database.table<integer_row_t>("wrong_value").rows(); }, "expected signed 64-bit integer; found string");
    expect_runtime([&] { (void)database.table<integer_row_t>("wrong_null").rows(); }, "found absent value");
    database.table<integer_row_t>("valid");
    database.table<integer_row_t>("valid").insert(integer_row_t{3});
    assert(database.table<integer_row_t>("valid").rows().front().number == 3);
}

void injected_failures(const std::filesystem::path& directory) {
    const auto filename = directory / "failures.db";
    assert(setenv("BUILDER_DATABASE_TEST_FAULT", "open", 1) == 0);
    expect_runtime([] { database_t database; }, "in-memory database");
    assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr);
    {
        database_t database(filename);
        const auto numbers = database.table<integer_row_t>("numbers");
        const auto failing = database.table<failing_row_t>("numbers");
        numbers.insert(integer_row_t{1});
        const auto inject = [](const char* fault) { assert(setenv("BUILDER_DATABASE_TEST_FAULT", fault, 1) == 0); };
        const auto consumed = [] { assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr); };
        inject("commit");
        expect_runtime([&] { numbers.insert(integer_row_t{2}); }, "insert");
        consumed();
        assert(numbers.rows().size() == 1);
        inject("commit");
        expect_runtime([&] { (void)database.table<integer_row_t>("uncommitted"); }, "table");
        consumed();
        {
            sqlite::database_t backend(filename.string());
            auto statement = backend.prepare("SELECT 1 FROM sqlite_schema WHERE name='uncommitted'");
            assert(!statement->step());
        }
        inject("commit");
        expect_runtime([&] { numbers.drop(); }, "drop");
        consumed();
        assert(numbers.rows().size() == 1);
        inject("commit");
        expect_runtime([&] {
            database.transaction([&](database_t&) { numbers.insert(integer_row_t{3}); });
        }, "commit");
        consumed();
        assert(numbers.rows().size() == 1);

        expect_runtime([&] {
            database.transaction([&](database_t&) {
                numbers.insert(integer_row_t{4});
                failure_hook = [&] { inject("recovery"); throw codec_failure_t{}; };
                bool caught = false;
                try { (void)failing.rows(); } catch (const codec_failure_t&) { caught = true; }
                failure_hook = {};
                consumed();
                assert(caught);
                expect_runtime([&] { numbers.insert(integer_row_t{5}); }, "invalid");
            });
        }, "cannot commit");
        assert(numbers.rows().size() == 1);

        // A callback exception survives an outer rollback failure; no future commit is allowed.
        bool original = false;
        try {
            database.transaction([&](database_t&) {
                numbers.insert(integer_row_t{6});
                inject("rollback");
                throw codec_failure_t{};
            });
        } catch (const codec_failure_t&) { original = true; }
        consumed();
        assert(original);
        expect_runtime([&] { (void)numbers.rows(); }, "invalid");
    }
    {
        database_t database(filename);
        const auto numbers = database.table<integer_row_t>("numbers");
        assert(numbers.rows().size() == 1);
        expect_runtime([&] {
            database.transaction([&](database_t&) {
                numbers.insert(integer_row_t{7});
                assert(setenv("BUILDER_DATABASE_TEST_FAULT", "automatic", 1) == 0);
                expect_runtime([&] { numbers.insert(integer_row_t{8}); }, "insert");
                assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr);
            });
        }, "cannot commit");
    }
    {
        database_t database(filename);
        assert(database.table<integer_row_t>("numbers").rows().size() == 1);
    }
}

void malformed_file(const std::filesystem::path& directory) {
    const auto filename = directory / "malformed.db";
    { std::ofstream output(filename); output << "This is not an SQLite database"; }
    expect_runtime([&] { database_t database(filename); }, "read schema");
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

int main() {
    using namespace m03h5hm86mnvc18opf1rdttsgh_database;
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-backend-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    filesystem::create_directories(directory);
    schemas(directory);
    stored_kinds(directory);
    injected_failures(directory);
    malformed_file(directory);
    filesystem::remove_all(directory);
}
