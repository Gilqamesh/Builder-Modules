#include "texture.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

size_t texture_format_bytes_per_texel(texture_format_t format) {
    switch (format) {
        case texture_format_t::RGBA_U8_NORMALIZED: return 4;
        case texture_format_t::RGBA_U8_SRGB: return 4;
        case texture_format_t::RGBA_F16: return 8;
        case texture_format_t::RGBA_F32: return 16;
        default: throw std::runtime_error(std::format("texture_format_bytes_per_texel: unknown texture format {}", static_cast<int>(format)));
    }
}

texture_t::texture_t():
    m_format(texture_format_t::RGBA_U8_NORMALIZED),
    m_width(0),
    m_height(0)
{
}

texture_t::texture_t(texture_format_t format, size_t width, size_t height, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t bytes):
    m_format(format),
    m_width(width),
    m_height(height),
    m_bytes(std::move(bytes))
{
    const auto expected_byte_count = width * height * texture_format_bytes_per_texel(format);
    const auto got_byte_count = m_bytes.size();
    if (got_byte_count != expected_byte_count) {
        throw std::runtime_error(std::format("texture_t::texture_t: byte count {} does not match expected byte count {} for texture format {} and dimensions {}x{}", got_byte_count, expected_byte_count, format, width, height));
    }
}

texture_format_t texture_t::format() const {
    return m_format;
}

size_t texture_t::width() const {
    return m_width;
}

size_t texture_t::height() const {
    return m_height;
}

const m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t& texture_t::bytes() const {
    return m_bytes;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
