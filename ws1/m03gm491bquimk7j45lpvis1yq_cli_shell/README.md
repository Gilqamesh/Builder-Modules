# Interactive command input

## Goal

Provide line editing, completion, and command history around a command application so tools can offer an interactive terminal interface. This module owns the input loop and history integration; the borrowed command application owns command definitions, validation, and execution.

See [api.h](api.h) for the public interface.
