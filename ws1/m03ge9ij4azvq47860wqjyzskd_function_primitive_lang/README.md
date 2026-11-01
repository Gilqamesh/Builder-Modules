# Primitive function construction

## Goal

Create named runtime function nodes backed by caller-supplied C++ callbacks so native operations can participate in function graphs. This module owns construction of primitive definitions; callbacks own operation behavior and the runtime owns argument propagation and node execution.

See [function_primitive_lang.h](function_primitive_lang.h) for the public interface.
