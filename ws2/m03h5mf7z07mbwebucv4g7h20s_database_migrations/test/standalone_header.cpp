#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>

#include <span>
#include <string_view>
#include <type_traits>

int main() {
    namespace migrations = m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    using callback_t = void (*)(migrations::database_t&);
    using history_t = std::span<const migrations::migration_t>;

    static_assert(std::is_constructible_v<migrations::migration_t, migrations::unique_id_t, migrations::unique_id_t, callback_t>);
    static_assert(!std::is_constructible_v<migrations::migration_t, std::string_view, migrations::unique_id_t, callback_t>);
    static_assert(!std::is_constructible_v<migrations::migration_t, migrations::unique_id_t, std::string_view, callback_t>);
    static_assert(!std::is_constructible_v<migrations::migration_t, const char*, migrations::unique_id_t, callback_t>);
    static_assert(!std::is_constructible_v<migrations::migration_t, migrations::unique_id_t, const char*, callback_t>);
    static_assert(std::is_invocable_v<decltype(&migrations::migrate), migrations::database_t&, const migrations::unique_id_t&, history_t>);
    static_assert(!std::is_invocable_v<decltype(&migrations::migrate), migrations::database_t&, std::string_view, history_t>);
    static_assert(!std::is_invocable_v<decltype(&migrations::migrate), migrations::database_t&, const char*, history_t>);

    migrations::database_t database;
    migrations::migrate(database, migrations::baseline_version, {});
    return database.tables().size() == 1 && migrations::baseline_version.string() == "03h5mgnvq9zym7wvkijwwaobj" ? 0 : 1;
}
