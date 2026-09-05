#ifndef M03GJFVD6I5JZBMNGB2LDOOOZA_TYPE_ERASED_ARRAY_API_H
# define M03GJFVD6I5JZBMNGB2LDOOOZA_TYPE_ERASED_ARRAY_API_H

# include <array>
# include <bit>
# include <concepts>
# include <cstddef>
# include <cstring>
# include <format>
# include <memory>
# include <span>
# include <stdexcept>
# include <string_view>
# include <type_traits>
# include <vector>

# include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

class type_erased_array_t {
public:
    type_erased_array_t();

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
    explicit type_erased_array_t(std::vector<T> values);

    std::span<const std::byte> data() const& noexcept;
    std::span<std::byte> data() & noexcept;

    std::span<const std::byte> data() const && = delete;
    std::span<std::byte> data() && = delete;

    size_t element_count() const noexcept;
    size_t element_size() const noexcept;
    size_t byte_size() const noexcept;

    void clear();

    /**
     * @brief Copies one element out after checking its stored size and index.
     */
    template <typename T>
    requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
    T read(size_t index) const;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
    T& operator[](size_t index) &;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
    const T& operator[](size_t index) const&;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
    void push_back(const T& value);

private:
    size_t checked_byte_offset(size_t expected_element_size, size_t index, std::string_view operation) const;

    m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t m_data;
    size_t m_element_size;
};

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array

namespace std {

template <>
struct formatter<m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t>;

} // namespace std

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

template <typename T>
requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
type_erased_array_t::type_erased_array_t(std::vector<T> values):
    m_data(std::as_bytes(std::span<const T>(values.begin(), values.end()))),
    m_element_size(sizeof(T))
{
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
T type_erased_array_t::read(size_t index) const {
    const size_t offset = checked_byte_offset(sizeof(T), index, "type_erased_array_t::read");
    std::array<std::byte, sizeof(T)> bytes;
    std::memcpy(bytes.data(), m_data.bytes().data() + offset, bytes.size());
    return std::bit_cast<T>(bytes);
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
T& type_erased_array_t::operator[](size_t index) & {
    const size_t offset = checked_byte_offset(sizeof(T), index, "type_erased_array_t::operator[]");
    return *reinterpret_cast<T*>(m_data.bytes().data() + offset);
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
const T& type_erased_array_t::operator[](size_t index) const& {
    const size_t offset = checked_byte_offset(sizeof(T), index, "type_erased_array_t::operator[]");
    return *reinterpret_cast<const T*>(m_data.bytes().data() + offset);
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && !std::same_as<T, bool> && alignof(T) <= alignof(std::max_align_t))
void type_erased_array_t::push_back(const T& value) {
    if (m_element_size == 0) {
        m_element_size = sizeof(T);
    } else if (m_element_size != sizeof(T)) {
        throw std::invalid_argument(std::format("type_erased_array_t::push_back: type mismatch, expected element size {}, got {}", m_element_size, sizeof(T)));
    }

    m_data.append(std::span<const std::byte>(reinterpret_cast<const std::byte*>(&value), sizeof(T)));
}

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array

namespace std {

template <>
struct formatter<m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t& tea, auto& ctx) const {
        auto out = ctx.out();

        out = format_to(out, "element_size: {}, element_count: {}", tea.element_size(), tea.element_count());

        if (0 < tea.element_count()) {
            out = format_to(out, ", data: 0x");
            for (const auto& byte : tea.data()) {
                out = format_to(out, "{:02x}", std::to_integer<unsigned int>(byte));
            }
        }

        return out;
    }
};

} // namespace std

#endif // M03GJFVD6I5JZBMNGB2LDOOOZA_TYPE_ERASED_ARRAY_API_H
