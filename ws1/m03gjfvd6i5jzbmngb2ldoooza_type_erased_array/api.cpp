#include "api.h"

#include <format>
#include <stdexcept>
#include <string_view>

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

size_t type_erased_array_t::checked_byte_offset(
    size_t expected_element_size,
    size_t index,
    std::string_view operation
) const {
    if (m_element_size != expected_element_size) {
        throw std::invalid_argument(std::format(
            "{}: type mismatch, expected element size {}, got {}",
            operation,
            m_element_size,
            expected_element_size
        ));
    }
    if (element_count() <= index) {
        throw std::out_of_range(std::format(
            "{}: index {} exceeds element count {}",
            operation,
            index,
            element_count()
        ));
    }
    return index * m_element_size;
}

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array
