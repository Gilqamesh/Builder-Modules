# Single-producer single-consumer queue

## Goal

Transfer values in FIFO order between one concurrent producer and one concurrent consumer through a fixed-capacity buffer. Coordinate queue access and report whether each operation succeeds, including full and empty states.

See [api.h](api.h) for the public interface.
