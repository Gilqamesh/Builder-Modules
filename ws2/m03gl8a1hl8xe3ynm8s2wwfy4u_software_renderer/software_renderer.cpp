#include "software_renderer.h"

#include <m03gjfvd6i5jzbmngb2ldoooza_type_erased_array/api.h>
#include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace {

namespace renderer = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace software_shader = m03gt1djvvy5atia5evkbg6rqy_software_shader;
namespace type_erased_array = m03gjfvd6i5jzbmngb2ldoooza_type_erased_array;

using vector2f_t = shader::vector_t<float, 2>;
using vector3f_t = shader::vector_t<float, 3>;
using vector4f_t = shader::vector_t<float, 4>;
using varying_t = std::variant<float, vector2f_t, vector3f_t, vector4f_t>;
using varying_map_t = std::map<std::uint32_t, varying_t>;

constexpr std::uint32_t clip_scale_binding = 0;
constexpr std::uint32_t clip_offset_binding = 1;

renderer::framebuffer_t validated_framebuffer(renderer::framebuffer_t framebuffer) {
    if (framebuffer.width < 0 || framebuffer.height < 0) {
        throw std::invalid_argument("software renderer framebuffer dimensions must be non-negative");
    }

    const std::size_t width = static_cast<std::size_t>(framebuffer.width);
    const std::size_t height = static_cast<std::size_t>(framebuffer.height);
    if (width != 0 && height > std::numeric_limits<std::size_t>::max() / width) {
        throw std::length_error("software renderer framebuffer size overflows size_t");
    }
    if (framebuffer.pixels.size() != width * height) {
        throw std::invalid_argument("software renderer framebuffer size does not match its dimensions");
    }

    return framebuffer;
}

software_shader::program_t make_program() {
    shader::vertex_shader_ast_builder_t vertex;
    const auto object_position = vertex.input<vector2f_t>(0);
    const auto clip_scale = vertex.uniform<vector2f_t>(clip_scale_binding);
    const auto clip_offset = vertex.uniform<vector2f_t>(clip_offset_binding);
    const auto clip_position = object_position * clip_scale + clip_offset;
    const auto texture_coordinates = object_position * 0.5F + vector2f_t({0.5F, 0.5F});
    vertex.position(vertex.construct<vector4f_t>(clip_position, 0.0F, 1.0F));
    vertex.output(0, texture_coordinates);

    shader::fragment_shader_ast_builder_t fragment;
    const auto interpolated_coordinates = fragment.input<vector2f_t>(0);
    const auto texture = fragment.resource<shader::shader_texture_2d_t>(0);
    const auto sampler = fragment.resource<shader::shader_sampler_t>(0);
    fragment.color(shader::sample(texture, sampler, interpolated_coordinates));

    return software_shader::program_t(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );
}

struct pipeline_vertex_t {
    vector4f_t clip_position;
    varying_map_t outputs;
};

struct screen_vertex_t {
    float x;
    float y;
    float ndc_x;
    float ndc_y;
    float ndc_z;
    float reciprocal_w;
    varying_map_t outputs;
};

bool finite(const vector4f_t& vector) {
    return std::ranges::all_of(vector, [](float component) { return std::isfinite(component); });
}

std::size_t shader_component_count(shader::shader_data_type_t type) {
    switch (type.category()) {
        case shader::shader_data_category_t::scalar:
            return 1;
        case shader::shader_data_category_t::vector:
            return type.rows();
        default:
            return 0;
    }
}

renderer::vertex_attribute_type_t expected_attribute_type(shader::shader_scalar_type_t type) {
    switch (type) {
        case shader::shader_scalar_type_t::floating_point:
            return renderer::vertex_attribute_type_t::R32;
        case shader::shader_scalar_type_t::signed_integer:
            return renderer::vertex_attribute_type_t::I32;
        case shader::shader_scalar_type_t::unsigned_integer:
            return renderer::vertex_attribute_type_t::U32;
        default:
            throw std::invalid_argument("software renderer does not support this shader vertex input scalar type");
    }
}

void validate_vertex_attribute(
    const renderer::vertex_attribute_t& attribute,
    shader::shader_data_type_t input_type
) {
    const std::size_t component_count = shader_component_count(input_type);
    if (component_count == 0 || component_count > 4) {
        throw std::invalid_argument("software renderer only supports scalar and vector vertex inputs");
    }
    if (attribute.type() != expected_attribute_type(input_type.scalar()) || attribute.component_count() != component_count) {
        throw std::invalid_argument("mesh vertex attribute is incompatible with the reflected shader input");
    }
}

template <typename T>
T read_scalar(
    const type_erased_array::type_erased_array_t& stream,
    std::uint32_t vertex_index
) {
    T result;
    const std::size_t offset = static_cast<std::size_t>(vertex_index) * sizeof(T);
    std::memcpy(&result, stream.data().data() + offset, sizeof(T));
    return result;
}

template <typename T, std::size_t N>
shader::vector_t<T, N> read_vector(
    const type_erased_array::type_erased_array_t& stream,
    std::uint32_t vertex_index
) {
    std::array<T, N> result;
    const std::size_t offset = static_cast<std::size_t>(vertex_index) * sizeof(result);
    std::memcpy(result.data(), stream.data().data() + offset, sizeof(result));
    return shader::vector_t<T, N>(result);
}

template <typename T>
void set_vertex_input_components(
    software_shader::vertex_io_t& io,
    std::uint32_t location,
    const type_erased_array::type_erased_array_t& stream,
    std::uint32_t vertex_index,
    std::size_t component_count
) {
    switch (component_count) {
        case 1:
            io.input(location, read_scalar<T>(stream, vertex_index));
            break;
        case 2:
            io.input(location, read_vector<T, 2>(stream, vertex_index));
            break;
        case 3:
            io.input(location, read_vector<T, 3>(stream, vertex_index));
            break;
        case 4:
            io.input(location, read_vector<T, 4>(stream, vertex_index));
            break;
        default:
            throw std::logic_error("unsupported validated vertex component count");
    }
}

void set_vertex_input(
    software_shader::vertex_io_t& io,
    const shader::shader_interface_element_t& input,
    const type_erased_array::type_erased_array_t& stream,
    const renderer::vertex_attribute_t& attribute,
    std::uint32_t vertex_index
) {
    switch (attribute.type()) {
        case renderer::vertex_attribute_type_t::R32:
            set_vertex_input_components<float>(io, input.index, stream, vertex_index, attribute.component_count());
            break;
        case renderer::vertex_attribute_type_t::I32:
            set_vertex_input_components<std::int32_t>(io, input.index, stream, vertex_index, attribute.component_count());
            break;
        case renderer::vertex_attribute_type_t::U32:
            set_vertex_input_components<std::uint32_t>(io, input.index, stream, vertex_index, attribute.component_count());
            break;
        default:
            throw std::logic_error("unsupported validated vertex attribute type");
    }
}

bool supported_fragment_input(shader::shader_data_type_t type) {
    if (type.scalar() != shader::shader_scalar_type_t::floating_point) {
        return false;
    }
    if (type.category() == shader::shader_data_category_t::scalar) {
        return true;
    }
    return type.category() == shader::shader_data_category_t::vector
        && 2 <= type.rows() && type.rows() <= 4 && type.columns() == 1;
}

template <typename T>
T require_vertex_output(
    const software_shader::vertex_io_t& io,
    std::uint32_t location
) {
    const auto output = io.output<T>(location);
    if (!output) {
        throw std::runtime_error("vertex shader did not write an output required by the fragment shader");
    }
    return *output;
}

varying_t vertex_output(
    const software_shader::vertex_io_t& io,
    const shader::shader_interface_element_t& input
) {
    if (input.type == shader::shader_data_type<float>()) {
        return require_vertex_output<float>(io, input.index);
    }
    if (input.type == shader::shader_data_type<vector2f_t>()) {
        return require_vertex_output<vector2f_t>(io, input.index);
    }
    if (input.type == shader::shader_data_type<vector3f_t>()) {
        return require_vertex_output<vector3f_t>(io, input.index);
    }
    if (input.type == shader::shader_data_type<vector4f_t>()) {
        return require_vertex_output<vector4f_t>(io, input.index);
    }
    throw std::logic_error("unsupported validated fragment input type");
}

varying_t interpolate(const varying_t& from, const varying_t& to, float factor) {
    return std::visit([&](const auto& first) -> varying_t {
        using type = std::remove_cvref_t<decltype(first)>;
        const auto* second = std::get_if<type>(&to);
        if (!second) {
            throw std::logic_error("inconsistent varying types during clipping");
        }
        return first + (*second - first) * factor;
    }, from);
}

pipeline_vertex_t interpolate(
    const pipeline_vertex_t& from,
    const pipeline_vertex_t& to,
    float factor
) {
    pipeline_vertex_t result {
        .clip_position = from.clip_position + (to.clip_position - from.clip_position) * factor,
        .outputs = {}
    };
    for (const auto& [location, output] : from.outputs) {
        const auto iterator = to.outputs.find(location);
        if (iterator == to.outputs.end()) {
            throw std::logic_error("inconsistent vertex outputs during clipping");
        }
        result.outputs.emplace(location, interpolate(output, iterator->second, factor));
    }
    return result;
}

float clip_distance(const pipeline_vertex_t& vertex, std::size_t plane) {
    const auto& position = vertex.clip_position;
    switch (plane) {
        case 0: return position[0] + position[3];
        case 1: return position[3] - position[0];
        case 2: return position[1] + position[3];
        case 3: return position[3] - position[1];
        case 4: return position[2] + position[3];
        case 5: return position[3] - position[2];
        default: throw std::logic_error("unknown clip plane");
    }
}

bool inside_clip_volume(const pipeline_vertex_t& vertex) {
    for (std::size_t plane = 0; plane < 6; ++plane) {
        if (clip_distance(vertex, plane) < 0.0F) {
            return false;
        }
    }
    return vertex.clip_position[3] != 0.0F;
}

bool clip_line(pipeline_vertex_t& from, pipeline_vertex_t& to) {
    for (std::size_t plane = 0; plane < 6; ++plane) {
        const float from_distance = clip_distance(from, plane);
        const float to_distance = clip_distance(to, plane);
        const bool from_inside = from_distance >= 0.0F;
        const bool to_inside = to_distance >= 0.0F;

        if (!from_inside && !to_inside) {
            return false;
        }
        if (from_inside == to_inside) {
            continue;
        }

        const float factor = from_distance / (from_distance - to_distance);
        const auto clipped = interpolate(from, to, factor);
        if (!from_inside) {
            from = clipped;
        } else {
            to = clipped;
        }
    }
    return from.clip_position[3] != 0.0F && to.clip_position[3] != 0.0F;
}

std::vector<pipeline_vertex_t> clip_triangle(
    const pipeline_vertex_t& first,
    const pipeline_vertex_t& second,
    const pipeline_vertex_t& third
) {
    std::vector<pipeline_vertex_t> polygon {first, second, third};
    for (std::size_t plane = 0; plane < 6 && !polygon.empty(); ++plane) {
        std::vector<pipeline_vertex_t> clipped;
        clipped.reserve(polygon.size() + 1);

        auto previous = polygon.back();
        float previous_distance = clip_distance(previous, plane);
        bool previous_inside = previous_distance >= 0.0F;
        for (const auto& current : polygon) {
            const float current_distance = clip_distance(current, plane);
            const bool current_inside = current_distance >= 0.0F;
            if (current_inside != previous_inside) {
                const float factor = previous_distance / (previous_distance - current_distance);
                clipped.push_back(interpolate(previous, current, factor));
            }
            if (current_inside) {
                clipped.push_back(current);
            }
            previous = current;
            previous_distance = current_distance;
            previous_inside = current_inside;
        }
        polygon = std::move(clipped);
    }

    if (std::ranges::any_of(polygon, [](const auto& vertex) { return vertex.clip_position[3] == 0.0F; })) {
        return {};
    }
    return polygon;
}

std::optional<screen_vertex_t> project(
    const pipeline_vertex_t& vertex,
    int width,
    int height
) {
    const float w = vertex.clip_position[3];
    if (w == 0.0F) {
        return std::nullopt;
    }
    const float reciprocal_w = 1.0F / w;
    const float ndc_x = vertex.clip_position[0] * reciprocal_w;
    const float ndc_y = vertex.clip_position[1] * reciprocal_w;
    const float ndc_z = vertex.clip_position[2] * reciprocal_w;
    const float x = (ndc_x * 0.5F + 0.5F) * static_cast<float>(width);
    const float y = (0.5F - ndc_y * 0.5F) * static_cast<float>(height);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(ndc_z) || !std::isfinite(reciprocal_w)) {
        return std::nullopt;
    }
    return screen_vertex_t {
        .x = x,
        .y = y,
        .ndc_x = ndc_x,
        .ndc_y = ndc_y,
        .ndc_z = ndc_z,
        .reciprocal_w = reciprocal_w,
        .outputs = vertex.outputs
    };
}

varying_t perspective_line(
    const varying_t& first,
    const varying_t& second,
    float factor,
    float first_reciprocal_w,
    float second_reciprocal_w,
    float denominator
) {
    return std::visit([&](const auto& first_value) -> varying_t {
        using type = std::remove_cvref_t<decltype(first_value)>;
        const auto* second_value = std::get_if<type>(&second);
        if (!second_value) {
            throw std::logic_error("inconsistent line varying types");
        }
        return (
            first_value * ((1.0F - factor) * first_reciprocal_w)
            + *second_value * (factor * second_reciprocal_w)
        ) / denominator;
    }, first);
}

varying_t perspective_triangle(
    const varying_t& first,
    const varying_t& second,
    const varying_t& third,
    const std::array<float, 3>& weights,
    const std::array<float, 3>& reciprocal_w,
    float denominator
) {
    return std::visit([&](const auto& first_value) -> varying_t {
        using type = std::remove_cvref_t<decltype(first_value)>;
        const auto* second_value = std::get_if<type>(&second);
        const auto* third_value = std::get_if<type>(&third);
        if (!second_value || !third_value) {
            throw std::logic_error("inconsistent triangle varying types");
        }
        return (
            first_value * (weights[0] * reciprocal_w[0])
            + *second_value * (weights[1] * reciprocal_w[1])
            + *third_value * (weights[2] * reciprocal_w[2])
        ) / denominator;
    }, first);
}

void set_fragment_inputs(
    software_shader::fragment_io_t& io,
    const varying_map_t& inputs
) {
    for (const auto& [location, input] : inputs) {
        std::visit([&](const auto& typed_input) { io.input(location, typed_input); }, input);
    }
}

std::uint8_t to_unorm8(float component) {
    if (std::isnan(component) || component == -std::numeric_limits<float>::infinity()) {
        return 0;
    }
    if (component == std::numeric_limits<float>::infinity()) {
        return 255;
    }
    const float clamped = std::clamp(component, 0.0F, 1.0F);
    return static_cast<std::uint8_t>(std::floor(clamped * 255.0F + 0.5F));
}

renderer::rgba8_t to_rgba8(const vector4f_t& color) {
    return {
        .red = to_unorm8(color[0]),
        .green = to_unorm8(color[1]),
        .blue = to_unorm8(color[2]),
        .alpha = to_unorm8(color[3])
    };
}

void shade_sample(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    int x,
    int y,
    float depth,
    float reciprocal_w,
    bool front_facing,
    const varying_map_t& inputs
) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return;
    }

    software_shader::fragment_io_t io(
        vector4f_t({
            static_cast<float>(x) + 0.5F,
            static_cast<float>(y) + 0.5F,
            std::clamp(depth, 0.0F, 1.0F),
            reciprocal_w
        }),
        front_facing
    );
    set_fragment_inputs(io, inputs);
    program.run(bindings, io);
    if (io.discarded()) {
        return;
    }
    const auto color = io.color();
    if (!color) {
        return;
    }
    framebuffer[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = to_rgba8(*color);
}

void rasterize_point(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    const pipeline_vertex_t& vertex
) {
    if (!inside_clip_volume(vertex)) {
        return;
    }
    const auto screen = project(vertex, width, height);
    if (!screen) {
        return;
    }

    constexpr int radius = 3;
    constexpr int radius_squared = radius * radius;
    const int center_x = static_cast<int>(std::floor(screen->x));
    const int center_y = static_cast<int>(std::floor(screen->y));
    const float depth = screen->ndc_z * 0.5F + 0.5F;
    for (int y = center_y - radius; y <= center_y + radius; ++y) {
        for (int x = center_x - radius; x <= center_x + radius; ++x) {
            const int dx = x - center_x;
            const int dy = y - center_y;
            if (dx * dx + dy * dy <= radius_squared) {
                shade_sample(program, bindings, width, height, framebuffer, x, y, depth, screen->reciprocal_w, true, screen->outputs);
            }
        }
    }
}

void rasterize_line(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    pipeline_vertex_t first,
    pipeline_vertex_t second
) {
    if (!clip_line(first, second)) {
        return;
    }
    const auto first_screen = project(first, width, height);
    const auto second_screen = project(second, width, height);
    if (!first_screen || !second_screen) {
        return;
    }

    int x = static_cast<int>(std::floor(first_screen->x));
    int y = static_cast<int>(std::floor(first_screen->y));
    const int target_x = static_cast<int>(std::floor(second_screen->x));
    const int target_y = static_cast<int>(std::floor(second_screen->y));
    const int dx = std::abs(target_x - x);
    const int step_x = x < target_x ? 1 : -1;
    const int dy = -std::abs(target_y - y);
    const int step_y = y < target_y ? 1 : -1;
    int error = dx + dy;

    const float line_x = second_screen->x - first_screen->x;
    const float line_y = second_screen->y - first_screen->y;
    const float line_length_squared = line_x * line_x + line_y * line_y;
    while (true) {
        float factor = 0.0F;
        if (line_length_squared != 0.0F) {
            factor = (
                (static_cast<float>(x) + 0.5F - first_screen->x) * line_x
                + (static_cast<float>(y) + 0.5F - first_screen->y) * line_y
            ) / line_length_squared;
            factor = std::clamp(factor, 0.0F, 1.0F);
        }

        const float reciprocal_w = (1.0F - factor) * first_screen->reciprocal_w + factor * second_screen->reciprocal_w;
        if (reciprocal_w != 0.0F) {
            varying_map_t inputs;
            for (const auto& [location, output] : first_screen->outputs) {
                inputs.emplace(location, perspective_line(
                    output,
                    second_screen->outputs.at(location),
                    factor,
                    first_screen->reciprocal_w,
                    second_screen->reciprocal_w,
                    reciprocal_w
                ));
            }
            const float ndc_z = (1.0F - factor) * first_screen->ndc_z + factor * second_screen->ndc_z;
            shade_sample(program, bindings, width, height, framebuffer, x, y, ndc_z * 0.5F + 0.5F, reciprocal_w, true, inputs);
        }

        if (x == target_x && y == target_y) {
            break;
        }
        const int twice_error = error * 2;
        if (twice_error >= dy) {
            error += dy;
            x += step_x;
        }
        if (twice_error <= dx) {
            error += dx;
            y += step_y;
        }
    }
}

float edge(float ax, float ay, float bx, float by, float px, float py) {
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

bool top_left_edge(const screen_vertex_t& from, const screen_vertex_t& to) {
    const float dx = to.x - from.x;
    const float dy = to.y - from.y;
    return dy < 0.0F || (dy == 0.0F && dx > 0.0F);
}

bool covered(float edge_value, bool top_left) {
    return edge_value > 0.0F || (edge_value == 0.0F && top_left);
}

void rasterize_projected_triangle(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    screen_vertex_t first,
    screen_vertex_t second,
    screen_vertex_t third
) {
    const bool front_facing = edge(
        first.ndc_x, first.ndc_y,
        second.ndc_x, second.ndc_y,
        third.ndc_x, third.ndc_y
    ) > 0.0F;

    float area = edge(first.x, first.y, second.x, second.y, third.x, third.y);
    if (area == 0.0F) {
        return;
    }
    if (area < 0.0F) {
        std::swap(second, third);
        area = -area;
    }

    const int minimum_x = std::max(0, static_cast<int>(std::ceil(std::min({first.x, second.x, third.x}) - 0.5F)));
    const int maximum_x = std::min(width - 1, static_cast<int>(std::floor(std::max({first.x, second.x, third.x}) - 0.5F)));
    const int minimum_y = std::max(0, static_cast<int>(std::ceil(std::min({first.y, second.y, third.y}) - 0.5F)));
    const int maximum_y = std::min(height - 1, static_cast<int>(std::floor(std::max({first.y, second.y, third.y}) - 0.5F)));
    if (minimum_x > maximum_x || minimum_y > maximum_y) {
        return;
    }

    const bool first_edge_top_left = top_left_edge(second, third);
    const bool second_edge_top_left = top_left_edge(third, first);
    const bool third_edge_top_left = top_left_edge(first, second);
    const std::array<float, 3> reciprocal_w {first.reciprocal_w, second.reciprocal_w, third.reciprocal_w};

    for (int y = minimum_y; y <= maximum_y; ++y) {
        for (int x = minimum_x; x <= maximum_x; ++x) {
            const float sample_x = static_cast<float>(x) + 0.5F;
            const float sample_y = static_cast<float>(y) + 0.5F;
            const float first_edge = edge(second.x, second.y, third.x, third.y, sample_x, sample_y);
            const float second_edge = edge(third.x, third.y, first.x, first.y, sample_x, sample_y);
            const float third_edge = edge(first.x, first.y, second.x, second.y, sample_x, sample_y);
            if (!covered(first_edge, first_edge_top_left)
                || !covered(second_edge, second_edge_top_left)
                || !covered(third_edge, third_edge_top_left)) {
                continue;
            }

            const std::array<float, 3> weights {first_edge / area, second_edge / area, third_edge / area};
            const float interpolated_reciprocal_w = weights[0] * reciprocal_w[0]
                + weights[1] * reciprocal_w[1]
                + weights[2] * reciprocal_w[2];
            if (interpolated_reciprocal_w == 0.0F) {
                continue;
            }

            varying_map_t inputs;
            for (const auto& [location, output] : first.outputs) {
                inputs.emplace(location, perspective_triangle(
                    output,
                    second.outputs.at(location),
                    third.outputs.at(location),
                    weights,
                    reciprocal_w,
                    interpolated_reciprocal_w
                ));
            }
            const float ndc_z = weights[0] * first.ndc_z + weights[1] * second.ndc_z + weights[2] * third.ndc_z;
            shade_sample(
                program,
                bindings,
                width,
                height,
                framebuffer,
                x,
                y,
                ndc_z * 0.5F + 0.5F,
                interpolated_reciprocal_w,
                front_facing,
                inputs
            );
        }
    }
}

void rasterize_triangle(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    const pipeline_vertex_t& first,
    const pipeline_vertex_t& second,
    const pipeline_vertex_t& third
) {
    const auto polygon = clip_triangle(first, second, third);
    for (std::size_t index = 1; index + 1 < polygon.size(); ++index) {
        const auto projected_first = project(polygon[0], width, height);
        const auto projected_second = project(polygon[index], width, height);
        const auto projected_third = project(polygon[index + 1], width, height);
        if (projected_first && projected_second && projected_third) {
            rasterize_projected_triangle(
                program,
                bindings,
                width,
                height,
                framebuffer,
                *projected_first,
                *projected_second,
                *projected_third
            );
        }
    }
}

} // namespace

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

software_renderer_t::software_renderer_t(framebuffer_t framebuffer):
    m_framebuffer(validated_framebuffer(framebuffer)),
    m_program(make_program())
{
}

software_renderer_t::~software_renderer_t() = default;

void software_renderer_t::framebuffer(framebuffer_t framebuffer) {
    m_framebuffer = validated_framebuffer(framebuffer);
}

framebuffer_t software_renderer_t::framebuffer() const noexcept {
    return m_framebuffer;
}

void software_renderer_t::clear(rgba8_t color) {
    std::ranges::fill(m_framebuffer.pixels, color);
}

void software_renderer_t::draw(
    const camera_t<float, int, 2>& camera,
    const render_item_t<float, 2>& render_item
) {
    if (m_framebuffer.width == 0 || m_framebuffer.height == 0) {
        throw std::invalid_argument("software_renderer_t::draw requires a non-empty framebuffer");
    }

    const auto geometry = render_item.geometry();
    if (!geometry) {
        throw std::invalid_argument("software_renderer_t::draw requires geometry");
    }
    geometry->finalize();
    const auto material = render_item.material();
    if (!material) {
        throw std::invalid_argument("software_renderer_t::draw requires a material");
    }

    const auto& world_rect = camera.world_rect();
    const auto& view_rect = camera.view_rect();
    const float world_width = world_rect[0].length();
    const float world_height = world_rect[1].length();
    if (world_width == 0.0F || world_height == 0.0F) {
        throw std::invalid_argument("software_renderer_t::draw requires non-empty camera world bounds");
    }

    const float view_world_scale_x = static_cast<float>(view_rect[0].length()) / world_width;
    const float view_world_scale_y = static_cast<float>(view_rect[1].length()) / world_height;
    const auto& object_scale = render_item.scale();
    const auto& translation = render_item.translation();
    const vector2f_t clip_scale({
        object_scale[0] * view_world_scale_x * 2.0F / static_cast<float>(m_framebuffer.width),
        object_scale[1] * view_world_scale_y * -2.0F / static_cast<float>(m_framebuffer.height)
    });
    const vector2f_t clip_offset({
        (static_cast<float>(view_rect[0][0]) + (translation[0] - world_rect[0][0]) * view_world_scale_x)
            * 2.0F / static_cast<float>(m_framebuffer.width) - 1.0F,
        1.0F - (static_cast<float>(view_rect[1][0]) + (translation[1] - world_rect[1][0]) * view_world_scale_y)
            * 2.0F / static_cast<float>(m_framebuffer.height)
    });

    software_shader::bindings_t bindings;
    bindings.uniform(clip_scale_binding, clip_scale);
    bindings.uniform(clip_offset_binding, clip_offset);

    const auto& texture_bindings = material->texture_bindings();
    const auto bind_interface = [&](const shader::shader_interface_t& interface) {
        for (const auto& binding : interface.bindings()) {
            if (binding.type.category() != shader::shader_data_category_t::texture_2d
                && binding.type.category() != shader::shader_data_category_t::sampler) {
                continue;
            }
            if (binding.index >= texture_bindings.size()) {
                throw std::invalid_argument("software renderer material is missing a required texture binding");
            }
            const auto& material_binding = texture_bindings[binding.index];
            if (binding.type.category() == shader::shader_data_category_t::texture_2d) {
                if (!material_binding.texture) {
                    throw std::invalid_argument("software renderer material texture binding is null");
                }
                bindings.texture(binding.index, *material_binding.texture);
            } else {
                if (!material_binding.sampler) {
                    throw std::invalid_argument("software renderer material sampler binding is null");
                }
                bindings.sampler(binding.index, *material_binding.sampler);
            }
        }
    };
    bind_interface(m_program.vertex_interface());
    bind_interface(m_program.fragment_interface());

    draw_pipeline(bindings, *geometry);
}

void software_renderer_t::draw_pipeline(
    const software_shader::bindings_t& bindings,
    const geometry_t& geometry
) {
    const auto& program = m_program;
    const int width = m_framebuffer.width;
    const int height = m_framebuffer.height;
    const auto framebuffer = m_framebuffer.pixels;

    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("software renderer pipeline requires a non-empty framebuffer");
    }
    const std::size_t expected_size = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (framebuffer.size() != expected_size) {
        throw std::invalid_argument("software renderer framebuffer size does not match its dimensions");
    }

    const auto mesh = geometry.mesh();
    if (!mesh) {
        throw std::invalid_argument("software renderer geometry has no mesh");
    }
    const auto& streams = mesh->vertex_streams();
    const auto attributes = mesh->vertex_attributes();
    for (const auto& input : program.vertex_interface().inputs()) {
        if (input.index >= streams.size()) {
            throw std::invalid_argument("shader vertex input location has no corresponding mesh stream");
        }
        validate_vertex_attribute(attributes[input.index], input.type);
    }
    for (const auto& input : program.fragment_interface().inputs()) {
        if (!supported_fragment_input(input.type)) {
            throw std::invalid_argument("software renderer cannot interpolate this fragment input type");
        }
    }

    const auto indices = geometry.indices();
    std::vector<std::optional<pipeline_vertex_t>> vertices(mesh->number_of_vertices());
    for (const std::uint32_t vertex_index : indices) {
        if (vertex_index > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            throw std::out_of_range("vertex index cannot be represented by software shader vertex_io_t");
        }
        if (vertex_index >= vertices.size()) {
            throw std::out_of_range("software renderer geometry index is outside the mesh");
        }
        if (vertices[vertex_index]) {
            continue;
        }

        software_shader::vertex_io_t io(static_cast<std::int32_t>(vertex_index), 0);
        for (const auto& input : program.vertex_interface().inputs()) {
            set_vertex_input(io, input, streams[input.index], attributes[input.index], vertex_index);
        }
        program.run(bindings, io);
        const vector4f_t clip_position = io.position();
        if (!finite(clip_position)) {
            throw std::runtime_error("vertex shader produced a non-finite clip position");
        }

        pipeline_vertex_t vertex {
            .clip_position = clip_position,
            .outputs = {}
        };
        for (const auto& input : program.fragment_interface().inputs()) {
            vertex.outputs.emplace(input.index, vertex_output(io, input));
        }
        vertices[vertex_index] = std::move(vertex);
    }

    const auto vertex = [&](std::size_t index) -> const pipeline_vertex_t& {
        return *vertices[indices[index]];
    };
    switch (geometry.primitive_topology()) {
        case vertex_primitive_topology_t::point:
            for (std::size_t index = 0; index < indices.size(); ++index) {
                rasterize_point(program, bindings, width, height, framebuffer, vertex(index));
            }
            break;
        case vertex_primitive_topology_t::line:
            for (std::size_t index = 0; index + 1 < indices.size(); index += 2) {
                rasterize_line(program, bindings, width, height, framebuffer, vertex(index), vertex(index + 1));
            }
            break;
        case vertex_primitive_topology_t::line_strip:
            for (std::size_t index = 0; index + 1 < indices.size(); ++index) {
                rasterize_line(program, bindings, width, height, framebuffer, vertex(index), vertex(index + 1));
            }
            break;
        case vertex_primitive_topology_t::line_loop:
            for (std::size_t index = 0; index < indices.size(); ++index) {
                rasterize_line(program, bindings, width, height, framebuffer, vertex(index), vertex((index + 1) % indices.size()));
            }
            break;
        case vertex_primitive_topology_t::triangle:
            for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
                rasterize_triangle(program, bindings, width, height, framebuffer, vertex(index), vertex(index + 1), vertex(index + 2));
            }
            break;
        case vertex_primitive_topology_t::triangle_strip:
            for (std::size_t index = 0; index + 2 < indices.size(); ++index) {
                if (index % 2 == 0) {
                    rasterize_triangle(program, bindings, width, height, framebuffer, vertex(index + 1), vertex(index), vertex(index + 2));
                } else {
                    rasterize_triangle(program, bindings, width, height, framebuffer, vertex(index), vertex(index + 1), vertex(index + 2));
                }
            }
            break;
        case vertex_primitive_topology_t::triangle_fan:
            for (std::size_t index = 1; index + 1 < indices.size(); ++index) {
                rasterize_triangle(program, bindings, width, height, framebuffer, vertex(0), vertex(index), vertex(index + 1));
            }
            break;
        default:
            throw std::invalid_argument(std::format("unknown vertex primitive topology: {}", geometry.primitive_topology()));
    }
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
