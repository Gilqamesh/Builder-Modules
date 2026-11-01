# Saturating arithmetic

## Goal

Provide numeric arithmetic that clamps unrepresentable results and rejects nonfinite floating inputs so callers can keep bounded calculations within representable limits. This module owns the arithmetic rules; callers own units, domain constraints, and the decision to use saturation.

See [api.h](api.h) for the public interface.
