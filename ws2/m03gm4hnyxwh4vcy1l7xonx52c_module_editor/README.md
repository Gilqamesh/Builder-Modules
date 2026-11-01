# Module-aware text editor

## Goal

Provide a graphical text-editing workspace with module browsing and name completion. Own editing and completion interaction using workspace discovery, identity, and window-rendering interfaces.

See [api.h](api.h) for the public interface.

## Limitations

The current editing buffer is in memory; opening and saving source files are not
implemented.
