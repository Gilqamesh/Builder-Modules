# `m03gkcdy62bnz808pmk4uzkjra_glfw`

## Purpose

Expose GLFW through module-owned C++ types while preserving GLFW's process-wide, main-thread event-loop and callback semantics.

## Invariants

- Construct exactly one `glfw_t` on the main thread and keep it alive longer than every GLFW-backed wrapper.
- Perform GLFW operations on that same thread unless a specific GLFW operation is documented and modeled otherwise.
- Callbacks run synchronously during event processing, do not throw across the C callback boundary, and do not destroy callback-relevant GLFW resources before returning.
- Raw GLFW handles are non-owning implementation handles; wrapper ownership does not imply ownership of externally managed monitor or device objects.
- A disconnected monitor has a null handle, retains its last successfully cached scalar properties, and reports no current video-mode list.
- Joystick and gamepad connection identifiers are optional; only the module runtime changes connection identity, name, and GUID.
- Polling writes a staging input snapshot; history publication remains explicit through the ring-buffer contract.
- Keyboard/mouse counters and scroll offsets are cumulative snapshots. Change views compare snapshots sharing the same origin and require both snapshots to remain alive and unchanged.
- Raw joystick change views require matching element counts.
- Semantic button names remain distinct even where GLFW exposes numeric aliases.

## Non-goals

Do not add a second global GLFW owner, background event thread, hidden callback exception channel, or automatic resource lifetime graph without explicit direction.

The CLI is a diagnostic and smoke-test surface, not the canonical module API.

## Validation

Run the module and CLI smoke-test scripts where available. Exercise initialization/termination, window creation and callbacks, monitor connect/disconnect, joystick/gamepad connect/disconnect and polling, mapping updates, and non-throwing callback failure handling. Identify display-server, controller, and manual-input requirements explicitly.

## Explicit decisions

Obtain direction before changing singleton lifetime, thread confinement, callback ownership, disconnected-wrapper semantics, cumulative input semantics, history publication, or the mapping between GLFW constants and project enums.
