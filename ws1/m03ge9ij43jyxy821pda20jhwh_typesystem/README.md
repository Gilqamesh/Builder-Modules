# Runtime type coercion

## Goal

Register value types and conversions and coerce values between registered types so dynamic function nodes can exchange typed data. This module owns runtime type metadata and conversion-path execution; consumers own the objects, the meaning of conversions, and function-graph behavior.

See [typesystem.h](typesystem.h) for the public interface.
