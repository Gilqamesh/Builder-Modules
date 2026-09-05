# Software renderer milestones

Status: proposed roadmap; each milestone is unstarted. This document records scope and decisions to discuss, not approval of a new API.

Baseline: [Builder-Modules at 540bbede71740d24292cc3b7cd9c8ed126eca0c3](https://github.com/Gilqamesh/Builder-Modules/tree/540bbede71740d24292cc3b7cd9c8ed126eca0c3).

Develop a general-purpose headless 3D CPU rasterizer while retaining the existing 2D path. Design and implementation follow the shared [agent workflow](../../../../Builder/docs/agent-workflow.md).

Keep later milestones at outcome level until their dependencies are settled. Existing tests inform validation; they do not determine the design. A milestone is complete when its behavior works through the public API, with focused evidence and updated contracts.

The current module boundaries remain the starting point: shader construction/reflection in shader, individual CPU invocation in software_shader, texture storage/sampling in texture, and rasterization/attachments in software_renderer. Applications own scene organization, allocation, frame sequencing, and presentation.

The baseline includes shared geometry/material resources, indexed ranges, all seven primitive topologies, homogeneous clipping, perspective-correct float varyings, texture sampling, fragment discard, and caller-owned offscreen RGBA8 storage. Public camera mapping and render-item transforms remain 2D. There is no depth/stencil testing, blending, or culling. These are source-review findings; tests were not run for this document.

## 0. Coordinates and shared-edge coverage

Status: unstarted.

- Outcome: a concise coordinate contract and consistent shared-edge ownership before attachment operations make duplicate coverage visible.
- Open decisions: world/view handedness, clip-depth range, framebuffer orientation, front-face convention, and compatibility with the current 2D mapping. Preserve settled behavior unless a concrete requirement justifies changing it.
- Acceptance criteria: fractional, reversed-winding, and clipped adjacent triangles demonstrate both no gaps and no duplicate sample coverage. The existing test's final opaque red pixels cannot detect double writes. Specify supported numeric limits; avoid an arbitrary epsilon workaround.

## 1. 3D transforms, cameras, viewport, and scissor

Status: unstarted.

- Outcome: perspective and orthographic scenes through the public API, with separate viewport mapping and scissor clipping.
- Open decisions: how callers supply object/view/projection transforms; which camera and transform helpers remain conveniences; how the 2D path maps into the same pipeline; viewport/scissor ownership and clear behavior.
- Acceptance criteria: a textured 3D object transforms and crosses the near plane correctly, perspective texture mapping is demonstrated, and a bounded view cannot overwrite pixels outside its scissor. Existing 2D scenes retain the agreed behavior.

## 2. Depth testing and face culling

Status: unstarted.

- Outcome: correct opaque visibility independent of submission order for unequal depths, plus configurable winding/cull state.
- Open decisions: depth attachment lifetime/format, clear value, comparison function, equal-depth tie behavior, independent depth writes, and the point at which fragment discard prevents writes. Decide draw-state ownership here before adding more state.
- Acceptance criteria: overlapping and intersecting surfaces render correctly in either order for unequal depths, with equal-depth samples following the chosen tie policy; depth-test and depth-write controls behave independently; discarded fragments do not occlude later geometry; culling follows the chosen convention.

## 3. Blending and color writes

Status: unstarted.

- Outcome: transparent overlays compose correctly with opaque geometry.
- Open decisions: blend factors/equations, color masks, straight versus premultiplied alpha, linear versus sRGB attachments, and encoding boundaries. Reuse the draw-state model from milestone 2.
- Acceptance criteria: known source/destination colors produce expected RGB and alpha, masks preserve disabled channels, sRGB conversion occurs at the agreed boundary, and shared edges have no blending seams. The application supplies transparent draw order.

## 4. Stencil and render-to-texture

Status: unstarted.

- Outcome: stencil masking and a clear path from rendering a pass to sampling its result. Basic offscreen color rendering already exists.
- Open decisions: color/depth/stencil attachment views and lifetimes, stencil comparisons/operations/masks, depth-only or unwritten-color behavior, clear semantics, and render-target/texture interoperability. Define or reject simultaneous sampling and writing of the same storage.
- Acceptance criteria: a stencil mask limits a draw correctly and one rendered pass is sampled by a later pass with matching orientation and color semantics.

## 5. Interpolation modes and mipmapped sampling

Status: unstarted.

- Outcome: flat varyings, including integer values; noperspective interpolation; mipmapped textures and explicit LOD sampling.
- Open decisions: interpolation metadata and linking, provoking-vertex rules through clipping/topology assembly, mip storage/generation, and LOD filtering. Keep reflection changes in shader, execution in software_shader, interpolation in the renderer, and sampling in texture.
- Acceptance criteria: flat values remain constant across clipped primitives, screen-linear and perspective interpolation visibly differ as intended, and explicit LOD selects/blends validated mip levels. Automatic derivative-based LOD is a separate later decision.

## Completion records

For each milestone, append its settled decisions, implementation commit, checks actually run, and remaining limitations. Preserve the original baseline and add subsequent review and completion references; do not count planned features as implemented.

## Deferred scope

Defer instancing, multiple color targets, MSAA, advanced sampling, and performance architecture work until a concrete scene or measurement warrants them. Point size/shape and line coverage also remain explicit future scope; the baseline uses a fixed point radius and single-pixel lines.

## Documentation ownership

This module-local roadmap records proposed outcomes, open decisions, and completion evidence. Public headers own public contracts; each module's AGENTS.md owns its durable module-specific invariants. Record cross-module decisions here with links to their authoritative contracts, without duplicating those contracts or the shared workflow.

## Baseline references

- [Renderer contract and intended direction](https://github.com/Gilqamesh/Builder-Modules/blob/540bbede71740d24292cc3b7cd9c8ed126eca0c3/ws2/m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/AGENTS.md)
- [Public renderer API](https://github.com/Gilqamesh/Builder-Modules/blob/540bbede71740d24292cc3b7cd9c8ed126eca0c3/ws2/m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_renderer.h)
- [Pipeline implementation](https://github.com/Gilqamesh/Builder-Modules/blob/540bbede71740d24292cc3b7cd9c8ed126eca0c3/ws2/m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_renderer.cpp)
- [Renderer tests](https://github.com/Gilqamesh/Builder-Modules/blob/540bbede71740d24292cc3b7cd9c8ed126eca0c3/ws2/m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/test/public_api.cpp)
- [Texture sampling API](https://github.com/Gilqamesh/Builder-Modules/blob/540bbede71740d24292cc3b7cd9c8ed126eca0c3/ws1/m03gt0l0q3l4b1k27eab5k7py1_texture/sampler.h)
