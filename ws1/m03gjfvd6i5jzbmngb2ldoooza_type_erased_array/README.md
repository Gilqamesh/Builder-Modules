# Type-erased arrays

## Goal

Own contiguous element data behind a byte-oriented interface with element counts, sizes, and typed access so consumers can handle streams without carrying their compile-time element type. This module owns storage and size/index checks; consumers retain responsibility for the actual element type and interpretation.

See [api.h](api.h) for the public interface.
