// expected: Unsupported field 'decoded'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <cstdint>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct domain_t { int number; };
template<> struct column_codec<domain_t> { using storage_t=std::int64_t; static storage_t encode(const domain_t& domain) { return domain.number; } static int decode(storage_t number) { return static_cast<int>(number); } };
struct record_t { domain_t decoded; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
