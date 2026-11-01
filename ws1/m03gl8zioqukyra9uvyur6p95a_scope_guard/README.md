# Scoped cleanup

## Goal

Run a supplied cleanup action at scope exit unless explicitly released, including across return and exception paths. The guard owns its cleanup callable. The cleanup action must not throw, and storage it references must remain valid until cleanup completes.

See [api.h](api.h) for the public interface.
