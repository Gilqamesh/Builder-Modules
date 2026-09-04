# `m03gt1djvvy5atia5evkbg6rqy_software_shader`

## Purpose

Execute individual vertex and fragment shader invocations on the CPU from backend-independent shader ASTs and canonical CPU resources.

This module owns program-link compatibility and per-invocation execution. Vertex assembly, interpolation, rasterization, framebuffer state, presentation, and renderer-facing resource lifetimes remain renderer or pipeline responsibilities.

## Invariants

- Every `run()` starts with fresh locals and invalidates prior stage results before execution.
- Program linking rejects incompatible stages, interfaces, and resource bindings before execution.
- Expressions are evaluated where they execute and observe the current local state rather than cached expression values.
- A normally completed vertex invocation has written position.
- Fragment discard terminates the invocation and invalidates all fragment outputs.
- Execution representation and strategy remain private to permit later alternatives without changing the public contract.
