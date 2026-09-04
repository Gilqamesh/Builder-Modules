# include "texture.h"

# include <format>
# include <limits>
# include <stdexcept>
# include <utility>

namespace m03gt0l0q3l4b1k27eab5k7py1_texture {

std::size_t bytes_per_texel(format_t format) {
    switch (format) {
        case format_t::rgba8_unorm:
        case format_t::rgba8_srgb:
            return 4;
        case format_t::rgba16_float:
            return 8;
        case format_t::rgba32_float:
            return 16;
        default:
            throw std::invalid_argument(std::format("bytes_per_texel: unknown texture format {}", static_cast<int>(format)));
    }
}

texture_t::texture_t(
    format_t format,
    std::size_t width,
    std::size_t height,
    m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t bytes
):
    m_bytes(std::move(bytes)),
    m_format(format),
    m_width(width),
    m_height(height)
{
    const auto texel_size = bytes_per_texel(format);

    if (width == 0 || height == 0) {
        throw std::invalid_argument("texture_t::texture_t: width and height must be positive");
    }

    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (maximum / height < width) {
        throw std::length_error("texture_t::texture_t: texture dimensions overflow the texel count");
    }

    const auto texel_count = width * height;
    if (maximum / texel_size < texel_count) {
        throw std::length_error("texture_t::texture_t: texture dimensions and format overflow the byte count");
    }

    const auto expected_byte_count = texel_count * texel_size;
    if (m_bytes.size() != expected_byte_count) {
        throw std::invalid_argument(std::format(
            "texture_t::texture_t: byte count {} does not match expected byte count {} for format {} and dimensions {}x{}",
            m_bytes.size(),
            expected_byte_count,
            format,
            width,
            height
        ));
    }
}

texture_t::texture_t(const texture_t& other) = default;

texture_t::texture_t(texture_t&& other) noexcept:
    m_bytes(std::move(other.m_bytes)),
    m_format(other.m_format),
    m_width(std::exchange(other.m_width, std::size_t(0))),
    m_height(std::exchange(other.m_height, std::size_t(0)))
{
    other.m_bytes.clear();
}

texture_t& texture_t::operator=(const texture_t& other) = default;

texture_t& texture_t::operator=(texture_t&& other) noexcept {
    if (this != &other) {
        m_bytes = std::move(other.m_bytes);
        m_format = other.m_format;
        m_width = std::exchange(other.m_width, std::size_t(0));
        m_height = std::exchange(other.m_height, std::size_t(0));
        other.m_bytes.clear();
    }

    return *this;
}

format_t texture_t::format() const noexcept {
    return m_format;
}

std::size_t texture_t::width() const noexcept {
    return m_width;
}

std::size_t texture_t::height() const noexcept {
    return m_height;
}

std::span<const std::byte> texture_t::bytes() const& noexcept {
    return m_bytes.bytes();
}

} // namespace m03gt0l0q3l4b1k27eab5k7py1_texture
