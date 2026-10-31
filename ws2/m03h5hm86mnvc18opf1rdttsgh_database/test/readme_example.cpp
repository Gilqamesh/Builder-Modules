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
    database::database_t people_database;
    auto people_table = people_database.table<person_t>("people");
    people_database.transaction([&](database::database_t& same_database) {
        people_table.insert(person_t{1, "Ada", 36});
        same_database.table<person_t>("temporary").drop();
    });
    auto people = people_table.rows();
    return people.size() == 1 && people.front().name == "Ada" ? 0 : 1;
}
