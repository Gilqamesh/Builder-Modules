# `m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer`

## Purpose

Provide a headless CPU rendering pipeline and own the renderer-facing framebuffer, camera, mesh, geometry, material, and render-item model consumed by that pipeline.

Backend-independent shader construction and reflection belong to `m03gsy25j4v7nccgmsdov9ioft_shader`; individual CPU shader invocation belongs to `m03gt1djvvy5atia5evkbg6rqy_software_shader`; CPU texture storage and sampling belong to `m03gt0l0q3l4b1k27eab5k7py1_texture`.

## Public model

- A `mesh_t` combines vertex streams with location-corresponding vertex attributes. Shader input location `N` consumes mesh stream `N` when their declared types are exactly compatible.
- A `geometry_t` combines a shared mesh, a selected range from a shared mutable index buffer, and primitive topology.
- A `material_t` supplies texture and sampler binding pair `N` to shader resource binding `N`; it does not own a shader program or generic uniform values.
- A `render_item_t` combines geometry, material, and object transform state for one draw.
- A `framebuffer_t` borrows caller-owned RGBA8 storage; the caller keeps it valid during renderer operations and rebinds after allocation changes.
- `software_renderer_t::draw()` synchronously consumes a camera and render item through its renderer-owned shader program.
- Points, lines, and triangles remain distinct rasterized primitive classes. Strip, loop, and fan topologies only define assembly within their corresponding primitive class.

## Invariants

- The CPU framebuffer is row-major, top-left-origin, non-premultiplied RGBA8 storage.
- The application owns framebuffer allocation, resizing, frame sequencing, and presentation.
- Shader vertex position is homogeneous clip position. Floating-point scalar and vector fragment inputs are perspective-correctly interpolated; unsupported interpolation types are rejected.
- Rendering has no depth test, blending, or culling. Fragment discard or an unwritten fragment color preserves the destination pixel.
- The module has no windowing, OpenGL, or presentation responsibility.
- Renderer resource validation observes current shared mutable state on every draw.

## Validation

Deterministic CPU pipeline behavior is validated headlessly within this module. Presentation and the tower-defense scene are integration checks owned by their respective consumers.
