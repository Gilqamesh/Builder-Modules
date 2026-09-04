# `m03gli1rb5p56mncplipxpf3he_ring_buffer`

## Purpose

Provide a fixed-capacity, single-threaded history with an explicit staging value that becomes committed only through `commit()`.

## Public model

History offset zero denotes the newest committed value. `stage()` denotes the value intended for the next commit and is not itself a committed history entry.

`staging_policy_t` selects whether staging overlaps committed storage. `commit_policy_t` selects whether commit merely advances or first copies the staged value into the next staging slot.

## Invariants

- Capacity is positive and fixed for the lifetime of the object.
- `history_size()` counts committed entries only and never exceeds `history_capacity()`.
- After a commit, `history(0)` is the newly committed value and increasing offsets move toward older committed values.
- Wraparound preserves newest-to-oldest history order.
- Construction establishes storage; ordinary staging, commit, and history access remain constant-time and do not resize storage.
- An overlapping staging policy may alias the oldest committed entry when full, and that destructive staging behavior must remain explicit.
- A dedicated staging policy must not modify committed history before `commit()`.
- `advance` leaves the next staging slot unchanged; `copy_with_advance` initializes it from the committed value before advancing.
- History access outside `history_size()` throws `std::out_of_range`.

## Boundary

This module owns fixed-capacity, single-threaded staged history. Dynamic resizing, persistence, and concurrent producer/consumer coordination belong to other abstractions, including the SPSC ring-buffer module.

## Validation

Build the module library to run `test/public_api.cpp`. Test zero-capacity rejection, empty state, first commit, partial fill, full capacity, multiple wraparounds, capacity one, out-of-range history, mutable and const history access, and every staging/commit policy combination.
