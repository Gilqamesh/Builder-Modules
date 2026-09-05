#include "rasterization.h"

#include <bit>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail {

static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);
static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53);
namespace {

void clear(clipping_buffer_t& buffer) {
    buffer.m_vertices.clear();
    buffer.m_values.clear();
}

void append_vertex(clipping_buffer_t& destination, const pipeline_vertex_view_t& source) {
    const auto offset = destination.m_values.size();
    destination.m_values.insert(destination.m_values.end(), source.m_outputs.begin(), source.m_outputs.end());
    destination.m_vertices.push_back({source.m_clip_position, {offset, source.m_outputs.size()}});
}

varying_t interpolate(const varying_t& from, const varying_t& to, double factor) {
    return std::visit([&](const auto& first) -> varying_t {
        using type_t = std::remove_cvref_t<decltype(first)>;
        const auto& second = std::get<type_t>(to);
        if constexpr (std::is_same_v<type_t, float>) {
            return float(std::lerp(double(first), double(second), factor));
        } else {
            type_t result;
            std::size_t i = 0;
            for (float& component : result) {
                component = float(std::lerp(double(first[i]), double(second[i]), factor));
                ++i;
            }
            return result;
        }
    }, from);
}

void append_intersection(clipping_buffer_t& destination, pipeline_vertex_view_t first, pipeline_vertex_view_t second, std::size_t plane) {
    if (std::lexicographical_compare(second.m_clip_position.begin(), second.m_clip_position.end(), first.m_clip_position.begin(), first.m_clip_position.end())) {
        std::swap(first, second);
    }
    unsigned pins = 1U << plane;
    for (std::size_t earlier = 0; earlier < plane; ++earlier) {
        if (clip_distance(first, earlier) == 0.0 && clip_distance(second, earlier) == 0.0) {
            pins |= 1U << earlier;
        }
    }
    double da = std::abs(clip_distance(first, plane));
    double db = std::abs(clip_distance(second, plane));
    if (db < da) {
        std::swap(first, second);
        std::swap(da, db);
    }
    // Ordering supplies direction independence; widening avoids float overflow.
    const double factor = da / (da + db);
    vector4f_t position;
    for (std::size_t i = 0; i < 4; ++i) {
        position[i] = float(std::lerp(double(first.m_clip_position[i]), double(second.m_clip_position[i]), factor));
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
        if ((pins & (3U << (2 * axis))) == (3U << (2 * axis))) {
            position[3] = 0.0F;
        }
    }
    if (1 <= plane && position[3] < 0.0F) {
        position[3] = 0.0F;
    }
    for (std::size_t constraint = 0; constraint <= plane; ++constraint) {
        if (pins & (1U << constraint)) {
            position[constraint / 2] = constraint % 2 == 0 ? -position[3] : position[3];
        }
    }
    // Repair narrowing against all processed half-spaces, never with an epsilon.
    for (std::size_t constraint = 0; constraint <= plane; ++constraint) {
        auto& component = position[constraint / 2];
        component = constraint % 2 == 0 ? std::max(component, -position[3]) : std::min(component, position[3]);
    }
    const auto offset = destination.m_values.size();
    if (first.m_outputs.size() != second.m_outputs.size()) {
        throw std::logic_error("clipping has inconsistent varying counts");
    }
    for (std::size_t i = 0; i < first.m_outputs.size(); ++i) {
        const auto& [location, from] = first.m_outputs[i];
        const auto& [other_location, to] = second.m_outputs[i];
        if (location != other_location || from.index() != to.index()) {
            throw std::logic_error("clipping has inconsistent varying locations or types");
        }
        destination.m_values.emplace_back(location, interpolate(from, to, factor));
    }
    destination.m_vertices.push_back({position, {offset, first.m_outputs.size()}});
}

bool between(grid_point_t p, grid_point_t a, grid_point_t b) {
    return std::min(a[0], b[0]) <= p[0] && p[0] <= std::max(a[0], b[0])
        && std::min(a[1], b[1]) <= p[1] && p[1] <= std::max(a[1], b[1]);
}

bool opposite(std::int64_t a, std::int64_t b) {
    return (a < 0 && 0 < b) || (b < 0 && 0 < a);
}

bool intersects(grid_point_t a, grid_point_t b, grid_point_t c, grid_point_t d) {
    const auto ac = edge(a, b, c), ad = edge(a, b, d);
    const auto ca = edge(c, d, a), cb = edge(c, d, b);
    return (opposite(ac, ad) && opposite(ca, cb))
        || (ac == 0 && between(c, a, b)) || (ad == 0 && between(d, a, b))
        || (ca == 0 && between(a, c, d)) || (cb == 0 && between(b, c, d));
}

bool simple_boundary(const raster_workspace_t& workspace) {
    const auto& ring = workspace.m_geometry;
    const auto point = [&](std::size_t i) { return workspace.m_vertices[ring[i % ring.size()]].m_point; };
    for (std::size_t i = 0; i < ring.size(); ++i) {
        const auto a = point(i), b = point(i + 1), c = point(i + 2);
        if (edge(a, b, c) == 0 && !between(b, a, c)) {
            return false;
        }
        for (std::size_t j = i + 2; j < ring.size(); ++j) {
            if (i == 0 && j + 1 == ring.size()) {
                continue;
            }
            if (intersects(a, b, point(j), point(j + 1))) {
                return false;
            }
        }
    }
    return true;
}

bool triangulate(raster_workspace_t& workspace) {
    auto& ring = workspace.m_ring;
    ring.resize(workspace.m_vertices.size());
    std::iota(ring.begin(), ring.end(), std::size_t(0));
    if (workspace.m_front_facing) {
        std::reverse(ring.begin(), ring.end());
    }
    const auto point = [&](std::size_t i) { return workspace.m_vertices[i].m_point; };
    std::size_t start = 0;
    bool found = false;
    for (std::size_t i = 0; i < ring.size(); ++i) {
        if (point(ring[i]) != point(ring[(i + ring.size() - 1) % ring.size()])
            && (!found || point(ring[i]) < point(ring[start]))) {
            start = i;
            found = true;
        }
    }
    std::rotate(ring.begin(), ring.begin() + start, ring.end());
    while (3 <= ring.size()) {
        std::optional<std::size_t> selected;
        for (std::size_t i = 0; i < ring.size(); ++i) {
            const triangle_t ids {ring[(i + ring.size() - 1) % ring.size()], ring[i], ring[(i + 1) % ring.size()]};
            const auto a = point(ids[0]), b = point(ids[1]), c = point(ids[2]);
            if (edge(a, b, c) <= 0) {
                continue;
            }
            const bool blocked = std::ranges::any_of(ring, [&](std::size_t other) {
                const auto p = point(other);
                return p != a && p != b && p != c
                    && 0 <= edge(a, b, p) && 0 <= edge(b, c, p) && 0 <= edge(c, a, p);
            });
            if (!blocked) {
                workspace.m_triangles.push_back(ids);
                selected = i;
                break;
            }
        }
        if (!selected) {
            for (std::size_t i = 0; i < ring.size(); ++i) {
                const auto a = point(ring[(i + ring.size() - 1) % ring.size()]);
                const auto b = point(ring[i]), c = point(ring[(i + 1) % ring.size()]);
                if (edge(a, b, c) == 0 && between(b, a, c)) {
                    selected = i;
                    break;
                }
            }
        }
        if (!selected) {
            workspace.m_triangles.clear();
            return false; // The complete boundary will use winding coverage.
        }
        ring.erase(ring.begin() + *selected);
    }
    return true;
}

bool crossed_facing(std::span<const projected_vertex_t> vertices) {
    const auto count = vertices.size();
    const auto at = [&](std::size_t start, bool reversed, std::size_t offset) {
        return vertices[(start + (reversed ? count - offset : offset)) % count].m_point;
    };
    std::size_t start = 0;
    bool reversed = false;
    for (bool candidate_reversed : {false, true}) {
        for (std::size_t candidate = 0; candidate < count; ++candidate) {
            for (std::size_t offset = 0; offset < count; ++offset) {
                const auto a = at(candidate, candidate_reversed, offset), b = at(start, reversed, offset);
                if (a != b) {
                    if (a < b) {
                        start = candidate;
                        reversed = candidate_reversed;
                    }
                    break;
                }
            }
        }
    }
    std::int64_t largest = 0;
    for (std::size_t i = 1; i + 1 < count; ++i) {
        const auto area = edge(at(start, reversed, 0), at(start, reversed, i), at(start, reversed, i + 1));
        if (std::abs(largest) < std::abs(area)) {
            largest = area;
        }
    }
    return (reversed ? -largest : largest) < 0;
}

int compare_varying(const varying_t& a, const varying_t& b) {
    if (a.index() != b.index()) {
        return a.index() < b.index() ? -1 : 1;
    }
    const auto bits_compare = [](float first, float second) {
        const auto x = std::bit_cast<std::uint32_t>(first), y = std::bit_cast<std::uint32_t>(second);
        return x == y ? 0 : (x < y ? -1 : 1);
    };
    return std::visit([&](const auto& first) {
        using type_t = std::remove_cvref_t<decltype(first)>;
        const auto& second = std::get<type_t>(b);
        if constexpr (std::is_same_v<type_t, float>) {
            return bits_compare(first, second);
        } else {
            std::size_t i = 0;
            for (float component : first) {
                const int comparison = bits_compare(component, second[i++]);
                if (comparison != 0) {
                    return comparison;
                }
            }
            return 0;
        }
    }, a);
}

int compare_record(const projected_vertex_t& a, const projected_vertex_t& b) {
    for (const auto& [x, y] : {std::pair(a.m_ndc_z, b.m_ndc_z), std::pair(a.m_reciprocal_w, b.m_reciprocal_w)}) {
        if (x != y) {
            return x < y ? -1 : 1;
        }
    }
    for (std::size_t i = 0; i < 4; ++i) {
        if (a.m_source.m_clip_position[i] != b.m_source.m_clip_position[i]) {
            return a.m_source.m_clip_position[i] < b.m_source.m_clip_position[i] ? -1 : 1;
        }
    }
    const auto& first = a.m_source.m_outputs;
    const auto& second = b.m_source.m_outputs;
    for (std::size_t i = 0; i < std::min(first.size(), second.size()); ++i) {
        if (first[i].first != second[i].first) {
            return first[i].first < second[i].first ? -1 : 1;
        }
        const int comparison = compare_varying(first[i].second, second[i].second);
        if (comparison != 0) {
            return comparison;
        }
    }
    return first.size() == second.size() ? 0 : (first.size() < second.size() ? -1 : 1);
}

bool edge_record_less(const scan_event_t& a, const scan_event_t& b, std::span<const projected_vertex_t> vertices) {
    for (const auto& [first, second] : {std::pair(a.m_lower, b.m_lower), std::pair(a.m_upper, b.m_upper)}) {
        if (vertices[first].m_point != vertices[second].m_point) {
            return vertices[first].m_point < vertices[second].m_point;
        }
    }
    const int lower = compare_record(vertices[a.m_lower], vertices[b.m_lower]);
    return lower != 0 ? lower < 0 : compare_record(vertices[a.m_upper], vertices[b.m_upper]) < 0;
}

} // namespace

pipeline_vertex_view_t view(const pipeline_vertex_t& vertex, const varying_values_t& values) {
    return {vertex.m_clip_position, std::span<const varying_entry_t>(values).subspan(vertex.m_outputs[0], vertex.m_outputs[1])};
}

double clip_distance(const pipeline_vertex_view_t& vertex, std::size_t plane) {
    const double component = vertex.m_clip_position[plane / 2];
    const double w = vertex.m_clip_position[3];
    return plane % 2 == 0 ? component + w : w - component;
}

bool inside_clip_volume(const pipeline_vertex_view_t& vertex) {
    for (std::size_t plane = 0; plane < 6; ++plane) {
        if (clip_distance(vertex, plane) < 0.0) {
            return false;
        }
    }
    return vertex.m_clip_position[3] != 0.0F;
}

std::optional<std::size_t> clip_line(const pipeline_vertex_view_t& first, const pipeline_vertex_view_t& second, clipping_workspace_t& workspace) {
    clear(workspace.m_buffers[0]);
    append_vertex(workspace.m_buffers[0], first);
    append_vertex(workspace.m_buffers[0], second);
    std::size_t source_index = 0;
    for (std::size_t plane = 0; plane < 6; ++plane) {
        auto& source = workspace.m_buffers[source_index];
        auto& destination = workspace.m_buffers[1 - source_index];
        clear(destination);
        const auto from = view(source.m_vertices[0], source.m_values), to = view(source.m_vertices[1], source.m_values);
        const double da = clip_distance(from, plane), db = clip_distance(to, plane);
        if (da < 0.0 && db < 0.0) {
            return std::nullopt;
        }
        if (0.0 <= da && 0.0 <= db) {
            append_vertex(destination, from);
            append_vertex(destination, to);
        } else if (da == 0.0 || db == 0.0) {
            const auto& on_plane = da == 0.0 ? from : to;
            append_vertex(destination, on_plane);
            append_vertex(destination, on_plane);
        } else if (da < 0.0) {
            append_intersection(destination, from, to, plane);
            append_vertex(destination, to);
        } else {
            append_vertex(destination, from);
            append_intersection(destination, from, to, plane);
        }
        source_index = 1 - source_index;
    }
    for (const auto& vertex : workspace.m_buffers[source_index].m_vertices) {
        if (vertex.m_clip_position[3] == 0.0F) {
            return std::nullopt;
        }
    }
    return source_index;
}

std::optional<std::size_t> clip_triangle(const pipeline_vertex_view_t& first, const pipeline_vertex_view_t& second, const pipeline_vertex_view_t& third, clipping_workspace_t& workspace) {
    clear(workspace.m_buffers[0]);
    append_vertex(workspace.m_buffers[0], first);
    append_vertex(workspace.m_buffers[0], second);
    append_vertex(workspace.m_buffers[0], third);
    std::size_t source_index = 0;
    for (std::size_t plane = 0; plane < 6 && !workspace.m_buffers[source_index].m_vertices.empty(); ++plane) {
        auto& source = workspace.m_buffers[source_index];
        auto& destination = workspace.m_buffers[1 - source_index];
        clear(destination);
        auto previous = view(source.m_vertices.back(), source.m_values);
        double dp = clip_distance(previous, plane);
        for (const auto& vertex : source.m_vertices) {
            const auto current = view(vertex, source.m_values);
            const double dc = clip_distance(current, plane);
            if (0.0 <= dc) {
                if (dp < 0.0 && 0.0 < dc) {
                    append_intersection(destination, previous, current, plane);
                }
                append_vertex(destination, current);
            } else if (0.0 < dp) {
                append_intersection(destination, previous, current, plane);
            }
            previous = current;
            dp = dc;
        }
        source_index = 1 - source_index;
    }
    const auto& vertices = workspace.m_buffers[source_index].m_vertices;
    if (vertices.empty() || std::ranges::any_of(vertices, [](const auto& vertex) { return vertex.m_clip_position[3] == 0.0F; })) {
        return std::nullopt;
    }
    return source_index;
}

double projectable_reciprocal_w(float w) {
    const double reciprocal = 1.0 / double(w);
    if (!(0.0F < w) || !std::isfinite(reciprocal) || double(std::numeric_limits<float>::max()) < reciprocal) {
        throw std::out_of_range(std::format("software renderer cannot project W={} into its float fragment coordinates", w));
    }
    return reciprocal;
}

std::int64_t snap(double screen, int extent) {
    const double scaled = screen * double(subpixels);
    if (!std::isfinite(scaled) || scaled < 0.0 || double(extent) * double(subpixels) < scaled) {
        throw std::out_of_range("software renderer projected position is outside its subpixel grid");
    }
    const double lower = std::floor(scaled);
    return std::int64_t(lower) + (0.5 <= scaled - lower ? 1 : 0);
}

std::int64_t edge(grid_point_t a, grid_point_t b, grid_point_t p) {
    // All three points are in [0,2^31]^2: each product AND the determinant
    // are bounded by 2^62. Do not replace this with a polygon-area sum.
    return (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0]);
}

std::int64_t ceil_div(std::int64_t numerator, std::int64_t denominator) {
    return numerator / denominator + (0 < numerator % denominator ? 1 : 0);
}

int compare_fraction(fraction_t a, fraction_t b) {
    // Quotient/remainder comparison avoids the ~93-bit cross products of n/d.
    // Reciprocating the positive remainders reverses their order.
    int sign = 1;
    for (;;) {
        const auto qa = a[0] / a[1], qb = b[0] / b[1];
        if (qa != qb) {
            return sign * (qa < qb ? -1 : 1);
        }
        const auto ra = a[0] % a[1], rb = b[0] % b[1];
        if (ra == 0 || rb == 0) {
            return sign * (ra == rb ? 0 : (ra == 0 ? -1 : 1));
        }
        a = {a[1], ra};
        b = {b[1], rb};
        sign = -sign;
    }
}

int sample_bound(fraction_t crossing, int extent) {
    const auto bound = ceil_div(crossing[0] - center_offset * crossing[1], subpixels * crossing[1]);
    return int(std::clamp<std::int64_t>(bound, 0, extent));
}

void prepare_polygon(raster_workspace_t& workspace) {
    workspace.m_empty = true;
    workspace.m_use_triangles = false;
    workspace.m_front_facing = false;
    workspace.m_triangles.clear();
    auto& geometry = workspace.m_geometry;
    geometry.clear();
    const auto& vertices = workspace.m_vertices;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        if (geometry.empty() || vertices[geometry.back()].m_point != vertices[i].m_point) {
            geometry.push_back(i);
        }
    }
    if (1 < geometry.size() && vertices[geometry.front()].m_point == vertices[geometry.back()].m_point) {
        geometry.pop_back();
    }
    if (geometry.size() < 3 || std::ranges::all_of(geometry, [&](std::size_t i) {
        return edge(vertices[geometry[0]].m_point, vertices[geometry[1]].m_point, vertices[i].m_point) == 0;
    })) {
        return;
    }
    workspace.m_empty = false;
    if (simple_boundary(workspace)) {
        const auto minimum = std::min_element(geometry.begin(), geometry.end(), [&](std::size_t a, std::size_t b) { return vertices[a].m_point < vertices[b].m_point; });
        const auto i = std::size_t(minimum - geometry.begin());
        const auto a = vertices[geometry[(i + geometry.size() - 1) % geometry.size()]].m_point;
        const auto b = vertices[*minimum].m_point;
        const auto c = vertices[geometry[(i + 1) % geometry.size()]].m_point;
        workspace.m_front_facing = edge(a, b, c) < 0;
        workspace.m_use_triangles = triangulate(workspace);
    } else {
        workspace.m_front_facing = crossed_facing(vertices);
    }
}

void prepare_triangle(const pipeline_vertex_view_t& first, const pipeline_vertex_view_t& second, const pipeline_vertex_view_t& third, int width, int height, raster_workspace_t& workspace) {
    workspace.m_empty = true;
    workspace.m_vertices.clear();
    const auto clipped = clip_triangle(first, second, third, workspace.m_clipping);
    if (!clipped) {
        return;
    }
    const auto& polygon = workspace.m_clipping.m_buffers[*clipped];
    for (const auto& vertex : polygon.m_vertices) {
        const auto source = view(vertex, polygon.m_values);
        const auto& p = source.m_clip_position;
        const double reciprocal = projectable_reciprocal_w(p[3]);
        const double ndc_x = double(p[0]) / double(p[3]);
        const double ndc_y = double(p[1]) / double(p[3]);
        workspace.m_vertices.push_back({
            {snap((ndc_x + 1.0) * (double(width) / 2.0), width), snap((1.0 - ndc_y) * (double(height) / 2.0), height)},
            double(p[2]) / double(p[3]), reciprocal, source
        });
    }
    prepare_polygon(workspace);
}

void scanline_events(std::span<const projected_vertex_t> vertices, std::int64_t y, std::vector<scan_event_t>& events) {
    events.clear();
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        std::size_t lower = i, upper = (i + 1) % vertices.size();
        if (vertices[lower].m_point[1] == vertices[upper].m_point[1]) {
            continue;
        }
        const int delta = vertices[lower].m_point[1] < vertices[upper].m_point[1] ? 1 : -1;
        if (delta < 0) {
            std::swap(lower, upper);
        }
        const auto a = vertices[lower].m_point, b = vertices[upper].m_point;
        if (a[1] <= y && y < b[1]) {
            const auto denominator = b[1] - a[1];
            const auto numerator = a[0] * (b[1] - y) + b[0] * (y - a[1]);
            events.push_back({{numerator, denominator}, lower, upper, delta});
        }
    }
    std::sort(events.begin(), events.end(), [&](const auto& a, const auto& b) {
        const int order = compare_fraction(a.m_x, b.m_x);
        return order != 0 ? order < 0 : edge_record_less(a, b, vertices);
    });
}

sample_t span_sample(const scan_event_t& left, const scan_event_t& right, std::span<const projected_vertex_t> vertices, int x, int y) {
    const auto px = std::int64_t(x) * subpixels + center_offset;
    const auto py = std::int64_t(y) * subpixels + center_offset;
    // Exact residuals survive even when two rational crossings round to the
    // same double screen coordinate. A covered sample has dl>=0 and dr>0.
    const double dl = double(px * left.m_x[1] - left.m_x[0]) / double(left.m_x[1]);
    const double dr = double(right.m_x[0] - px * right.m_x[1]) / double(right.m_x[1]);
    const double across = dl / (dl + dr);
    const double along_left = double(py - vertices[left.m_lower].m_point[1]) / double(left.m_x[1]);
    const double along_right = double(py - vertices[right.m_lower].m_point[1]) / double(right.m_x[1]);
    return {x, y, {left.m_lower, left.m_upper, right.m_lower, right.m_upper},
        {(1.0 - across) * (1.0 - along_left), (1.0 - across) * along_left,
         across * (1.0 - along_right), across * along_right}, 4};
}

std::array<double, 2> interpolate_sample(std::span<const projected_vertex_t> vertices, const sample_t& sample, varying_values_t& outputs) {
    double reciprocal = 0.0, ndc_z = 0.0;
    double minimum_q = std::numeric_limits<double>::infinity(), maximum_q = 0.0;
    double minimum_z = 1.0, maximum_z = -1.0;
    for (std::size_t i = 0; i < sample.m_count; ++i) {
        const auto& vertex = vertices[sample.m_vertices[i]];
        reciprocal += sample.m_weights[i] * vertex.m_reciprocal_w;
        ndc_z += sample.m_weights[i] * vertex.m_ndc_z;
        minimum_q = std::min(minimum_q, vertex.m_reciprocal_w);
        maximum_q = std::max(maximum_q, vertex.m_reciprocal_w);
        minimum_z = std::min(minimum_z, vertex.m_ndc_z);
        maximum_z = std::max(maximum_z, vertex.m_ndc_z);
    }
    reciprocal = std::clamp(reciprocal, minimum_q, maximum_q);
    ndc_z = std::clamp(ndc_z, minimum_z, maximum_z);
    outputs.clear();
    const auto first_outputs = vertices[sample.m_vertices[0]].m_source.m_outputs;
    for (std::size_t output = 0; output < first_outputs.size(); ++output) {
        const auto& [location, first] = first_outputs[output];
        const auto interpolated = std::visit([&](const auto& first_value) -> varying_t {
            using type_t = std::remove_cvref_t<decltype(first_value)>;
            const auto component = [&](std::size_t axis) {
                double numerator = 0.0;
                double minimum = std::numeric_limits<double>::infinity(), maximum = -minimum;
                for (std::size_t i = 0; i < sample.m_count; ++i) {
                    const auto& vertex = vertices[sample.m_vertices[i]];
                    if (vertex.m_source.m_outputs.size() != first_outputs.size() || vertex.m_source.m_outputs[output].first != location) {
                        throw std::logic_error("rasterization has inconsistent varying counts or locations");
                    }
                    const auto& typed = std::get<type_t>(vertex.m_source.m_outputs[output].second);
                    const double value = [&] {
                        if constexpr (std::is_same_v<type_t, float>) { return double(typed); }
                        else { return double(typed[axis]); }
                    }();
                    numerator += sample.m_weights[i] * (vertex.m_reciprocal_w * value);
                    minimum = std::min(minimum, value);
                    maximum = std::max(maximum, value);
                }
                const double result = numerator / reciprocal;
                return float(minimum <= maximum ? std::clamp(result, minimum, maximum) : result);
            };
            if constexpr (std::is_same_v<type_t, float>) {
                return component(0);
            } else {
                type_t result;
                std::size_t i = 0;
                for (float& value : result) {
                    value = component(i++);
                }
                return result;
            }
        }, first);
        outputs.emplace_back(location, interpolated);
    }
    return {ndc_z * 0.5 + 0.5, reciprocal};
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::detail
