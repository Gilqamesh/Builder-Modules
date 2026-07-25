#include "material.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

material_t::material_t() = default;

material_t::material_t(std::shared_ptr<shader_t> shader):
    m_shader(std::move(shader))
{
}

void material_t::shader(std::shared_ptr<shader_t> shader) {
    m_shader = std::move(shader);
}

std::shared_ptr<shader_t> material_t::shader() const {
    return m_shader;
}

void material_t::set_texture_binding(size_t index, texture_binding_t binding) {
    if (m_texture_bindings.size() <= index) {
        m_texture_bindings.resize(index + 1);
    }
    m_texture_bindings[index] = std::move(binding);
}

const std::vector<texture_binding_t>& material_t::texture_bindings() const {
    return m_texture_bindings;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
