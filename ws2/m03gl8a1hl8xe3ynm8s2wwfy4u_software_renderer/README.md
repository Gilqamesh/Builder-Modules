# `m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer`

## Goal

Provide a headless CPU pipeline with framebuffer, camera, mesh, geometry, material, and render-item abstractions for deterministic image rendering without a window or graphics device. Render into borrowed attachments with explicit draw and clear operations.

Use [`shader`](../m03gsy25j4v7nccgmsdov9ioft_shader/README.md) for shader construction/reflection, [`software_shader`](../m03gt1djvvy5atia5evkbg6rqy_software_shader/README.md) for individual CPU invocations, and [`texture`](../../ws1/m03gt0l0q3l4b1k27eab5k7py1_texture/README.md) for texture storage/sampling.

## Boundaries

- [Materials](material.h) own shared draw state and sampled resources;
  [framebuffers](framebuffer.h) borrow attachments. Clearing is explicit.
  Material getters expose current values and borrowed resources; draw preparation resolves
  current bindings without retaining resource borrows after the draw.
- [Render items](render_item.h) own placement; [cameras](camera.h) own pose,
  projection, and destination rectangles. Planar scenes use this same model.
- Quaternion mathematics belongs to
  [`quaternion`](../../ws1/m03gtgtrh2smvh28qlwgm7gdl4_quaternion/README.md).
- Shader inputs declare interpolation. Primitive assembly, provoking-vertex
  selection, clipping, coverage, and interpolation execution belong here.
  [software_renderer.h](software_renderer.h) owns the drawing contract.
- Renderer metrics, counter meanings, and stage boundaries belong here;
  [`profiling`](../../ws1/m03gtjqkhqacstl3luv2ojsz3q_profiling/README.md) owns timing,
  storage, and reporting. Rendering executes outside metric update callbacks.

## Validation and direction

Validate deterministic pipeline behavior headlessly through attachment results,
resource validation, and failure guarantees. Outstanding feature choices are
recorded in [milestones.md](docs/milestones.md).
