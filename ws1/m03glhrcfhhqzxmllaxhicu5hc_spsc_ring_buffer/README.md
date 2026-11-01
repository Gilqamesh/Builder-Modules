# Single-producer single-consumer queue

## Goal

Transfer values in FIFO order between one concurrent producer and one concurrent consumer through a fixed-capacity buffer. This module owns queue coordination and success/failure reporting for queue operations; callers own producer and consumer roles, scheduling, and the response to a full or empty queue.

See [api.h](api.h) for the public interface.
