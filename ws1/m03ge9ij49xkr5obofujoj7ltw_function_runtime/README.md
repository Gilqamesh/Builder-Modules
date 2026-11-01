# Function runtime

## Goal

Execute connected function nodes, propagate argument values, and expand compound definitions so function graphs can perform computation. The runtime owns live node state, connections, and invocation behavior; type conversions, persisted definitions, and interactive visualization belong to their respective modules.

See [function.h](function.h) for the public interface.
