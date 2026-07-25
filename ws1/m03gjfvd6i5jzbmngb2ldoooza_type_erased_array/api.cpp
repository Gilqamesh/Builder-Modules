#include "api.h"

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

type_erased_array_t::type_erased_array_t():
    m_element_size(0)
{
}

std::span<const std::byte> type_erased_array_t::data() const& noexcept {
    return m_data.bytes();
}

std::span<std::byte> type_erased_array_t::data() & noexcept {
    return m_data.bytes();
}

size_t type_erased_array_t::element_count() const noexcept {
    if (m_element_size == 0) {
        return 0;
    }

    return m_data.size() / m_element_size;
}

size_t type_erased_array_t::element_size() const noexcept {
    return m_element_size;
}

size_t type_erased_array_t::byte_size() const noexcept {
    return m_data.size();
}

void type_erased_array_t::clear() {
    m_data.clear();
}

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array
