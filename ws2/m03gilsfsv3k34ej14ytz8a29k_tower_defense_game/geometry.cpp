#include "geometry.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

geometry_t::geometry_t():
    m_primitive_topology(vertex_primitive_topology_t::triangle),
    m_index_range{0, 0}
{
}

void geometry_t::finalize() {
    if (!m_mesh) {
        throw std::runtime_error("geometry_t::finalize: mesh is not set");
    }

    if (!m_index_buffer) {
        throw std::runtime_error("geometry_t::finalize: index_buffer is not set");
    }

    const auto& indices = m_index_buffer->indices();

    if (indices.empty()) {
        throw std::runtime_error("geometry_t::finalize: does not support geometry with no indices");
    }

    if (m_index_range.end_index <= m_index_range.start_index) {
        throw std::runtime_error(std::format("geometry_t::finalize: index_range end_index ({}) is not greater than start_index ({})", m_index_range.end_index, m_index_range.start_index));
    }

    if (indices.size() <= m_index_range.end_index) {
        throw std::runtime_error(std::format("geometry_t::finalize: index_range end_index ({}) is out of bounds for index_buffer indices size ({})", m_index_range.end_index, indices.size()));
    }

    if (indices.size() <= m_index_range.start_index) {
        throw std::runtime_error(std::format("geometry_t::finalize: index_range start_index ({}) is out of bounds for index_buffer indices size ({})", m_index_range.start_index, indices.size()));
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
        default: throw std::runtime_error(std::format("geometry_t::finalize: unknown vertex_primitive_topology_t: {}", m_primitive_topology));
    }

    if (indices.size() < expected_minimum_index_count) {
        throw std::runtime_error(std::format("geometry_t::finalize: index count ({}) is less than expected minimum index count ({}) for vertex_primitive_topology_t: {}", indices.size(), expected_minimum_index_count, m_primitive_topology));
    }

    if (indices.size() % expected_index_count_divisor != 0) {
        throw std::runtime_error(std::format("geometry_t::finalize: index count ({}) is not divisible by expected index count divisor ({}) for vertex_primitive_topology_t: {}", indices.size(), expected_index_count_divisor, m_primitive_topology));
    }

    const auto& vertex_streams = m_mesh->vertex_streams();
    if (vertex_streams.size() == 0) {
        throw std::runtime_error("geometry_t::finalize: does not support mesh with no vertex streams");
    }
    const auto vertex_count = vertex_streams[0].element_count();
    for (size_t i = 1; i < vertex_streams.size(); ++i) {
        if (vertex_streams[i].element_count() != vertex_count) {
            throw std::runtime_error(std::format("geometry_t::finalize: vertex stream {} element count ({}) does not match vertex stream 0 element count ({})", i, vertex_streams[i].element_count(), vertex_count));
        }
    }
    for (auto index : indices) {
        if (vertex_count <= index) {
            throw std::runtime_error(std::format("geometry_t::finalize: index ({}) is out of bounds for vertex count ({})", index, vertex_count));
        }
    }
}

std::shared_ptr<mesh_t>& geometry_t::mesh() {
    return m_mesh;
}

std::shared_ptr<mesh_t> geometry_t::mesh() const {
    return m_mesh;
}

std::shared_ptr<index_buffer_t>& geometry_t::index_buffer() {
    return m_index_buffer;
}

std::shared_ptr<index_buffer_t> geometry_t::index_buffer() const {
    return m_index_buffer;
}

index_range_t& geometry_t::index_range() {
    return m_index_range;
}

index_range_t geometry_t::index_range() const {
    return m_index_range;
}

vertex_primitive_topology_t& geometry_t::primitive_topology() {
    return m_primitive_topology;
}

vertex_primitive_topology_t geometry_t::primitive_topology() const {
    return m_primitive_topology;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
