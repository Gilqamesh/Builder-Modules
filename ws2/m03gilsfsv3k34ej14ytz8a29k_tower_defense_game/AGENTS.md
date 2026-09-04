# `m03gilsfsv3k34ej14ytz8a29k_tower_defense_game`

## Purpose

Implement the tower-defense application while serving as the current proving ground for the graphics resource model. The graphics architecture is still evolving, so stable decisions and open decisions must be kept separate.

## Current graphics model

- A `mesh_t` means vertex streams plus their vertex attributes. It does not semantically own primitive topology or index selection.
- Every mesh stream has the same vertex count, and each stream element size matches its declared attribute.
- An `index_buffer_t` is reusable index data independent of a mesh.
- A `geometry_t` combines a mesh, index data or selection, and primitive topology for drawing.
- A `material_t` combines a shader program with texture/sampler bindings.
- A `render_item_t` combines geometry, material, and object transform state.
- Expensive reusable resources may use shared ownership when multiple render items or backend representations genuinely share them.

Use `render_item` for the current model. Treat legacy `entity` references and the numbered renderer variants as experiments; do not merge, rename, or remove them unless the task defines the destination model.

## Invariants

- Resource boundaries reflect semantic reuse: vertex data, index data, geometry, material, and render-item state are not collapsed merely to reduce the number of types.
- Backend-specific handles and GPU objects do not redefine the generic semantic meaning of the resource.
- Compatibility checks use declared vertex attributes, shader inputs, texture bindings, and topology rather than relying on incidental storage layout.
- Shared ownership is used for actual shared lifetime, not as the default substitute for deciding ownership.
- Application behavior must remain buildable while architecture experiments are isolated to the files named by the task.

## Non-goals

Do not automatically:

- split the graphics types into separate modules;
- introduce a generic representation framework;
- create a shader language or reflection system;
- replace transform components with matrices;
- normalize all renderer experiments into one design;
- add instancing, draw batching, or generalized render graphs.

Each requires an explicit task and semantic model.

## Validation

At minimum, build the application and exercise construction/finalization of the changed resources. Mesh changes require mismatch cases for stream count, vertex count, and element size. Rendering changes require a visible smoke test and must identify the backend, display, and GPU assumptions.

## Open decisions

An agent must obtain explicit direction before settling:

- whether geometry references complete index buffers or index views/ranges and who owns those views;
- the final boundary between generic definitions and OpenGL/software representations;
- whether transform remains translation/rotation/scale or becomes a matrix abstraction;
- how software-renderer shader behavior relates to vertex and fragment shader concepts;
- whether render targets replace the renderer's direct window dependency.
