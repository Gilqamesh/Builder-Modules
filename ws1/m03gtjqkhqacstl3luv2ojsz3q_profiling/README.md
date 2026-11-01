# `m03gtjqkhqacstl3luv2ojsz3q_profiling`

## Goal

Measure synchronous code execution through persistent metric storage, timing statistics, and deferred reporting. Metric types define data, constructors, counter meanings, formatters, and measurement boundaries. Metrics borrow a profiler and explicitly create child metrics; the profiler must outlive them. Data accumulates per parent/type path, with children reported in first-use order. See [the public contract](api.h) for enablement, lifetime, and type requirements.

## Validation

`test/public_api.cpp` exercises the public contract. Build it with
`PROFILING_TEST_CLOCK` for deterministic timing evidence and without the macro for
the production clock. Validate metric counters and verify that measured operations
have identical observable results with profiling enabled and disabled.
