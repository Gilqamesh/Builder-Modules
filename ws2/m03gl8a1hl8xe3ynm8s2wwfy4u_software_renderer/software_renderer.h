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
     *
     * Each dimension must be at most 2^23. Vertex positions are finite homogeneous
     * clip coordinates, clipped in X, Y, Z to [-W,W]. A surviving zero-W vertex
     * makes its primitive empty; positive W must have a reciprocal representable
     * by the float fragment-coordinate interface. Unsupported dimensions and W,
     * and non-finite shader positions, are rejected.
     *
     * Triangle X/Y positions are projected and rounded once to a 1/256-pixel grid;
     * half-grid ties go toward the greater coordinate. Samples are pixel centers
     * (x+0.5,y+0.5). Coverage is the nonzero winding fill of the snapped boundary,
     * with top/left inclusion: equivalently, classify the sample infinitesimally
     * to the right, then infinitesimally below. Each covered sample is shaded once
     * per original triangle. Collapsed or cancelling boundaries emit no fragments;
     * snapped crossings, touches and overlaps are handled without geometry errors.
     * Matching shared boundaries with filled interiors on opposite sides have
     * complementary sample ownership; overlapping interiors of separate primitives
     * retain their independent coverage.
     *
     * Simple polygons use deterministic ears: normalize screen winding, start at
     * the least (X,Y), and remove the first unblocked convex ear. Collinear and
     * coincident occurrences retain their own payloads; a zero-area occurrence
     * may contribute no samples. Non-simple polygons use winding scanline spans,
     * interpolating reciprocal W, Z/W and varying/W along their boundary edges and
     * across each span. Equal crossing positions choose the least endpoint-record
     * key with the net crossing direction (geometry, projection, then payload bits).
     * Both paths divide interpolated varying/W by interpolated reciprocal W.
     * Interpolation describes the snapped geometry and preserves primitive-local
     * payloads; distinct coincident values can cause interpolation discontinuities.
     *
     * Every generated piece has one original-primitive facing value. For a simple
     * snapped polygon, CCW NDC (negative screen winding) is front-facing. A non-simple
     * boundary uses the largest absolute fan determinant of its least cyclic grid
     * sequence over both directions, first on ties, with submitted direction restored.
     * Fragment Z is (interpolated Z/W+1)/2 clamped to [0,1]; fragment W is reciprocal W.
     * There is no depth test, blending or culling. Point/line coverage and the
     * established 2D camera and T*R*S transforms are preserved.
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
