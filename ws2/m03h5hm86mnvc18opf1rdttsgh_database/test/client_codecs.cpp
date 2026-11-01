#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <chrono>
#include <concepts>
#include <cstdint>
#include <optional>
#include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

template <>
struct column_codec<std::chrono::seconds> {
    using storage_t = std::int64_t;
    static storage_t encode(const std::chrono::seconds& elapsed) { return elapsed.count(); }
    static std::chrono::seconds decode(storage_t seconds) { return std::chrono::seconds(seconds); }
};

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

struct timing_t {
    const std::chrono::seconds elapsed;
    std::optional<std::chrono::seconds> pause;
};

int main() {
    namespace database = m03h5hm86mnvc18opf1rdttsgh_database;
    database::database_t timing_database;
    auto timings_table = timing_database.create_table<timing_t>("timings");
    static_assert(std::same_as<decltype(&decltype(timings_table)::rows<&timing_t::elapsed>), std::vector<timing_t> (decltype(timings_table)::*)(const std::chrono::seconds&) const>);
    timings_table.insert(timing_t{std::chrono::seconds{7}, std::chrono::seconds{2}});
    const auto timings = timings_table.rows();
    const auto matching = timings_table.rows<&timing_t::elapsed>(std::chrono::seconds{7});
    const auto replaced = timings_table.replace<&timing_t::pause>(std::chrono::seconds{2}, timing_t{std::chrono::seconds{8}, {}});
    const auto erased = timings_table.erase<&timing_t::pause>(std::nullopt);
    return timings.size() == 1 && timings.front().elapsed.count() == 7 && timings.front().pause->count() == 2 && matching.size() == 1 && replaced == 1 && erased == 1 ? 0 : 1;
}
