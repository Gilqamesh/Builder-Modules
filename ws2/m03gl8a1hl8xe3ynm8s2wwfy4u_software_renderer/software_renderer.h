#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H

# include "camera.h"
# include "render_item.h"

# include <m03gt1djvvy5atia5evkbg6rqy_software_shader/software_shader.h>

# include <cstddef>
# include <cstdint>
# include <format>
# include <memory>
# include <span>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

/**
 * @brief Represents one non-premultiplied eight-bit RGBA framebuffer pixel.
 */
struct rgba8_t {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;
};

static_assert(sizeof(rgba8_t) == 4);

/**
 * @brief Describes borrowed row-major, top-left-origin, non-premultiplied RGBA8 storage.
 *
 * Dimensions are non-negative, the pixel count equals `width * height`, and the
 * storage must outlive every renderer operation that uses it.
 */
struct framebuffer_t {
    std::span<rgba8_t> pixels;
    int width;
    int height;
};

/**
 * @brief Returns the checked pixel count for non-negative framebuffer dimensions.
 */
std::size_t framebuffer_pixel_count(int width, int height);

/**
 * @brief Renders camera-relative render items into a borrowed CPU framebuffer.
 */
class software_renderer_t {
public:
    explicit software_renderer_t(framebuffer_t framebuffer);
    ~software_renderer_t();

    software_renderer_t(const software_renderer_t&) = delete;
    software_renderer_t& operator=(const software_renderer_t&) = delete;
    software_renderer_t(software_renderer_t&&) = delete;
    software_renderer_t& operator=(software_renderer_t&&) = delete;

    void framebuffer(framebuffer_t framebuffer);
    framebuffer_t framebuffer() const noexcept;

    void clear(rgba8_t color);

    /**
     * @brief Draws a render item into the current non-empty framebuffer.
     */
    void draw(const camera_t<float, int, 2>& camera, const render_item_t& render_item);

private:
    void draw_pipeline(
        const m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t& program,
        const m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t& bindings,
        const geometry_t& geometry,
        const m03gt1djvvy5atia5evkbg6rqy_software_shader::shader::matrix_t<float, 4, 4>& object_to_world,
        const m03gt1djvvy5atia5evkbg6rqy_software_shader::shader::matrix_t<float, 4, 4>& world_to_clip
    );

private:
    class scratch_t;

    framebuffer_t m_framebuffer;
    std::unique_ptr<scratch_t> m_scratch;
};

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::rgba8_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::framebuffer_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::software_renderer_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::rgba8_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid rgba8_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::rgba8_t& color, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "red: {}", color.red);
        out = std::format_to(out, ", green: {}", color.green);
        out = std::format_to(out, ", blue: {}", color.blue);
        out = std::format_to(out, ", alpha: {}", color.alpha);
        out = std::format_to(out, " }}");

        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::framebuffer_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid framebuffer_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::framebuffer_t& framebuffer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "width: {}", framebuffer.width);
        out = std::format_to(out, ", height: {}", framebuffer.height);
        out = std::format_to(out, ", pixels: {}", framebuffer.pixels.size());
        out = std::format_to(out, " }}");

        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::software_renderer_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid software_renderer_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::software_renderer_t& renderer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "framebuffer: {}", renderer.framebuffer());
        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
