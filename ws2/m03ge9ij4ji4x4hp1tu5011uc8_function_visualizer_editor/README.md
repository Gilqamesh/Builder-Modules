# Function visualizer text input

## Goal

Provide a text-entry overlay with text-submission callbacks and input-capture reporting so the function visualizer can collect names and other text without also interpreting those keystrokes as scene controls. This module owns overlay interaction; the visualizer owns what submitted text means and any resulting graph changes.

See [function_visualizer_editor.h](function_visualizer_editor.h) for the public interface.
