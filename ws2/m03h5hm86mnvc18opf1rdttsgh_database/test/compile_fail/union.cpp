// expected: database record must be a nonunion aggregate
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

union record_t { int number; double real; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
