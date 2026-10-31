#include "sqlite.h"
#include "helpers.h"
#include "sqlite_external.h"

#include <format>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

class statement_impl_t final : public statement_t {
public:
    explicit statement_impl_t(sqlite3_stmt* statement);
    ~statement_impl_t() override;

    void bind(int index, std::nullptr_t) override;
    void bind(int index, std::int64_t number) override;
    void bind(int index, double number) override;
    void bind(int index, std::string_view text) override;
    void bind(int index, std::span<const std::byte> bytes) override;
    bool step() override;
    void reset() override;
    void clear_bindings() override;
    std::size_t column_count() const override;
    bool column_null(int column) const override;
    std::int64_t column_integer(int column) const override;
    double column_real(int column) const override;
    std::string column_text(int column) const override;
    std::vector<std::byte> column_blob(int column) const override;

private:
    void check_column(int column) const;

    sqlite3_stmt* m_handle;
};

database_t::database_t(std::string_view filename):
    m_handle(nullptr)
{
    check_no_nul(filename, "open database", "filename");
    const std::string terminated_filename(filename);
    sqlite3* connection = nullptr;
    const auto result = sqlite3_open_v2(terminated_filename.c_str(), &connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    std::unique_ptr<sqlite3, decltype(&sqlite3_close_v2)> cleanup(connection, sqlite3_close_v2);
    check_result(connection, result, "open database");
    m_handle = cleanup.release();
}

database_t::~database_t() {
    sqlite3_close_v2(m_handle);
}

std::unique_ptr<statement_t> database_t::prepare(std::string_view sql) {
    check_no_nul(sql, "prepare statement", "SQL");
    if (static_cast<std::size_t>(std::numeric_limits<int>::max()) <= sql.size()) {
        throw std::length_error(std::format("SQLite prepare statement: {} SQL bytes exceed the maximum {}", sql.size(), std::numeric_limits<int>::max() - 1));
    }
    const std::string terminated_sql(sql);
    sqlite3_stmt* statement = nullptr;
    const auto result = sqlite3_prepare_v2(m_handle, terminated_sql.c_str(), static_cast<int>(terminated_sql.size() + 1), &statement, nullptr);
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> cleanup(statement, sqlite3_finalize);
    check_result(m_handle, result, "prepare statement");
    if (statement == nullptr) {
        throw std::invalid_argument("SQLite prepare statement: SQL does not contain a statement");
    }
    auto prepared_statement = std::make_unique<statement_impl_t>(statement);
    cleanup.release();
    return prepared_statement;
}

void database_t::execute(std::string_view sql) {
    check_no_nul(sql, "execute script", "SQL");
    const std::string terminated_sql(sql);
    check_result(m_handle, sqlite3_exec(m_handle, terminated_sql.c_str(), nullptr, nullptr, nullptr), "execute script");
}

statement_t::~statement_t() = default;

statement_t::statement_t() = default;

statement_impl_t::statement_impl_t(sqlite3_stmt* statement):
    m_handle(statement)
{
}

statement_impl_t::~statement_impl_t() {
    sqlite3_finalize(m_handle);
}

void statement_impl_t::bind(int index, std::nullptr_t) {
    check_result(sqlite3_db_handle(m_handle), sqlite3_bind_null(m_handle, index), "bind NULL");
}

void statement_impl_t::bind(int index, std::int64_t number) {
    check_result(sqlite3_db_handle(m_handle), sqlite3_bind_int64(m_handle, index, number), "bind integer");
}

void statement_impl_t::bind(int index, double number) {
    check_result(sqlite3_db_handle(m_handle), sqlite3_bind_double(m_handle, index, number), "bind real");
}

void statement_impl_t::bind(int index, std::string_view text) {
    const auto* data = text.empty() ? "" : text.data();
    check_result(sqlite3_db_handle(m_handle), sqlite3_bind_text64(m_handle, index, data, text.size(), SQLITE_TRANSIENT, SQLITE_UTF8), "bind text");
}

void statement_impl_t::bind(int index, std::span<const std::byte> bytes) {
    const void* data = bytes.empty() ? static_cast<const void*>("") : bytes.data();
    check_result(sqlite3_db_handle(m_handle), sqlite3_bind_blob64(m_handle, index, data, bytes.size(), SQLITE_TRANSIENT), "bind BLOB");
}

bool statement_impl_t::step() {
    const auto result = sqlite3_step(m_handle);
    if (result == SQLITE_ROW) {
        return true;
    }
    if (result != SQLITE_DONE) {
        check_result(sqlite3_db_handle(m_handle), result, "step statement");
    }
    return false;
}

void statement_impl_t::reset() {
    check_result(sqlite3_db_handle(m_handle), sqlite3_reset(m_handle), "reset statement");
}

void statement_impl_t::clear_bindings() {
    check_result(sqlite3_db_handle(m_handle), sqlite3_clear_bindings(m_handle), "clear bindings");
}

std::size_t statement_impl_t::column_count() const {
    return static_cast<std::size_t>(sqlite3_column_count(m_handle));
}

bool statement_impl_t::column_null(int column) const {
    check_column(column);
    return sqlite3_column_type(m_handle, column) == SQLITE_NULL;
}

std::int64_t statement_impl_t::column_integer(int column) const {
    check_column(column);
    return sqlite3_column_int64(m_handle, column);
}

double statement_impl_t::column_real(int column) const {
    check_column(column);
    return sqlite3_column_double(m_handle, column);
}

std::string statement_impl_t::column_text(int column) const {
    check_column(column);
    const auto* text = sqlite3_column_text(m_handle, column);
    if (text == nullptr) {
        const auto result = sqlite3_errcode(sqlite3_db_handle(m_handle));
        if (result == SQLITE_NOMEM) {
            check_result(sqlite3_db_handle(m_handle), result, "read text");
        }
        return {};
    }
    const auto size = static_cast<std::size_t>(sqlite3_column_bytes(m_handle, column));
    return std::string(reinterpret_cast<const char*>(text), size);
}

std::vector<std::byte> statement_impl_t::column_blob(int column) const {
    check_column(column);
    const auto* data = static_cast<const std::byte*>(sqlite3_column_blob(m_handle, column));
    if (data == nullptr) {
        // SQLite returns a null pointer for both NULL and zero-length BLOBs.
        const auto result = sqlite3_errcode(sqlite3_db_handle(m_handle));
        if (result == SQLITE_NOMEM) {
            check_result(sqlite3_db_handle(m_handle), result, "read BLOB");
        }
        return {};
    }
    const auto size = static_cast<std::size_t>(sqlite3_column_bytes(m_handle, column));
    return std::vector<std::byte>(data, data + size);
}

void statement_impl_t::check_column(int column) const {
    const auto count = sqlite3_column_count(m_handle);
    if (column < 0 || count <= column) {
        throw std::out_of_range(std::format("SQLite read column: index {} is outside [0, {})", column, count));
    }
    if (sqlite3_data_count(m_handle) == 0) {
        throw std::logic_error("SQLite read column: statement has no current row");
    }
}

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite
