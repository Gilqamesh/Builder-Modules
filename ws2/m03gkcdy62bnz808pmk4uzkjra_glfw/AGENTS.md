# `m03gkcdy62bnz808pmk4uzkjra_glfw`

## Purpose

Expose GLFW through module-owned C++ types while preserving GLFW's process-wide, main-thread event-loop and callback semantics.

## Invariants

- Construct exactly one `glfw_t` on the main thread and keep it alive longer than every GLFW-backed wrapper.
- Perform GLFW operations on that same thread unless a specific GLFW operation is documented and modeled otherwise.
- Callbacks run synchronously during event processing, do not throw across the C callback boundary, and do not destroy callback-relevant GLFW resources before returning.
- Handles returned to callers are borrowed. `window_t` owns its GLFW window and custom cursor; monitor wrappers refer to externally managed GLFW monitor objects.
- A disconnected monitor has a null handle, retains its last successfully cached scalar properties, and reports no current video-mode list.
- Joystick and gamepad connection identifiers are optional; only the module runtime changes connection identity, name, and GUID.
- Polling writes a staging input snapshot; history publication remains explicit through the ring-buffer contract.
- Keyboard/mouse counters and scroll offsets are cumulative snapshots. Change views compare snapshots sharing the same origin and require both snapshots to remain alive and unchanged.
- Raw joystick change views require matching element counts.
- Semantic button names remain distinct even where GLFW exposes numeric aliases.

## Boundary

This module owns one process-wide GLFW lifetime, same-thread event processing, GLFW-backed windows/monitors/devices, and their synchronous callbacks. A second global owner, background event loop, callback exception transport, or automatic cross-resource lifetime graph requires a different explicit contract.

The CLI is a diagnostic and smoke-test surface, not the canonical module API.

## Validation

Build the module library to run `test/public_api.cpp`, then run the module's `test` binary and CLI smoke-test scripts where the environment permits. Exercise initialization/termination, window creation and callbacks, monitor connect/disconnect, joystick/gamepad connect/disconnect and polling, mapping updates, and callback-boundary behavior. Identify display-server, controller, and manual-input requirements explicitly.

## Direction required

Obtain direction before changing singleton lifetime, thread confinement, callback ownership, disconnected-wrapper semantics, cumulative input semantics, history publication, or the mapping between GLFW constants and project enums.
