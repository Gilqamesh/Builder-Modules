# Dependency DOT diagrams

## Goal

Write dependency IR as a DOT graph with a selected module highlighted so graph-layout tools can display workspace, module, and builder relationships. This module owns DOT construction; discovery and dependency rules belong to the build system and image rendering belongs to a rendering adapter.

See [module_dependency_ir_dot_renderer.h](module_dependency_ir_dot_renderer.h) for the public interface.
