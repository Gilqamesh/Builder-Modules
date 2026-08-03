#include "glfw_window_renderer.h"

namespace m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer {

glfw_window_renderer_t::glfw_window_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window):
    m_window(std::move(window))
{
    if (!m_window) {
        throw std::invalid_argument("glfw_window_renderer_t: window is null");
    }
}

std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> glfw_window_renderer_t::window() const noexcept {
    return m_window;
}

} // namespace m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer
