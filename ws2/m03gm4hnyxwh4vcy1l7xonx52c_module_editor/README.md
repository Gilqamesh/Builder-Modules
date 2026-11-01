# Module-aware text editor

## Goal

Provide a graphical text-editing workspace with module browsing and name completion so developers can compose text using discovered workspace module names. This application owns editing and completion interaction; foundation modules own discovery and identity, and rendering modules own window presentation.

See [api.h](api.h) for the public interface.

## Limitations

The current editing buffer is in memory; opening and saving source files are not
implemented.
