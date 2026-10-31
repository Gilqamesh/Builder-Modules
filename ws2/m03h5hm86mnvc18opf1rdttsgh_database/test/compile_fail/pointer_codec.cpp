// expected: Unsupported field 'pointer'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <cstdint>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

template<> struct column_codec<int*> { using storage_t=std::int64_t; static storage_t encode(int* const&) { return 0; } static int* decode(storage_t) { return nullptr; } };
struct record_t { int* pointer; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
