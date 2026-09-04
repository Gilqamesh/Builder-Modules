#include "renderer2.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

namespace tower_defense_api = m03gilsfsv3k34ej14ytz8a29k_tower_defense_game;
namespace vector_api = m03ginwy24ng8o487c4beoms6l_vector;
namespace hyperrectangle_api = m03gintxczohr63y44o77b4pyj_hyperrectangle;
namespace software_renderer_api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;

using rgba8_t = software_renderer_api::rgba8_t;
using renderer2_color_t = tower_defense_api::renderer2_color_t;
using vector2f_t = vector_api::vector_t<float, 2>;
using vector2i_t = vector_api::vector_t<int, 2>;

constexpr renderer2_color_t ray_white_color() noexcept {
    return { 245, 245, 245, 255 };
}

constexpr rgba8_t green_color() noexcept {
    return { 0, 228, 48, 255 };
}

rgba8_t to_rgba8(renderer2_color_t color) noexcept {
    return {
        .red = color.red,
        .green = color.green,
        .blue = color.blue,
        .alpha = color.alpha
    };
}

struct framebuffer_view_t {
    int width;
    int height;
    std::span<rgba8_t> pixels;
};

void set_pixel(framebuffer_view_t framebuffer, int x, int y, rgba8_t color) noexcept {
    if (x < 0 || y < 0 || x >= framebuffer.width || y >= framebuffer.height) {
        return;
    }

    framebuffer.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(framebuffer.width) + static_cast<std::size_t>(x)] = color;
}

void draw_filled_circle(framebuffer_view_t framebuffer, vector2i_t center, int radius, rgba8_t color) {
    const int radius_squared = radius * radius;

    for (int y = center[1] - radius; y <= center[1] + radius; ++y) {
        for (int x = center[0] - radius; x <= center[0] + radius; ++x) {
            const int dx = x - center[0];
            const int dy = y - center[1];
            if (dx * dx + dy * dy <= radius_squared) {
                set_pixel(framebuffer, x, y, color);
            }
        }
    }
}

void draw_line(framebuffer_view_t framebuffer, vector2i_t from, vector2i_t to, rgba8_t color) {
    int x0 = from[0];
    int y0 = from[1];
    const int x1 = to[0];
    const int y1 = to[1];

    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        set_pixel(framebuffer, x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int twice_error = error * 2;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

float edge_function(vector2i_t a, vector2i_t b, float x, float y) noexcept {
    return (x - static_cast<float>(a[0])) * static_cast<float>(b[1] - a[1])
        - (y - static_cast<float>(a[1])) * static_cast<float>(b[0] - a[0]);
}

void draw_filled_triangle(framebuffer_view_t framebuffer, vector2i_t p0, vector2i_t p1, vector2i_t p2, rgba8_t color) {
    const int min_x = std::max(0, std::min({ p0[0], p1[0], p2[0] }));
    const int max_x = std::min(framebuffer.width - 1, std::max({ p0[0], p1[0], p2[0] }));
    const int min_y = std::max(0, std::min({ p0[1], p1[1], p2[1] }));
    const int max_y = std::min(framebuffer.height - 1, std::max({ p0[1], p1[1], p2[1] }));

    if (min_x > max_x || min_y > max_y) {
        return;
    }

    const float area = edge_function(p0, p1, static_cast<float>(p2[0]), static_cast<float>(p2[1]));
    if (area == 0.0f) {
        return;
    }

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const float sample_x = static_cast<float>(x) + 0.5f;
            const float sample_y = static_cast<float>(y) + 0.5f;
            const float edge0 = edge_function(p1, p2, sample_x, sample_y);
            const float edge1 = edge_function(p2, p0, sample_x, sample_y);
            const float edge2 = edge_function(p0, p1, sample_x, sample_y);

            if ((edge0 >= 0.0f && edge1 >= 0.0f && edge2 >= 0.0f) || (edge0 <= 0.0f && edge1 <= 0.0f && edge2 <= 0.0f)) {
                set_pixel(framebuffer, x, y, color);
            }
        }
    }
}

vector2f_t read_position(
    const m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t& vertex_stream,
    std::uint32_t index
) {
    std::array<float, 2> values;
    const auto bytes = vertex_stream.data();
    const std::size_t offset = static_cast<std::size_t>(index) * sizeof(values);
    std::memcpy(values.data(), bytes.data() + offset, sizeof(values));
    return { values[0], values[1] };
}

vector2i_t transform_position(
    const tower_defense_api::camera_t<float, int, 2>& camera,
    vector2f_t position,
    vector2f_t translation,
    vector2f_t scale
) {
    return camera.to_view(position * scale + translation);
}

bool point_intersects_camera_view(
    const tower_defense_api::camera_t<float, int, 2>& camera,
    vector2i_t point
) {
    hyperrectangle_api::hyperrectangle_t<int, 2> point_rect({
        { point[0], point[0] + 1 },
        { point[1], point[1] + 1 }
    });
    return !point_rect.intersect(camera.view_rect()).is_empty();
}

void require_supported_texture_format(tower_defense_api::texture_format_t format) {
    switch (format) {
        case tower_defense_api::texture_format_t::RGBA_U8_NORMALIZED:
        case tower_defense_api::texture_format_t::RGBA_U8_SRGB:
        case tower_defense_api::texture_format_t::RGBA_F16:
        case tower_defense_api::texture_format_t::RGBA_F32: {
            return;
        } break;
        default: {
            throw std::runtime_error(std::format(
                "renderer2_t::draw: does not support texture format {}",
                static_cast<int>(format)
            ));
        } break;
    }
}

struct render_item_data_t {
    const m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t* vertex_stream;
    const std::vector<std::uint32_t>* indices;
    tower_defense_api::vertex_primitive_topology_t primitive_topology;
    vector2f_t translation;
    vector2f_t scale;
};

render_item_data_t validate_render_item(const tower_defense_api::render_item_t<float, 2>& render_item) {
    const auto geometry = render_item.geometry();
    if (!geometry) {
        throw std::runtime_error("renderer2_t::draw: does not support a render item with no geometry");
    }

    const auto mesh = geometry->mesh();
    if (!mesh) {
        throw std::runtime_error("renderer2_t::draw: does not support geometry with no mesh");
    }

    const auto index_buffer = geometry->index_buffer();
    if (!index_buffer) {
        throw std::runtime_error("renderer2_t::draw: does not support geometry with no index buffer");
    }

    const auto material = render_item.material();
    if (!material) {
        throw std::runtime_error("renderer2_t::draw: does not support a render item with no material");
    }

    const auto& texture_bindings = material->texture_bindings();
    if (texture_bindings.empty()) {
        throw std::runtime_error("renderer2_t::draw: does not support entity_material with no texture_bindings");
    }

    const auto& texture_binding = texture_bindings[0];
    if (!texture_binding.texture) {
        throw std::runtime_error("renderer2_t::draw: does not support entity_material with no texture in first texture_binding");
    }
    require_supported_texture_format(texture_binding.texture->format());

    const auto& vertex_streams = mesh->vertex_streams();
    if (vertex_streams.size() == 0) {
        throw std::runtime_error("renderer2_t::draw: does not support entity_mesh with no vertex_streams");
    }
    const auto& vertex_stream = vertex_streams[0];

    const auto& vertex_attributes = mesh->vertex_attributes();
    if (vertex_attributes.size() == 0) {
        throw std::runtime_error("renderer2_t::draw: does not support entity_mesh with no vertex_attributes");
    }

    const auto expected_first_vertex_attribute_count = 2;
    const auto& first_vertex_attribute = vertex_attributes[0];
    if (first_vertex_attribute.component_count() != expected_first_vertex_attribute_count) {
        throw std::runtime_error(std::format(
            "renderer2_t::draw: does not support entity_mesh with vertex_attributes that do not have {} components",
            expected_first_vertex_attribute_count
        ));
    }

    const auto expected_vertex_attribute_type = tower_defense_api::vertex_attribute_type_t::R32;
    if (first_vertex_attribute.type() != expected_vertex_attribute_type) {
        throw std::runtime_error(std::format(
            "renderer2_t::draw: does not support entity_mesh with vertex_attributes that are not of type {}",
            static_cast<int>(expected_vertex_attribute_type)
        ));
    }

    return {
        .vertex_stream = &vertex_stream,
        .indices = &index_buffer->indices(),
        .primitive_topology = geometry->primitive_topology(),
        .translation = render_item.translation(),
        .scale = render_item.scale()
    };
}

std::vector<vector2i_t> build_view_positions(
    const tower_defense_api::camera_t<float, int, 2>& camera,
    const render_item_data_t& render_data
) {
    std::vector<vector2i_t> view_positions;
    view_positions.reserve(render_data.vertex_stream->element_count());

    for (std::size_t i = 0; i < render_data.vertex_stream->element_count(); ++i) {
        const auto position = read_position(*render_data.vertex_stream, static_cast<std::uint32_t>(i));
        view_positions.push_back(transform_position(camera, position, render_data.translation, render_data.scale));
    }

    return view_positions;
}

void draw_render_item(
    framebuffer_view_t framebuffer,
    const tower_defense_api::camera_t<float, int, 2>& camera,
    const tower_defense_api::render_item_t<float, 2>& render_item
) {
    const auto render_data = validate_render_item(render_item);
    const auto view_positions = build_view_positions(camera, render_data);
    const auto& indices = *render_data.indices;
    constexpr int point_radius = 3;
    const rgba8_t color = green_color();

    switch (render_data.primitive_topology) {
        case tower_defense_api::vertex_primitive_topology_t::point: {
            for (std::size_t i = 0; i < indices.size(); ++i) {
                const auto point = view_positions[indices[i]];
                if (point_intersects_camera_view(camera, point)) {
                    draw_filled_circle(framebuffer, point, point_radius, color);
                }
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::line: {
            for (std::size_t i = 0; i + 1 < indices.size(); i += 2) {
                draw_line(framebuffer, view_positions[indices[i]], view_positions[indices[i + 1]], color);
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::line_strip: {
            for (std::size_t i = 0; i + 1 < indices.size(); ++i) {
                draw_line(framebuffer, view_positions[indices[i]], view_positions[indices[i + 1]], color);
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::line_loop: {
            for (std::size_t i = 0; i < indices.size(); ++i) {
                draw_line(framebuffer, view_positions[indices[i]], view_positions[indices[(i + 1) % indices.size()]], color);
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::triangle: {
            for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
                draw_filled_triangle(framebuffer, view_positions[indices[i]], view_positions[indices[i + 1]], view_positions[indices[i + 2]], color);
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::triangle_strip: {
            for (std::size_t i = 0; i + 2 < indices.size(); ++i) {
                if (i % 2 == 0) {
                    draw_filled_triangle(framebuffer, view_positions[indices[i + 1]], view_positions[indices[i]], view_positions[indices[i + 2]], color);
                } else {
                    draw_filled_triangle(framebuffer, view_positions[indices[i]], view_positions[indices[i + 1]], view_positions[indices[i + 2]], color);
                }
            }
        } break;
        case tower_defense_api::vertex_primitive_topology_t::triangle_fan: {
            for (std::size_t i = 1; i + 1 < indices.size(); ++i) {
                draw_filled_triangle(framebuffer, view_positions[indices[0]], view_positions[indices[i]], view_positions[indices[i + 1]], color);
            }
        } break;
        default: {
            throw std::runtime_error(std::format(
                "renderer2_t::draw: does not support entity with vertex_primitive_topology_t: {}",
                render_data.primitive_topology
            ));
        } break;
    }
}

} // namespace

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

renderer2_t::renderer2_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window):
    m_software_renderer(std::move(window)),
    m_frame_active(false)
{
}

bool renderer2_t::begin_frame() {
    return begin_frame(ray_white_color());
}

bool renderer2_t::begin_frame(renderer2_color_t clear_color) {
    m_frame_active = m_software_renderer.begin_frame();

    if (!m_frame_active) {
        return false;
    }

    std::ranges::fill(m_software_renderer.pixels(), to_rgba8(clear_color));
    return true;
}

void renderer2_t::present() {
    if (!m_frame_active) {
        return;
    }

    m_software_renderer.present();
    m_frame_active = false;
}

int renderer2_t::width() const noexcept {
    return m_software_renderer.width();
}

int renderer2_t::height() const noexcept {
    return m_software_renderer.height();
}

void renderer2_t::draw(const camera_t<float, int, 2>& camera, const render_item_t<float, 2>& render_item) {
    if (!m_frame_active) {
        throw std::runtime_error("renderer2_t::draw: begin_frame must be called before draw");
    }

    draw_render_item(
        {
            .width = m_software_renderer.width(),
            .height = m_software_renderer.height(),
            .pixels = m_software_renderer.pixels()
        },
        camera,
        render_item
    );
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
