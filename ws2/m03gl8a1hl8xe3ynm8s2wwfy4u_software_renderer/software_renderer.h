#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H

# include <memory>
# include <cstdint>
# include <span>
# include <vector>

# include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>
# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

struct rgba8_t {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;
};

class software_renderer_t : public m03gl22hn0dqmosreqjie9tg5m_opengl_renderer::opengl_renderer_t {
public:
    explicit software_renderer_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    ~software_renderer_t();

    /**
     * @brief Resizes the CPU framebuffer to the window framebuffer size.
     *
     * @return Whether the framebuffer has a non-zero size.
     */
    bool begin_frame();

    std::span<rgba8_t> pixels() noexcept;
    std::span<const rgba8_t> pixels() const noexcept;

    int width() const noexcept;
    int height() const noexcept;

    /**
     * @brief Uploads the CPU framebuffer and presents it.
     */
    void present();

private:
    void create_resources();
    void destroy_resources() noexcept;
    void resize(int width, int height);

private:
    std::vector<rgba8_t> m_pixels;
    int m_width;
    int m_height;
    GLuint m_program;
    GLuint m_texture;
    GLuint m_vertex_array;
};

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_RENDERER_H
