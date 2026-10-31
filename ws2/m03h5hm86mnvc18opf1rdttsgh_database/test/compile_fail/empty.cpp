// expected: database record must contain at least one field
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct record_t {};

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
