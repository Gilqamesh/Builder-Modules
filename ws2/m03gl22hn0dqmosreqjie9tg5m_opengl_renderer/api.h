#ifndef M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_API_H
# define M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_API_H

# include "opengl_renderer_external.h"

# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
# include <m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer/glfw_window_renderer.h>

# include <cstddef>
# include <memory>
# include <span>

namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer {

class opengl_renderer_t : public m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer::glfw_window_renderer_t {
public:
    explicit opengl_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    ~opengl_renderer_t();

    /**
     * @brief Returns the OpenGL context associated with this renderer.
     */
    const GladGLContext& get_gl() const;

     /**
     * @brief Temporarily presents tightly packed RGBA8 pixels to the window.
     *
     * `width` and `height` are expected to match the current GLFW framebuffer size.
     */
    void present_rgba8(std::span<const std::byte> pixels, int width, int height);

private:
    void create_present_resources();
    void destroy_present_resources() noexcept;
    void resize_present_texture(int width, int height);

private:
    GladGLContext m_gl;
    GLuint m_present_program;
    GLuint m_present_texture;
    GLuint m_present_vertex_array;
    int m_present_texture_width;
    int m_present_texture_height;
};

} // namespace m03gl22hn0dqmosreqjie9tg5m_opengl_renderer

#endif // M03GL22HN0DQMOSREQJIE9TG5M_OPENGL_RENDERER_API_H
