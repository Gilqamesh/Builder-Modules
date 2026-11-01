#ifndef M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_API_H
# define M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_API_H

# include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>
# include <m03h5n4ecgpxtl2skl0hn35s0n_unique_id/unique_id.h>

# include <format>
# include <span>
# include <string_view>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

using m03h5hm86mnvc18opf1rdttsgh_database::database_t;
using m03h5n4ecgpxtl2skl0hn35s0n_unique_id::unique_id_t;

/** @brief Universal, immutable starting version for application data. */
inline const unique_id_t baseline_version{"03h5mgnvq9zym7wvkijwwaobj"};

/** @brief Reserved logical table holding the application-data version, visible in database_t::tables(). */
inline constexpr std::string_view metadata_table_name = "__builder_application_migrations";

/**
 * @brief Owns the version identities for one supplied forward transition.
 *
 * apply must be nonnull, use the supplied database's public API, and leave
 * migration metadata untouched. Only changes through that database are rolled back.
 */
struct migration_t {
    unique_id_t from;
    unique_id_t to;
    void (*apply)(database_t&);
};

/**
 * @brief Validates one complete baseline-rooted chain and atomically upgrades to target.
 *
 * Call before accessing versioned tables, outside any transaction. The target and
 * migration span are borrowed and must remain valid and unchanged until return.
 * Identities own their UUID bytes and are validated when constructed. Remaining callbacks
 * and the final version commit in one transaction or roll back together, without
 * implicit retries. An existing target runs no callbacks.
 *
 * Invalid history structure, null callbacks, or an unreachable target throw
 * invalid_argument before opening a transaction.
 * Unsupported persisted state or callback metadata changes throw runtime_error.
 * Database, callback, and allocation errors propagate.
 */
void migrate(database_t& database, const unique_id_t& target, std::span<const migration_t> migrations);

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

namespace std {

template <>
struct formatter<m03h5mf7z07mbwebucv4g7h20s_database_migrations::migration_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03h5mf7z07mbwebucv4g7h20s_database_migrations::migration_t> {
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw format_error("migration requires an empty format specification");
        }
        return it;
    }

    auto format(const m03h5mf7z07mbwebucv4g7h20s_database_migrations::migration_t& migration, auto& ctx) const {
        auto out = ctx.out();
        out = format_to(out, "{}", migration.from);
        out = format_to(out, " -> {}", migration.to);
        return out;
    }
};

} // namespace std

#endif // M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_API_H
