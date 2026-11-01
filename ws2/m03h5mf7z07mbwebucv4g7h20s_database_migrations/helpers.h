#ifndef M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_HELPERS_H
# define M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_HELPERS_H

# include "api.h"

# include <span>
# include <string_view>
# include <vector>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

bool valid_version(std::string_view version);
bool metadata_name(std::string_view name);
std::vector<const migration_t*> ordered_migrations(std::span<const migration_t> migrations);

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

#endif // M03H5MF7Z07MBWEBUCV4G7H20S_DATABASE_MIGRATIONS_HELPERS_H
