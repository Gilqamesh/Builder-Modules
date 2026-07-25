#ifndef M03GILSFSV3K34EJ14YTZ8A29K_GAME_H
# define M03GILSFSV3K34EJ14YTZ8A29K_GAME_H

# include "camera.h"
# include "entity.h"
# include "renderer.h"

# include <vector>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

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
    renderer_t m_renderer;
    camera_t<float, int, 2> m_camera;
    std::vector<entity_t<float, 2>> m_entities;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

#endif // M03GILSFSV3K34EJ14YTZ8A29K_GAME_H
