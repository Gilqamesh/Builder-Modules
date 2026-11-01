#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>

int main() {
    namespace migrations = m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    migrations::database_t database;
    migrations::migrate(database, migrations::baseline_version, {});
    return database.tables().size() == 1 ? 0 : 1;
}
