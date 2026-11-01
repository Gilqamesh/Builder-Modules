// Private native-call interposition; linked only into the backend integration executable.
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <functional>
#include <sqlite3.h>
#include <string_view>
#include <utility>

namespace m03h5hm86mnvc18opf1rdttsgh_database {
std::function<void()> enumeration_hook;
} // namespace m03h5hm86mnvc18opf1rdttsgh_database

extern "C" int sqlite3_step(sqlite3_stmt* statement) {
    static const auto step = reinterpret_cast<decltype(&sqlite3_step)>(dlsym(RTLD_NEXT, "sqlite3_step"));
    if (!step) std::abort();
    const auto result = step(statement);
    const auto* sql = sqlite3_sql(statement);
    const std::string_view command = sql ? sql : "";
    if (result == SQLITE_ROW && command == "SELECT name FROM main.sqlite_schema WHERE type = 'table'") {
        auto hook = std::exchange(m03h5hm86mnvc18opf1rdttsgh_database::enumeration_hook, {});
        if (hook) hook();
    }
    const auto* selected = std::getenv("BUILDER_DATABASE_TEST_FAULT");
    const std::string_view fault = selected ? selected : "";
    if (result == SQLITE_ROW && (fault == "mutation" || fault == "mutation_recovery") &&
        (command.starts_with("UPDATE main.") || command.starts_with("DELETE FROM main."))) {
        const bool recovery = fault == "mutation_recovery";
        unsetenv("BUILDER_DATABASE_TEST_FAULT");
        if (recovery) setenv("BUILDER_DATABASE_TEST_FAULT", "recovery", 1);
        return SQLITE_IOERR;
    }
    return result;
}

extern "C" int rename(const char* from, const char* to) noexcept {
    static const auto rename_file = reinterpret_cast<decltype(&rename)>(dlsym(RTLD_NEXT, "rename"));
    if (!rename_file) std::abort();
    const auto* selected = std::getenv("BUILDER_DATABASE_TEST_FAULT");
    if (selected && std::string_view(selected) == "publish" && std::string_view(to).ends_with("/storage.meta")) {
        unsetenv("BUILDER_DATABASE_TEST_FAULT");
        errno = EIO;
        return -1;
    }
    return rename_file(from, to);
}

extern "C" int sqlite3_open_v2(const char* filename, sqlite3** database, int flags, const char* vfs) {
    static const auto open = reinterpret_cast<decltype(&sqlite3_open_v2)>(dlsym(RTLD_NEXT, "sqlite3_open_v2"));
    if (!open) std::abort();
    const auto* selected = std::getenv("BUILDER_DATABASE_TEST_FAULT");
    if (selected && std::string_view(selected) == "open") {
        unsetenv("BUILDER_DATABASE_TEST_FAULT");
        *database = nullptr;
        return SQLITE_IOERR;
    }
    return open(filename, database, flags, vfs);
}

extern "C" int sqlite3_prepare_v2(sqlite3* database, const char* sql, int bytes, sqlite3_stmt** statement, const char** tail) {
    static const auto prepare = reinterpret_cast<decltype(&sqlite3_prepare_v2)>(dlsym(RTLD_NEXT, "sqlite3_prepare_v2"));
    if (!prepare) std::abort();
    const auto* selected = std::getenv("BUILDER_DATABASE_TEST_FAULT");
    if (selected && std::string_view(selected) == "wal" && std::string_view(sql) == "PRAGMA journal_mode=WAL") {
        unsetenv("BUILDER_DATABASE_TEST_FAULT");
        *statement = nullptr;
        return SQLITE_IOERR;
    }
    return prepare(database, sql, bytes, statement, tail);
}

extern "C" int sqlite3_exec(sqlite3* database, const char* sql, int (*callback)(void*, int, char**, char**), void* context, char** error) {
    static const auto execute = reinterpret_cast<decltype(&sqlite3_exec)>(dlsym(RTLD_NEXT, "sqlite3_exec"));
    if (!execute) std::abort();
    const auto* selected = std::getenv("BUILDER_DATABASE_TEST_FAULT");
    const std::string_view fault = selected ? selected : "";
    const std::string_view command = sql ? sql : "";
    const bool commit = fault == "commit" && command == "COMMIT";
    const bool recovery = fault == "recovery" && command.starts_with("ROLLBACK TO ");
    const bool rollback = fault == "rollback" && command == "ROLLBACK";
    const bool automatic = fault == "automatic" && command == "RELEASE database_operation";
    if (commit || recovery || rollback || automatic) {
        unsetenv("BUILDER_DATABASE_TEST_FAULT");
        if (automatic && execute(database, "ROLLBACK", nullptr, nullptr, nullptr) != SQLITE_OK) std::abort();
        return SQLITE_IOERR;
    }
    return execute(database, sql, callback, context, error);
}
