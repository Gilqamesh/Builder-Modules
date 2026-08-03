#ifndef M03GLHRCFHHQZXMLLAXHICU5HC_SPSC_RING_BUFFER_API_H
# define M03GLHRCFHHQZXMLLAXHICU5HC_SPSC_RING_BUFFER_API_H

# include <vector>
# include <atomic>
# include <format>
# include <stdexcept>
# include <typeinfo>
# include <type_traits>
# include <utility>
# include <cstddef>
# include <limits>

namespace m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer {

/**
 * @brief Provides a fixed-capacity ring buffer for one concurrent producer and one concurrent consumer.
 */
template <typename T>
class spsc_ring_buffer_t {
    static_assert(std::is_default_constructible_v<T>);

public:
    /**
     * @brief Constructs an empty ring buffer with capacity 1 << capacity_power_of_two.
     */
    explicit spsc_ring_buffer_t(std::size_t capacity_power_of_two);

    /**
     * @brief Attempts to copy the value into the buffer and returns whether it succeeded.
     */
    bool try_write(const T& value);

    /**
     * @brief Attempts to move the value into the buffer and returns whether it succeeded.
     */
    bool try_write(T&& value);

    /**
     * @brief Attempts to move the oldest buffered value into the destination and returns whether it succeeded.
     */
    bool try_read(T& value);

    /**
     * @brief Returns the underlying storage, which may only be inspected while no producer or consumer is accessing the buffer.
     */
    const std::vector<T>& buffer() const;

    /**
     * @brief Returns the current producer position, which may only be inspected while no producer or consumer is accessing the buffer.
     */
    std::size_t head() const;

    /**
     * @brief Returns the current consumer position, which may only be inspected while no producer or consumer is accessing the buffer.
     */
    std::size_t tail() const;

private:
    std::vector<T> m_buffer;
    std::atomic<std::size_t> m_head;
    std::atomic<std::size_t> m_tail;
};

} // namespace m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer

namespace std {

template <typename T>
struct formatter<m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer::spsc_ring_buffer_t<T>>;

} // namespace std

namespace m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer {

template <typename T>
spsc_ring_buffer_t<T>::spsc_ring_buffer_t(std::size_t capacity_power_of_two):
    m_head(0),
    m_tail(0)
{
    if (std::numeric_limits<std::size_t>::digits <= capacity_power_of_two) {
        throw std::invalid_argument(std::format("spsc_ring_buffer_t<{}>::spsc_ring_buffer_t: capacity power {} is too large for std::size_t", typeid(T).name(), capacity_power_of_two));
    }

    const auto capacity = std::size_t{1} << capacity_power_of_two;
    if (m_buffer.max_size() < capacity) {
        throw std::length_error(std::format("spsc_ring_buffer_t<{}>::spsc_ring_buffer_t: capacity 1 << {} ({}) exceeds maximum capacity {}", typeid(T).name(), capacity_power_of_two, capacity, m_buffer.max_size()));
    }

    m_buffer.resize(capacity);
}

template <typename T>
bool spsc_ring_buffer_t<T>::try_write(const T& value) {
    const auto head = m_head.load(std::memory_order_relaxed);
    const auto tail = m_tail.load(std::memory_order_acquire);
    if (head - tail == m_buffer.size()) {
        return false;
    }

    m_buffer[head & (m_buffer.size() - 1)] = value;
    m_head.store(head + 1, std::memory_order_release);

    return true;
}

template <typename T>
bool spsc_ring_buffer_t<T>::try_write(T&& value) {
    const auto head = m_head.load(std::memory_order_relaxed);
    const auto tail = m_tail.load(std::memory_order_acquire);
    if (head - tail == m_buffer.size()) {
        return false;
    }

    m_buffer[head & (m_buffer.size() - 1)] = std::move(value);
    m_head.store(head + 1, std::memory_order_release);

    return true;
}

template <typename T>
bool spsc_ring_buffer_t<T>::try_read(T& value) {
    const auto tail = m_tail.load(std::memory_order_relaxed);
    const auto head = m_head.load(std::memory_order_acquire);
    if (tail == head) {
        return false;
    }

    value = std::move(m_buffer[tail & (m_buffer.size() - 1)]);
    m_tail.store(tail + 1, std::memory_order_release);

    return true;
}

template <typename T>
const std::vector<T>& spsc_ring_buffer_t<T>::buffer() const {
    return m_buffer;
}

template <typename T>
std::size_t spsc_ring_buffer_t<T>::head() const {
    return m_head;
}

template <typename T>
std::size_t spsc_ring_buffer_t<T>::tail() const {
    return m_tail;
}

} // namespace m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer

namespace std {

template <typename T>
struct formatter<m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer::spsc_ring_buffer_t<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer::spsc_ring_buffer_t<T>& ring_buffer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        const auto& buffer = ring_buffer.buffer();
        out = std::format_to(out, "capacity: {}, ", buffer.size());
        out = std::format_to(out, "head: {}, ", ring_buffer.head());
        out = std::format_to(out, "tail: {}, ", ring_buffer.tail());
        out = std::format_to(out, "values: [");
        for (std::size_t i = 0; i < buffer.size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}", buffer[i]);
        }
        out = std::format_to(out, "]");

        out = std::format_to(out, " }}");

        return out;
    };
};

} // namespace std

#endif // M03GLHRCFHHQZXMLLAXHICU5HC_SPSC_RING_BUFFER_API_H
