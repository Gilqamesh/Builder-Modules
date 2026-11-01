# Renderer window association

## Goal

Keep a renderer associated with a valid, shared GLFW window so concrete window renderers can retain and expose their presentation destination consistently. This module owns that window association; GLFW owns window behavior and concrete renderers own graphics resources and drawing.

See [glfw_window_renderer.h](glfw_window_renderer.h) for the public interface.
