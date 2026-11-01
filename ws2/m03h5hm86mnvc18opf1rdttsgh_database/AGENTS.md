# Module `m03h5hm86mnvc18opf1rdttsgh_database`

## Purpose

Own the typed record database abstraction. Keep the storage backend, physical representation, and recovery mechanisms private.

## Documentation

- Keep the shared model and detailed requirements in [README.md](README.md).
- Use [database.h](database.h) comments for concise, operation-specific semantics.
- Keep documentation navigation in Markdown and C++ comments focused on local semantics.
- Assume an experienced C++ reader.
- Avoid duplicated explanations and examples.
- Keep guarantees and limitations explicit in documentation.

## Validation

Exercise observable behavior through the database API, including literal storage directory paths and arbitrary logical table names. Reserve private integration tests for backend structure, mechanisms, and failure injection. Compile client codecs against the published header without backend include paths.
