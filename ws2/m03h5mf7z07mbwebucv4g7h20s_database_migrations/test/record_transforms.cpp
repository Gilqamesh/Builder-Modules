#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

constexpr std::string_view v1 = "03h5mgnvq9zym7wvkijwwaobp";
constexpr std::string_view v2 = "03h5mgnvq9zym7wvkijwwaobo";
constexpr std::string_view v3 = "03h5mgnvq9zym7wvkijwwaobn";

struct v1_person_t {
    std::int64_t id;
    std::string name;
    std::int64_t balance_cents;
};

struct v2_person_t {
    std::int64_t id;
    std::string name;
    std::int64_t balance_cents;
    std::string display_name;
};

struct v3_person_t {
    std::int64_t id;
    std::string display_name;
    double balance;
};

struct version_row_t { std::string version; };

void initialize_people(database_t& database) {
    const auto v1_people = database.create_table<v1_person_t>("people");
}

void add_display_names(database_t& database) {
    const auto v1_people = database.create_table<v1_person_t>("people");
    const auto v1_rows = v1_people.rows();
    database.drop_table<v1_person_t>("people");
    const auto v2_people = database.create_table<v2_person_t>("people");
    for (const auto& v1_person : v1_rows) {
        const auto display_name = "Customer: " + v1_person.name;
        const v2_person_t v2_person{v1_person.id, v1_person.name, v1_person.balance_cents, display_name};
        v2_people.insert(v2_person);
    }
}

void convert_balances(database_t& database) {
    const auto v2_people = database.create_table<v2_person_t>("people");
    const auto v2_rows = v2_people.rows();
    database.drop_table<v2_person_t>("people");
    const auto v3_people = database.create_table<v3_person_t>("people");
    for (const auto& v2_person : v2_rows) {
        const auto balance = static_cast<double>(v2_person.balance_cents) / 100.0;
        const v3_person_t v3_person{v2_person.id, v2_person.display_name, balance};
        v3_people.insert(v3_person);
    }
}

const std::array migrations = {
    migration_t{baseline_version, v1, initialize_people},
    migration_t{v1, v2, add_display_names},
    migration_t{v2, v3, convert_balances}
};

void seed(database_t& database) {
    migrate(database, v1, migrations);
    const auto v1_people = database.create_table<v1_person_t>("people");
    v1_people.insert(v1_person_t{1, "Ada", 1250});
    v1_people.insert(v1_person_t{2, "Grace", -325});
}

void verify_v1(database_t& database) {
    assert(database.create_table<version_row_t>(metadata_table_name).rows().front().version == v1);
    const auto v1_people = database.create_table<v1_person_t>("people");
    assert(v1_people.rows().size() == 2);
    const auto ada = v1_people.rows<&v1_person_t::id>(1);
    const auto grace = v1_people.rows<&v1_person_t::id>(2);
    assert(ada.size() == 1 && ada.front().name == "Ada" && ada.front().balance_cents == 1250);
    assert(grace.size() == 1 && grace.front().name == "Grace" && grace.front().balance_cents == -325);
}

void verify_v2(database_t& database) {
    const auto metadata = database.create_table<version_row_t>(metadata_table_name);
    const auto version_rows = metadata.rows();
    assert(version_rows.size() == 1);
    assert(version_rows.front().version == v2);

    const auto v2_people = database.create_table<v2_person_t>("people");
    const auto v2_rows = v2_people.rows();
    assert(v2_rows.size() == 2);

    const auto ada_rows = v2_people.rows<&v2_person_t::id>(1);
    assert(ada_rows.size() == 1);
    const auto& ada_person = ada_rows.front();
    assert(ada_person.name == "Ada");
    assert(ada_person.balance_cents == 1250);
    assert(ada_person.display_name == "Customer: Ada");

    const auto grace_rows = v2_people.rows<&v2_person_t::id>(2);
    assert(grace_rows.size() == 1);
    const auto& grace_person = grace_rows.front();
    assert(grace_person.name == "Grace");
    assert(grace_person.balance_cents == -325);
    assert(grace_person.display_name == "Customer: Grace");
}

void verify_v3(database_t& database) {
    assert(database.create_table<version_row_t>(metadata_table_name).rows().front().version == v3);
    const auto v3_people = database.create_table<v3_person_t>("people");
    assert(v3_people.rows().size() == 2);
    const auto ada = v3_people.rows<&v3_person_t::id>(1);
    const auto grace = v3_people.rows<&v3_person_t::id>(2);
    assert(ada.size() == 1 && ada.front().display_name == "Customer: Ada" && ada.front().balance == 12.5);
    assert(grace.size() == 1 && grace.front().display_name == "Customer: Grace" && grace.front().balance == -3.25);

    bool obsolete_schema_rejected = false;
    try {
        const auto v2_people = database.create_table<v2_person_t>("people");
    } catch (const std::runtime_error&) {
        obsolete_schema_rejected = true;
    }
    assert(obsolete_schema_rejected);
}

void failed_transform(database_t& database) {
    bool caught = false;
    try {
        const std::array failing_migrations = {
            migrations[0],
            migrations[1],
            migration_t{v2, v3, [](database_t& database) {
                convert_balances(database);
                throw std::runtime_error("transformation sentinel");
            }}
        };
        migrate(database, v3, failing_migrations);
    } catch (const std::runtime_error& error) {
        assert(std::string(error.what()) == "transformation sentinel");
        caught = true;
    }
    assert(caught);
    verify_v1(database);
}

void successful_transform(database_t& database) {
    migrate(database, v3, migrations);
    migrate(database, v3, migrations);
    verify_v3(database);
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    {
        database_t empty_database;
        migrate(empty_database, v3, migrations);
        assert(empty_database.create_table<v3_person_t>("people").rows().empty());
    }
    {
        database_t v2_database;
        seed(v2_database);
        migrate(v2_database, v2, migrations);
        verify_v2(v2_database);
        successful_transform(v2_database);
    }
    database_t database;
    seed(database);
    failed_transform(database);
    successful_transform(database);

    const auto directory = std::filesystem::temp_directory_path() / std::format("migration-records-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    {
        database_t persistent_database(directory);
        seed(persistent_database);
        failed_transform(persistent_database);
    }
    {
        database_t reopened_database(directory);
        verify_v1(reopened_database);
        successful_transform(reopened_database);
    }
    {
        database_t reopened_database(directory);
        verify_v3(reopened_database);
    }
    std::filesystem::remove_all(directory);
}
