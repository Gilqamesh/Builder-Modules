# Typed record database

Store owning aggregate records in typed tables with atomic transactions and optional persistence.

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
    return 0;
}
```

## Model

Table names are logical, nonempty strings without NUL. Lookup is ASCII case-insensitive
and independent of private storage naming rules: acquiring `"PEOPLE"` after creating
`"people"` reuses the table without changing its spelling.

A schema consists of exact field names, canonical storage kinds, and optionality.
Compatibility requires all three to match; declaration order and C++ field types
may differ. Each record operation revalidates compatibility. Incompatibility is
rejected without changing existing data.

| Table operation | Behavior |
| --- | --- |
| `create_table<T>(name)` | Atomically creates a missing table or validates and reuses a compatible one. |
| `drop_table<T>(name)` | Atomically validates and deletes a compatible table. A missing table is a successful no-op and is never created along the way. |
| `tables()` | Returns all application table names in ordinary `std::string` order, preserving creation spelling and excluding private storage objects. The owning names survive database destruction; an empty database returns an empty vector. |

`table_t<T>` borrows its database, which must outlive the handle, its copies, and
all operations. Handles own their logical names and can be copied or moved; use a
moved-from handle only for reassignment or destruction. A const handle still permits
record changes. Dropping a table leaves handles associated with their database and
name; record operations fail until a compatible table is recreated.

Records returned by `rows()` are detached, owning C++ values that survive handle and
database destruction. Editing them does not persist changes: use `replace()`
explicitly and a transaction for atomic read–modify–write sequences.

| Record operation | Behavior |
| --- | --- |
| `insert(record)` | Inserts the record's current field values. |
| `rows()` | Returns unordered records with duplicates preserved. Failures return no partial result. |
| `rows<Member>(match)` | Returns every matching record under the same contract as `rows()`. |
| `replace<Member>(match, replacement)` | Completely replaces every record matching the original match, including duplicates; the selected field may change. Returns the matched count, including unchanged records. |
| `erase<Member>(match)` | Deletes every match, including duplicates, and returns the deleted count. |

`Member` must select a supported, accessible, direct nonstatic data member of `T`;
invalid selectors fail compilation. The match parameter has that field's type and
uses its codec. Equality compares canonical encoded values: strings and bytes
compare exactly, including embedded NULs; numbers compare numerically; absent
optionals match absence. Encoded equality can differ from C++ equality. Zero-match
mutations succeed.

## Transactions and errors

Outside `transaction()`, each table or record mutation is atomic and commits before
returning. Each read uses one consistent snapshot covering schema validation,
enumeration or row retrieval, and decoding.

`transaction(action)` invokes `action(*this)` synchronously once after setup; it
never retains or retries the callback. All handles borrowing the database participate,
including previously acquired handles. Reads share one external snapshot established
before the callback and observe the transaction's own changes, including table
creations and deletions. Operations do not independently commit. Other connections
may commit while this transaction reads; conflicting writes fail without automatic
retries.

Normal callback return commits only if the transaction remains valid. An escaping
exception rolls back and propagates unchanged. On an operation error, the database
attempts to undo that operation's effects. Catching the error permits continuation
only if those effects were undone and the transaction remains valid;
otherwise `transaction()` fails even if the callback returns normally. Commit
failures propagate. Cleanup failures never mask the original exception and may
leave the connection unusable. Database destruction never throws.

Serialize access to a database and all its handles, including destruction. Nested
transactions on the same database are rejected before invoking the nested callback.
Codecs must not re-enter the same database; decoding runs inside the read snapshot.

| Failure | Exception |
| --- | --- |
| Invalid runtime arguments, including empty names or paths, embedded NULs, and encoded NaN | `std::invalid_argument` |
| Built-in numeric range failure | `std::out_of_range` |
| Nested transaction or codec re-entry on the same database | `std::logic_error` |
| Schema incompatibility, invalid database or transaction state, conflicting writes, or storage failure | Contextual `std::runtime_error` |
| Codec or allocation failure | Original exception propagates unchanged. |

## Fields and codecs

`T` must be a nonempty, nonunion aggregate with named, accessible fields, no bases
or bitfields, and no reference or raw-pointer fields. Each field needs a valid
codec; duplicate and ASCII case-colliding member names are rejected. Unsupported
fields fail compilation with their names. `T` must support vector construction
from decoded records; default construction and assignment are unnecessary, and
move-only owning fields are supported.

Here `byte_stream_t` means
`m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t`.

| Field type | Canonical storage | Rules |
| --- | --- | --- |
| Standard integral types, including `bool` | `std::int64_t` | Encoding and decoding check destination ranges; `bool` decodes only 0 or 1. |
| `float`, `double` | `double` | Rounding and underflow are allowed, finite overflow is rejected, and infinities are preserved. |
| `std::string` | `std::string` | Owns its data; empty data differs from absence. |
| `byte_stream_t` | `byte_stream_t` | Owns its data; empty data differs from absence. |
| `std::optional<U>` | `U`'s canonical storage or absence | Always uses `U`'s codec. Direct codecs for optional types and nested optionals are unsupported; use a domain wrapper instead. |

Nested objects and containers require explicit codecs. Views, enums, and
`long double` have no built-in codec. Storage kinds are not converted. Canonical
`double` values must not be NaN when encoding, including through custom codecs.

Specialize `column_codec<T>` in namespace
`m03h5hm86mnvc18opf1rdttsgh_database` before instantiating record operations.
For an owning field type `T`, provide this exact interface, with `S` denoting
`column_codec<T>::storage_t`:

| Interface | Requirement |
| --- | --- |
| `using storage_t = S;` | `S` is exactly `std::int64_t`, `double`, `std::string`, or `byte_stream_t`. |
| `column_codec<T>::encode(field)` with `field` a `const T&` | Static function returning exactly `S` or `const S&`. Borrowed storage must remain valid until the operation consumes it. |
| `column_codec<T>::decode(std::move(stored))` with `stored` an owned `S` | Static function returning exactly `T` by value. The result must own its data independently of the argument and database. |

Codec encoding compatibility belongs to the application. No codec identifier or
version is persisted; changing an encoding can require migration even when the
schema remains compatible.

## Persistence

The default constructor creates an independent in-memory database. Pass a storage
directory, for example `database_t("people")`, for persistence. The module owns the
entire opaque directory. Paths must be nonempty and contain no NUL; relative paths
become normalized absolute paths. All paths are literal, including `:memory:`.
The parent directory must already exist.

Missing directories and existing empty directories are initialized. Nonempty
directories must contain recognized, supported-version module storage without extra
entries or linked private files. Opening reads the schema but does not perform an
integrity scan; damaged data may be detected by later operations.

Close all connections before copying or moving the entire storage directory. Live
filesystem copying, external changes, and externally authored storage layouts are
unsupported.

Legacy single-file databases, unsupported versions, and incomplete initialization
are rejected without conversion or repair. Failed initialization may leave
incomplete storage that must be removed before retrying. No legacy conversion tool
is provided. Field renaming and changes to codec encodings require application-managed
migration; there is no automatic migration.

## Validation

From the Builder-Layout root:

```sh
./cli m03h5hm86mnvc18opf1rdttsgh_database:validate
```

The library phase runs six runtime validations, including
[`test/readme_example.cpp`](test/readme_example.cpp), before publishing its artifact.
The example test retains the workflow's verification. Completed artifacts are reused
normally.
