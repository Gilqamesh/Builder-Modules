#include "software_renderer.h"

#include <m03gjfvd6i5jzbmngb2ldoooza_type_erased_array/api.h>
#include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

std::size_t framebuffer_pixel_count(int width, int height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument(std::format("framebuffer_pixel_count requires non-negative dimensions, got {}x{}", width, height));
    }

    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);
    if (width_size != 0 && std::numeric_limits<std::size_t>::max() / width_size < height_size) {
        throw std::length_error(std::format("framebuffer_pixel_count overflows size_t for dimensions {}x{}", width, height));
    }
    return width_size * height_size;
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace {

namespace renderer = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace software_shader = m03gt1djvvy5atia5evkbg6rqy_software_shader;
namespace type_erased_array = m03gjfvd6i5jzbmngb2ldoooza_type_erased_array;

using vector2f_t = shader::vector_t<float, 2>;
using vector3f_t = shader::vector_t<float, 3>;
using vector4f_t = shader::vector_t<float, 4>;
using matrix4f_t = shader::matrix_t<float, 4, 4>;
using varying_t = std::variant<float, vector2f_t, vector3f_t, vector4f_t>;
using varying_entry_t = std::pair<std::uint32_t, varying_t>;
using varying_values_t = std::vector<varying_entry_t>;

renderer::framebuffer_t validated_framebuffer(renderer::framebuffer_t framebuffer) {
    const std::size_t pixel_count = renderer::framebuffer_pixel_count(framebuffer.width, framebuffer.height);
    if (framebuffer.pixels.size() != pixel_count) {
        throw std::invalid_argument(std::format(
            "software renderer framebuffer has {} pixels, expected {} for dimensions {}x{}",
            framebuffer.pixels.size(),
            pixel_count,
            framebuffer.width,
            framebuffer.height
        ));
    }

    return framebuffer;
}

struct varying_range_t {
    std::size_t offset;
    std::size_t count;
};

struct pipeline_vertex_t {
    vector4f_t clip_position;
    varying_range_t outputs;
};

struct pipeline_vertex_view_t {
    vector4f_t clip_position;
    std::span<const varying_entry_t> outputs;
};

struct screen_vertex_t {
    float x;
    float y;
    float ndc_x;
    float ndc_y;
    float ndc_z;
    float reciprocal_w;
    std::span<const varying_entry_t> outputs;
};

struct clipping_buffer_t {
    std::vector<pipeline_vertex_t> vertices;
    varying_values_t values;
};

struct clipping_workspace_t {
    std::array<clipping_buffer_t, 2> buffers;
};

pipeline_vertex_view_t view(const pipeline_vertex_t& vertex, const varying_values_t& values) {
    return {
        .clip_position = vertex.clip_position,
        .outputs = std::span<const varying_entry_t>(values).subspan(vertex.outputs.offset, vertex.outputs.count)
    };
}

bool finite(const vector4f_t& vector) {
    return std::ranges::all_of(vector, [](float component) { return std::isfinite(component); });
}

std::size_t shader_component_count(shader::shader_data_type_t type) {
    switch (type.category()) {
        case shader::shader_data_category_t::scalar: {
            return 1;
        }
        case shader::shader_data_category_t::vector: {
            return type.rows();
        }
        default: {
            return 0;
        }
    }
}

renderer::vertex_attribute_type_t expected_attribute_type(shader::shader_scalar_type_t type) {
    switch (type) {
        case shader::shader_scalar_type_t::floating_point: {
            return renderer::vertex_attribute_type_t::R32;
        }
        case shader::shader_scalar_type_t::signed_integer: {
            return renderer::vertex_attribute_type_t::I32;
        }
        case shader::shader_scalar_type_t::unsigned_integer: {
            return renderer::vertex_attribute_type_t::U32;
        }
        default: {
            throw std::invalid_argument("software renderer does not support this shader vertex input scalar type");
        }
    }
}

void validate_vertex_attribute(
    const renderer::vertex_attribute_t& attribute,
    shader::shader_data_type_t input_type
) {
    const std::size_t component_count = shader_component_count(input_type);
    if (component_count == 0 || 4 < component_count) {
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
    return stream.read<T>(vertex_index);
}

template <typename T, std::size_t N>
shader::vector_t<T, N> read_vector(
    const type_erased_array::type_erased_array_t& stream,
    std::uint32_t vertex_index
) {
    return shader::vector_t<T, N>(stream.read<std::array<T, N>>(vertex_index));
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
        case 1: {
            io.input(location, read_scalar<T>(stream, vertex_index));
        } break;
        case 2: {
            io.input(location, read_vector<T, 2>(stream, vertex_index));
        } break;
        case 3: {
            io.input(location, read_vector<T, 3>(stream, vertex_index));
        } break;
        case 4: {
            io.input(location, read_vector<T, 4>(stream, vertex_index));
        } break;
        default: {
            throw std::logic_error("unsupported validated vertex component count");
        }
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
        case renderer::vertex_attribute_type_t::R32: {
            set_vertex_input_components<float>(io, input.index, stream, vertex_index, attribute.component_count());
        } break;
        case renderer::vertex_attribute_type_t::I32: {
            set_vertex_input_components<std::int32_t>(io, input.index, stream, vertex_index, attribute.component_count());
        } break;
        case renderer::vertex_attribute_type_t::U32: {
            set_vertex_input_components<std::uint32_t>(io, input.index, stream, vertex_index, attribute.component_count());
        } break;
        default: {
            throw std::logic_error("unsupported validated vertex attribute type");
        }
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
        throw std::runtime_error(std::format(
            "vertex shader did not write output location {} required by the fragment shader",
            location
        ));
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

void append_vertex(clipping_buffer_t& destination, const pipeline_vertex_view_t& source) {
    const std::size_t offset = destination.values.size();
    destination.values.insert(destination.values.end(), source.outputs.begin(), source.outputs.end());
    destination.vertices.push_back({
        .clip_position = source.clip_position,
        .outputs = {offset, source.outputs.size()}
    });
}

void append_interpolated_vertex(
    clipping_buffer_t& destination,
    const pipeline_vertex_view_t& from,
    const pipeline_vertex_view_t& to,
    float factor
) {
    if (from.outputs.size() != to.outputs.size()) {
        throw std::logic_error("inconsistent vertex output counts during clipping");
    }

    const std::size_t offset = destination.values.size();
    for (std::size_t index = 0; index < from.outputs.size(); ++index) {
        const auto& [from_location, from_value] = from.outputs[index];
        const auto& [to_location, to_value] = to.outputs[index];
        if (from_location != to_location) {
            throw std::logic_error("inconsistent vertex output locations during clipping");
        }
        destination.values.emplace_back(from_location, interpolate(from_value, to_value, factor));
    }
    destination.vertices.push_back({
        .clip_position = from.clip_position + (to.clip_position - from.clip_position) * factor,
        .outputs = {offset, from.outputs.size()}
    });
}

void clear(clipping_buffer_t& buffer) {
    buffer.vertices.clear();
    buffer.values.clear();
}

float clip_distance(const pipeline_vertex_view_t& vertex, std::size_t plane) {
    const auto& position = vertex.clip_position;
    switch (plane) {
        case 0: {
            return position[0] + position[3];
        }
        case 1: {
            return position[3] - position[0];
        }
        case 2: {
            return position[1] + position[3];
        }
        case 3: {
            return position[3] - position[1];
        }
        case 4: {
            return position[2] + position[3];
        }
        case 5: {
            return position[3] - position[2];
        }
        default: {
            throw std::logic_error(std::format("unknown clip plane: {}", plane));
        }
    }
}

bool inside_clip_volume(const pipeline_vertex_view_t& vertex) {
    for (std::size_t plane = 0; plane < 6; ++plane) {
        if (clip_distance(vertex, plane) < 0.0F) {
            return false;
        }
    }
    return vertex.clip_position[3] != 0.0F;
}

std::optional<std::size_t> clip_line(
    const pipeline_vertex_view_t& first,
    const pipeline_vertex_view_t& second,
    clipping_workspace_t& workspace
) {
    clear(workspace.buffers[0]);
    append_vertex(workspace.buffers[0], first);
    append_vertex(workspace.buffers[0], second);

    std::size_t source_index = 0;
    for (std::size_t plane = 0; plane < 6; ++plane) {
        const std::size_t destination_index = 1 - source_index;
        auto& source = workspace.buffers[source_index];
        auto& destination = workspace.buffers[destination_index];
        clear(destination);

        const auto from = view(source.vertices[0], source.values);
        const auto to = view(source.vertices[1], source.values);
        const float from_distance = clip_distance(from, plane);
        const float to_distance = clip_distance(to, plane);
        const bool from_inside = 0.0F <= from_distance;
        const bool to_inside = 0.0F <= to_distance;

        if (!from_inside && !to_inside) {
            return std::nullopt;
        }
        if (from_inside == to_inside) {
            append_vertex(destination, from);
            append_vertex(destination, to);
        } else {
            const float factor = from_distance / (from_distance - to_distance);
            if (!from_inside) {
                append_interpolated_vertex(destination, from, to, factor);
                append_vertex(destination, to);
            } else {
                append_vertex(destination, from);
                append_interpolated_vertex(destination, from, to, factor);
            }
        }
        source_index = destination_index;
    }

    const auto& result = workspace.buffers[source_index].vertices;
    if (result[0].clip_position[3] == 0.0F || result[1].clip_position[3] == 0.0F) {
        return std::nullopt;
    }
    return source_index;
}

std::optional<std::size_t> clip_triangle(
    const pipeline_vertex_view_t& first,
    const pipeline_vertex_view_t& second,
    const pipeline_vertex_view_t& third,
    clipping_workspace_t& workspace
) {
    clear(workspace.buffers[0]);
    append_vertex(workspace.buffers[0], first);
    append_vertex(workspace.buffers[0], second);
    append_vertex(workspace.buffers[0], third);

    std::size_t source_index = 0;
    for (std::size_t plane = 0; plane < 6 && !workspace.buffers[source_index].vertices.empty(); ++plane) {
        const std::size_t destination_index = 1 - source_index;
        auto& source = workspace.buffers[source_index];
        auto& destination = workspace.buffers[destination_index];
        clear(destination);

        auto previous = view(source.vertices.back(), source.values);
        float previous_distance = clip_distance(previous, plane);
        bool previous_inside = 0.0F <= previous_distance;
        for (const auto& current_vertex : source.vertices) {
            const auto current = view(current_vertex, source.values);
            const float current_distance = clip_distance(current, plane);
            const bool current_inside = 0.0F <= current_distance;
            if (current_inside != previous_inside) {
                const float factor = previous_distance / (previous_distance - current_distance);
                append_interpolated_vertex(destination, previous, current, factor);
            }
            if (current_inside) {
                append_vertex(destination, current);
            }
            previous = current;
            previous_distance = current_distance;
            previous_inside = current_inside;
        }
        source_index = destination_index;
    }

    const auto& result = workspace.buffers[source_index].vertices;
    if (result.empty() || std::ranges::any_of(result, [](const auto& vertex) { return vertex.clip_position[3] == 0.0F; })) {
        return std::nullopt;
    }
    return source_index;
}

std::optional<screen_vertex_t> project(
    const pipeline_vertex_view_t& vertex,
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
    std::span<const varying_entry_t> inputs
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
    std::span<const varying_entry_t> inputs,
    software_shader::fragment_io_t& io
) {
    if (x < 0 || y < 0 || width <= x || height <= y) {
        return;
    }

    io.reset(
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
    const pipeline_vertex_view_t& vertex,
    varying_values_t& fragment_inputs,
    software_shader::fragment_io_t& fragment_io
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
                fragment_inputs.assign(screen->outputs.begin(), screen->outputs.end());
                shade_sample(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    x,
                    y,
                    depth,
                    screen->reciprocal_w,
                    true,
                    fragment_inputs,
                    fragment_io
                );
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
    const pipeline_vertex_view_t& first,
    const pipeline_vertex_view_t& second,
    clipping_workspace_t& clipping,
    varying_values_t& fragment_inputs,
    software_shader::fragment_io_t& fragment_io
) {
    const auto clipped_index = clip_line(first, second, clipping);
    if (!clipped_index) {
        return;
    }
    const auto& clipped = clipping.buffers[*clipped_index];
    const auto clipped_first = view(clipped.vertices[0], clipped.values);
    const auto clipped_second = view(clipped.vertices[1], clipped.values);
    const auto first_screen = project(clipped_first, width, height);
    const auto second_screen = project(clipped_second, width, height);
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
            fragment_inputs.clear();
            if (first_screen->outputs.size() != second_screen->outputs.size()) {
                throw std::logic_error("inconsistent line varying counts");
            }
            for (std::size_t index = 0; index < first_screen->outputs.size(); ++index) {
                const auto& [first_location, first_output] = first_screen->outputs[index];
                const auto& [second_location, second_output] = second_screen->outputs[index];
                if (first_location != second_location) {
                    throw std::logic_error("inconsistent line varying locations");
                }
                fragment_inputs.emplace_back(first_location, perspective_line(
                    first_output,
                    second_output,
                    factor,
                    first_screen->reciprocal_w,
                    second_screen->reciprocal_w,
                    reciprocal_w
                ));
            }
            const float ndc_z = (1.0F - factor) * first_screen->ndc_z + factor * second_screen->ndc_z;
            shade_sample(
                program,
                bindings,
                width,
                height,
                framebuffer,
                x,
                y,
                ndc_z * 0.5F + 0.5F,
                reciprocal_w,
                true,
                fragment_inputs,
                fragment_io
            );
        }

        if (x == target_x && y == target_y) {
            break;
        }
        const int twice_error = error * 2;
        if (dy <= twice_error) {
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
    return dy < 0.0F || (dy == 0.0F && 0.0F < dx);
}

bool covered(float edge_value, bool top_left) {
    return 0.0F < edge_value || (edge_value == 0.0F && top_left);
}

void rasterize_projected_triangle(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    screen_vertex_t first,
    screen_vertex_t second,
    screen_vertex_t third,
    varying_values_t& fragment_inputs,
    software_shader::fragment_io_t& fragment_io
) {
    const float front_area = edge(
        first.ndc_x, first.ndc_y,
        second.ndc_x, second.ndc_y,
        third.ndc_x, third.ndc_y
    );
    const bool front_facing = 0.0F < front_area;

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
    if (maximum_x < minimum_x || maximum_y < minimum_y) {
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
            const float interpolated_reciprocal_w = weights[0] * reciprocal_w[0] + weights[1] * reciprocal_w[1] + weights[2] * reciprocal_w[2];
            if (interpolated_reciprocal_w == 0.0F) {
                continue;
            }

            fragment_inputs.clear();
            if (first.outputs.size() != second.outputs.size() || first.outputs.size() != third.outputs.size()) {
                throw std::logic_error("inconsistent triangle varying counts");
            }
            for (std::size_t index = 0; index < first.outputs.size(); ++index) {
                const auto& [first_location, first_output] = first.outputs[index];
                const auto& [second_location, second_output] = second.outputs[index];
                const auto& [third_location, third_output] = third.outputs[index];
                if (first_location != second_location || first_location != third_location) {
                    throw std::logic_error("inconsistent triangle varying locations");
                }
                fragment_inputs.emplace_back(first_location, perspective_triangle(
                    first_output,
                    second_output,
                    third_output,
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
                fragment_inputs,
                fragment_io
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
    const pipeline_vertex_view_t& first,
    const pipeline_vertex_view_t& second,
    const pipeline_vertex_view_t& third,
    clipping_workspace_t& clipping,
    varying_values_t& fragment_inputs,
    software_shader::fragment_io_t& fragment_io
) {
    const auto clipped_index = clip_triangle(first, second, third, clipping);
    if (!clipped_index) {
        return;
    }

    const auto& polygon = clipping.buffers[*clipped_index];
    for (std::size_t index = 1; index + 1 < polygon.vertices.size(); ++index) {
        const auto projected_first = project(view(polygon.vertices[0], polygon.values), width, height);
        const auto projected_second = project(view(polygon.vertices[index], polygon.values), width, height);
        const auto projected_third = project(view(polygon.vertices[index + 1], polygon.values), width, height);
        if (projected_first && projected_second && projected_third) {
            rasterize_projected_triangle(
                program,
                bindings,
                width,
                height,
                framebuffer,
                *projected_first,
                *projected_second,
                *projected_third,
                fragment_inputs,
                fragment_io
            );
        }
    }
}

matrix4f_t object_to_world_matrix(const renderer::render_item_t& render_item) {
    const auto& translation = render_item.translation();
    const auto& scale = render_item.scale();
    const float sine = std::sin(render_item.rotation());
    const float cosine = std::cos(render_item.rotation());

    const matrix4f_t translation_matrix {
        1.0F, 0.0F, 0.0F, translation[0],
        0.0F, 1.0F, 0.0F, translation[1],
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    };
    const matrix4f_t rotation_matrix {
        cosine, -sine, 0.0F, 0.0F,
        sine, cosine, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    };
    const matrix4f_t scale_matrix {
        scale[0], 0.0F, 0.0F, 0.0F,
        0.0F, scale[1], 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    };
    return translation_matrix * rotation_matrix * scale_matrix;
}

matrix4f_t world_to_clip_matrix(
    const renderer::camera_t<float, int, 2>& camera,
    renderer::framebuffer_t framebuffer
) {
    const auto& world_rect = camera.world_rect();
    const auto& view_rect = camera.view_rect();
    const float world_width = world_rect[0].length();
    const float world_height = world_rect[1].length();
    const float view_world_scale_x = static_cast<float>(view_rect[0].length()) / world_width;
    const float view_world_scale_y = static_cast<float>(view_rect[1].length()) / world_height;
    const float framebuffer_width = static_cast<float>(framebuffer.width);
    const float framebuffer_height = static_cast<float>(framebuffer.height);
    const float scale_x = view_world_scale_x * 2.0F / framebuffer_width;
    const float scale_y = view_world_scale_y * -2.0F / framebuffer_height;
    const float offset_x = (static_cast<float>(view_rect[0][0]) - world_rect[0][0] * view_world_scale_x) * 2.0F / framebuffer_width - 1.0F;
    const float offset_y = 1.0F - (static_cast<float>(view_rect[1][0]) - world_rect[1][0] * view_world_scale_y) * 2.0F / framebuffer_height;

    return {
        scale_x, 0.0F, 0.0F, offset_x,
        0.0F, scale_y, 0.0F, offset_y,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    };
}

} // namespace

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

class software_renderer_t::scratch_t {
public:
    std::vector<pipeline_vertex_t> vertex_results;
    varying_values_t vertex_values;
    clipping_workspace_t clipping;
    varying_values_t fragment_inputs;
    software_shader::vertex_io_t vertex_io {0, 0};
    software_shader::fragment_io_t fragment_io {vector4f_t(0.0F), true};
};

software_renderer_t::software_renderer_t(framebuffer_t framebuffer):
    m_framebuffer(validated_framebuffer(framebuffer)),
    m_scratch(std::make_unique<scratch_t>())
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
    const render_item_t& render_item
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
    const auto& program = *material->program();
    const auto& bindings = material->bindings();
    program.validate_bindings(bindings);

    const auto& world_rect = camera.world_rect();
    const float world_width = world_rect[0].length();
    const float world_height = world_rect[1].length();
    if (world_width == 0.0F || world_height == 0.0F) {
        throw std::invalid_argument("software_renderer_t::draw requires non-empty camera world bounds");
    }

    draw_pipeline(program, bindings, *geometry, object_to_world_matrix(render_item), world_to_clip_matrix(camera, m_framebuffer));
}

void software_renderer_t::draw_pipeline(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    const geometry_t& geometry,
    const matrix4f_t& object_to_world,
    const matrix4f_t& world_to_clip
) {
    const int width = m_framebuffer.width;
    const int height = m_framebuffer.height;
    const auto framebuffer = m_framebuffer.pixels;

    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("software renderer pipeline requires a non-empty framebuffer");
    }
    const std::size_t expected_size = framebuffer_pixel_count(width, height);
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
        if (streams.size() <= input.index) {
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
    auto& scratch = *m_scratch;
    scratch.vertex_results.clear();
    scratch.vertex_values.clear();
    scratch.fragment_inputs.clear();
    scratch.vertex_results.reserve(indices.size());
    scratch.vertex_io.object_to_world(object_to_world);
    scratch.vertex_io.world_to_clip(world_to_clip);
    for (const std::uint32_t vertex_index : indices) {
        if (static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()) < vertex_index) {
            throw std::out_of_range("vertex index cannot be represented by software shader vertex_io_t");
        }
        if (mesh->number_of_vertices() <= vertex_index) {
            throw std::out_of_range("software renderer geometry index is outside the mesh");
        }

        auto& io = scratch.vertex_io;
        io.reset(static_cast<std::int32_t>(vertex_index), 0);
        for (const auto& input : program.vertex_interface().inputs()) {
            set_vertex_input(io, input, streams[input.index], attributes[input.index], vertex_index);
        }
        program.run(bindings, io);
        const vector4f_t clip_position = io.position();
        if (!finite(clip_position)) {
            throw std::runtime_error("vertex shader produced a non-finite clip position");
        }

        const std::size_t output_offset = scratch.vertex_values.size();
        for (const auto& input : program.fragment_interface().inputs()) {
            scratch.vertex_values.emplace_back(input.index, vertex_output(io, input));
        }
        scratch.vertex_results.push_back({
            .clip_position = clip_position,
            .outputs = {output_offset, program.fragment_interface().inputs().size()}
        });
    }

    const auto vertex = [&](std::size_t index) {
        return view(scratch.vertex_results[index], scratch.vertex_values);
    };
    switch (geometry.primitive_topology()) {
        case vertex_primitive_topology_t::point: {
            for (std::size_t index = 0; index < indices.size(); ++index) {
                rasterize_point(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(index),
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        case vertex_primitive_topology_t::line: {
            for (std::size_t index = 0; index + 1 < indices.size(); index += 2) {
                rasterize_line(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(index),
                    vertex(index + 1),
                    scratch.clipping,
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        case vertex_primitive_topology_t::line_strip: {
            for (std::size_t index = 0; index + 1 < indices.size(); ++index) {
                rasterize_line(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(index),
                    vertex(index + 1),
                    scratch.clipping,
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        case vertex_primitive_topology_t::line_loop: {
            for (std::size_t index = 0; index < indices.size(); ++index) {
                rasterize_line(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(index),
                    vertex((index + 1) % indices.size()),
                    scratch.clipping,
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        case vertex_primitive_topology_t::triangle: {
            for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
                rasterize_triangle(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(index),
                    vertex(index + 1),
                    vertex(index + 2),
                    scratch.clipping,
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        case vertex_primitive_topology_t::triangle_strip: {
            for (std::size_t index = 0; index + 2 < indices.size(); ++index) {
                if (index % 2 == 0) {
                    rasterize_triangle(
                        program,
                        bindings,
                        width,
                        height,
                        framebuffer,
                        vertex(index + 1),
                        vertex(index),
                        vertex(index + 2),
                        scratch.clipping,
                        scratch.fragment_inputs,
                        scratch.fragment_io
                    );
                } else {
                    rasterize_triangle(
                        program,
                        bindings,
                        width,
                        height,
                        framebuffer,
                        vertex(index),
                        vertex(index + 1),
                        vertex(index + 2),
                        scratch.clipping,
                        scratch.fragment_inputs,
                        scratch.fragment_io
                    );
                }
            }
        } break;
        case vertex_primitive_topology_t::triangle_fan: {
            for (std::size_t index = 1; index + 1 < indices.size(); ++index) {
                rasterize_triangle(
                    program,
                    bindings,
                    width,
                    height,
                    framebuffer,
                    vertex(0),
                    vertex(index),
                    vertex(index + 1),
                    scratch.clipping,
                    scratch.fragment_inputs,
                    scratch.fragment_io
                );
            }
        } break;
        default: {
            throw std::invalid_argument(std::format("unknown vertex primitive topology: {}", geometry.primitive_topology()));
        }
    }
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
