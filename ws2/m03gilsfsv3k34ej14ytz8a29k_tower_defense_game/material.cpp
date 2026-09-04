#include "material.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

material_t::material_t()
{
}

std::vector<texture_binding_t>& material_t::texture_bindings() {
    return m_texture_bindings;
}

const std::vector<texture_binding_t>& material_t::texture_bindings() const {
    return m_texture_bindings;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
