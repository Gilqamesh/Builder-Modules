# Module dependency representation

## Goal

Represent workspaces, modules, and separate module and builder dependency lists as detached data so graph tools can share build structure without depending on workspace discovery objects. This module owns the representation; adapters derive it from authoritative build data and renderers choose its presentation.

See [module_dependency_ir.h](module_dependency_ir.h) for the public interface.
