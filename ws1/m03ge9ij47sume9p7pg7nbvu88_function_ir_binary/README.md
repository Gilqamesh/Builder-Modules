# Binary function representation

## Goal

Encode function IR into bytes and reconstruct it so function repositories can store and transfer definitions. This module owns the binary transformation; the IR owns the graph model, repositories own file selection and storage, and the runtime owns execution.

See [function_ir_binary.h](function_ir_binary.h) for the public interface.
