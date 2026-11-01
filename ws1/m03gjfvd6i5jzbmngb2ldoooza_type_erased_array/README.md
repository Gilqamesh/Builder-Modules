# Type-erased arrays

## Goal

Own contiguous element data behind a byte-oriented interface with element counts, sizes, and typed access. Check sizes and indices without retaining the compile-time element type; typed access must match the stored element type.

See [api.h](api.h) for the public interface.
