// expected: Unsupported field 'encoding'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <string>
#include <string_view>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

struct domain_t { std::string text; };
template<> struct column_codec<domain_t> { using storage_t=std::string; static std::string_view encode(const domain_t& domain) { return domain.text; } static domain_t decode(storage_t text) { return {text}; } };
struct record_t { domain_t encoding; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
