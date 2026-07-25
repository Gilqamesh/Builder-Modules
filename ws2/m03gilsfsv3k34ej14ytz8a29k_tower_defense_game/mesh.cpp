#include "mesh.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

mesh_t::mesh_t()
{
}

const m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t& mesh_t::vertex_streams() const& {
    return m_vertex_streams;
}

std::span<const vertex_attribute_t> mesh_t::vertex_attributes() const& {
    return std::span<const vertex_attribute_t>(m_vertex_attributes.begin(), m_vertex_attributes.end());
}

size_t mesh_t::number_of_vertices() const {
    if (m_vertex_streams.size() == 0) {
        return 0;
    }

    return m_vertex_streams[0].element_count();
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
