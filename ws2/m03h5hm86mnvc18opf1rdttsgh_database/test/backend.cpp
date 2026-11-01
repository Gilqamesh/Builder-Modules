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
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <sys/stat.h>

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
    const auto storage = directory / "schemas.db";
    { database_t database(storage); }
    const auto filename = storage / "storage.db";
    {
        sqlite::database_t backend(filename.string());
        backend.execute(
            "CREATE TABLE table_reordered(label TEXT, number INTEGER NOT NULL) STRICT;"
            "CREATE INDEX reordered_number ON table_reordered(number);"
            "INSERT INTO table_reordered(number,label) VALUES (7,'legacy');"
            "CREATE TABLE table_not_strict(number INTEGER NOT NULL);"
            "CREATE TABLE table_wrong_kind(number REAL NOT NULL) STRICT;"
            "CREATE TABLE table_nullable(number INTEGER) STRICT;"
            "CREATE TABLE table_extra(number INTEGER NOT NULL, other INTEGER) STRICT;"
            "CREATE TABLE table_generated(number INTEGER NOT NULL, other INTEGER GENERATED ALWAYS AS(number+1)) STRICT;"
            "CREATE TABLE table_defaults(number INTEGER NOT NULL DEFAULT 1) STRICT;"
            "CREATE TABLE table_primary_key(number INTEGER PRIMARY KEY) STRICT;"
            "CREATE TABLE table_unique_key(number INTEGER NOT NULL UNIQUE) STRICT;"
            "CREATE TABLE table_parent(number INTEGER PRIMARY KEY) STRICT;"
            "CREATE TABLE table_foreign_key(number INTEGER NOT NULL REFERENCES table_parent(number)) STRICT;"
            "CREATE TABLE table_triggered(number INTEGER NOT NULL) STRICT;"
            "CREATE TRIGGER modify_insert AFTER INSERT ON table_triggered BEGIN UPDATE table_triggered SET number=number+1; END;"
            "CREATE VIEW table_view_table AS SELECT number FROM table_triggered;"
            "CREATE TABLE table_without_rowid(number INTEGER NOT NULL PRIMARY KEY) STRICT, WITHOUT ROWID;"
            "CREATE TABLE table_other_case(Number INTEGER NOT NULL) STRICT;"
        );
    }
    database_t database(storage);
    const auto reordered = database.table<pair_row_t>("reordered");
    assert(reordered.rows().size() == 1 && reordered.rows().front().label == "legacy");
    reordered.insert(pair_row_t{42, "reordered"});
    const auto rows = reordered.rows();
    assert(rows.size() == 2);
    assert(std::ranges::any_of(rows, [](const pair_row_t& row) { return row.number == 42 && row.label == "reordered"; }));
    for (const auto table : { "not_strict", "wrong_kind", "nullable", "extra", "generated", "defaults", "primary_key", "unique_key", "foreign_key", "triggered", "view_table", "without_rowid", "other_case" }) {
        sqlite::database_t backend(filename.string());
        const auto physical = std::format("table_{}", table);
        if (std::string_view(table) != "view_table") backend.execute(std::format("INSERT INTO \"{}\"(number) VALUES (11)", physical));
        const auto before = contents(backend, physical);
        expect_runtime([&] { (void)database.table<integer_row_t>(table); }, "table");
        assert(contents(backend, physical) == before);
        reordered.insert(pair_row_t{1, {}});
    }
    database.table<integer_row_t>("after errors");
    database.table<integer_row_t>("after errors").insert(integer_row_t{7});
    assert(database.table<integer_row_t>("after errors").rows().front().number == 7);
}

void stored_kinds(const std::filesystem::path& directory) {
    const auto storage = directory / "stored-kinds.db";
    { database_t database(storage); }
    const auto filename = storage / "storage.db";
    {
        sqlite::database_t backend(filename.string());
        backend.execute(
            "CREATE TABLE table_wrong_value(number INTEGER NOT NULL);"
            "INSERT INTO table_wrong_value VALUES ('123');"
            "UPDATE table_wrong_value SET number='not an integer';"
            "CREATE TABLE table_wrong_null(number INTEGER);"
            "INSERT INTO table_wrong_null VALUES (NULL);"
            "PRAGMA writable_schema=ON;"
            "UPDATE sqlite_schema SET sql='CREATE TABLE table_wrong_value(number INTEGER NOT NULL) STRICT' WHERE name='table_wrong_value';"
            "UPDATE sqlite_schema SET sql='CREATE TABLE table_wrong_null(number INTEGER NOT NULL) STRICT' WHERE name='table_wrong_null';"
            "PRAGMA writable_schema=OFF;"
        );
    }
    // Deliberately inconsistent fixture: STRICT metadata does not make old values valid.
    database_t database(storage);
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
            sqlite::database_t backend((filename / "storage.db").string());
            auto statement = backend.prepare("SELECT 1 FROM sqlite_schema WHERE name='table_uncommitted'");
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
    const auto storage = directory / "malformed.db";
    { database_t database(storage); }
    const auto filename = storage / "storage.db";
    { std::ofstream output(filename); output << "This is not an SQLite database"; }
    expect_runtime([&] { database_t database(storage); }, "read schema");
}

std::map<std::string, std::string> storage_snapshot(const std::filesystem::path& directory) {
    std::map<std::string, std::string> snapshot;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        const auto name = entry.path().lexically_relative(directory).string();
        if (entry.is_symlink()) {
            snapshot[name] = "link:" + std::filesystem::read_symlink(entry.path()).string();
        } else if (entry.is_regular_file()) {
            std::ifstream input(entry.path(), std::ios::binary);
            snapshot[name] = std::string(std::istreambuf_iterator<char>(input), {});
        } else {
            snapshot[name] = std::format("type:{}", static_cast<int>(entry.symlink_status().type()));
        }
    }
    return snapshot;
}

void rejected_storage(const std::filesystem::path& directory) {
    const auto baseline = directory / "baseline";
    {
        database_t database(baseline);
        database.table<integer_row_t>("numbers").insert(integer_row_t{42});
    }
    const auto metadata = storage_snapshot(baseline).at("storage.meta");
    const auto reject_unchanged = [&](const std::filesystem::path& storage, std::string_view fragment) {
        const auto before = storage_snapshot(directory);
        expect_runtime([&] { database_t database(storage); }, fragment);
        assert(storage_snapshot(directory) == before);
    };
    const auto check = [&](std::string_view name, auto alter, std::string_view fragment) {
        const auto storage = directory / name;
        std::filesystem::copy(baseline, storage, std::filesystem::copy_options::recursive);
        alter(storage);
        reject_unchanged(storage, fragment);
        std::filesystem::remove_all(storage);
    };
    check("missing-metadata", [](const auto& storage) { std::filesystem::remove(storage / "storage.meta"); }, "incomplete");
    check("missing-database", [](const auto& storage) { std::filesystem::remove(storage / "storage.db"); }, "incomplete");
    check("invalid-metadata", [](const auto& storage) { std::ofstream(storage / "storage.meta") << "invalid"; }, "metadata");
    check("unknown-identity", [](const auto& storage) { std::ofstream(storage / "storage.meta") << "unrelated\n1\nsqlite\n"; }, "identity");
    check("future-version", [](const auto& storage) { std::ofstream(storage / "storage.meta") << "m03h5hm86mnvc18opf1rdttsgh_database\n2\nsqlite\n"; }, "unsupported storage version '2'");
    check("unknown-backend", [](const auto& storage) { std::ofstream(storage / "storage.meta") << "m03h5hm86mnvc18opf1rdttsgh_database\n1\nother\n"; }, "representation");
    check("extra-metadata", [&](const auto& storage) { std::ofstream(storage / "storage.meta") << metadata << "extra\n"; }, "metadata");
    check("pending-metadata", [](const auto& storage) { std::filesystem::rename(storage / "storage.meta", storage / "storage.meta.pending"); }, "unrecognized storage entry");
    check("extra-file", [](const auto& storage) { std::ofstream(storage / "notes.txt") << "keep me"; }, "unrecognized storage entry");
    check("extra-directory", [](const auto& storage) {
        std::filesystem::create_directory(storage / "unrelated");
        std::ofstream(storage / "unrelated" / "notes.txt") << "keep me";
    }, "unrecognized storage entry");
    for (const auto member : { "storage.meta", "storage.db", "storage.db-wal", "storage.db-shm", "storage.db-journal" }) {
        const auto external = directory / "external-file";
        { std::ofstream(external) << "do not touch"; }
        check("symlink", [&](const auto& storage) {
            std::filesystem::remove(storage / member);
            std::filesystem::create_symlink(external, storage / member);
        }, "unlinked file");
        check("dangling-symlink", [&](const auto& storage) {
            std::filesystem::remove(storage / member);
            std::filesystem::create_symlink(directory / "absent", storage / member);
        }, "open");
        check("hard-link", [&](const auto& storage) {
            std::filesystem::remove(storage / member);
            std::filesystem::create_hard_link(external, storage / member);
        }, "unlinked file");
        assert(std::filesystem::hard_link_count(external) == 1);
        check("special-file", [&](const auto& storage) {
            std::filesystem::remove(storage / member);
            assert(mkfifo((storage / member).c_str(), 0600) == 0);
        }, "unlinked file");
        std::filesystem::remove(external);
    }
    const auto legacy = directory / "legacy.db";
    {
        sqlite::database_t backend(legacy.string());
        backend.execute("CREATE TABLE numbers(number INTEGER NOT NULL) STRICT; INSERT INTO numbers VALUES(1); CREATE TABLE table_numbers(number INTEGER NOT NULL) STRICT; INSERT INTO table_numbers VALUES(2)");
    }
    { std::ofstream(legacy.string() + "-wal") << "unrelated neighbor"; }
    reject_unchanged(legacy, "legacy database files");
}

void physical_tables(const std::filesystem::path& directory) {
    const auto storage = directory / "physical-tables";
    {
        database_t database(storage);
        database.table<integer_row_t>("sqlite_people").insert(integer_row_t{1});
        database.table<integer_row_t>("people").insert(integer_row_t{2});
        database.table<integer_row_t>("table_people").insert(integer_row_t{3});
    }
    {
        sqlite::database_t backend((storage / "storage.db").string());
        auto statement = backend.prepare("SELECT name FROM sqlite_schema WHERE type='table' ORDER BY name");
        for (const auto name : { "table_people", "table_sqlite_people", "table_table_people" }) {
            assert(statement->step() && statement->column_text(0) == name);
        }
        assert(!statement->step());
        backend.execute("CREATE TABLE people(number INTEGER NOT NULL) STRICT; INSERT INTO people VALUES(99); CREATE TABLE orphan(number INTEGER NOT NULL) STRICT; INSERT INTO orphan VALUES(100)");
    }
    {
        database_t database(storage);
        const auto people = database.table<integer_row_t>("PEOPLE");
        assert(people.rows().size() == 1 && people.rows().front().number == 2);
        people.insert(integer_row_t{4});
        people.drop();
        assert(database.table<integer_row_t>("orphan").rows().empty());
        database.table<integer_row_t>("orphan").drop();
        assert(database.table<integer_row_t>("table_people").rows().front().number == 3);
    }
    sqlite::database_t backend((storage / "storage.db").string());
    auto statement = backend.prepare("SELECT number FROM people UNION ALL SELECT number FROM orphan ORDER BY number");
    assert(statement->step() && statement->column_integer(0) == 99);
    assert(statement->step() && statement->column_integer(0) == 100);
    assert(!statement->step());
}

void initialization_failures(const std::filesystem::path& directory) {
    const auto failed_open = directory / "failed-open";
    assert(setenv("BUILDER_DATABASE_TEST_FAULT", "open", 1) == 0);
    expect_runtime([&] { database_t database(failed_open); }, "open");
    assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr);
    assert(!std::filesystem::exists(failed_open / "storage.meta"));

    const auto failed_wal = directory / "failed-wal";
    assert(setenv("BUILDER_DATABASE_TEST_FAULT", "wal", 1) == 0);
    expect_runtime([&] { database_t database(failed_wal); }, "open");
    assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr);
    assert(std::filesystem::exists(failed_wal / "storage.db"));
    assert(!std::filesystem::exists(failed_wal / "storage.meta"));
    const auto before = storage_snapshot(failed_wal);
    expect_runtime([&] { database_t database(failed_wal); }, "incomplete");
    assert(storage_snapshot(failed_wal) == before);

    const auto failed_publish = directory / "failed-publish";
    assert(setenv("BUILDER_DATABASE_TEST_FAULT", "publish", 1) == 0);
    expect_runtime([&] { database_t database(failed_publish); }, "open");
    assert(std::getenv("BUILDER_DATABASE_TEST_FAULT") == nullptr);
    assert(std::filesystem::exists(failed_publish / "storage.db"));
    assert(std::filesystem::exists(failed_publish / "storage.meta.pending"));
    assert(!std::filesystem::exists(failed_publish / "storage.meta"));
    const auto unpublished = storage_snapshot(failed_publish);
    expect_runtime([&] { database_t database(failed_publish); }, "unrecognized storage entry");
    assert(storage_snapshot(failed_publish) == unpublished);
    // All failed-construction connections must have closed before cleanup/retry.
    std::filesystem::remove_all(failed_wal);
    { database_t database(failed_wal); database.table<integer_row_t>("numbers").insert(integer_row_t{7}); }
    { database_t database(failed_wal); assert(database.table<integer_row_t>("numbers").rows().front().number == 7); }
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
    rejected_storage(directory);
    physical_tables(directory);
    initialization_failures(directory);
    filesystem::remove_all(directory);
}
