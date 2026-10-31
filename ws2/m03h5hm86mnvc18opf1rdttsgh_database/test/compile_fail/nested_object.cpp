// expected: Unsupported field 'nested'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct nested_t { int number; }; struct record_t { nested_t nested; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
