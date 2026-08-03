#include "material.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

material_t::material_t()
{
}

material_t::material_t(std::shared_ptr<shader_program_t> shader_program):
    m_shader(std::move(shader_program))
{
}

std::shared_ptr<shader_program_t>& material_t::shader_program() {
    return m_shader;
}

std::shared_ptr<shader_program_t> material_t::shader_program() const {
    return m_shader;
}

std::vector<texture_binding_t>& material_t::texture_bindings() {
    return m_texture_bindings;
}

const std::vector<texture_binding_t>& material_t::texture_bindings() const {
    return m_texture_bindings;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
