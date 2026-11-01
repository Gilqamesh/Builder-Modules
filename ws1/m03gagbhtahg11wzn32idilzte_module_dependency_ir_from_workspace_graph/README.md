# Workspace dependency IR adapter

## Goal

Convert discovered workspace modules and their dependency relationships into a detached dependency representation so renderers can consume build structure without owning a live workspace graph. The adapter owns the translation; foundation modules own discovery and dependency semantics, and renderers own presentation.

See [module_dependency_ir_from_workspace_graph.h](module_dependency_ir_from_workspace_graph.h) for the public interface.
