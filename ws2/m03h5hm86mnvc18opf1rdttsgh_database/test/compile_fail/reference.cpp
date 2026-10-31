// expected: Unsupported field 'reference'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <string>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct record_t { std::string& reference; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
