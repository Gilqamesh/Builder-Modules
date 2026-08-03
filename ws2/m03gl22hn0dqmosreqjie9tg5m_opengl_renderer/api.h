#ifndef M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_OPENGL_RENDERER_H
# define M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_OPENGL_RENDERER_H

# include "opengl_renderer_external.h"

# include <memory>

# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
# include <m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer/glfw_window_renderer.h>

namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer {

class opengl_renderer_t : public m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer::glfw_window_renderer_t {
public:
    explicit opengl_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    ~opengl_renderer_t() = default;

    /**
     * @brief Returns the OpenGL context associated with this renderer.
     */
    const GladGLContext& get_gl() const;

private:
    GladGLContext m_gl;
};

} // namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer

#endif // M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_OPENGL_RENDERER_H
