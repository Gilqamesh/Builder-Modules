# Forward-only application-data migrations

## Goal

Upgrade application records through the public `database_t` API so applications can
move persisted data to their required version through explicit, atomic, forward-only
transformations. Applications own
migration identities, transformations, and their target version; the database owns
physical storage and its format version. [The public API](api.h) specifies callback
obligations, borrowing, atomicity, and exceptions.

## Versions and history

`baseline_version` is universal and independent of application schemas; never
regenerate it. Version fields, the baseline, and the `migrate()` target use owning
`unique_id_t` values, re-exported by this module. Construct an identity explicitly
from a published literal; malformed text throws `std::invalid_argument` during
construction, before calling `migrate()`. Identities do not borrow their input text.
The baseline is an `inline const` value, not a compile-time constant.

Other versions are independently generated UUIDv7 identities encoded
as exactly 25 lowercase base36 characters, without a module prefix or name suffix.
The [unique identity module](https://github.com/Gilqamesh/Builder/blob/main/ws0/m03h5n4ecgpxtl2skl0hn35s0n_unique_id/README.md)
owns their shared representation and validation contract.

Supply one complete linear history rooted at `baseline_version`, with at most one
predecessor and successor per version. Definitions may be unordered: only `from`/`to`
links establish order, never UUID timestamps or lexical order. Duplicate transitions
are invalid. The entire history is validated even when the database already has the
target version. An empty history supports only baseline; a target within a longer
history is allowed.

Upgrades are forward-only. Persisted versions absent from the history or following
the requested target are rejected before application callbacks run. Applications
must call `migrate()` before accessing their tables; it provides no automatic
compatibility for applications that omit this check.

## Reserved metadata

`metadata_table_name` is the stable reserved logical name of a table visible in
`database_t::tables()`. Its schema is exactly one nonnullable string column named
`version`, with exactly one row containing a valid identity from the supplied
history, including baseline. Storage retains canonical strings, preserving existing
metadata and identifiers. Stored identities are parsed when read; malformed text
throws `std::runtime_error` and is rejected without repair.
Applications must not mutate this table; exclude its name, regardless of ASCII
case, from application-table deletion or transformation.

Missing metadata is accepted only when `tables()` is empty and means baseline.
Even a baseline-only call initializes the version record transactionally. An empty
application table makes an unversioned database nonempty. Legacy adoption/import
is outside this module and must verify the source schema and transactionally
establish its version; application schemas are never inferred.

## Concurrent attempts

Concurrent migration attempts require independent connections and may fail on
conflict. Running different application versions concurrently against one database
is unsupported. Connection access, snapshots, and recovery follow the
[database transaction contract](../m03h5hm86mnvc18opf1rdttsgh_database/README.md#transactions-and-errors).

## Example

This example adds and populates `display_name`, then removes `name` and converts
the balance from cents. Both callbacks rebuild `"people"` inside the transaction
owned by `migrate()`.

| Version | Columns |
| --- | --- |
| `v1` | `id`, `name`, `balance_cents` |
| `v2` | `id`, `name`, `balance_cents`, `display_name` |
| `v3` | `id`, `display_name`, `balance` |

```cpp
#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>

#include <array>
#include <cstdint>
#include <string>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

struct v1_person_t { std::int64_t id; std::string name; std::int64_t balance_cents; };
struct v2_person_t { std::int64_t id; std::string name; std::int64_t balance_cents; std::string display_name; };
struct v3_person_t { std::int64_t id; std::string display_name; double balance; };

void initialize_people(database_t& database) {
    const auto v1_people = database.create_table<v1_person_t>("people");
}

void add_display_names(database_t& database) {
    const auto v1_people = database.create_table<v1_person_t>("people");
    const auto v1_rows = v1_people.rows();
    database.drop_table<v1_person_t>("people");
    const auto v2_people = database.create_table<v2_person_t>("people");
    for (const auto& v1_person : v1_rows) {
        const auto display_name = "Customer: " + v1_person.name;
        const v2_person_t v2_person{v1_person.id, v1_person.name, v1_person.balance_cents, display_name};
        v2_people.insert(v2_person);
    }
}

void convert_balances(database_t& database) {
    const auto v2_people = database.create_table<v2_person_t>("people");
    const auto v2_rows = v2_people.rows();
    database.drop_table<v2_person_t>("people");
    const auto v3_people = database.create_table<v3_person_t>("people");
    for (const auto& v2_person : v2_rows) {
        const auto balance = static_cast<double>(v2_person.balance_cents) / 100.0;
        const v3_person_t v3_person{v2_person.id, v2_person.display_name, balance};
        v3_people.insert(v3_person);
    }
}

const unique_id_t v1{"03h5mgnvq9zym7wvkijwwaobp"};
const unique_id_t v2{"03h5mgnvq9zym7wvkijwwaobo"};
const unique_id_t v3{"03h5mgnvq9zym7wvkijwwaobn"};
const std::array migrations = {
    migration_t{baseline_version, v1, initialize_people},
    migration_t{v1, v2, add_display_names},
    migration_t{v2, v3, convert_balances}
};
} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    database_t database; // Use a storage-directory path for persistence.

    // The older release initialized its version before storing application data.
    migrate(database, v1, migrations);
    const auto v1_people = database.create_table<v1_person_t>("people");
    const v1_person_t v1_person{1, "Ada", 1250};
    v1_people.insert(v1_person);

    // The updated release calls this before opening its application tables.
    migrate(database, v3, migrations);
    migrate(database, v3, migrations); // No transformations.
    const auto v3_people = database.create_table<v3_person_t>("people");
    const auto v3_rows = v3_people.rows();
    if (v3_rows.size() != 1) {
        return 1;
    }

    const auto& v3_person = v3_rows.front();
    if (v3_person.id != 1) {
        return 1;
    }
    if (v3_person.display_name != "Customer: Ada") {
        return 1;
    }
    if (v3_person.balance != 12.5) {
        return 1;
    }

    return 0;
}
```

For production initialization, open `database_t("application-data")` and call only
`migrate(database, v3, migrations)`. The historical insert above only
demonstrates an upgrade. Storage paths follow the
[database persistence contract](../m03h5hm86mnvc18opf1rdttsgh_database/README.md#persistence).

## Authoring migrations

Generate each new identity once when authoring a migration:
`unique_id_t::generate().string()` from
`m03h5n4ecgpxtl2skl0hn35s0n_unique_id/unique_id.h`. Store the result as a permanent
literal in a `const unique_id_t` declaration, as in the example. Do not generate it at startup
or reuse a module ID. Use `baseline_version` as the first predecessor.

When feature branches introduce competing successors, update the later unpublished
migration's `from` to the new main-branch head, keep its `to`, and revalidate its
transformation against that head's records. Preserve published migrations while
upgrades from their source versions remain supported.

When updating a caller of the former string-based API, replace version
`constexpr std::string_view` declarations with `const unique_id_t`, explicitly parse
text supplied at runtime, and rebuild the caller. There are no string overloads.
Use `.string()` when a version must be written to a text record or compared with
stored metadata; the database schema does not change.

In application CI, call `migrate()` with the complete history and current target on
a fresh temporary database, then test upgrades from supported historical records.
