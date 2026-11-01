# Dependency SVG diagrams

## Goal

Turn dependency IR into an SVG diagram with a selected module highlighted so developers can inspect module and builder relationships visually. This module owns SVG diagram production; the IR supplies graph data and foundation modules remain authoritative for dependency meaning.

See [module_dependency_ir_svg_renderer.h](module_dependency_ir_svg_renderer.h) for the public interface.
