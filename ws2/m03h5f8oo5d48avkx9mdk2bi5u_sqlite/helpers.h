#ifndef M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_HELPERS_H
# define M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_HELPERS_H

# include "sqlite_external.h"

# include <string_view>

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

void check_result(sqlite3* database, int result, std::string_view operation);
void check_no_nul(std::string_view text, std::string_view operation, std::string_view argument);

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite

#endif // M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_HELPERS_H
