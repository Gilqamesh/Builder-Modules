# Compound function construction

## Goal

Create executable function nodes from child-and-connection IR so reusable graphs can act as larger functions. This module supplies compound-node construction and its expansion callback; the IR owns the definition model and the runtime owns live expansion, connections, and value propagation.

See [function_compound.h](function_compound.h) for the public interface.
