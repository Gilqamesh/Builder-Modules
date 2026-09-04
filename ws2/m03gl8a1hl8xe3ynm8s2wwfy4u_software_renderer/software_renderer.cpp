#include "software_renderer.h"
#include "software_pipeline.h"

#include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/opengl_renderer_external.h>
#include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

namespace renderer = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace software_shader = m03gt1djvvy5atia5evkbg6rqy_software_shader;
namespace glfw = m03gkcdy62bnz808pmk4uzkjra_glfw;

using vector2f_t = shader::vector_t<float, 2>;
using vector4f_t = shader::vector_t<float, 4>;

constexpr std::uint32_t clip_scale_binding = 0;
constexpr std::uint32_t clip_offset_binding = 1;

constexpr renderer::rgba8_t ray_white() {
    return {245, 245, 245, 255};
}

software_shader::program_t make_software_program() {
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

std::string shader_log(const GladGLContext& gl, GLuint shader_object) {
    GLint length = 0;
    gl.GetShaderiv(shader_object, GL_INFO_LOG_LENGTH, &length);
    if (length <= 0) {
        return {};
    }

    std::string result(static_cast<std::size_t>(length), '\0');
    gl.GetShaderInfoLog(shader_object, length, nullptr, result.data());
    return result;
}

std::string program_log(const GladGLContext& gl, GLuint program) {
    GLint length = 0;
    gl.GetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length <= 0) {
        return {};
    }

    std::string result(static_cast<std::size_t>(length), '\0');
    gl.GetProgramInfoLog(program, length, nullptr, result.data());
    return result;
}

GLuint compile_shader(const GladGLContext& gl, GLenum type, const char* source) {
    const GLuint shader_object = gl.CreateShader(type);
    if (shader_object == 0) {
        throw std::runtime_error("software renderer presentation failed to create a shader");
    }

    gl.ShaderSource(shader_object, 1, &source, nullptr);
    gl.CompileShader(shader_object);

    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) {
        return shader_object;
    }

    const std::string log = shader_log(gl, shader_object);
    gl.DeleteShader(shader_object);
    throw std::runtime_error(std::format("software renderer presentation shader compilation failed: {}", log));
}

GLuint make_present_program(const GladGLContext& gl) {
    static constexpr const char* vertex_source = R"(
#version 330 core

out vec2 texture_coordinates;

const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

const vec2 coordinates[3] = vec2[](
    vec2(0.0,  1.0),
    vec2(2.0,  1.0),
    vec2(0.0, -1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    texture_coordinates = coordinates[gl_VertexID];
}
)";

    static constexpr const char* fragment_source = R"(
#version 330 core

in vec2 texture_coordinates;
layout(location = 0) out vec4 fragment_color;
uniform sampler2D image;

void main() {
    fragment_color = texture(image, texture_coordinates);
}
)";

    const GLuint vertex_shader = compile_shader(gl, GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = 0;
    GLuint program = 0;
    try {
        fragment_shader = compile_shader(gl, GL_FRAGMENT_SHADER, fragment_source);
        program = gl.CreateProgram();
        if (program == 0) {
            throw std::runtime_error("software renderer presentation failed to create a program");
        }
        gl.AttachShader(program, vertex_shader);
        gl.AttachShader(program, fragment_shader);
        gl.LinkProgram(program);

        GLint linked = GL_FALSE;
        gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            throw std::runtime_error(std::format("software renderer presentation program linking failed: {}", program_log(gl, program)));
        }
    } catch (...) {
        if (program != 0) {
            gl.DeleteProgram(program);
        }
        if (fragment_shader != 0) {
            gl.DeleteShader(fragment_shader);
        }
        gl.DeleteShader(vertex_shader);
        throw;
    }

    gl.DeleteShader(fragment_shader);
    gl.DeleteShader(vertex_shader);
    return program;
}

} // namespace

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

class software_renderer_t::presentation_t {
public:
    explicit presentation_t(std::shared_ptr<glfw::window_t> window);
    ~presentation_t();

    shader::vector_t<int, 2> framebuffer_size() const;
    void present(int width, int height, std::span<const rgba8_t> pixels);

private:
    void create_resources();
    void destroy_resources() noexcept;
    void resize_texture(int width, int height);

private:
    std::shared_ptr<glfw::window_t> m_window;
    GladGLContext m_gl;
    GLuint m_present_program;
    GLuint m_texture;
    GLuint m_vertex_array;
    int m_texture_width;
    int m_texture_height;
};

software_renderer_t::presentation_t::presentation_t(std::shared_ptr<glfw::window_t> window):
    m_window(std::move(window)),
    m_gl(),
    m_present_program(0),
    m_texture(0),
    m_vertex_array(0),
    m_texture_width(0),
    m_texture_height(0)
{
    if (!m_window) {
        throw std::invalid_argument("software_renderer_t requires a window");
    }
    if (m_window->client_api() != glfw::client_api_t::opengl) {
        throw std::invalid_argument("software_renderer_t presentation requires an OpenGL window");
    }
    if (!m_window->context_current(true)) {
        throw std::runtime_error("software_renderer_t failed to make its presentation context current");
    }
    if (gladLoadGLContext(&m_gl, glfw::get_proc_address) == 0) {
        throw std::runtime_error("software_renderer_t failed to load its presentation OpenGL context");
    }
    try {
        create_resources();
    } catch (...) {
        destroy_resources();
        throw;
    }
}

software_renderer_t::presentation_t::~presentation_t() {
    destroy_resources();
}

shader::vector_t<int, 2> software_renderer_t::presentation_t::framebuffer_size() const {
    return m_window->framebuffer_size();
}

void software_renderer_t::presentation_t::present(
    int width,
    int height,
    std::span<const rgba8_t> pixels
) {
    if (!m_window->context_current(true)) {
        throw std::runtime_error("software_renderer_t failed to make its presentation context current");
    }
    if (width != m_texture_width || height != m_texture_height) {
        resize_texture(width, height);
    }

    m_gl.Viewport(0, 0, width, height);
    m_gl.Disable(GL_DEPTH_TEST);
    m_gl.Disable(GL_CULL_FACE);
    m_gl.Disable(GL_BLEND);
    m_gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    m_gl.ActiveTexture(GL_TEXTURE0);
    m_gl.BindTexture(GL_TEXTURE_2D, m_texture);
    m_gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    m_gl.UseProgram(m_present_program);
    m_gl.BindVertexArray(m_vertex_array);
    m_gl.DrawArrays(GL_TRIANGLES, 0, 3);
    m_gl.BindVertexArray(0);
    m_gl.UseProgram(0);
    m_gl.BindTexture(GL_TEXTURE_2D, 0);
    m_window->swap_buffers();
}

void software_renderer_t::presentation_t::create_resources() {
    m_present_program = make_present_program(m_gl);

    m_gl.GenVertexArrays(1, &m_vertex_array);
    if (m_vertex_array == 0) {
        throw std::runtime_error("software renderer presentation failed to create a vertex array");
    }

    m_gl.GenTextures(1, &m_texture);
    if (m_texture == 0) {
        throw std::runtime_error("software renderer presentation failed to create a texture");
    }
    m_gl.BindTexture(GL_TEXTURE_2D, m_texture);
    m_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_gl.BindTexture(GL_TEXTURE_2D, 0);

    m_gl.UseProgram(m_present_program);
    const GLint image_location = m_gl.GetUniformLocation(m_present_program, "image");
    if (image_location >= 0) {
        m_gl.Uniform1i(image_location, 0);
    }
    m_gl.UseProgram(0);
}

void software_renderer_t::presentation_t::destroy_resources() noexcept {
    if (!m_window || !m_window->context_current(true)) {
        return;
    }
    if (m_texture != 0) {
        m_gl.DeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_vertex_array != 0) {
        m_gl.DeleteVertexArrays(1, &m_vertex_array);
        m_vertex_array = 0;
    }
    if (m_present_program != 0) {
        m_gl.DeleteProgram(m_present_program);
        m_present_program = 0;
    }
}

void software_renderer_t::presentation_t::resize_texture(int width, int height) {
    m_gl.BindTexture(GL_TEXTURE_2D, m_texture);
    m_gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_gl.BindTexture(GL_TEXTURE_2D, 0);
    m_texture_width = width;
    m_texture_height = height;
}

software_renderer_t::software_renderer_t(std::shared_ptr<glfw::window_t> window):
    m_presentation(std::make_unique<presentation_t>(std::move(window))),
    m_software_program(make_software_program()),
    m_width(0),
    m_height(0),
    m_frame_active(false)
{
}

software_renderer_t::~software_renderer_t() = default;

bool software_renderer_t::begin_frame() {
    return begin_frame(ray_white());
}

bool software_renderer_t::begin_frame(rgba8_t clear_color) {
    m_frame_active = false;
    const auto size = m_presentation->framebuffer_size();
    resize(size[0], size[1]);
    if (m_width == 0 || m_height == 0) {
        return false;
    }
    std::ranges::fill(m_pixels, clear_color);
    m_frame_active = true;
    return true;
}

void software_renderer_t::draw(
    const camera_t<float, int, 2>& camera,
    const render_item_t<float, 2>& render_item
) {
    if (!m_frame_active) {
        throw std::logic_error("software_renderer_t::draw requires an active frame");
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
        object_scale[0] * view_world_scale_x * 2.0F / static_cast<float>(m_width),
        object_scale[1] * view_world_scale_y * -2.0F / static_cast<float>(m_height)
    });
    const vector2f_t clip_offset({
        (static_cast<float>(view_rect[0][0]) + (translation[0] - world_rect[0][0]) * view_world_scale_x)
            * 2.0F / static_cast<float>(m_width) - 1.0F,
        1.0F - (static_cast<float>(view_rect[1][0]) + (translation[1] - world_rect[1][0]) * view_world_scale_y)
            * 2.0F / static_cast<float>(m_height)
    });

    software_shader::bindings_t bindings;
    bindings.uniform(clip_scale_binding, clip_scale);
    bindings.uniform(clip_offset_binding, clip_offset);

    bind_material_resources(m_software_program, *material, bindings);

    draw_software_pipeline(m_software_program, bindings, *geometry, m_width, m_height, m_pixels);
}

void software_renderer_t::present() {
    if (!m_frame_active) {
        return;
    }
    try {
        m_presentation->present(m_width, m_height, m_pixels);
    } catch (...) {
        m_frame_active = false;
        throw;
    }
    m_frame_active = false;
}

std::span<rgba8_t> software_renderer_t::pixels() noexcept {
    return m_pixels;
}

std::span<const rgba8_t> software_renderer_t::pixels() const noexcept {
    return m_pixels;
}

int software_renderer_t::width() const noexcept {
    return m_width;
}

int software_renderer_t::height() const noexcept {
    return m_height;
}

void software_renderer_t::resize(int width, int height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("software_renderer_t framebuffer dimensions must be non-negative");
    }
    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);
    if (width_size != 0 && height_size > std::numeric_limits<std::size_t>::max() / width_size) {
        throw std::length_error("software_renderer_t framebuffer size overflows size_t");
    }
    m_pixels.resize(width_size * height_size);
    m_width = width;
    m_height = height;
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
