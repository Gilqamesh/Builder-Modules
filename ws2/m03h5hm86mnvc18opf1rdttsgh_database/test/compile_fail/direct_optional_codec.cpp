// expected: Unsupported field 'custom_optional'
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <optional>
#include <string>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

template<> struct column_codec<std::optional<std::string>> { using storage_t=std::string; static storage_t encode(const std::optional<std::string>& text) { return text ? *text : ""; } static std::optional<std::string> decode(storage_t text) { return text; } };
struct record_t { std::optional<std::string> custom_optional; };

void instantiate(database_t& database) { database.table<record_t>("probe"); }

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
