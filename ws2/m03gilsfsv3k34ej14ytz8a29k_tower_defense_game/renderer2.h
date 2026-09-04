#ifndef M03GILSFSV3K34EJ14YTZ8A29K_RENDERER2_H
# define M03GILSFSV3K34EJ14YTZ8A29K_RENDERER2_H

# include "camera.h"
# include "render_item.h"

# include <cstdint>
# include <format>
# include <memory>

# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
# include <m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_renderer.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

struct renderer2_color_t {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;
};

class renderer2_t {
public:
    explicit renderer2_t(std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> window);
    ~renderer2_t() = default;

    renderer2_t(const renderer2_t&) = delete;
    renderer2_t& operator=(const renderer2_t&) = delete;
    renderer2_t(renderer2_t&&) = delete;
    renderer2_t& operator=(renderer2_t&&) = delete;

    bool begin_frame();
    bool begin_frame(renderer2_color_t clear_color);
    void present();

    int width() const noexcept;
    int height() const noexcept;

    void draw(const camera_t<float, int, 2>& camera, const render_item_t<float, 2>& render_item);

private:
    m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::software_renderer_t m_software_renderer;
    bool m_frame_active;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::renderer2_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::renderer2_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid renderer2_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::renderer2_t& renderer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ width: {}, height: {} }}", renderer.width(), renderer.height());

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_RENDERER2_H
