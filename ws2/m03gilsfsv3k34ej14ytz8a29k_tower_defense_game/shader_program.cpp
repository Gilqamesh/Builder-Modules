#include "shader_program.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

shader_program_t::shader_program_t() {
}

void shader_program_t::finalize() {
}

std::shared_ptr<shader_t>& shader_program_t::shader(shader_type_t type) {
    const auto index = static_cast<size_t>(type);
    if (m_shaders.size() <= index) {
        throw std::out_of_range(std::format("shader_program_t::shader: invalid shader_type_t value: {}", static_cast<int>(type)));
    }

    return m_shaders[index];
}

std::shared_ptr<shader_t> shader_program_t::shader(shader_type_t type) const
{
    const auto index = static_cast<size_t>(type);
    if (m_shaders.size() <= index) {
        throw std::out_of_range(std::format("shader_program_t::shader: invalid shader_type_t value: {}", static_cast<int>(type)));
    }

    return m_shaders[index];
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
