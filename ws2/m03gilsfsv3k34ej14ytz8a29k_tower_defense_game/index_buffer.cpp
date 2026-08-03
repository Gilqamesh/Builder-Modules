#include "index_buffer.h"

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

index_buffer_t::indices_t& index_buffer_t::indices() {
    return m_indices;
}

const index_buffer_t::indices_t& index_buffer_t::indices() const {
    return m_indices;
}

index_buffer_t::indices_t::iterator index_buffer_t::begin() {
    return m_indices.begin();
}

index_buffer_t::indices_t::const_iterator index_buffer_t::begin() const {
    return m_indices.begin();
}

index_buffer_t::indices_t::iterator index_buffer_t::end() {
    return m_indices.end();
}

index_buffer_t::indices_t::const_iterator index_buffer_t::end() const {
    return m_indices.end();
}

index_buffer_t::index_t& index_buffer_t::operator[](size_t index) {
    return m_indices[index];
}

const index_buffer_t::index_t& index_buffer_t::operator[](size_t index) const {
    return m_indices[index];
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
