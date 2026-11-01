# Function intermediate representation

## Goal

Describe a function definition through its identity, layout bounds, child functions, and argument connections so editors, repositories, and execution code can exchange the same graph description. This module owns that data model; serialization, persistence, evaluation, and interactive presentation belong to consumers.

See [function_ir.h](function_ir.h) for the public interface.
