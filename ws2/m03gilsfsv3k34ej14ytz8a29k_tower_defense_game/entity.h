#ifndef M03GILSFSV3K34EJ14YTZ8A29K_ENTITY_H
# define M03GILSFSV3K34EJ14YTZ8A29K_ENTITY_H

# include "mesh.h"
# include "material.h"
# include "vertex_primitive_topology.h"

# include <memory>
# include <format>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

template <typename T, std::size_t N>
class entity_t {
public:
    entity_t();

    /**
     * Ensures that the entity's index count is valid for its primitive topology.
     * Must have at least 1 drawn primitive.
     */
    void finalize();

    void mesh(std::shared_ptr<mesh_t> mesh);
    std::shared_ptr<mesh_t> mesh() const;

    void indices(const std::vector<uint32_t>& indices);
    const std::vector<uint32_t>& indices() const;

    void primitive_topology(vertex_primitive_topology_t primitive_topology);
    vertex_primitive_topology_t primitive_topology() const;

    void material(std::shared_ptr<material_t> material);
    std::shared_ptr<material_t> material() const;

    void translation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation() const;

    void rotation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation() const;

    void scale(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale() const;

private:
    std::shared_ptr<mesh_t> m_mesh;
    std::vector<uint32_t> m_indices;
    vertex_primitive_topology_t m_primitive_topology;
    std::shared_ptr<material_t> m_material;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_translation;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_rotation;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_scale;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <typename T, std::size_t N>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::entity_t<T, N>>;

} // namespace std

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

template <typename T, std::size_t N>
entity_t<T, N>::entity_t():
    m_primitive_topology(vertex_primitive_topology_t::triangle),
    m_translation(static_cast<T>(0)),
    m_rotation(static_cast<T>(0)),
    m_scale(static_cast<T>(1))
{
}

template <typename T, std::size_t N>
void entity_t<T, N>::finalize() {
    if (!m_mesh) {
        throw std::runtime_error("entity_t::finalize: mesh is not set");
    }

    if (!m_material) {
        throw std::runtime_error("entity_t::finalize: material is not set");
    }

    if (m_indices.empty()) {
        throw std::runtime_error("entity_t::finalize: does not support entity with no indices");
    }

    size_t expected_index_count_divisor = 1;
    size_t expected_minimum_index_count = 0;
    switch (m_primitive_topology) {
        case vertex_primitive_topology_t::point: {
            expected_index_count_divisor = 1;
            expected_minimum_index_count = 1;
        } break;
        case vertex_primitive_topology_t::line: {
            expected_index_count_divisor = 2;
            expected_minimum_index_count = 2;
        } break;
        case vertex_primitive_topology_t::line_strip: {
            expected_index_count_divisor = 1;
            expected_minimum_index_count = 2;
        } break;
        case vertex_primitive_topology_t::line_loop: {
            expected_index_count_divisor = 1;
            expected_minimum_index_count = 2;
        } break;
        case vertex_primitive_topology_t::triangle: {
            expected_index_count_divisor = 3;
            expected_minimum_index_count = 3;
        } break;
        case vertex_primitive_topology_t::triangle_strip: {
            expected_index_count_divisor = 1;
            expected_minimum_index_count = 3;
        } break;
        case vertex_primitive_topology_t::triangle_fan: {
            expected_index_count_divisor = 1;
            expected_minimum_index_count = 3;
        } break;
        default: throw std::runtime_error(std::format("entity_t::finalize: unknown vertex_primitive_topology_t: {}", m_primitive_topology));
    }

    if (m_indices.size() < expected_minimum_index_count) {
        throw std::runtime_error(std::format("entity_t::finalize: index count ({}) is less than expected minimum index count ({}) for vertex_primitive_topology_t: {}", m_indices.size(), expected_minimum_index_count, m_primitive_topology));
    }

    if (m_indices.size() % expected_index_count_divisor != 0) {
        throw std::runtime_error(std::format("entity_t::finalize: index count ({}) is not divisible by expected index count divisor ({}) for vertex_primitive_topology_t: {}", m_indices.size(), expected_index_count_divisor, m_primitive_topology));
    }

    const auto& vertex_streams = m_mesh->vertex_streams();
    if (vertex_streams.size() == 0) {
        throw std::runtime_error("entity_t::finalize: does not support mesh with no vertex streams");
    }
    const auto vertex_count = vertex_streams[0].element_count();
    for (size_t i = 1; i < vertex_streams.size(); ++i) {
        if (vertex_streams[i].element_count() != vertex_count) {
            throw std::runtime_error(std::format("entity_t::finalize: vertex stream {} element count ({}) does not match vertex stream 0 element count ({})", i, vertex_streams[i].element_count(), vertex_count));
        }
    }
    for (auto index : m_indices) {
        if (vertex_count <= index) {
            throw std::runtime_error(std::format("entity_t::finalize: index ({}) is out of bounds for vertex count ({})", index, vertex_count));
        }
    }
}

template <typename T, std::size_t N>
void entity_t<T, N>::mesh(std::shared_ptr<mesh_t> mesh) {
    m_mesh = mesh;
}

template <typename T, std::size_t N>
std::shared_ptr<mesh_t> entity_t<T, N>::mesh() const {
    return m_mesh;
}

template <typename T, std::size_t N>
void entity_t<T, N>::indices(const std::vector<uint32_t>& indices) {
    m_indices = indices;
}

template <typename T, std::size_t N>
const std::vector<uint32_t>& entity_t<T, N>::indices() const {
    return m_indices;
}

template <typename T, std::size_t N>
void entity_t<T, N>::primitive_topology(vertex_primitive_topology_t primitive_topology) {
    m_primitive_topology = primitive_topology;
}

template <typename T, std::size_t N>
vertex_primitive_topology_t entity_t<T, N>::primitive_topology() const {
    return m_primitive_topology;
}

template <typename T, std::size_t N>
void entity_t<T, N>::material(std::shared_ptr<material_t> material) {
    m_material = material;
}

template <typename T, std::size_t N>
std::shared_ptr<material_t> entity_t<T, N>::material() const {
    return m_material;
}

template <typename T, std::size_t N>
void entity_t<T, N>::translation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation) {
    m_translation = translation;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& entity_t<T, N>::translation() const {
    return m_translation;
}

template <typename T, std::size_t N>
void entity_t<T, N>::rotation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation) {
    m_rotation = rotation;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& entity_t<T, N>::rotation() const {
    return m_rotation;
}

template <typename T, std::size_t N>
void entity_t<T, N>::scale(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale) {
    m_scale = scale;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& entity_t<T, N>::scale() const {
    return m_scale;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <typename T, std::size_t N>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::entity_t<T, N>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid entity_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::entity_t<T, N>& entity, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        {
            if (entity.mesh()) {
                out = std::format_to(out, "mesh: {}, ", *entity.mesh());
            } else {
                out = std::format_to(out, "mesh: -, ");
            }
        }
        
        {
            out = std::format_to(out, "indices: [");
            const auto& indices = entity.indices();
            for (size_t i = 0; i < indices.size(); ++i) {
                if (0 < i) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}", indices[i]);
            }
            out = std::format_to(out, "], ");
        }

        {
            out = std::format_to(out, "primitive_topology: {}, ", entity.primitive_topology());
        }

        {
            if (entity.material()) {
                out = std::format_to(out, "material: {}, ", *entity.material());
            } else {
                out = std::format_to(out, "material: -, ");
            }
        }

        {
            out = std::format_to(out, "translation: {}, ", entity.translation());
        }

        {
            out = std::format_to(out, "rotation: {}, ", entity.rotation());
        }

        {
            out = std::format_to(out, "scale: {}", entity.scale());
        }

        out = std::format_to(out, " }}");

        return out;
    }
};

}

#endif // M03GILSFSV3K34EJ14YTZ8A29K_ENTITY_H
