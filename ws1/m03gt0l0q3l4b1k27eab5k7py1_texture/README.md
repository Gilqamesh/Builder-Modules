# `m03gt0l0q3l4b1k27eab5k7py1_texture`

## Goal

Own CPU-side 2D texture data, sampler descriptions, and backend-independent sampling behavior. Provide validated non-owning pixel views and explicit mip-level generation. See [pixel views](pixel_view.h) and [texture ownership](texture.h).

## Model

Textures own CPU storage; pixel views borrow it. Borrowed storage must remain
valid for every operation using a view. Storage mutations and sampling must be
sequenced explicitly. GPU resource creation, uploads, and binding are not performed.

Sampling does not change alpha association. Its transfer and filtering contract
is owned by [sampler.h](sampler.h).

- Textures allocate a fixed contiguous prefix of mip levels at construction.
  Explicit generation updates lower-level contents in place; writable edits never
  trigger regeneration. Sequence writes, regeneration, and sampling explicitly.
  Pixel-view lifetimes and sampling rules are owned by the public headers.
