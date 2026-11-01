# Scoped cleanup

## Goal

Run a supplied cleanup action at scope exit unless explicitly released so callers can pair temporary resource acquisition with cleanup across return and exception paths. This module owns the cleanup trigger; callers own the resource and supply a nonthrowing cleanup action.

See [api.h](api.h) for the public interface.
