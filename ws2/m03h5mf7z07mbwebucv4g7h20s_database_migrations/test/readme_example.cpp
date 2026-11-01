#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>

#include <array>
#include <cstdint>
#include <string>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

struct v1_person_t { std::int64_t id; std::string name; std::int64_t balance_cents; };
struct v2_person_t { std::int64_t id; std::string name; std::int64_t balance_cents; std::string display_name; };
struct v3_person_t { std::int64_t id; std::string display_name; double balance; };

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

const unique_id_t v1{"03h5mgnvq9zym7wvkijwwaobp"};
const unique_id_t v2{"03h5mgnvq9zym7wvkijwwaobo"};
const unique_id_t v3{"03h5mgnvq9zym7wvkijwwaobn"};
const std::array migrations = {
    migration_t{baseline_version, v1, initialize_people},
    migration_t{v1, v2, add_display_names},
    migration_t{v2, v3, convert_balances}
};
} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    database_t database; // Use a storage-directory path for persistence.

    // Establish v1 before inserting records with the v1 schema.
    migrate(database, v1, migrations);
    const auto v1_people = database.create_table<v1_person_t>("people");
    const v1_person_t v1_person{1, "Ada", 1250};
    v1_people.insert(v1_person);

    // Reach v3 before accessing records with the v3 schema.
    migrate(database, v3, migrations);
    migrate(database, v3, migrations); // No transformations.
    const auto v3_people = database.create_table<v3_person_t>("people");
    const auto v3_rows = v3_people.rows();
    if (v3_rows.size() != 1) {
        return 1;
    }

    const auto& v3_person = v3_rows.front();
    if (v3_person.id != 1) {
        return 1;
    }
    if (v3_person.display_name != "Customer: Ada") {
        return 1;
    }
    if (v3_person.balance != 12.5) {
        return 1;
    }

    return 0;
}
