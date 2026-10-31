#include "database.h"

#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>
#include <m03h5f8oo5d48avkx9mdk2bi5u_sqlite/sqlite.h>

#include <algorithm>
#include <array>
#include <exception>
#include <new>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

class database_t::state_t {
private:
    class failure_t : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    template <typename F>
    static decltype(auto) call(std::string_view operation, std::string_view table, F&& action);
    static std::string quote(std::string_view identifier);
    static void validate_name(std::string_view name);
    void read_schema();
    bool table_exists(std::string_view table);
    void validate_schema(std::string_view table, std::span<const column_t> columns);
    void operation(std::string_view operation, std::string_view table, bool mutation, std::function_ref<void()> action);
    void rollback() noexcept;

public:
    state_t(std::string_view filename, bool persistent);
    static std::string filename(const std::filesystem::path& filename);
    void transaction(database_t& database, std::function_ref<void(database_t&)> action);
    void ensure_table(std::string_view table, std::span<const column_t> columns);
    void insert(std::string_view table, std::span<const column_t> columns, std::span<const cell_t> stored);
    void rows(std::string_view table, std::span<const column_t> columns, std::function_ref<void(std::span<cell_t>)> consume);
    void drop(std::string_view table, std::span<const column_t> columns);

private:
    m03h5f8oo5d48avkx9mdk2bi5u_sqlite::database_t m_database;
    bool m_transaction = false;
    bool m_operation = false;
    bool m_valid = true;
    bool m_usable = true;
};

template <typename F>
decltype(auto) database_t::state_t::call(std::string_view operation, std::string_view table, F&& action) {
    try {
        return std::invoke(std::forward<F>(action));
    } catch (const failure_t&) {
        throw;
    } catch (const std::bad_alloc&) {
        throw;
    } catch (const std::exception&) {
        // Never append native messages or retain native exceptions as nested causes.
        throw failure_t(std::format("database {} '{}': storage operation failed", operation, table));
    }
}

std::string database_t::state_t::quote(std::string_view identifier) {
    std::string quoted = "\"";
    for (const auto character : identifier) {
        quoted += character;
        if (character == '"') quoted += '"';
    }
    quoted += '"';
    return quoted;
}

void database_t::state_t::validate_name(std::string_view name) {
    if (name.empty() || name.find('\0') != std::string_view::npos) {
        throw std::invalid_argument("database table name must be nonempty and contain no NUL");
    }
}

void database_t::state_t::read_schema() {
    auto statement = m_database.prepare("SELECT name, type, sql FROM main.sqlite_schema");
    while (statement->step()) {
        (void)statement->column_text(0);
        (void)statement->column_text(1);
        if (!statement->column_null(2)) (void)statement->column_text(2);
    }
}

bool database_t::state_t::table_exists(std::string_view table) {
    auto statement = m_database.prepare("SELECT 1 FROM main.sqlite_schema WHERE name = ?1 COLLATE NOCASE");
    statement->bind(1, table);
    return statement->step();
}

void database_t::state_t::validate_schema(std::string_view table, std::span<const column_t> columns) {
    const auto reject = [&](std::string_view reason) {
        throw failure_t(std::format("database table '{}': {}", table, reason));
    };
    // Direct PRAGMAs cannot be shadowed by application tables named pragma_*.
    {
        auto tables = m_database.prepare(std::format("PRAGMA main.table_list({})", quote(table)));
        if (!tables->step()) reject("table does not exist");
        if (tables->column_text(2) != "table" || tables->column_integer(5) != 1 || tables->column_integer(4) != 0) {
            reject("table representation is incompatible");
        }
    }
    constexpr std::array<std::string_view, 5> types = { "NULL", "INTEGER", "REAL", "TEXT", "BLOB" };
    constexpr std::array<std::string_view, 5> kinds = { "absent", "signed 64-bit integer", "double", "string", "byte stream" };
    auto fields = m_database.prepare(std::format("PRAGMA main.table_xinfo({})", quote(table)));
    std::vector<std::string> names;
    while (fields->step()) {
        const auto name = fields->column_text(1);
        for (const auto& previous : names) {
            const auto fold = [](char character) { return 'A' <= character && character <= 'Z' ? character + ('a' - 'A') : character; };
            if (std::ranges::equal(name, previous, {}, fold, fold)) {
                reject(std::format("duplicate or ASCII case-colliding column '{}'", name));
            }
        }
        names.push_back(name);
        const auto expected = std::ranges::find_if(columns, [&](const column_t& column) { return std::get<0>(column) == name; });
        if (expected == columns.end()) reject(std::format("unexpected column '{}'", name));
        const auto& [expected_name, kind, nullable] = *expected;
        const auto actual_type = fields->column_text(2);
        if (actual_type != types[kind] || (fields->column_integer(3) == 0) != nullable) {
            const auto actual_kind = std::ranges::find(types, actual_type);
            const auto actual = actual_kind == types.end() ? "unsupported representation" : kinds[static_cast<std::size_t>(actual_kind - types.begin())];
            reject(std::format("field '{}': expected {}{}; found {}{}", expected_name, kinds[kind], nullable ? " nullable" : " required", actual, fields->column_integer(3) == 0 ? " nullable" : " required"));
        }
        if (!fields->column_null(4) || fields->column_integer(5) != 0 || fields->column_integer(6) != 0) {
            reject(std::format("field '{}': column behavior is incompatible", name));
        }
    }
    fields.reset();
    if (names.size() != columns.size()) reject(std::format("expected {} columns; found {}", columns.size(), names.size()));

    // Reject readily inspectable behavior changes without promising an arbitrary DDL parser.
    constexpr auto unsupported_constraints = "table behavior is incompatible";
    {
        auto indexes = m_database.prepare(std::format("PRAGMA main.index_list({})", quote(table)));
        while (indexes->step()) {
            if (indexes->column_integer(2) != 0) reject(unsupported_constraints);
        }
    }
    {
        auto foreign_keys = m_database.prepare(std::format("PRAGMA main.foreign_key_list({})", quote(table)));
        if (foreign_keys->step()) reject(unsupported_constraints);
    }
    auto triggers = m_database.prepare("SELECT 1 FROM main.sqlite_schema WHERE type = 'trigger' AND tbl_name = ?1 COLLATE NOCASE");
    triggers->bind(1, table);
    if (triggers->step()) reject(unsupported_constraints);
}

void database_t::state_t::operation(std::string_view operation, std::string_view table, bool mutation, std::function_ref<void()> action) {
    if (!m_usable || (m_transaction && !m_valid)) {
        throw failure_t(std::format("database {} '{}': transaction or connection is invalid", operation, table));
    }
    if (m_operation) throw std::logic_error("database operation: codecs must not re-enter the same database");
    if (m_transaction) {
        try {
            call(operation, table, [&] { m_database.execute("SAVEPOINT database_operation"); });
        } catch (...) {
            m_valid = false;
            throw;
        }
    } else {
        call(operation, table, [&] { m_database.execute(mutation ? "BEGIN IMMEDIATE" : "BEGIN"); });
    }
    m_operation = true;
    try {
        action();
        call(operation, table, [&] { m_database.execute(m_transaction ? "RELEASE database_operation" : "COMMIT"); });
        m_operation = false;
    } catch (...) {
        const auto original = std::current_exception();
        if (m_transaction) {
            try {
                m_database.execute("ROLLBACK TO database_operation; RELEASE database_operation");
            } catch (...) {
                // An automatic rollback or failed recovery makes outer commit unsafe.
                m_valid = false;
            }
        } else {
            rollback();
        }
        m_operation = false;
        std::rethrow_exception(original);
    }
}

void database_t::state_t::rollback() noexcept {
    try {
        m_database.execute("ROLLBACK");
    } catch (...) {
        m_usable = false;
    }
}

database_t::state_t::state_t(std::string_view filename, bool persistent):
    m_database(call("open", persistent ? filename : "in-memory database", [&] { return m03h5f8oo5d48avkx9mdk2bi5u_sqlite::database_t(filename); }))
{
    call("read schema", persistent ? filename : "in-memory database", [&] { read_schema(); });
    if (persistent) {
        call("open", filename, [&] {
            // WAL is private: readers retain snapshots while another connection commits.
            auto statement = m_database.prepare("PRAGMA journal_mode=WAL");
            if (!statement->step() || statement->column_text(0) != "wal") {
                throw failure_t(std::format("database open '{}': concurrent snapshots are unavailable", filename));
            }
        });
    }
}

std::string database_t::state_t::filename(const std::filesystem::path& filename) {
    const auto spelling = filename.string();
    if (spelling.empty() || spelling.find('\0') != std::string::npos) {
        throw std::invalid_argument("database open: filename must be nonempty and contain no NUL");
    }
    return call("open", spelling, [&] {
        namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
        const filesystem::path_t path(filename);
        if (!filesystem::is_directory(path.parent())) {
            throw failure_t(std::format("database open '{}': parent directory '{}' does not exist", spelling, path.parent()));
        }
        return path.string();
    });
}

void database_t::state_t::transaction(database_t& database, std::function_ref<void(database_t&)> action) {
    if (m_transaction || m_operation) throw std::logic_error("database transaction: nested transactions are unsupported");
    if (!m_usable) throw failure_t("database transaction: connection is invalid");
    call("transaction", "begin", [&] { m_database.execute("BEGIN"); });
    m_transaction = true;
    m_valid = true;
    try {
        // Anchor the external snapshot before the callback, including an empty schema.
        call("transaction", "snapshot", [&] { read_schema(); });
        action(database);
        if (!m_valid) throw failure_t("database transaction: operation recovery failed; cannot commit");
        call("transaction", "commit", [&] { m_database.execute("COMMIT"); });
        m_transaction = false;
    } catch (...) {
        const auto original = std::current_exception();
        rollback();
        m_transaction = false;
        std::rethrow_exception(original);
    }
}

void database_t::state_t::ensure_table(std::string_view table, std::span<const column_t> columns) {
    validate_name(table);
    operation("table", table, true, [&] {
        call("table", table, [&] {
            constexpr std::array<std::string_view, 5> types = { "NULL", "INTEGER", "REAL", "TEXT", "BLOB" };
            auto sql = std::format("CREATE TABLE IF NOT EXISTS main.{} (", quote(table));
            for (std::size_t index = 0; index < columns.size(); ++index) {
                const auto& [name, kind, nullable] = columns[index];
                if (index != 0) sql += ", ";
                sql += std::format("{} {}{}", quote(name), types[kind], nullable ? "" : " NOT NULL");
            }
            sql += ") STRICT";
            m_database.execute(sql);
            validate_schema(table, columns);
        });
    });
}

void database_t::state_t::insert(std::string_view table, std::span<const column_t> columns, std::span<const cell_t> stored) {
    operation("insert", table, true, [&] {
        call("insert", table, [&] {
            validate_schema(table, columns);
            auto sql = std::format("INSERT INTO main.{} (", quote(table));
            std::string parameters;
            for (std::size_t index = 0; index < columns.size(); ++index) {
                if (index != 0) {
                    sql += ", ";
                    parameters += ", ";
                }
                sql += quote(std::get<0>(columns[index]));
                parameters += '?';
            }
            sql += std::format(") VALUES ({})", parameters);
            auto statement = m_database.prepare(sql);
            for (std::size_t index = 0; index < stored.size(); ++index) {
                const auto parameter = static_cast<int>(index + 1);
                const auto& cell = stored[index];
                switch (cell.index()) {
                    case 0: {
                        statement->bind(parameter, nullptr);
                    } break;
                    case 1: {
                        statement->bind(parameter, std::get<1>(cell));
                    } break;
                    case 2: {
                        statement->bind(parameter, std::get<2>(cell));
                    } break;
                    case 3: {
                        statement->bind(parameter, std::string_view(std::get<3>(cell)));
                    } break;
                    case 4: {
                        statement->bind(parameter, std::get<4>(cell).bytes());
                    } break;
                }
            }
            if (statement->step()) throw failure_t(std::format("database insert '{}': unexpected result", table));
        });
    });
}

void database_t::state_t::rows(std::string_view table, std::span<const column_t> columns, std::function_ref<void(std::span<cell_t>)> consume) {
    validate_name(table);
    operation("rows", table, false, [&] {
        auto statement = call("rows", table, [&] {
            validate_schema(table, columns);
            std::string sql = "SELECT ";
            std::string kinds;
            for (std::size_t index = 0; index < columns.size(); ++index) {
                const auto name = quote(std::get<0>(columns[index]));
                if (index != 0) {
                    sql += ", ";
                    kinds += ", ";
                }
                sql += name;
                kinds += std::format("typeof({})", name);
            }
            sql += std::format(", {} FROM main.{}", kinds, quote(table));
            return m_database.prepare(sql);
        });
        std::vector<cell_t> stored;
        stored.reserve(columns.size());
        while (call("rows", table, [&] {
            if (!statement->step()) return false;
            stored.clear();
            constexpr std::array<std::string_view, 5> types = { "null", "integer", "real", "text", "blob" };
            constexpr std::array<std::string_view, 5> kinds = { "absent", "signed 64-bit integer", "double", "string", "byte stream" };
            for (std::size_t index = 0; index < columns.size(); ++index) {
                const auto& [name, kind, nullable] = columns[index];
                const auto column = static_cast<int>(index);
                const auto actual = statement->column_text(static_cast<int>(columns.size() + index));
                if (statement->column_null(column)) {
                    if (!nullable) throw failure_t(std::format("database rows '{}': field '{}': expected {}; found absent value", table, name, kinds[kind]));
                    stored.emplace_back(std::monostate{});
                    continue;
                }
                if (actual != types[kind]) {
                    const auto actual_kind = std::ranges::find(types, actual);
                    const auto found = actual_kind == types.end() ? "unsupported representation" : kinds[static_cast<std::size_t>(actual_kind - types.begin())];
                    throw failure_t(std::format("database rows '{}': field '{}': expected {}; found {}", table, name, kinds[kind], found));
                }
                switch (kind) {
                    case 1: {
                        stored.emplace_back(statement->column_integer(column));
                    } break;
                    case 2: {
                        stored.emplace_back(statement->column_real(column));
                    } break;
                    case 3: {
                        stored.emplace_back(statement->column_text(column));
                    } break;
                    case 4: {
                        stored.emplace_back(std::in_place_type<m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>, statement->column_blob(column));
                    } break;
                }
            }
            return true;
        })) {
            // Keep user codecs outside backend exception translation, inside the snapshot.
            consume(stored);
        }
    });
}

void database_t::state_t::drop(std::string_view table, std::span<const column_t> columns) {
    validate_name(table);
    operation("drop", table, true, [&] {
        call("drop", table, [&] {
            if (!table_exists(table)) return;
            validate_schema(table, columns);
            m_database.execute(std::format("DROP TABLE main.{}", quote(table)));
        });
    });
}

database_t::database_t():
    m_state(std::make_unique<state_t>(":memory:", false))
{
}

database_t::database_t(const std::filesystem::path& filename):
    m_state(std::make_unique<state_t>(state_t::filename(filename), true))
{
}

database_t::~database_t() noexcept = default;

void database_t::transaction(std::function_ref<void(database_t&)> action) {
    m_state->transaction(*this, action);
}

void database_t::ensure_table(std::string_view table, std::span<const column_t> columns) {
    m_state->ensure_table(table, columns);
}

void database_t::insert_impl(std::string_view table, std::span<const column_t> columns, std::span<const cell_t> stored) {
    m_state->insert(table, columns, stored);
}

void database_t::rows_impl(std::string_view table, std::span<const column_t> columns, std::function_ref<void(std::span<cell_t>)> consume) const {
    m_state->rows(table, columns, consume);
}

void database_t::drop_impl(std::string_view table, std::span<const column_t> columns) {
    m_state->drop(table, columns);
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database
