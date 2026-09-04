# `m03gkcdy62bnz808pmk4uzkjra_glfw`

## Purpose

Expose GLFW through module-owned C++ types while preserving GLFW's process-wide, main-thread event-loop and callback semantics.

This module owns one process-wide GLFW lifetime, same-thread event processing, GLFW-backed windows, monitors, and devices, and their synchronous callbacks. A second global owner, background event loop, callback exception transport, or automatic cross-resource lifetime graph requires a different explicit contract. The CLI is a diagnostic and smoke-test surface, not the canonical module API.

## Invariants

- Construct exactly one `glfw_t` on the main thread. It must outlive windows and every operation that calls GLFW.
- Perform GLFW operations on that same thread unless a specific GLFW operation is documented and modeled otherwise.
- Callbacks run synchronously during event processing. Callback implementations must not throw across the C callback boundary or destroy callback-relevant GLFW resources before returning.
- Handles returned to callers are borrowed. `window_t` owns its GLFW window and custom cursor; monitor wrappers refer to externally managed GLFW monitor objects.
- Monitor, joystick, and gamepad wrappers may outlive `glfw_t` only as disconnected snapshots. A disconnected monitor has a null handle, retains its last successfully cached scalar properties, and reports no current video-mode list; disconnected input devices retain cached identity text but have no connection identifier.
- Polling writes a staging input snapshot; history publication remains explicit through the ring-buffer contract.
- Keyboard/mouse counters and scroll offsets are cumulative snapshots. Change views compare snapshots sharing the same origin and require both snapshots to remain alive and unchanged.
- Raw joystick change views require matching element counts.
- Semantic button names remain distinct even where GLFW exposes numeric aliases.

## Validation

The automatic `test/public_api.cpp` target is currently a placeholder. Behavioral coverage lives in the module's `test` binary and CLI smoke scripts; run the relevant surface where the environment permits and identify display-server, controller, and manual-input requirements explicitly.
