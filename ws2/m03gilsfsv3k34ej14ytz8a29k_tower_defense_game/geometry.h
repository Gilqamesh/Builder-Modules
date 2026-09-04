#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_GEOMETRY_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_GEOMETRY_H

# include "mesh.h"
# include "index_buffer.h"
# include "vertex_primitive_topology.h"

# include <vector>
# include <memory>
# include <format>
# include <stdexcept>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

struct index_range_t {
    std::uint32_t start_index;
    std::uint32_t end_index;
};

class geometry_t {
public:
    geometry_t();

    void finalize();

    std::shared_ptr<mesh_t>& mesh();
    std::shared_ptr<mesh_t> mesh() const;

    std::shared_ptr<index_buffer_t>& index_buffer();
    std::shared_ptr<index_buffer_t> index_buffer() const;

    index_range_t& index_range();
    index_range_t index_range() const;

    vertex_primitive_topology_t& primitive_topology();
    vertex_primitive_topology_t primitive_topology() const;

private:
    std::shared_ptr<mesh_t> m_mesh;
    std::shared_ptr<index_buffer_t> m_index_buffer;
    vertex_primitive_topology_t m_primitive_topology;
    index_range_t m_index_range;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::index_range_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid index_range_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::index_range_t& index_range, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "start_index: {}", index_range.start_index);
        out = std::format_to(out, ", end_index: {}", index_range.end_index);

        out = std::format_to(out, " }}");

        return out;
    }
};

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::geometry_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::geometry_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid geometry_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::geometry_t& geometry, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        const auto& mesh = geometry.mesh();
        if (mesh) {
            out = std::format_to(out, "mesh: {}", *mesh);
        } else {
            out = std::format_to(out, "mesh: -");
        }

        const auto& index_buffer = geometry.index_buffer();
        if (index_buffer) {
            out = std::format_to(out, ", index_buffer: {}", *index_buffer);
        } else {
            out = std::format_to(out, ", index_buffer: -");
        }

        out = std::format_to(out, ", index_range: {}", geometry.index_range());

        out = std::format_to(out, ", primitive_topology: {}", geometry.primitive_topology());

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_GEOMETRY_H
