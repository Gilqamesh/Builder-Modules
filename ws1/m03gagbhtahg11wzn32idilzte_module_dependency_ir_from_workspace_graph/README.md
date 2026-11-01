# Workspace dependency IR adapter

## Goal

Convert discovered workspace modules and their dependency relationships into a detached dependency representation. Translate authoritative workspace and source-dependency data without retaining a live workspace graph or producing a presentation.

See [module_dependency_ir_from_workspace_graph.h](module_dependency_ir_from_workspace_graph.h) for the public interface.
