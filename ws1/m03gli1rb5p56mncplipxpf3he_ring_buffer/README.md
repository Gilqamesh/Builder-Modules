# `m03gli1rb5p56mncplipxpf3he_ring_buffer`

## Goal

Provide a fixed-capacity, single-threaded history with an explicit staging value that becomes committed only through `commit()`. Keep updates unpublished until explicitly committed.

The [public header](api.h) owns the staging, commit, and history semantics. Dynamic resizing, persistence, and concurrent producer/consumer coordination are unsupported.
