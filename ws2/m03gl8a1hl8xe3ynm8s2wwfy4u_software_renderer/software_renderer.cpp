#include "software_renderer.h"

#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using window_t = m03gkcdy62bnz808pmk4uzkjra_glfw::window_t;
using client_api_t = m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t;

std::string shader_log(const GladGLContext& gl, GLuint shader) {
    GLint length = 0;
    gl.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    if (length <= 0) {
        return {};
    }

    std::string result(static_cast<std::size_t>(length), '\0');
    gl.GetShaderInfoLog(shader, length, nullptr, result.data());
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
    const GLuint shader = gl.CreateShader(type);
    if (shader == 0) {
        throw std::runtime_error("software_renderer_t: failed to create shader");
    }

    gl.ShaderSource(shader, 1, &source, nullptr);
    gl.CompileShader(shader);

    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (compiled == GL_TRUE) {
        return shader;
    }

    const std::string log = shader_log(gl, shader);
    gl.DeleteShader(shader);
    throw std::runtime_error(std::format("software_renderer_t: shader compilation failed: {}", log));
}

GLuint create_program(const GladGLContext& gl) {
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
            throw std::runtime_error("software_renderer_t: failed to create shader program");
        }

        gl.AttachShader(program, vertex_shader);
        gl.AttachShader(program, fragment_shader);
        gl.LinkProgram(program);

        GLint linked = GL_FALSE;
        gl.GetProgramiv(program, GL_LINK_STATUS, &linked);

        if (linked != GL_TRUE) {
            throw std::runtime_error(std::format("software_renderer_t: program linking failed: {}", program_log(gl, program)));
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

software_renderer_t::software_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window):
    m03gl22hn0dqmosreqjie9tg5m_opengl_renderer::opengl_renderer_t(window),
    m_width(0),
    m_height(0),
    m_program(0),
    m_texture(0),
    m_vertex_array(0)
{
    try {
        create_resources();
    
        const auto size = window->framebuffer_size();
        resize(size[0], size[1]);
    } catch (...) {
        destroy_resources();
        throw;
    }
}

software_renderer_t::~software_renderer_t() {
    destroy_resources();
}

bool software_renderer_t::begin_frame() {
    if (!window()->make_context_current()) {
        throw std::runtime_error("software_renderer_t::begin_frame: failed to make the OpenGL context current");
    }

    const auto size = window()->framebuffer_size();

    if (size[0] != m_width || size[1] != m_height) {
        resize(size[0], size[1]);
    }

    return m_width > 0 && m_height > 0;
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

void software_renderer_t::present() {
    if (m_width == 0 || m_height == 0) {
        return;
    }

    if (!window()->make_context_current()) {
        throw std::runtime_error("software_renderer_t::present: failed to make the OpenGL context current");
    }

    const GladGLContext& gl = get_gl();

    gl.Viewport(0, 0, m_width, m_height);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_CULL_FACE);
    gl.Disable(GL_BLEND);
    gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl.ActiveTexture(GL_TEXTURE0);
    gl.BindTexture(GL_TEXTURE_2D, m_texture);
    gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
    gl.UseProgram(m_program);
    gl.BindVertexArray(m_vertex_array);
    gl.DrawArrays(GL_TRIANGLES, 0, 3);
    gl.BindVertexArray(0);
    gl.UseProgram(0);
    gl.BindTexture(GL_TEXTURE_2D, 0);

    window()->swap_buffers();
}

void software_renderer_t::create_resources() {
    if (!window()->make_context_current()) {
        throw std::runtime_error("software_renderer_t::create_resources: failed to make the OpenGL context current");
    }

    const GladGLContext& gl = get_gl();

    m_program = create_program(gl);

    gl.GenVertexArrays(1, &m_vertex_array);
    if (m_vertex_array == 0) {
        throw std::runtime_error("software_renderer_t::create_resources: failed to create vertex array");
    }

    gl.GenTextures(1, &m_texture);
    if (m_texture == 0) {
        throw std::runtime_error("software_renderer_t::create_resources: failed to create texture");
    }

    gl.BindTexture(GL_TEXTURE_2D, m_texture);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl.BindTexture(GL_TEXTURE_2D, 0);

    gl.UseProgram(m_program);

    const GLint image_location = gl.GetUniformLocation(m_program, "image");
    if (image_location >= 0) {
        gl.Uniform1i(image_location, 0);
    }

    gl.UseProgram(0);
}

void software_renderer_t::destroy_resources() noexcept {
    if (!window() || !window()->make_context_current()) {
        return;
    }

    const GladGLContext& gl = get_gl();

    if (m_texture != 0) {
        gl.DeleteTextures(1, &m_texture);
        m_texture = 0;
    }

    if (m_vertex_array != 0) {
        gl.DeleteVertexArrays(1, &m_vertex_array);
        m_vertex_array = 0;
    }

    if (m_program != 0) {
        gl.DeleteProgram(m_program);
        m_program = 0;
    }
}

void software_renderer_t::resize(int width, int height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("software_renderer_t::resize: width and height must be non-negative");
    }

    if (width == m_width && height == m_height) {
        return;
    }

    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);

    if (width_size != 0 && height_size > std::numeric_limits<std::size_t>::max() / width_size) {
        throw std::length_error("software_renderer_t::resize: pixel count overflows size_t");
    }

    m_pixels.resize(width_size * height_size);
    m_width = width;
    m_height = height;

    if (width == 0 || height == 0) {
        return;
    }

    if (!window()->make_context_current()) {
        throw std::runtime_error("software_renderer_t::resize: failed to make the OpenGL context current");
    }

    const GladGLContext& gl = get_gl();

    gl.BindTexture(GL_TEXTURE_2D, m_texture);
    gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl.BindTexture(GL_TEXTURE_2D, 0);
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
