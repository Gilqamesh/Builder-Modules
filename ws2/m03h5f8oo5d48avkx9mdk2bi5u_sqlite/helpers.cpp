#include "helpers.h"

#include <format>
#include <stdexcept>

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

void check_result(sqlite3* database, int result, std::string_view operation) {
    if (result != SQLITE_OK) {
        const auto* message = database == nullptr ? sqlite3_errstr(result) : sqlite3_errmsg(database);
        throw std::runtime_error(std::format("SQLite {} failed (code {}): {}", operation, result, message));
    }
}

void check_no_nul(std::string_view text, std::string_view operation, std::string_view argument) {
    if (text.find('\0') != std::string_view::npos) {
        throw std::invalid_argument(std::format("SQLite {}: {} contains an embedded NUL", operation, argument));
    }
}

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite
