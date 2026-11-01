# Stored function definitions

## Goal

Save and retrieve function definitions by identity, including the latest definition for a namespace and name, so graph-authoring sessions can retain and reopen their work. This module owns file-backed definition lookup and storage; the IR and binary modules own representation and the runtime owns execution.

See [function_ir_file_repository.h](function_ir_file_repository.h) for the public interface.
