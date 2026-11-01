# Primitive computation nodes

## Goal

Provide ready-made arithmetic, conditional, and utility function nodes so function graphs can perform basic computations and expose results without repeating callback construction. This module owns those primitive operations; the runtime owns node execution and consumers own graph composition and application meaning.

See [function_alu.h](function_alu.h) for the public interface.
