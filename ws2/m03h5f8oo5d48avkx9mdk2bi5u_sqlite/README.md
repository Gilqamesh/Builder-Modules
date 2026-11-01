# SQLite

## Goal

Provide a C++ ownership API for SQLite connections, statements, and transactions. Execute supplied SQL while managing native handles and resource lifetimes, without imposing schemas or domain data semantics.

## Model

Packages the unmodified SQLite 3.54.1 amalgamation as a Builder library. The
amalgamation is compiled as C; this module has no CLI.

Include `<m03h5f8oo5d48avkx9mdk2bi5u_sqlite/sqlite.h>` for the C++ API:

```cpp
#include <m03h5f8oo5d48avkx9mdk2bi5u_sqlite/sqlite.h>

#include <iostream>

namespace sqlite = m03h5f8oo5d48avkx9mdk2bi5u_sqlite;

int main() {
    sqlite::database_t database(":memory:");
    database.execute("CREATE TABLE notes (title TEXT NOT NULL)");

    auto insert = database.prepare("INSERT INTO notes VALUES (?1)");
    insert->bind(1, "Example");
    insert->step();
    insert->reset();
    insert->bind(1, "Another");
    insert->step();

    auto select = database.prepare("SELECT title FROM notes ORDER BY rowid");
    while (select->step()) {
        std::cout << select->column_text(0) << '\n';
    }
}
```

`prepare()` compiles a statement. Bind its parameters, call `step()` until it
returns `false`, and read columns whenever it returns `true`. To reuse a
statement, call `reset()` before binding again; existing bindings are kept.
`execute()` runs a SQL script and discards rows.

For SQL syntax and SQLite-specific behavior, see the upstream
[SQL language reference](https://www.sqlite.org/lang.html).

[sqlite.h](sqlite.h) is the sole public header and documents ownership,
conversions, errors, and threading. SQLite's C API and native handles are
implementation details.

Builder automatically builds and runs `test/public_api.cpp` during library
validation.

The source phase downloads the pinned
[SQLite autoconf archive](https://www.sqlite.org/2026/sqlite-autoconf-3540100.tar.gz)
and verifies the [upstream published SHA3-256](https://www.sqlite.org/download.html)
`b6ced585c750916b1052667239a12c25a7ff34e7f06e1e6e01ecb0427657d9c4`
before extracting it. SHA3-256 verification uses GNU `cksum` with SHA3 support.

SQLite is [in the public domain](https://www.sqlite.org/copyright.html).
