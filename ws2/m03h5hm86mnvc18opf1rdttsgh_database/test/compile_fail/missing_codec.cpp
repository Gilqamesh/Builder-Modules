// expected: Unsupported field 'history'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct record_t { std::vector<int> history; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
