// expected: Unsupported field 'nested_optional'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <optional>
#include <string>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct record_t { std::optional<std::optional<std::string>> nested_optional; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
