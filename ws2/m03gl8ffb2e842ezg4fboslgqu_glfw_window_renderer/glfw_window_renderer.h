#ifndef M03GL8FFB2E842EZG4FBOSLGQU_GLFW_WINDOW_RENDERER_GLFW_WINDOW_RENDERER_H
# define M03GL8FFB2E842EZG4FBOSLGQU_GLFW_WINDOW_RENDERER_GLFW_WINDOW_RENDERER_H

# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>

namespace m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer {

class glfw_window_renderer_t {
public:
    /**
     * @brief Creates a new GLFW window renderer.
     * 
     * @throws std::invalid_argument if the window is null.
     */
    explicit glfw_window_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    virtual ~glfw_window_renderer_t() = default;

    std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window() const noexcept;

    glfw_window_renderer_t(const glfw_window_renderer_t&) = delete;
    glfw_window_renderer_t& operator=(const glfw_window_renderer_t&) = delete;
    glfw_window_renderer_t(glfw_window_renderer_t&&) = delete;
    glfw_window_renderer_t& operator=(glfw_window_renderer_t&&) = delete;

private:
    std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> m_window;
};

} // namespace m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer

#endif // M03GL8FFB2E842EZG4FBOSLGQU_GLFW_WINDOW_RENDERER_GLFW_WINDOW_RENDERER_H
