// expected: Unsupported field 'storage'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct domain_t { int number; };
template<> struct column_codec<domain_t> { using storage_t=int; static storage_t encode(const domain_t& domain) { return domain.number; } static domain_t decode(storage_t number) { return {number}; } };
struct record_t { domain_t storage; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
