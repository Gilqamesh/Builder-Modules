#ifndef M03GLI1RB5P56MNCPLIPXPF3HE_RING_BUFFER_API_H
# define M03GLI1RB5P56MNCPLIPXPF3HE_RING_BUFFER_API_H

# include <cstddef>
# include <format>
# include <limits>
# include <stdexcept>
# include <type_traits>
# include <typeinfo>
# include <vector>

namespace m03gli1rb5p56mncplipxpf3he_ring_buffer {

/**
 * @brief Determines whether the staging slot shares storage with committed history.
 */
enum class staging_policy_t {
    /**
     * @brief Uses the next history slot for staging, which aliases the oldest entry when full.
    */
    overlapping,

    /**
     * @brief Uses one additional staging slot that never aliases committed history.
    */
    dedicated
};

/**
 * @brief Determines how commit() prepares the next staging slot.
 */
enum class commit_policy_t {
    /**
     * @brief Advances to the next staging slot without modifying it.
     */
    advance,

    /**
     * @brief Copies the staged value into the next staging slot and advances to it.
     */
    copy_with_advance
};

/**
 * @brief Stores a fixed-capacity single-threaded history with a mutable slot published by commit().
 */
template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
class ring_buffer_t {
    static_assert(std::is_default_constructible_v<T>, "ring_buffer_t requires T to be default constructible");
    static_assert(!(CommitPolicy == commit_policy_t::copy_with_advance && !std::is_copy_assignable_v<T>), "ring_buffer_t copy-with-advance commit requires T to be copy assignable");

public:
    /**
     * @brief Constructs an empty ring buffer with the specified positive history capacity.
     */
    explicit ring_buffer_t(std::size_t history_capacity);

    /**
     * @brief Returns the slot that will become current on the next call to commit().
     *
     * With overlapping staging, this slot aliases the oldest history entry when the buffer is full.
     */
    T& stage();

    /**
     * @brief Publishes the staged slot, advances to the next slot, and increases the history size up to the history capacity.
     *
     * Propagates exceptions thrown by copy assignment when using copy-and-advance commit.
     */
    void commit();

    /**
     * @brief Returns the value at the specified history offset, where zero denotes the newest committed value.
     *
     * Fails if the offset is not less than the history size.
     */
    T& history(std::size_t offset);

    /**
     * @brief Returns the value at the specified history offset, where zero denotes the newest committed value.
     *
     * Fails if the offset is not less than the history size.
     */
    const T& history(std::size_t offset) const;

    /**
     * @brief Returns the number of valid history entries.
     */
    std::size_t history_size() const;

    /**
     * @brief Returns the maximum number of history entries, excluding any dedicated staging slot.
     */
    std::size_t history_capacity() const;

private:
    static std::size_t buffer_size(std::size_t history_capacity);

private:
    std::vector<T> m_buffer;
    std::size_t m_head;
    std::size_t m_history_size;
};

} // namespace m03gli1rb5p56mncplipxpf3he_ring_buffer

namespace std {

template <typename T, m03gli1rb5p56mncplipxpf3he_ring_buffer::staging_policy_t StagingPolicy, m03gli1rb5p56mncplipxpf3he_ring_buffer::commit_policy_t CommitPolicy>
struct formatter<m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T, StagingPolicy, CommitPolicy>>;

} // namespace std

namespace m03gli1rb5p56mncplipxpf3he_ring_buffer {

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
ring_buffer_t<T, StagingPolicy, CommitPolicy>::ring_buffer_t(std::size_t history_capacity):
    m_buffer(buffer_size(history_capacity)),
    m_head(0),
    m_history_size(0)
{
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
T& ring_buffer_t<T, StagingPolicy, CommitPolicy>::stage() {
    return m_buffer[m_head];
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
void ring_buffer_t<T, StagingPolicy, CommitPolicy>::commit() {
    const auto current_head = m_head;
    const auto next_head = (m_head + 1) % m_buffer.size();

    if constexpr (CommitPolicy == commit_policy_t::copy_with_advance) {
        if (current_head != next_head) {
            m_buffer[next_head] = m_buffer[current_head];
        }
    }

    m_head = next_head;

    if (m_history_size < history_capacity()) {
        ++m_history_size;
    }
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
T& ring_buffer_t<T, StagingPolicy, CommitPolicy>::history(std::size_t offset) {
    if (m_history_size <= offset) {
        throw std::out_of_range(std::format("ring_buffer_t<{}>::history: offset {} exceeds history size {}", typeid(T).name(), offset, m_history_size));
    }

    const auto index = (m_head + m_buffer.size() - 1 - offset) % m_buffer.size();
    return m_buffer[index];
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
const T& ring_buffer_t<T, StagingPolicy, CommitPolicy>::history(std::size_t offset) const {
    if (m_history_size <= offset) {
        throw std::out_of_range(std::format("ring_buffer_t<{}>::history: offset {} exceeds history size {}", typeid(T).name(), offset, m_history_size));
    }

    const auto index = (m_head + m_buffer.size() - 1 - offset) % m_buffer.size();
    return m_buffer[index];
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
std::size_t ring_buffer_t<T, StagingPolicy, CommitPolicy>::history_size() const {
    return m_history_size;
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
std::size_t ring_buffer_t<T, StagingPolicy, CommitPolicy>::history_capacity() const {
    if constexpr (StagingPolicy == staging_policy_t::dedicated) {
        return m_buffer.size() - 1;
    } else {
        return m_buffer.size();
    }
}

template <typename T, staging_policy_t StagingPolicy, commit_policy_t CommitPolicy>
std::size_t ring_buffer_t<T, StagingPolicy, CommitPolicy>::buffer_size(std::size_t history_capacity) {
    std::size_t result = history_capacity;

    if (result == 0) {
        throw std::invalid_argument(std::format("ring_buffer_t<{}>::ring_buffer_t: capacity must be positive", typeid(T).name()));
    }

    if constexpr (StagingPolicy == staging_policy_t::dedicated) {
        if (result == std::numeric_limits<std::size_t>::max()) {
            throw std::length_error(std::format("ring_buffer_t<{}>::ring_buffer_t: history capacity is too large", typeid(T).name()));
        }

        ++result;
    }

    if (std::vector<T>().max_size() < result) {
        throw std::length_error(std::format("ring_buffer_t<{}>::ring_buffer_t: history capacity is too large", typeid(T).name()));
    }

    return result;
}

} // namespace m03gli1rb5p56mncplipxpf3he_ring_buffer

namespace std {

template <typename T, m03gli1rb5p56mncplipxpf3he_ring_buffer::staging_policy_t StagingPolicy, m03gli1rb5p56mncplipxpf3he_ring_buffer::commit_policy_t CommitPolicy>
struct formatter<m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T, StagingPolicy, CommitPolicy>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<T, StagingPolicy, CommitPolicy>& ring_buffer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "history capacity: {}, ", ring_buffer.history_capacity());

        out = std::format_to(out, "history size: {}, ", ring_buffer.history_size());

        out = std::format_to(out, "history: {{ ");
        for (std::size_t offset = 0; offset < ring_buffer.history_size(); ++offset) {
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
