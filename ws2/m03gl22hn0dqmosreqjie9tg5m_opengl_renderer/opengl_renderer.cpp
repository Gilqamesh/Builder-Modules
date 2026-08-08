#define GLAD_GL_IMPLEMENTATION
#include "api.h"

#include <stdexcept>
#include <format>

namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer {

opengl_renderer_t::opengl_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window):
    glfw_window_renderer_t(window)
{
    const auto expected_client_api = m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t::opengl;
    if (window->client_api() != expected_client_api) {
        throw std::runtime_error(std::format("opengl_renderer_t: expected window client API to be {}, got {}", expected_client_api, window->client_api()));
    }

    if (!window->context_current(true)) {
        throw std::runtime_error("opengl_renderer_t: failed to make the OpenGL context current");
    }

    int version = gladLoadGLContext(&m_gl, m03gkcdy62bnz808pmk4uzkjra_glfw::get_proc_address);
    if (version == 0) {
        throw std::runtime_error("opengl_renderer_t: failed to load OpenGL context");
    }
}

const GladGLContext& opengl_renderer_t::get_gl() const {
    return m_gl;
}

} // namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer
