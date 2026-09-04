# `m03gilsfsv3k34ej14ytz8a29k_tower_defense_game`

## Purpose

Implement the tower-defense application while serving as the current proving ground for the graphics resource model. The graphics architecture is still evolving, so stable decisions and open decisions must be kept separate.

The application module owns the game and its evolving graphics integration. CPU-side texture storage and sampling belong to `m03gt0l0q3l4b1k27eab5k7py1_texture`. Backend-independent shader-language, frontend, and reflection semantics belong to `m03gsy25j4v7nccgmsdov9ioft_shader`; individual CPU shader invocation belongs to `m03gt1djvvy5atia5evkbg6rqy_software_shader`. The application owns integration of those abstractions with its material, draw-resource, and renderer model.

## Public model

- A `mesh_t` means vertex streams plus their vertex attributes. It does not semantically own primitive topology or index selection.
- Every mesh stream has the same vertex count, and each stream element size matches its declared attribute.
- An `index_buffer_t` is reusable mutable index data independent of a mesh.
- A `geometry_t` retains shared ownership of one complete index buffer, selects an immutable offset/count range from it, and combines that selection with a shared mesh and primitive topology.
- `geometry_t::finalize()` validates the current mesh, selected indices, and topology; it does not freeze resources that remain publicly mutable.
- CPU-side texture data and sampler behavior are owned by `m03gt0l0q3l4b1k27eab5k7py1_texture`.
- A `material_t` currently contains an ordered collection of bindings to those texture and sampler resources. Shader programs are not yet part of its public model.
- A `render_item_t` combines geometry, material, and object transform state.
- Reusable geometry, mesh, index, material, texture, and sampler resources use shared ownership where the current API shares their lifetime.

Use `render_item` for the current model. `renderer3_t` is the active application integration path. The inactive `renderer_t` and `renderer2_t` paths, standalone application-local `opengl_*` wrappers, and remaining `entity` terminology are legacy or experimental evidence, not co-equal statements of intended architecture.

## Invariants

- Resource boundaries reflect semantic reuse: vertex data, index data, geometry, material, and render-item state are not collapsed merely to reduce the number of types.
- Backend-specific handles and GPU objects do not redefine the generic semantic meaning of the resource.
- Compatibility checks use declared vertex attributes, texture bindings, selected indices, and topology rather than incidental storage layout.
- Geometry construction rejects a null index buffer and an initially invalid range; `indices()` revalidates the range because the shared index buffer remains mutable.
- Geometry finalization requires a mesh, a non-empty valid index selection, a topology-compatible index count, and indices within the mesh's vertex count.
- Shared ownership expresses actual shared lifetime rather than substituting for an ownership decision.

## Validation

The automatic `test/public_api.cpp` target covers mesh and geometry construction, range revalidation, topology, and selected-index invariants. Rendering changes also require a visible smoke test that identifies backend, display, and GPU assumptions.

## Open decisions

Obtain explicit direction before settling:

- the final boundary between the remaining generic definitions and OpenGL/software representations;
- whether and how an immutable shader program participates in the material model;
- whether transform remains translation/rotation/scale or becomes a matrix abstraction;
- how `software_shader` invocations integrate with vertex assembly, interpolation, rasterization, and renderer resource binding;
- whether render targets replace the renderer's direct window dependency.
