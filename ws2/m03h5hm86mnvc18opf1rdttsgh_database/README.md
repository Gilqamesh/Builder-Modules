# Typed record database

Store ordinary aggregate records in typed tables. The default constructor creates
an independent in-memory database; use `database_t("people.db")` for persistence.

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

Persistent storage is opaque and may include backend-managed auxiliary files.
Close all connections before copying or moving the complete storage. Live
filesystem copying and externally authored storage layouts are outside the contract.

## Validation

From the Builder-Layout root, with the foundation libraries built:

```sh
python3 ws2/m03h5hm86mnvc18opf1rdttsgh_database/test/check.py
```
