# `m03gl22hn0dqmosreqjie9tg5m_opengl_renderer`

## Purpose

Provide OpenGL rendering for an OpenGL-capable GLFW window. The renderer owns the OpenGL mechanisms and resources that turn supplied data into a window image.

## CPU framebuffer presentation

`opengl_renderer_t::present_rgba8()` is a temporary convenience operation for presenting externally rendered CPU images. It owns texture upload, the fullscreen textured draw, and buffer swapping; the producer owns the CPU-visible row-major, top-left-origin, non-premultiplied RGBA8 memory.

The handoff is defined by that pixel-memory format. The OpenGL renderer does not depend on the software renderer.

## Intended evolution

- Fold CPU-framebuffer presentation into the normal OpenGL resource-update and `draw()` model as that model develops.
- Allocate and reuse the fullscreen geometry, texture, copy-texture program, and presentation render item; only pixel contents change each frame.
- A persistently mapped generic OpenGL buffer may eventually provide CPU-writable storage and serve as a pixel-unpack source.
- Buffer mapping and buffer-to-texture transfer remain OpenGL implementation details and must not enter the software-renderer API.
