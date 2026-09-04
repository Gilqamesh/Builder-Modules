# include "api.h"

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

type_erased_array_t::type_erased_array_t() = default;

type_erased_array_t::type_erased_array_t(const type_erased_array_t& other):
    m_storage(other.m_storage ? other.m_storage->clone() : nullptr)
{
}

type_erased_array_t& type_erased_array_t::operator=(const type_erased_array_t& other) {
    if (this != &other) {
        m_storage = other.m_storage ? other.m_storage->clone() : nullptr;
    }

    return *this;
}

std::span<const std::byte> type_erased_array_t::data() const& noexcept {
    if (!m_storage) {
        return {};
    }

    return m_storage->readable_data();
}

std::span<std::byte> type_erased_array_t::data() & noexcept {
    if (!m_storage) {
        return {};
    }

    return m_storage->writable_data();
}

size_t type_erased_array_t::element_count() const noexcept {
    if (!m_storage) {
        return 0;
    }

    return m_storage->element_count();
}

size_t type_erased_array_t::element_size() const noexcept {
    if (!m_storage) {
        return 0;
    }

    return m_storage->element_size();
}

size_t type_erased_array_t::byte_size() const noexcept {
    return data().size();
}

void type_erased_array_t::clear() {
    if (m_storage) {
        m_storage->clear();
    }
}

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array
