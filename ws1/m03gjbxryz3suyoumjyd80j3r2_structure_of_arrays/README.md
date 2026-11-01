# Structure of arrays

## Goal

Store related record fields in parallel arrays and expose an erased collection of streams so consumers can process field data independently of record layout. This module owns coordinated stream construction and access; consumers own field meaning and any rendering or computation performed on the streams.

See [api.h](api.h) for the public interface.
