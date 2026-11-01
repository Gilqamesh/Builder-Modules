#ifndef M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_SQLITE_H
# define M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_SQLITE_H

# include <concepts>
# include <cstddef>
# include <cstdint>
# include <format>
# include <limits>
# include <memory>
# include <span>
# include <stdexcept>
# include <string>
# include <string_view>
# include <utility>
# include <vector>

struct sqlite3;

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

class statement_t;

/**
 * @brief Owns a SQLite connection.
 *
 * The connection must outlive its statements. Serialize access to the
 * connection and its statements, including destruction; the wrapper adds no locks.
 * Operations throw standard exceptions. SQLite failures report the operation,
 * result code, and diagnostic message. Destruction never throws.
 *
 * @code
 * database_t database(":memory:");
 * database.execute("CREATE TABLE notes (title TEXT NOT NULL)");
 * auto insert = database.prepare("INSERT INTO notes VALUES (?1)");
 * insert->bind(1, "Example");
 * insert->step();
 * @endcode
 */
class database_t {
public:
    /**
     * @brief Opens or creates a database for reading and writing.
     *
     * Use :memory: for an in-memory database or an empty filename for a
     * temporary database. Embedded NULs are rejected.
     */
    explicit database_t(std::string_view filename);
    ~database_t();

    database_t(const database_t&) = delete;
    database_t& operator=(const database_t&) = delete;
    database_t(database_t&&) = delete;
    database_t& operator=(database_t&&) = delete;

    /**
     * @brief Compiles the first SQL statement and returns its unique owner.
     *
     * Never returns null. Rejects empty or comment-only SQL and embedded NULs;
     * trailing SQL is ignored.
     */
    std::unique_ptr<statement_t> prepare(std::string_view sql);

    /**
     * @brief Executes a SQL script and discards result rows.
     *
     * Rejects embedded NULs. Stops on the first error; earlier effects remain
     * unless an explicit SQL transaction controls those effects.
     */
    void execute(std::string_view sql);

private:
    sqlite3* m_handle;
};

/**
 * @brief Owns a prepared SQLite statement.
 *
 * The statement cannot be copied or moved; its std::unique_ptr owner can be moved.
 * Parameter indices start at one; column indices start at zero. Column getters
 * use SQLite conversions. Reading a column without a current row throws
 * std::logic_error; an invalid column index throws std::out_of_range.
 * Destruction never throws.
 */
class statement_t {
public:
    virtual ~statement_t();

    statement_t(const statement_t&) = delete;
    statement_t& operator=(const statement_t&) = delete;
    statement_t(statement_t&&) = delete;
    statement_t& operator=(statement_t&&) = delete;

    /** @brief Binds SQL NULL. */
    virtual void bind(int index, std::nullptr_t) = 0;
    /** @brief Binds a signed 64-bit integer. */
    virtual void bind(int index, std::int64_t number) = 0;
    /** @brief Binds an integer, rejecting numbers outside the signed 64-bit range. */
    template <std::integral T>
    void bind(int index, T number);
    /** @brief Binds a real number using SQLite's floating-point rules. */
    virtual void bind(int index, double number) = 0;
    /** @brief Copies UTF-8 text, preserving its byte length; empty text remains distinct from NULL. */
    virtual void bind(int index, std::string_view text) = 0;
    /** @brief Copies bytes; an empty BLOB remains distinct from NULL. */
    virtual void bind(int index, std::span<const std::byte> bytes) = 0;

    /**
     * @brief Advances execution, returning true for a row or false for completion, and throwing on failure.
     *
     * Calling step() after completion restarts execution. Call reset() before
     * rebinding parameters or deliberately reusing the statement.
     */
    virtual bool step() = 0;
    /**
     * @brief Rewinds execution and keeps bindings.
     *
     * May throw an error from the preceding execution, even if step() returned
     * a row. The statement is reset before the error is thrown.
     */
    virtual void reset() = 0;
    /** @brief Replaces every parameter binding with SQL NULL. */
    virtual void clear_bindings() = 0;

    /** @brief Returns the number of result columns, including before execution. */
    virtual std::size_t column_count() const = 0;
    /**
     * @brief Returns whether the column contains SQL NULL.
     *
     * Check before converting getters when the original storage type matters.
     * Other getters return zero or empty values for SQL NULL.
     */
    virtual bool column_null(int column) const = 0;
    /** @brief Returns the column as a signed 64-bit integer. */
    virtual std::int64_t column_integer(int column) const = 0;
    /** @brief Returns the column as a real number. */
    virtual double column_real(int column) const = 0;
    /** @brief Returns the column as an owning UTF-8 string. */
    virtual std::string column_text(int column) const = 0;
    /** @brief Returns the column as owning bytes. */
    virtual std::vector<std::byte> column_blob(int column) const = 0;

protected:
    statement_t();
};

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite

namespace std {

template <>
struct formatter<m03h5f8oo5d48avkx9mdk2bi5u_sqlite::database_t>;
template <>
struct formatter<m03h5f8oo5d48avkx9mdk2bi5u_sqlite::statement_t>;

} // namespace std

namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite {

template <std::integral T>
void statement_t::bind(int index, T number) {
    if constexpr (std::numeric_limits<std::int64_t>::digits < std::numeric_limits<T>::digits) {
        if (!std::in_range<std::int64_t>(number)) {
            throw std::out_of_range(std::format("SQLite bind integer: {} is outside the signed 64-bit range", static_cast<std::uintmax_t>(number)));
        }
    }
    bind(index, static_cast<std::int64_t>(number));
}

} // namespace m03h5f8oo5d48avkx9mdk2bi5u_sqlite

namespace std {

template <>
struct formatter<m03h5f8oo5d48avkx9mdk2bi5u_sqlite::database_t> {
    constexpr auto parse(format_parse_context& ctx) {
        auto position = ctx.begin();
        if (position != ctx.end() && *position != '}') {
            throw format_error("invalid SQLite database format specifier");
        }
        return position;
    }

    auto format(const m03h5f8oo5d48avkx9mdk2bi5u_sqlite::database_t&, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "database");
        return out;
    }
};

template <>
struct formatter<m03h5f8oo5d48avkx9mdk2bi5u_sqlite::statement_t> {
    constexpr auto parse(format_parse_context& ctx) {
        auto position = ctx.begin();
        if (position != ctx.end() && *position != '}') {
            throw format_error("invalid SQLite statement format specifier");
        }
        return position;
    }

    auto format(const m03h5f8oo5d48avkx9mdk2bi5u_sqlite::statement_t&, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "statement");
        return out;
    }
};

} // namespace std

#endif // M03H5F8OO5D48AVKX9MDK2BI5U_SQLITE_SQLITE_H
