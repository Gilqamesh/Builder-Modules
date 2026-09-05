#include "software_renderer.h"

#include <m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/detail/rasterization.h>
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
namespace raster = renderer::detail;
namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace software_shader = m03gt1djvvy5atia5evkbg6rqy_software_shader;
namespace type_erased_array = m03gjfvd6i5jzbmngb2ldoooza_type_erased_array;

using vector2f_t = shader::vector_t<float, 2>;
using vector3f_t = shader::vector_t<float, 3>;
using vector4f_t = shader::vector_t<float, 4>;
using matrix4f_t = shader::matrix_t<float, 4, 4>;
using raster::varying_t;
using raster::varying_entry_t;
using raster::varying_values_t;
using raster::pipeline_vertex_t;
using raster::pipeline_vertex_view_t;
using raster::clipping_workspace_t;
using raster::view;
using raster::clip_line;
using raster::inside_clip_volume;

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

struct screen_vertex_t {
    float x;
    float y;
    float ndc_x;
    float ndc_y;
    float ndc_z;
    float reciprocal_w;
    std::span<const varying_entry_t> m_outputs;
};

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

std::optional<screen_vertex_t> project(
    const pipeline_vertex_view_t& vertex,
    int width,
    int height
) {
    const float w = vertex.m_clip_position[3];
    if (w == 0.0F) {
        return std::nullopt;
    }
    const float reciprocal_w = float(raster::projectable_reciprocal_w(w));
    const float ndc_x = vertex.m_clip_position[0] * reciprocal_w;
    const float ndc_y = vertex.m_clip_position[1] * reciprocal_w;
    const float ndc_z = vertex.m_clip_position[2] * reciprocal_w;
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
        .m_outputs = vertex.m_outputs
    };
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
                fragment_inputs.assign(screen->m_outputs.begin(), screen->m_outputs.end());
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
    const auto& clipped = clipping.m_buffers[*clipped_index];
    const auto clipped_first = view(clipped.m_vertices[0], clipped.m_values);
    const auto clipped_second = view(clipped.m_vertices[1], clipped.m_values);
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

    const std::array<raster::projected_vertex_t, 2> endpoints {{
        {{0, 0}, first_screen->ndc_z, first_screen->reciprocal_w, clipped_first},
        {{0, 0}, second_screen->ndc_z, second_screen->reciprocal_w, clipped_second}
    }};
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

        const raster::sample_t sample {x, y, {0, 1, 0, 0}, {1.0 - double(factor), double(factor), 0.0, 0.0}, 2};
        const auto depth_w = raster::interpolate_sample(endpoints, sample, fragment_inputs);
        shade_sample(program, bindings, width, height, framebuffer, x, y,
            float(depth_w[0]), float(depth_w[1]), true, fragment_inputs, fragment_io);

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

void rasterize_triangle(
    const software_shader::program_t& program,
    const software_shader::bindings_t& bindings,
    int width,
    int height,
    std::span<renderer::rgba8_t> framebuffer,
    const pipeline_vertex_view_t& first,
    const pipeline_vertex_view_t& second,
    const pipeline_vertex_view_t& third,
    raster::raster_workspace_t& workspace,
    varying_values_t& fragment_inputs,
    software_shader::fragment_io_t& fragment_io
) {
    raster::prepare_triangle(first, second, third, width, height, workspace);
    raster::visit_samples(workspace, width, height, [&](const raster::sample_t& sample) {
        const auto depth_w = raster::interpolate_sample(workspace.m_vertices, sample, fragment_inputs);
        shade_sample(program, bindings, width, height, framebuffer,
            sample.m_x, sample.m_y, float(depth_w[0]), float(depth_w[1]),
            workspace.m_front_facing, fragment_inputs, fragment_io);
    });
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
    raster::raster_workspace_t raster;
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

    if (raster::maximum_extent < m_framebuffer.width || raster::maximum_extent < m_framebuffer.height) {
        throw std::out_of_range(std::format("software_renderer_t::draw dimensions {}x{} exceed the supported limit {}", m_framebuffer.width, m_framebuffer.height, raster::maximum_extent));
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
            .m_clip_position = clip_position,
            .m_outputs = {output_offset, program.fragment_interface().inputs().size()}
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
                    scratch.raster,
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
                        scratch.raster,
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
                        scratch.raster,
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
                    scratch.raster,
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
