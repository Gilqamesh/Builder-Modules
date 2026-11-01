#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <cstdint>
#include <optional>
#include <string>

struct person_t {
    std::int64_t id;
    std::string name;
    std::optional<int> age;
};

int main() {
    namespace database = m03h5hm86mnvc18opf1rdttsgh_database;
    database::database_t db;
    auto people = db.create_table<person_t>("people");

    people.insert(person_t{1, "Ada", 36});

    auto names = db.tables();
    auto matches = people.rows<&person_t::id>(1);
    auto all_people = people.rows();

    const auto replaced = people.replace<&person_t::id>(1, person_t{1, "Ada", 37});
    const auto erased = people.erase<&person_t::id>(1);

    db.drop_table<person_t>("people");
    return names.size() == 1 && names.front() == "people" && matches.size() == 1 &&
           matches.front().name == "Ada" && all_people.size() == 1 &&
           replaced == 1 && erased == 1 && db.tables().empty() ? 0 : 1;
}
