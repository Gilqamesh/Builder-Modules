#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TEXTURE_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TEXTURE_H

# include <cstddef>
# include <format>
# include <stdexcept>

# include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

/**
 * @brief Texture texel format.
 *
 * Names use `<channels>_<type><bits>[_<interpretation>]`:
 * - `RGBA`: red, green, blue and alpha channels.
 * - `U8`: 8-bit unsigned integer channels.
 * - `F16` / `F32`: 16-bit / 32-bit floating-point channels.
 * - `NORMALIZED`: integers interpreted in the range [0, 1].
 * - `SRGB`: sRGB-encoded RGB with linear alpha.
 */
enum class texture_format_t {
    RGBA_U8_NORMALIZED,
    RGBA_U8_SRGB,
    RGBA_F16,
    RGBA_F32
};

size_t texture_format_bytes_per_texel(texture_format_t format);

class texture_t {
public:
    texture_t();
    texture_t(texture_format_t format, size_t width, size_t height, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t bytes);

    texture_format_t format() const;
    size_t width() const;
    size_t height() const;
    const m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t& bytes() const;

private:
    texture_format_t m_format;
    size_t m_width;
    size_t m_height;
    m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t m_bytes;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t>;

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid texture_format_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t& format, auto& ctx) const {
        auto out = ctx.out();

        switch (format) {
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t::RGBA_U8_NORMALIZED: {
                out = std::format_to(out, "RGBA_U8_NORMALIZED");
            } break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t::RGBA_U8_SRGB: {
                out = std::format_to(out, "RGBA_U8_SRGB");
            } break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t::RGBA_F16: {
                out = std::format_to(out, "RGBA_F16");
            } break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_format_t::RGBA_F32: {
                out = std::format_to(out, "RGBA_F32");
            } break;
            default: {
                throw std::runtime_error("formatter<texture_format_t>::format: unknown texture format");
            } break;
        }

        return out;
    }
};

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid texture_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::texture_t& texture, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "format: {}, width: {}, height: {}, bytes: {}", texture.format(), texture.width(), texture.height(), texture.bytes());

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TEXTURE_H
