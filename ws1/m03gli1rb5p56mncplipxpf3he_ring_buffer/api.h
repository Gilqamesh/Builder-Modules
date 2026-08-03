#ifndef M03GLI1RB5P56MNCPLIPXPF3HE_RING_BUFFER_API_H
# define M03GLI1RB5P56MNCPLIPXPF3HE_RING_BUFFER_API_H

# include <vector>
# include <cstddef>
# include <format>
# include <stdexcept>
# include <typeinfo>
# include <type_traits>

namespace m03gli1rb5p56mncplipxpf3he_ring_buffer {

/**
 * @brief Stores a fixed-capacity single-threaded history whose next slot is published by calling commit().
 */
template <typename T>
class ring_buffer_t {
    static_assert(std::is_default_constructible_v<T>, "ring_buffer_t requires T to be default constructible");

public:
    /**
     * @brief Constructs an empty ring buffer with the specified positive capacity.
     */
    explicit ring_buffer_t(std::size_t capacity);

    /**
     * @brief Returns the slot that will become current on the next call to commit()
     *
     * When the buffer is full, modifying this slot immediately overwrites the oldest history entry.
     */
    T& stage();

    /**
     * @brief Publishes the next slot and increases the history size up to the capacity.
     */
    void commit();

    /**
     * @brief Returns the value at the specified history offset, where zero denotes the current value.
     *
     * @throws std::out_of_range if the offset is not less than the history size.
     */
    T& history(std::size_t offset);

    /**
     * @brief Returns the value at the specified history offset, where zero denotes the current value.
     * 
     * @throws std::out_of_range if the offset is not less than the history size.
     */
    const T& history(std::size_t offset) const;

    /**
     * @brief Returns the number of valid history entries.
     */
    std::size_t size() const;

    /**
     * @brief Returns the maximum number of history entries.
     */
    std::size_t capacity() const;

private:
    std::vector<T> m_buffer;
    std::size_t m_head;
    std::size_t m_size;
};

} // namespace m03gli1rb5p56mncplipxpf3he_ring_buffer

namespace std {

template <typename T>
struct formatter<m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T>>;

} // namespace std

namespace m03gli1rb5p56mncplipxpf3he_ring_buffer {

template <typename T>
ring_buffer_t<T>::ring_buffer_t(std::size_t capacity):
    m_head(0),
    m_size(0)
{
    if (capacity == 0) {
        throw std::invalid_argument(std::format("ring_buffer_t<{}>::ring_buffer_t: capacity must be positive", typeid(T).name()));
    }

    m_buffer.resize(capacity);
}

template <typename T>
T& ring_buffer_t<T>::stage() {
    return m_buffer[m_head];
}

template <typename T>
void ring_buffer_t<T>::commit() {
    m_head = (m_head + 1) % m_buffer.size();

    if (m_size < m_buffer.size()) {
        ++m_size;
    }
}

template <typename T>
T& ring_buffer_t<T>::history(std::size_t offset) {
    if (m_size <= offset) {
        throw std::out_of_range(std::format("ring_buffer_t<{}>::history: offset {} exceeds history size {}", typeid(T).name(), offset, m_size));
    }

    const auto index = (m_head + m_buffer.size() - 1 - offset) % m_buffer.size();
    return m_buffer[index];
}

template <typename T>
const T& ring_buffer_t<T>::history(std::size_t offset) const {
    if (m_size <= offset) {
        throw std::out_of_range(std::format("ring_buffer_t<{}>::history: offset {} exceeds history size {}", typeid(T).name(), offset, m_size));
    }

    const auto index = (m_head + m_buffer.size() - 1 - offset) % m_buffer.size();
    return m_buffer[index];
}

template <typename T>
std::size_t ring_buffer_t<T>::size() const {
    return m_size;
}

template <typename T>
std::size_t ring_buffer_t<T>::capacity() const {
    return m_buffer.size();
}

} // namespace m03gli1rb5p56mncplipxpf3he_ring_buffer

namespace std {

template <typename T>
struct formatter<m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T>& ring_buffer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "capacity: {}, ", ring_buffer.capacity());

        out = std::format_to(out, "size: {}, ", ring_buffer.size());

        out = std::format_to(out, "history: {{ ");
        for (std::size_t offset = 0; offset < ring_buffer.size(); ++offset) {
            if (0 < offset) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", offset, ring_buffer.history(offset));
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GLI1RB5P56MNCPLIPXPF3HE_RING_BUFFER_API_H