#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RASTERIZATION_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RASTERIZATION_H

# include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>

# include <algorithm>
# include <array>
# include <cstddef>
# include <cstdint>
# include <format>
# include <optional>
# include <span>
# include <utility>
# include <variant>
# include <vector>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail {

using vector2f_t = m03gsy25j4v7nccgmsdov9ioft_shader::vector_t<float, 2>;
using vector3f_t = m03gsy25j4v7nccgmsdov9ioft_shader::vector_t<float, 3>;
using vector4f_t = m03gsy25j4v7nccgmsdov9ioft_shader::vector_t<float, 4>;
using varying_t = std::variant<float, vector2f_t, vector3f_t, vector4f_t>;
using varying_entry_t = std::pair<std::uint32_t, varying_t>;
using varying_values_t = std::vector<varying_entry_t>;
using grid_point_t = std::array<std::int64_t, 2>;
using triangle_t = std::array<std::size_t, 3>;
// Unreduced nonnegative numerator and positive denominator.
using fraction_t = std::array<std::int64_t, 2>;

constexpr int maximum_extent = 1 << 23;
constexpr std::int64_t subpixels = 256;
constexpr std::int64_t center_offset = 128;

struct pipeline_vertex_t {
    vector4f_t m_clip_position;
    std::array<std::size_t, 2> m_outputs; // Offset and count in the owning value buffer.
};

struct pipeline_vertex_view_t {
    vector4f_t m_clip_position;
    std::span<const varying_entry_t> m_outputs;
};

struct clipping_buffer_t {
    std::vector<pipeline_vertex_t> m_vertices;
    varying_values_t m_values;
};

struct clipping_workspace_t {
    std::array<clipping_buffer_t, 2> m_buffers;
};

struct projected_vertex_t {
    grid_point_t m_point;
    double m_ndc_z;
    double m_reciprocal_w;
    pipeline_vertex_view_t m_source;
};

struct scan_event_t {
    fraction_t m_x;
    std::size_t m_lower;
    std::size_t m_upper;
    int m_delta;
};

struct sample_t {
    int m_x;
    int m_y;
    std::array<std::size_t, 4> m_vertices;
    std::array<double, 4> m_weights;
    std::size_t m_count;
};

struct raster_workspace_t {
    clipping_workspace_t m_clipping;
    std::vector<projected_vertex_t> m_vertices;
    std::vector<std::size_t> m_geometry;
    std::vector<std::size_t> m_ring;
    std::vector<triangle_t> m_triangles;
    std::vector<scan_event_t> m_events;
    bool m_empty = true;
    bool m_use_triangles = false;
    bool m_front_facing = false;
};

pipeline_vertex_view_t view(const pipeline_vertex_t&, const varying_values_t&);
double clip_distance(const pipeline_vertex_view_t&, std::size_t plane);
bool inside_clip_volume(const pipeline_vertex_view_t&);
std::optional<std::size_t> clip_line(const pipeline_vertex_view_t&, const pipeline_vertex_view_t&, clipping_workspace_t&);
std::optional<std::size_t> clip_triangle(const pipeline_vertex_view_t&, const pipeline_vertex_view_t&, const pipeline_vertex_view_t&, clipping_workspace_t&);
double projectable_reciprocal_w(float w);
std::int64_t snap(double screen, int extent);
std::int64_t edge(grid_point_t a, grid_point_t b, grid_point_t p);
std::int64_t ceil_div(std::int64_t numerator, std::int64_t denominator);
int compare_fraction(fraction_t a, fraction_t b);
int sample_bound(fraction_t crossing, int extent);
void prepare_polygon(raster_workspace_t&);
void prepare_triangle(const pipeline_vertex_view_t&, const pipeline_vertex_view_t&, const pipeline_vertex_view_t&, int width, int height, raster_workspace_t&);
void scanline_events(std::span<const projected_vertex_t>, std::int64_t y, std::vector<scan_event_t>&);
sample_t span_sample(const scan_event_t& left, const scan_event_t& right, std::span<const projected_vertex_t>, int x, int y);
// Returns window depth and reciprocal W, and writes perspective-correct varyings.
std::array<double, 2> interpolate_sample(std::span<const projected_vertex_t>, const sample_t&, varying_values_t&);

// The same pre-shading coverage events are consumed by the renderer and tests.
template <typename Emit>
void visit_samples(raster_workspace_t&, int width, int height, Emit&&);

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_view_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_buffer_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_workspace_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::projected_vertex_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::scan_event_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::sample_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::raster_workspace_t>;

} // namespace std

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail {

template <typename Emit>
void visit_samples(raster_workspace_t& workspace, int width, int height, Emit&& emit) {
    if (workspace.m_empty) {
        return;
    }
    const auto& vertices = workspace.m_vertices;
    if (workspace.m_use_triangles) {
        for (const auto& indices : workspace.m_triangles) {
            const auto a = vertices[indices[0]].m_point;
            const auto b = vertices[indices[1]].m_point;
            const auto c = vertices[indices[2]].m_point;
            const auto area = edge(a, b, c);
            const auto inclusive = [](grid_point_t from, grid_point_t to) {
                return to[1] < from[1] || (to[1] == from[1] && from[0] < to[0]);
            };
            const std::array top_left {inclusive(b, c), inclusive(c, a), inclusive(a, b)};
            const int first_x = sample_bound({std::min({a[0], b[0], c[0]}), 1}, width);
            const int end_x = sample_bound({std::max({a[0], b[0], c[0]}) + 1, 1}, width);
            const int first_y = sample_bound({std::min({a[1], b[1], c[1]}), 1}, height);
            const int end_y = sample_bound({std::max({a[1], b[1], c[1]}) + 1, 1}, height);
            for (int y = first_y; y < end_y; ++y) {
                for (int x = first_x; x < end_x; ++x) {
                    const grid_point_t p {std::int64_t(x) * subpixels + center_offset, std::int64_t(y) * subpixels + center_offset};
                    const std::array values {edge(b, c, p), edge(c, a, p), edge(a, b, p)};
                    bool covered = true;
                    for (std::size_t i = 0; i < 3; ++i) {
                        covered = covered && (0 < values[i] || (values[i] == 0 && top_left[i]));
                    }
                    if (covered) {
                        emit(sample_t {x, y, {indices[0], indices[1], indices[2], 0},
                            {double(values[0]) / double(area), double(values[1]) / double(area), double(values[2]) / double(area), 0.0}, 3});
                    }
                }
            }
        }
        return;
    }

    const auto [minimum, maximum] = std::ranges::minmax_element(vertices, {}, [](const auto& vertex) { return vertex.m_point[1]; });
    const int first_y = sample_bound({minimum->m_point[1], 1}, height);
    const int end_y = sample_bound({maximum->m_point[1], 1}, height);
    for (int y = first_y; y < end_y; ++y) {
        scanline_events(vertices, std::int64_t(y) * subpixels + center_offset, workspace.m_events);
        const auto& events = workspace.m_events;
        int winding = 0;
        scan_event_t left {};
        for (std::size_t begin = 0; begin < events.size();) {
            std::size_t end = begin;
            int delta = 0;
            do {
                delta += events[end++].m_delta;
            } while (end < events.size() && compare_fraction(events[begin].m_x, events[end].m_x) == 0);
            const int after = winding + delta;
            if ((winding == 0) != (after == 0)) {
                std::size_t selected = begin;
                while ((events[selected].m_delta < 0) != (delta < 0)) {
                    ++selected;
                }
                const auto& boundary = events[selected];
                if (winding == 0) {
                    left = boundary;
                } else {
                    const int first_x = sample_bound(left.m_x, width);
                    const int end_x = sample_bound(boundary.m_x, width);
                    for (int x = first_x; x < end_x; ++x) {
                        emit(span_sample(left, boundary, vertices, x, y));
                    }
                }
            }
            winding = after;
            begin = end;
        }
    }
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "position: {}", v.m_clip_position);
        out = std::format_to(out, ", offset: {}", v.m_outputs[0]);
        out = std::format_to(out, ", count: {}", v.m_outputs[1]);
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_view_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::pipeline_vertex_view_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "position: {}", v.m_clip_position);
        out = std::format_to(out, ", outputs: {}", v.m_outputs.size());
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_buffer_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_buffer_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "vertices: {}", v.m_vertices.size());
        out = std::format_to(out, ", values: {}", v.m_values.size());
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_workspace_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::clipping_workspace_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "first: {}", v.m_buffers[0]);
        out = std::format_to(out, ", second: {}", v.m_buffers[1]);
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::projected_vertex_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::projected_vertex_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "x: {}", v.m_point[0]);
        out = std::format_to(out, ", y: {}", v.m_point[1]);
        out = std::format_to(out, ", ndc_z: {}", v.m_ndc_z);
        out = std::format_to(out, ", reciprocal_w: {}", v.m_reciprocal_w);
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::scan_event_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::scan_event_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "numerator: {}", v.m_x[0]);
        out = std::format_to(out, ", denominator: {}", v.m_x[1]);
        out = std::format_to(out, ", lower: {}", v.m_lower);
        out = std::format_to(out, ", upper: {}", v.m_upper);
        out = std::format_to(out, ", delta: {}", v.m_delta);
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::sample_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::sample_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "x: {}", v.m_x);
        out = std::format_to(out, ", y: {}", v.m_y);
        out = std::format_to(out, ", vertices: {}", v.m_count);
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::raster_workspace_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail::raster_workspace_t& v, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "vertices: {}", v.m_vertices.size());
        out = std::format_to(out, ", triangles: {}", v.m_triangles.size());
        out = std::format_to(out, ", empty: {}", v.m_empty);
        out = std::format_to(out, ", front_facing: {}", v.m_front_facing);
        out = std::format_to(out, " }}");
        return out;
    }
};

} // namespace std

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RASTERIZATION_H
