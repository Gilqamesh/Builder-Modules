#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H

# include "camera.h"
# include "render_item.h"

# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
# include <m03gt1djvvy5atia5evkbg6rqy_software_shader/software_shader.h>

# include <cstdint>
# include <format>
# include <memory>
# include <span>
# include <vector>

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
 * @brief Renders camera-relative render items into a CPU framebuffer and presents it to a window.
 */
class software_renderer_t {
public:
    explicit software_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    ~software_renderer_t();

    software_renderer_t(const software_renderer_t&) = delete;
    software_renderer_t& operator=(const software_renderer_t&) = delete;
    software_renderer_t(software_renderer_t&&) = delete;
    software_renderer_t& operator=(software_renderer_t&&) = delete;

    /**
     * @brief Begins and clears a frame after resizing the framebuffer to the window.
     *
     * @return Whether the framebuffer has a non-zero size.
     */
    bool begin_frame();
    bool begin_frame(rgba8_t clear_color);

    void draw(const camera_t<float, int, 2>& camera, const render_item_t<float, 2>& render_item);

    /**
     * @brief Presents an active frame and ends it, or does nothing when no frame is active.
     */
    void present();

    /**
     * @brief Returns borrowed row-major, top-left-origin framebuffer storage.
     *
     * The span may be invalidated when `begin_frame()` resizes the framebuffer.
     */
    std::span<rgba8_t> pixels() noexcept;
    std::span<const rgba8_t> pixels() const noexcept;

    int width() const noexcept;
    int height() const noexcept;

private:
    class presentation_t;

    void resize(int width, int height);

private:
    std::unique_ptr<presentation_t> m_presentation;
    m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t m_software_program;
    std::vector<rgba8_t> m_pixels;
    int m_width;
    int m_height;
    bool m_frame_active;
};

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::rgba8_t>;

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

        out = std::format_to(out, "{{ red: {}, green: {}, blue: {}, alpha: {} }}", color.red, color.green, color.blue, color.alpha);

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

        out = std::format_to(out, "{{ width: {}, height: {} }}", renderer.width(), renderer.height());

        return out;
    }
};

} // namespace std

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
