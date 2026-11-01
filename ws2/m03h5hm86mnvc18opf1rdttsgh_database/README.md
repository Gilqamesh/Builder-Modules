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
    database::database_t db;
    auto people = db.create_table<person_t>("people");

    people.insert(person_t{1, "Ada", 36});

    auto names = db.tables();
    auto matches = people.rows<&person_t::id>(1);
    auto all_people = people.rows();

    const auto replaced = people.replace<&person_t::id>(1, person_t{1, "Ada", 37});
    const auto erased = people.erase<&person_t::id>(1);

    db.drop_table<person_t>("people");
    return names.size() == 1 && names.front() == "people" && matches.size() == 1 &&
           matches.front().name == "Ada" && all_people.size() == 1 &&
           replaced == 1 && erased == 1 && db.tables().empty() ? 0 : 1;
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

Use `db.create_table<T>(name)` to create a missing table or reuse a compatible
existing table. Incompatible schemas are rejected without changes. Every record
operation revalidates compatibility. `db.drop_table<T>(name)` atomically validates
and deletes a compatible table; a missing table is a successful no-op and is never
created along the way. Handles retain their database and logical name. After
deletion, record operations fail until a compatible table is recreated.

`rows()` returns detached, owning C++ records, preserving duplicates without an
ordering guarantee. Editing those values does not persist changes: use `replace()`
explicitly, and use a transaction for read–modify–write sequences.

`rows<Member>(match)`, `replace<Member>(match, replacement)`, and `erase<Member>(match)`
select every matching record, including duplicates. The selector must be a
supported accessible direct data member of the record; the match parameter has
that field's type and uses its codec. Equality uses canonical encoded values:
strings and binary data compare exactly (including embedded NULs), numbers compare
numerically, and absent optionals match absence. Custom encoded equality can differ
from C++ equality. Replacement writes the complete record and may change the
selected field. Mutations return the number matched or deleted; identical
replacements count as matches, and zero matches succeed.

`db.tables()` returns logical names for all application tables, regardless of record
type, sorted using ordinary `std::string` ordering. Names preserve their creation
spelling and own their data independently of the database. An empty database
returns an empty vector. Private storage names and backend objects are excluded.
Enumeration uses one read snapshot, including the enclosing transaction's own
creations and deletions. Outside a transaction, table and record mutations commit
before returning; inside one they participate without independently committing.

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
without NUL are supported independently of private backend naming rules. Acquiring
`"PEOPLE"` after creating `"people"` reuses it without changing its spelling.

## Validation

From the Builder-Layout root:

```sh
./cli m03h5hm86mnvc18opf1rdttsgh_database:validate
```

The library phase runs six runtime validations before publishing its artifact.
Completed artifacts are reused normally.
