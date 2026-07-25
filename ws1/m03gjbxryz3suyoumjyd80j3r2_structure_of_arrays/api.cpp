#include "api.h"

namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays {

const erased_structure_of_arrays_t::data_t& erased_structure_of_arrays_t::data() const& {
    return m_data;
}

erased_structure_of_arrays_t::data_t erased_structure_of_arrays_t::data()&& {
    return std::move(m_data);
}

size_t erased_structure_of_arrays_t::size() const noexcept {
    return m_data.size();
}

const m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t& erased_structure_of_arrays_t::operator[](size_t index) const& {
    return m_data[index];
}

} // namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays
