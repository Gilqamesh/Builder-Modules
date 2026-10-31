// Private native-call interposition; linked only into the backend integration executable.
#include <cstdlib>
#include <dlfcn.h>
#include <sqlite3.h>
#include <string_view>

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
