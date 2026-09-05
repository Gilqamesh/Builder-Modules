# `m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer`

## Purpose

Provide a headless CPU rendering pipeline and own the renderer-facing framebuffer, camera, mesh, geometry, material, and render-item model consumed by that pipeline.

Backend-independent shader construction and reflection belong to `m03gsy25j4v7nccgmsdov9ioft_shader`; individual CPU shader invocation belongs to `m03gt1djvvy5atia5evkbg6rqy_software_shader`; CPU texture storage and sampling belong to `m03gt0l0q3l4b1k27eab5k7py1_texture`.

## Public model

- A `mesh_t` combines vertex streams with location-corresponding vertex attributes. Shader input location `N` consumes mesh stream `N` when their declared types are exactly compatible.
- A `geometry_t` combines a shared mesh, a selected range from a shared mutable index buffer, and primitive topology.
- A `material_t` owns shared access to one immutable software-shader program, owns its uniform values, and owns shared texture and sampler lifetimes in independent binding namespaces.
- A `render_item_t` combines geometry, material, and 2D translation, counter-clockwise rotation in radians, and scale for one draw.
- A `framebuffer_t` borrows caller-owned RGBA8 storage; the caller keeps it valid during renderer operations and rebinds after allocation changes.
- `software_renderer_t::draw()` synchronously consumes a camera and render item through the material-owned shader program.
- Points, lines, and triangles remain distinct rasterized primitive classes. Strip, loop, and fan topologies only define assembly within their corresponding primitive class.

## Invariants

- The CPU framebuffer is row-major, top-left-origin, non-premultiplied RGBA8 storage.
- The application owns framebuffer allocation, resizing, frame sequencing, and presentation.
- Shader vertex position is homogeneous clip position. Floating-point scalar and vector fragment inputs are perspective-correctly interpolated; unsupported interpolation types are rejected.
- Object-to-world is `T * R * S` for column vectors and is supplied with the current camera-derived world-to-clip matrix through vertex invocation state. Both are homogeneous float 4x4 matrices that preserve Z and W.
- The existing camera mapping sends increasing world Y toward increasing framebuffer Y; mathematical counter-clockwise object rotation therefore appears clockwise in the top-left-origin framebuffer.
- Rendering has no depth test, blending, or culling. Fragment discard or an unwritten fragment color preserves the destination pixel.
- The module has no windowing, OpenGL, or presentation responsibility.
- Renderer resource validation observes current shared mutable state on every draw.
- Renderer-owned reusable storage retains its peak capacity for the renderer lifetime.
- Large finite coordinates can lose precision during clipping and rasterization; this remains deferred without an epsilon workaround.

## Validation

Deterministic CPU pipeline behavior is validated headlessly within this module. Presentation and the tower-defense scene are integration checks owned by their respective consumers.

## Intended direction

Evolve into a general-purpose, headless 3D CPU rasterizer while retaining
2D rendering. Target perspective and orthographic cameras, 3D transforms,
depth/stencil testing, face culling, blending, viewport/scissor control,
offscreen rendering, and mipmapped texture sampling.

Shader construction, shader execution, texture storage/sampling, and
application-owned scene organization and presentation retain their
existing ownership boundaries.

These are target capabilities, not claims of current implementation.

## Rasterization correctness

Adjacent triangles with identical shared-edge endpoints must assign
boundary samples consistently, without cracks or duplicate coverage.
Half-open edge ownership requires numerically consistent edge evaluation,
including after clipping.

## Open decisions

- Coordinate conventions, clip-depth range, and compatibility with the
  existing 2D camera and transforms.
- Ownership of draw state and color/depth/stencil attachments.
- Color-space and alpha conventions; fragment discard and attachment-write
  ordering.
- Completion scope: interpolation modes, instancing, multiple color
  targets, multisampling, and advanced texture sampling.
