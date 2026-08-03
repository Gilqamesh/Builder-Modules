#include "api.h"

#include <exception>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>

#include <m03gkcdy62bnz808pmk4uzkjra_glfw/glfw.h>
#include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>

namespace {

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;
namespace opengl_renderer_api = m03gl22hn0dqmosreqjie9tg5m_opengl_renderer;

using steady_clock_t = std::chrono::steady_clock;

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
        throw std::runtime_error("opengl_renderer_cli: failed to create shader");
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
    throw std::runtime_error(std::format("opengl_renderer_cli: shader compilation failed: {}", log));
}

GLuint create_program(const GladGLContext& gl) {
    static constexpr const char* vertex_source = R"(
#version 330 core

out vec3 vertex_color;

uniform float time_seconds;

const vec2 positions[3] = vec2[](
    vec2( 0.0,  0.62),
    vec2(-0.58, -0.42),
    vec2( 0.58, -0.42)
);

const vec3 colors[3] = vec3[](
    vec3(0.95, 0.28, 0.22),
    vec3(0.20, 0.72, 0.42),
    vec3(0.22, 0.42, 0.95)
);

void main() {
    float angle = time_seconds * 0.8;
    mat2 rotation = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
    gl_Position = vec4(rotation * positions[gl_VertexID], 0.0, 1.0);
    vertex_color = colors[gl_VertexID];
}
)";

    static constexpr const char* fragment_source = R"(
#version 330 core

in vec3 vertex_color;
layout(location = 0) out vec4 fragment_color;

uniform float time_seconds;

void main() {
    float pulse = 0.85 + 0.15 * sin(time_seconds * 2.0);
    fragment_color = vec4(vertex_color * pulse, 1.0);
}
)";

    const GLuint vertex_shader = compile_shader(gl, GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = 0;
    GLuint program = 0;

    try {
        fragment_shader = compile_shader(gl, GL_FRAGMENT_SHADER, fragment_source);
        program = gl.CreateProgram();

        if (program == 0) {
            throw std::runtime_error("opengl_renderer_cli: failed to create shader program");
        }

        gl.AttachShader(program, vertex_shader);
        gl.AttachShader(program, fragment_shader);
        gl.LinkProgram(program);

        GLint linked = GL_FALSE;
        gl.GetProgramiv(program, GL_LINK_STATUS, &linked);

        if (linked != GL_TRUE) {
            throw std::runtime_error(std::format("opengl_renderer_cli: program linking failed: {}", program_log(gl, program)));
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

class triangle_demo_t {
public:
    explicit triangle_demo_t(const GladGLContext& gl):
        m_gl(gl),
        m_program(0),
        m_vertex_array(0),
        m_time_location(-1)
    {
        m_program = create_program(m_gl);

        try {
            m_gl.GenVertexArrays(1, &m_vertex_array);
            if (m_vertex_array == 0) {
                throw std::runtime_error("opengl_renderer_cli: failed to create vertex array");
            }

            m_time_location = m_gl.GetUniformLocation(m_program, "time_seconds");
        } catch (...) {
            destroy();
            throw;
        }
    }

    ~triangle_demo_t() {
        destroy();
    }

    triangle_demo_t(const triangle_demo_t&) = delete;
    triangle_demo_t& operator=(const triangle_demo_t&) = delete;
    triangle_demo_t(triangle_demo_t&&) = delete;
    triangle_demo_t& operator=(triangle_demo_t&&) = delete;

    void render(int width, int height, float seconds) const {
        m_gl.Viewport(0, 0, width, height);
        m_gl.ClearColor(0.05f, 0.07f, 0.09f, 1.0f);
        m_gl.Clear(GL_COLOR_BUFFER_BIT);

        m_gl.UseProgram(m_program);
        if (m_time_location >= 0) {
            m_gl.Uniform1f(m_time_location, seconds);
        }
        m_gl.BindVertexArray(m_vertex_array);
        m_gl.DrawArrays(GL_TRIANGLES, 0, 3);
        m_gl.BindVertexArray(0);
        m_gl.UseProgram(0);
    }

private:
    void destroy() noexcept {
        if (m_vertex_array != 0) {
            m_gl.DeleteVertexArrays(1, &m_vertex_array);
            m_vertex_array = 0;
        }

        if (m_program != 0) {
            m_gl.DeleteProgram(m_program);
            m_program = 0;
        }
    }

private:
    const GladGLContext& m_gl;
    GLuint m_program;
    GLuint m_vertex_array;
    GLint m_time_location;
};

} // namespace

int main() {
    try {
        glfw_api::glfw_t glfw;

        glfw_api::window_creation_settings_t settings;
        settings.opengl(3, 3, glfw_api::opengl_profile_t::core);

        auto window = glfw_api::window_t::create("OpenGL Renderer", { 300, 200, 960, 540 }, settings);
        if (!window) {
            throw std::runtime_error("opengl_renderer_cli: failed to create window");
        }

        opengl_renderer_api::opengl_renderer_t opengl_renderer(window);
        window->swap_interval(1);

        const auto& gl = opengl_renderer.get_gl();
        const triangle_demo_t demo(gl);

        const auto started_at = steady_clock_t::now();
        auto previous_frame_started_at = started_at;

        while (!window->should_close()) {
            const auto frame_started_at = steady_clock_t::now();
            const auto seconds = std::chrono::duration<float>(frame_started_at - started_at).count();

            glfw.poll_events();

            const auto framebuffer_size = window->framebuffer_size();
            if (framebuffer_size[0] > 0 && framebuffer_size[1] > 0) {
                if (!window->make_context_current()) {
                    throw std::runtime_error("opengl_renderer_cli: failed to make the OpenGL context current");
                }

                demo.render(framebuffer_size[0], framebuffer_size[1], seconds);
                window->swap_buffers();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            const auto frame_time = std::chrono::duration<double, std::milli>(frame_started_at - previous_frame_started_at);
            previous_frame_started_at = frame_started_at;
            std::cout << std::format("frame: {:.2f} ms, framebuffer: {}x{}\n", frame_time.count(), framebuffer_size[0], framebuffer_size[1]);
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << std::format("opengl_renderer: {}\n", error.what());
        return 1;
    }
}
