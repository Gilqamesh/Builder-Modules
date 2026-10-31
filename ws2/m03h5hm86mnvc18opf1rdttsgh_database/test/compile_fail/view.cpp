// expected: Unsupported field 'view'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <string_view>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct record_t { std::string_view view; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
