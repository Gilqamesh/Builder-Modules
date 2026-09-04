#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_MESH_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_MESH_H

# include "vertex_attribute.h"

# include <vector>
# include <span>
# include <format>

# include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

class mesh_t {
public:
    mesh_t();

    template <typename... Ts>
    mesh_t(m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<Ts...> vertex_streams, std::vector<vertex_attribute_t> vertex_attributes);

    const m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t& vertex_streams() const&;
    std::span<const vertex_attribute_t> vertex_attributes() const&;

    size_t number_of_vertices() const;

private:
    m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t m_vertex_streams;
    std::vector<vertex_attribute_t> m_vertex_attributes;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::mesh_t>;

} // namespace std

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

template <typename... Ts>
mesh_t::mesh_t(m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<Ts...> vertex_streams, std::vector<vertex_attribute_t> vertex_attributes):
    m_vertex_streams(std::move(vertex_streams)),
    m_vertex_attributes(std::move(vertex_attributes))
{
    const auto vertex_stream_count = m_vertex_streams.size();
    const auto vertex_attribute_count = m_vertex_attributes.size();
    if (vertex_stream_count != vertex_attribute_count) {
        throw std::runtime_error(std::format("mesh_t::mesh_t: number of vertex streams ({}) does not match number of vertex attributes ({})", vertex_stream_count, vertex_attribute_count));
    }

    size_t element_count = 0;
    for (size_t i = 0; i < vertex_stream_count; ++i) {
        if (i == 0) {
            element_count = m_vertex_streams[i].element_count();
        } else if (m_vertex_streams[i].element_count() != element_count) {
            throw std::logic_error(std::format("mesh_t::mesh_t: vertex stream {} element count ({}) does not match vertex stream 0 element count ({})", i, m_vertex_streams[i].element_count(), element_count));
        }
        
        const auto& attribute = m_vertex_attributes[i];
        const auto& stream = m_vertex_streams[i];
        const auto stream_element_size = stream.element_size();
        const auto attribute_element_size = vertex_attribute_type_size(attribute.type()) * attribute.component_count();
        if (stream_element_size != attribute_element_size) {
            throw std::runtime_error(std::format("mesh_t::mesh_t: vertex stream {} element size ({}) does not match vertex attribute element size ({})", i, stream_element_size, attribute_element_size));
        }
    }
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::mesh_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::mesh_t& mesh, auto& ctx) const {
        auto out = ctx.out();

        out = format_to(out, "vertex_attributes: [ ");
        for (size_t i = 0; i < mesh.vertex_attributes().size(); ++i) {
            if (0 < i) {
                out = format_to(out, ",");
            }
            out = format_to(out, "{} ", mesh.vertex_attributes()[i]);
        }
        out = format_to(out, "], vertex_streams: {}", mesh.vertex_streams());

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_MESH_H
