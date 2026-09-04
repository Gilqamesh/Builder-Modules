#define GLAD_GL_IMPLEMENTATION
#include "api.h"

#include <cstddef>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

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
        throw std::runtime_error("opengl_renderer_t::present_rgba8 failed to create a shader");
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
    throw std::runtime_error(std::format("opengl_renderer_t::present_rgba8 shader compilation failed: {}", log));
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
            throw std::runtime_error("opengl_renderer_t::present_rgba8 failed to create a program");
        }
        gl.AttachShader(program, vertex_shader);
        gl.AttachShader(program, fragment_shader);
        gl.LinkProgram(program);

        GLint linked = GL_FALSE;
        gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            throw std::runtime_error(std::format("opengl_renderer_t::present_rgba8 program linking failed: {}", program_log(gl, program)));
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

namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer {

opengl_renderer_t::opengl_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window):
    glfw_window_renderer_t(window),
    m_gl(),
    m_present_program(0),
    m_present_texture(0),
    m_present_vertex_array(0),
    m_present_texture_width(0),
    m_present_texture_height(0)
{
    const auto expected_client_api = m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t::opengl;
    if (window->client_api() != expected_client_api) {
        throw std::runtime_error(std::format("opengl_renderer_t: expected window client API to be {}, got {}", expected_client_api, window->client_api()));
    }

    if (!window->context_current(true)) {
        throw std::runtime_error("opengl_renderer_t: failed to make the OpenGL context current");
    }

    const int version = gladLoadGLContext(&m_gl, m03gkcdy62bnz808pmk4uzkjra_glfw::get_proc_address);
    if (version == 0) {
        throw std::runtime_error("opengl_renderer_t: failed to load OpenGL context");
    }
}

opengl_renderer_t::~opengl_renderer_t() {
    destroy_present_resources();
}

const GladGLContext& opengl_renderer_t::get_gl() const {
    return m_gl;
}

void opengl_renderer_t::present_rgba8(
    std::span<const std::byte> pixels,
    int width,
    int height
) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("opengl_renderer_t::present_rgba8 requires positive dimensions");
    }

    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);
    if (height_size > std::numeric_limits<std::size_t>::max() / width_size) {
        throw std::length_error("opengl_renderer_t::present_rgba8 pixel count overflows size_t");
    }
    const std::size_t pixel_count = width_size * height_size;
    if (pixel_count > std::numeric_limits<std::size_t>::max() / 4) {
        throw std::length_error("opengl_renderer_t::present_rgba8 byte count overflows size_t");
    }
    if (pixels.size() != pixel_count * 4) {
        throw std::invalid_argument("opengl_renderer_t::present_rgba8 byte count does not match its dimensions");
    }

    const auto render_window = window();
    if (!render_window->context_current(true)) {
        throw std::runtime_error("opengl_renderer_t::present_rgba8 failed to make the OpenGL context current");
    }
    if (m_present_program == 0) {
        create_present_resources();
    }
    if (width != m_present_texture_width || height != m_present_texture_height) {
        resize_present_texture(width, height);
    }

    m_gl.Viewport(0, 0, width, height);
    m_gl.Disable(GL_DEPTH_TEST);
    m_gl.Disable(GL_CULL_FACE);
    m_gl.Disable(GL_BLEND);
    m_gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    m_gl.ActiveTexture(GL_TEXTURE0);
    m_gl.BindTexture(GL_TEXTURE_2D, m_present_texture);
    m_gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    m_gl.UseProgram(m_present_program);
    m_gl.BindVertexArray(m_present_vertex_array);
    m_gl.DrawArrays(GL_TRIANGLES, 0, 3);
    m_gl.BindVertexArray(0);
    m_gl.UseProgram(0);
    m_gl.BindTexture(GL_TEXTURE_2D, 0);
    render_window->swap_buffers();
}

void opengl_renderer_t::create_present_resources() {
    m_present_program = make_present_program(m_gl);
    try {
        m_gl.GenVertexArrays(1, &m_present_vertex_array);
        if (m_present_vertex_array == 0) {
            throw std::runtime_error("opengl_renderer_t::present_rgba8 failed to create a vertex array");
        }

        m_gl.GenTextures(1, &m_present_texture);
        if (m_present_texture == 0) {
            throw std::runtime_error("opengl_renderer_t::present_rgba8 failed to create a texture");
        }
        m_gl.BindTexture(GL_TEXTURE_2D, m_present_texture);
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
    } catch (...) {
        destroy_present_resources();
        throw;
    }
}

void opengl_renderer_t::destroy_present_resources() noexcept {
    const auto render_window = window();
    if (!render_window || !render_window->context_current(true)) {
        return;
    }
    if (m_present_texture != 0) {
        m_gl.DeleteTextures(1, &m_present_texture);
        m_present_texture = 0;
    }
    if (m_present_vertex_array != 0) {
        m_gl.DeleteVertexArrays(1, &m_present_vertex_array);
        m_present_vertex_array = 0;
    }
    if (m_present_program != 0) {
        m_gl.DeleteProgram(m_present_program);
        m_present_program = 0;
    }
    m_present_texture_width = 0;
    m_present_texture_height = 0;
}

void opengl_renderer_t::resize_present_texture(int width, int height) {
    m_gl.BindTexture(GL_TEXTURE_2D, m_present_texture);
    m_gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_gl.BindTexture(GL_TEXTURE_2D, 0);
    m_present_texture_width = width;
    m_present_texture_height = height;
}

} // namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer
