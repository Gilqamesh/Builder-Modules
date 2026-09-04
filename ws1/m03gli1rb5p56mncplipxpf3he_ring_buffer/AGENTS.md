# `m03gli1rb5p56mncplipxpf3he_ring_buffer`

## Purpose

Provide a fixed-capacity, single-threaded history with an explicit staging value that becomes committed only through `commit()`.

## Public model

History offset zero denotes the newest committed value. `stage()` denotes the value intended for the next commit and is not itself a committed history entry.

The implementation may support different staging/storage policies, but policy names must describe their observable aliasing and commit behavior rather than implementation accidents.

## Invariants

- Capacity is positive and fixed for the lifetime of the object.
- `size()` counts committed history entries only and never exceeds `capacity()`.
- After a commit, `history(0)` is the newly committed value and increasing offsets move toward older committed values.
- Wraparound preserves newest-to-oldest history order.
- Construction establishes storage; ordinary staging, commit, and history access remain constant-time and do not resize storage.
- An overlapping staging policy may alias the oldest committed entry when full, and that destructive staging behavior must remain explicit.
- A dedicated staging policy must not modify committed history before `commit()`.
- Copy/advance behavior on commit must be defined by the selected policy and tested at capacity one as well as larger capacities.

## Non-goals

The module is not thread-safe, dynamically resizable, persistent, or a replacement for the SPSC ring-buffer module.

## Validation

Test empty state, first commit, partial fill, full capacity, multiple wraparounds, capacity one, out-of-range history, staging before commit, and every supported staging/commit policy combination.

## Open decisions

The final public names and exact post-commit staging behavior of policy variants must follow explicit user direction. Do not rename or collapse policies merely because two implementations can share code.
