# `m03gt1djvvy5atia5evkbg6rqy_software_shader`

## Purpose

Execute backend-independent vertex and fragment shader ASTs on the CPU. Mesh ownership, vertex assembly, interpolation, rasterization, framebuffer state, and presentation remain renderer responsibilities.

## Public model

- A `program_t` owns one vertex AST and one fragment AST by value.
- A `bindings_t` owns uniform values and borrows bound CPU textures and samplers. A borrowed resource must outlive every use of the binding that refers to it.
- Uniform, texture, and sampler indices occupy separate binding namespaces.
- `vertex_io_t` and `fragment_io_t` carry typed stage inputs, built-ins, and stage results for one invocation at a time.

## Invariants

- Every `run()` starts with fresh locals and invalidates prior stage results before execution.
- Expressions are evaluated where they execute and observe the current local state rather than cached expression values.
- A normally completed vertex invocation has written position.
- Fragment discard terminates the invocation and invalidates all fragment outputs.
- Execution representation and strategy remain private to permit later alternatives without changing the public contract.
