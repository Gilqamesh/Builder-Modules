# Runtime function repository

## Goal

Associate function identities with definitions and invocation callbacks so live function graphs can retrieve the code and structure needed for expansion. This module owns the in-memory registry; identity, graph semantics, durable storage, and execution belong to the neighboring function modules.

See [function_repository.h](function_repository.h) for the public interface.
