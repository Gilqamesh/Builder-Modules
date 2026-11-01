# Typed record database

Store ordinary aggregate records in typed tables. The default constructor creates
an independent in-memory database; use `database_t("people")` for persistence in
the module-owned directory `people`.

```cpp
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <cstdint>
#include <optional>
#include <string>

struct person_t {
    std::int64_t id;
    std::string name;
    std::optional<int> age;
};

int main() {
    namespace database = m03h5hm86mnvc18opf1rdttsgh_database;
    database::database_t people_database;
    auto people_table = people_database.table<person_t>("people");
    people_database.transaction([&](database::database_t& same_database) {
        people_table.insert(person_t{1, "Ada", 36});
        same_database.table<person_t>("temporary").drop();
    });
    auto people = people_table.rows();
    return people.size() == 1 && people.front().name == "Ada" ? 0 : 1;
}
```

See [database.h](database.h) for the public contract, supported field types, and
custom column codecs.

Table handles borrow their database, which must outlive them and their operations.
Transactions synchronously invoke the callback once with that same database;
previously acquired handles participate. Reads share one external snapshot and
observe the transaction's own changes. Changes commit together when the callback
returns normally and the transaction remains valid. An escaping exception rolls
back; a caught operation error allows continuation only after successful recovery.

Use `table.drop()` to remove a compatible table. Missing tables are a no-op;
a stale handle rejects an incompatible replacement without deleting it.

Persistent storage occupies one opaque directory per database. Its parent must
exist. Missing directories and existing empty directories are initialized;
nonempty directories must contain recognized module storage, without extra entries
or linked private files. Paths are literal, including names such as `:memory:`.
Close all connections before copying or moving the entire storage directory. Live
filesystem copying, external changes, and externally authored storage layouts are
outside the contract.

This storage format is versioned. Legacy single-file databases, unsupported
versions, and incomplete initialization are rejected without conversion or repair.
A failed first initialization may leave incomplete storage; remove it before
retrying. No legacy conversion tool is provided.

Table names are logical names with ASCII case-insensitive lookup. Nonempty names
without NUL are supported independently of private backend naming rules.

## Validation

From the Builder-Layout root:

```sh
./cli m03h5hm86mnvc18opf1rdttsgh_database:validate
```

The library phase runs six runtime validations before publishing its artifact.
Completed artifacts are reused normally.
