#include "vertex_attribute.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

size_t vertex_attribute_type_size(vertex_attribute_type_t type) {
    switch (type) {
        case vertex_attribute_type_t::R32: return 4;
        case vertex_attribute_type_t::R64: return 8;
        case vertex_attribute_type_t::I8:  return 1;
        case vertex_attribute_type_t::I16: return 2;
        case vertex_attribute_type_t::I32: return 4;
        case vertex_attribute_type_t::I64: return 8;
        case vertex_attribute_type_t::U8:  return 1;
        case vertex_attribute_type_t::U16: return 2;
        case vertex_attribute_type_t::U32: return 4;
        case vertex_attribute_type_t::U64: return 8;
        default: throw std::runtime_error("vertex_attribute_type_size: unknown vertex attribute type");
    }
}

vertex_attribute_t::vertex_attribute_t(): vertex_attribute_t(vertex_attribute_type_t::R32, 1)
{
}

vertex_attribute_t::vertex_attribute_t(vertex_attribute_type_t type, size_t component_count):
    m_type(type),
    m_component_count(component_count)
{
    if (m_component_count == 0) {
        throw std::runtime_error(std::format("vertex_attribute_t::vertex_attribute_t: component count must be greater than zero, got {}", m_component_count));
    }
}

void vertex_attribute_t::type(vertex_attribute_type_t type) {
    m_type = type;
}

vertex_attribute_type_t vertex_attribute_t::type() const {
    return m_type;
}

void vertex_attribute_t::component_count(size_t count) {
    if (count == 0) {
        throw std::runtime_error(std::format("vertex_attribute_t::component_count: component count must be greater than zero, got {}", count));
    }
    m_component_count = count;
}

size_t vertex_attribute_t::component_count() const {
    return m_component_count;
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
