# Module `m03h5hm86mnvc18opf1rdttsgh_database`

## Purpose

Own the typed record database abstraction. Keep the storage backend, physical representation, and recovery mechanisms private. The public contract belongs in [database.h](database.h).

## Validation

Exercise observable behavior through the database API, including literal filenames and arbitrary table names. Reserve private integration tests for backend structure, mechanisms, and failure injection. Compile client codecs and negative reflection cases against the published header without backend include paths.
