#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_OPENGL_SHADER_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_OPENGL_SHADER_H

# include "shader_builder.h"

# include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

class opengl_shader_t {
public:
    explicit opengl_shader_t(const shader_ast_t& shader_ast_t);

private:
    m03gl22hn0dqmosreqjie9tg5m_opengl_renderer::opengl_renderer_t* m_opengl_renderer;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_OPENGL_SHADER_H
