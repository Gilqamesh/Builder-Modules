#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

// Generation order deliberately differs from chain order.
const unique_id_t v1{"03h5mgnvq9zym7wvkijwwaobs"};
const unique_id_t v2{"03h5mgnvq9zym7wvkijwwaobl"};
const unique_id_t v3{"03h5mgnvq9zym7wvkijwwaobq"};
const unique_id_t unknown_version{"03h5mgnvq9zym7wvkijwwaobm"};

struct number_row_t { int number; };
struct metadata_row_t { std::string version; };
struct wrong_metadata_t { int version; };
struct nullable_metadata_t { std::optional<std::string> version; };
struct extra_metadata_t { std::string version; int extra; };
struct renamed_metadata_t { std::string revision; };
struct callback_error_t {};

std::vector<int> calls;

void initialize(database_t& database) {
    calls.push_back(1);
    database.create_table<number_row_t>("numbers").insert(number_row_t{10});
}

void advance(database_t& database) {
    calls.push_back(2);
    const auto numbers = database.create_table<number_row_t>("numbers");
    assert(numbers.replace<&number_row_t::number>(10, number_row_t{20}) == 1);
}

void finish(database_t& database) {
    calls.push_back(3);
    const auto numbers = database.create_table<number_row_t>("numbers");
    assert(numbers.replace<&number_row_t::number>(20, number_row_t{30}) == 1);
}

void fail(database_t& database) {
    calls.push_back(4);
    database.drop_table<number_row_t>("numbers");
    database.create_table<number_row_t>("partial").insert(number_row_t{99});
    throw callback_error_t{};
}

void alter_metadata(database_t& database) {
    database.create_table<metadata_row_t>(metadata_table_name).insert(metadata_row_t{unknown_version.string()});
}

void delete_metadata(database_t& database) {
    database.drop_table<metadata_row_t>(metadata_table_name);
}

void rewrite_metadata(database_t& database) {
    const auto metadata = database.create_table<metadata_row_t>(metadata_table_name);
    assert(metadata.replace<&metadata_row_t::version>(v1.string(), metadata_row_t{unknown_version.string()}) == 1);
}

void nested_migration(database_t& database) {
    migrate(database, baseline_version, {});
}

const std::array history = {
    migration_t{v2, v3, finish},
    migration_t{baseline_version, v1, initialize},
    migration_t{v1, v2, advance}
};

template <typename E, typename F>
void expect_failure(F&& action, std::string_view fragment = {}) {
    bool caught = false;
    try {
        action();
    } catch (const E& error) {
        caught = true;
        if constexpr (requires { error.what(); }) {
            if (!fragment.empty()) assert(std::string_view(error.what()).contains(fragment));
        }
    }
    assert(caught);
}

std::string persisted_version(database_t& database) {
    const auto records = database.create_table<metadata_row_t>(metadata_table_name).rows();
    assert(records.size() == 1);
    return records.front().version;
}

void verify_number(database_t& database, int expected) {
    const auto numbers = database.create_table<number_row_t>("numbers").rows();
    assert(numbers.size() == 1 && numbers.front().number == expected);
}

void progression(database_t& database) {
    calls.clear();
    migrate(database, baseline_version, {});
    assert(persisted_version(database) == baseline_version.string());
    assert(database.tables() == std::vector<std::string>{std::string(metadata_table_name)});
    migrate(database, baseline_version, {});
    assert(calls.empty());

    migrate(database, v1, history);
    assert(calls == std::vector<int>{1});
    verify_number(database, 10);
    assert(persisted_version(database) == v1.string());
    calls.clear();
    migrate(database, v3, history);
    assert((calls == std::vector<int>{2, 3}));
    verify_number(database, 30);
    assert(persisted_version(database) == v3.string());
    const auto names = database.tables();
    calls.clear();
    migrate(database, v3, history);
    migrate(database, v3, history);
    assert(calls.empty() && database.tables() == names);

    expect_failure<std::runtime_error>([&] { migrate(database, v1, history); }, "downgrade");
    expect_failure<std::runtime_error>([&] { migrate(database, baseline_version, history); }, "downgrade");
    const std::array v1_history = {migration_t{baseline_version, v1, initialize}};
    expect_failure<std::runtime_error>([&] { migrate(database, v1, v1_history); }, "unknown");
    assert(calls.empty() && database.tables() == names);
    verify_number(database, 30);
    assert(persisted_version(database) == v3.string());
}

void malformed_chains(database_t& database) {
    calls.clear();
    const auto reject = [&](std::span<const migration_t> migrations, const unique_id_t& target = v3) {
        const auto names = database.tables();
        expect_failure<std::invalid_argument>([&] { migrate(database, target, migrations); });
        assert(database.tables() == names && calls.empty());
    };
    const migration_t start{baseline_version, v1, initialize};
    const migration_t next{v1, v2, advance};
    const migration_t last{v2, v3, finish};
    reject({});
    reject(std::array{next, last}); // Missing baseline edge.
    reject(std::array{start, last}); // Missing intermediate transition.
    reject(std::array{start, start}); // Duplicate edge.
    reject(std::array{start, migration_t{baseline_version, v2, advance}}); // Fork.
    reject(std::array{start, next, migration_t{v3, v2, finish}}); // Merge.
    reject(std::array{start, next, migration_t{v2, v1, finish}}); // Cycle.
    reject(std::array{start, migration_t{v2, v3, finish}, migration_t{v3, v2, finish}}); // Disconnected cycle.
    reject(std::array{migration_t{v1, v1, initialize}});
    reject(std::array{start, migration_t{v1, baseline_version, advance}});
    reject(std::array{migration_t{baseline_version, v1, nullptr}});
    reject(history, unknown_version);
    // Even a no-op target validates all supplied definitions first.
    reject(std::array{start, start}, baseline_version);
}

void rollback(database_t& database) {
    const std::array start = {migration_t{baseline_version, v1, initialize}};
    migrate(database, v1, start);
    calls.clear();
    const std::array failing_history = {
        migration_t{baseline_version, v1, initialize},
        migration_t{v1, v2, advance},
        migration_t{v2, v3, fail}
    };
    const auto names = database.tables();
    expect_failure<callback_error_t>([&] { migrate(database, v3, failing_history); });
    assert((calls == std::vector<int>{2, 4}));
    assert(database.tables() == names);
    verify_number(database, 10);
    assert(persisted_version(database) == v1.string());

    for (const auto callback : {alter_metadata, delete_metadata, rewrite_metadata}) {
        const std::array corrupting_history = {start.front(), migration_t{v1, v2, callback}};
        expect_failure<std::runtime_error>([&] { migrate(database, v2, corrupting_history); });
        assert(database.tables() == names);
        assert(persisted_version(database) == v1.string());
        verify_number(database, 10);
    }
    const std::array nested_history = {start.front(), migration_t{v1, v2, nested_migration}};
    expect_failure<std::logic_error>([&] { migrate(database, v2, nested_history); });
    assert(persisted_version(database) == v1.string());

    // A later explicit call may succeed, but the failed call did not retry.
    calls.clear();
    migrate(database, v3, history);
    assert((calls == std::vector<int>{2, 3}));
    verify_number(database, 30);
}

void failed_initialization(database_t& database) {
    calls.clear();
    const std::array failing_history = {
        migration_t{baseline_version, v1, initialize},
        migration_t{v1, v2, fail}
    };
    expect_failure<callback_error_t>([&] { migrate(database, v2, failing_history); });
    assert((calls == std::vector<int>{1, 4}));
    assert(database.tables().empty());
}

void unversioned(database_t& database) {
    const auto numbers = database.create_table<number_row_t>("numbers");
    calls.clear();
    // Even an empty application table prevents implicit adoption.
    expect_failure<std::runtime_error>([&] { migrate(database, v3, history); }, "unversioned");
    assert(database.tables() == std::vector<std::string>{"numbers"} && numbers.rows().empty());
    numbers.insert(number_row_t{7});
    expect_failure<std::runtime_error>([&] { migrate(database, v3, history); }, "unversioned");
    verify_number(database, 7);
    assert(calls.empty());
}

void corrupt_metadata(database_t& database) {
    const auto metadata = database.create_table<metadata_row_t>(metadata_table_name);
    calls.clear();
    const auto rejected = [&] {
        const auto records = metadata.rows();
        const auto names = database.tables();
        expect_failure<std::runtime_error>([&] { migrate(database, v3, history); });
        const auto preserved = metadata.rows();
        assert(preserved.size() == records.size() && database.tables() == names && calls.empty());
        std::vector<std::string> before;
        std::vector<std::string> after;
        for (const auto& metadata_row : records) before.push_back(metadata_row.version);
        for (const auto& metadata_row : preserved) after.push_back(metadata_row.version);
        std::ranges::sort(before);
        std::ranges::sort(after);
        assert(before == after);
    };
    rejected(); // No record.
    metadata.insert(metadata_row_t{baseline_version.string()});
    metadata.insert(metadata_row_t{baseline_version.string()});
    rejected(); // Duplicate identical records.
    assert(metadata.replace<&metadata_row_t::version>(baseline_version.string(), metadata_row_t{v1.string()}) == 2);
    metadata.insert(metadata_row_t{v3.string()});
    rejected(); // Conflicting records.
    database.drop_table<metadata_row_t>(metadata_table_name);
    database.create_table<metadata_row_t>(metadata_table_name);
    for (const auto& version : {std::string(""), std::string("broken"), std::string("0000000000000000000000000"), unknown_version.string()}) {
        metadata.insert(metadata_row_t{version});
        rejected();
        assert(metadata.erase<&metadata_row_t::version>(version) == 1);
    }
}

template <typename T>
void incompatible_metadata(database_t& database, const T& record) {
    database.create_table<T>(metadata_table_name).insert(record);
    calls.clear();
    expect_failure<std::runtime_error>([&] { migrate(database, v3, history); });
    assert(calls.empty() && database.create_table<T>(metadata_table_name).rows().size() == 1);
    database.drop_table<T>(metadata_table_name);
}

void metadata_schema(database_t& database) {
    incompatible_metadata(database, wrong_metadata_t{7});
    incompatible_metadata(database, nullable_metadata_t{baseline_version.string()});
    incompatible_metadata(database, extra_metadata_t{baseline_version.string(), 7});
    incompatible_metadata(database, renamed_metadata_t{baseline_version.string()});
    // The database's ASCII-insensitive naming must not hide a reserved table.
    constexpr std::string_view uppercase_name = "__BUILDER_APPLICATION_MIGRATIONS";
    database.create_table<metadata_row_t>(uppercase_name).insert(metadata_row_t{baseline_version.string()});
    migrate(database, v3, history);
    assert(persisted_version(database) == v3.string());
    assert((database.tables() == std::vector<std::string>{std::string(uppercase_name), "numbers"}));
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    assert(baseline_version.string() == "03h5mgnvq9zym7wvkijwwaobj"); // Published identity must remain immutable.
    assert(std::format("{}", history.front()).contains(" -> "));
    for (const auto exercise : {progression, malformed_chains, rollback, failed_initialization, unversioned, corrupt_metadata, metadata_schema}) {
        database_t database;
        exercise(database);
    }

    const auto root = std::filesystem::temp_directory_path() / std::format("application-migrations-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::create_directories(root);
    std::size_t directory_index = 0;
    for (const auto exercise : {progression, malformed_chains, rollback, failed_initialization, unversioned, corrupt_metadata, metadata_schema}) {
        const auto directory = root / std::to_string(directory_index++);
        database_t database(directory);
        exercise(database);
    }
    {
        database_t database(root / "failed");
        const std::array start = {migration_t{baseline_version, v1, initialize}};
        migrate(database, v1, start);
        const std::array failing_history = {start.front(), migration_t{v1, v2, advance}, migration_t{v2, v3, fail}};
        expect_failure<callback_error_t>([&] { migrate(database, v3, failing_history); });
    }
    {
        database_t reopened_database(root / "failed");
        assert(persisted_version(reopened_database) == v1.string());
        verify_number(reopened_database, 10);
        assert((reopened_database.tables() == std::vector<std::string>{std::string(metadata_table_name), "numbers"}));
    }
    {
        database_t database(root / "new-failed");
        failed_initialization(database);
    }
    {
        database_t reopened_database(root / "new-failed");
        assert(reopened_database.tables().empty());
    }
    {
        database_t database(root / "committed");
        progression(database);
    }
    {
        database_t reopened_database(root / "committed");
        calls.clear();
        migrate(reopened_database, v3, history);
        assert(calls.empty() && persisted_version(reopened_database) == v3.string());
        verify_number(reopened_database, 30);
        malformed_chains(reopened_database);
        assert(persisted_version(reopened_database) == v3.string());
        verify_number(reopened_database, 30);
    }
    std::filesystem::remove_all(root);
}
