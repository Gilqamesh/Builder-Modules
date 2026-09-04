#ifndef M03GILSFSV3K34EJ14YTZ8A29K_GAME_H
# define M03GILSFSV3K34EJ14YTZ8A29K_GAME_H

# include "camera.h"
# include "render_item.h"
# include "renderer3.h"

# include <vector>
# include <memory>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>
# include <m03gkcdy62bnz808pmk4uzkjra_glfw/glfw.h>
# include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

class game_t {
public:
    game_t();
    ~game_t();

    void run();

private:
    void update(float dt);
    void render();

private:
    m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_t glfw;
    std::shared_ptr<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t> m_window;
    std::shared_ptr<renderer3_t> m_renderer;
    camera_t<float, int, 2> m_camera;
    std::vector<render_item_t<float, 2>> m_render_items;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

#endif // M03GILSFSV3K34EJ14YTZ8A29K_GAME_H
