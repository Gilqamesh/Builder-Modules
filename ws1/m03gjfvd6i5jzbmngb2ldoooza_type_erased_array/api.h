#ifndef M03GJFVD6I5JZBMNGB2LDOOOZA_TYPE_ERASED_ARRAY_API_H
# define M03GJFVD6I5JZBMNGB2LDOOOZA_TYPE_ERASED_ARRAY_API_H

# include <concepts>
# include <cstddef>
# include <format>
# include <memory>
# include <span>
# include <stdexcept>
# include <typeinfo>
# include <type_traits>
# include <utility>
# include <vector>

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

class type_erased_array_t {
public:
    type_erased_array_t();

    type_erased_array_t(const type_erased_array_t& other);
    type_erased_array_t& operator=(const type_erased_array_t& other);

    type_erased_array_t(type_erased_array_t&& other) noexcept = default;
    type_erased_array_t& operator=(type_erased_array_t&& other) noexcept = default;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
    explicit type_erased_array_t(std::vector<T> values);

    std::span<const std::byte> data() const& noexcept;
    std::span<std::byte> data() & noexcept;

    std::span<const std::byte> data() const && = delete;
    std::span<std::byte> data() && = delete;

    size_t element_count() const noexcept;
    size_t element_size() const noexcept;
    size_t byte_size() const noexcept;

    void clear();

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
    T& operator[](size_t index) &;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
    const T& operator[](size_t index) const&;

    template <typename T>
    requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
    void push_back(const T& value);

private:
    class storage_t {
    public:
        virtual ~storage_t() = default;

        virtual std::unique_ptr<storage_t> clone() const = 0;
        virtual std::span<const std::byte> readable_data() const = 0;
        virtual std::span<std::byte> writable_data() = 0;
        virtual size_t element_count() const = 0;
        virtual size_t element_size() const = 0;
        virtual const std::type_info& element_type() const = 0;
        virtual void clear() = 0;
    };

    template <typename T>
    class typed_storage_t final : public storage_t {
    public:
        explicit typed_storage_t(std::vector<T> values):
            m_values(std::move(values))
        {
        }

        std::unique_ptr<storage_t> clone() const override {
            return std::make_unique<typed_storage_t<T>>(m_values);
        }

        std::span<const std::byte> readable_data() const override {
            return std::as_bytes(std::span<const T>(m_values));
        }

        std::span<std::byte> writable_data() override {
            return std::as_writable_bytes(std::span<T>(m_values));
        }

        size_t element_count() const override {
            return m_values.size();
        }

        size_t element_size() const override {
            return sizeof(T);
        }

        const std::type_info& element_type() const override {
            return typeid(T);
        }

        void clear() override {
            m_values.clear();
        }

        std::vector<T> m_values;
    };

    std::unique_ptr<storage_t> m_storage;
};

} // namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array

namespace std {

template <>
struct formatter<m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t>;

} // namespace std

namespace m03gjfvd6i5jzbmngb2ldoooza_type_erased_array {

template <typename T>
requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
type_erased_array_t::type_erased_array_t(std::vector<T> values):
    m_storage(std::make_unique<typed_storage_t<T>>(std::move(values)))
{
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
T& type_erased_array_t::operator[](size_t index) & {
    if (!m_storage || m_storage->element_type() != typeid(T)) {
        throw std::invalid_argument("type_erased_array_t::operator[]: element type does not match requested type");
    }
    if (element_count() <= index) {
        throw std::out_of_range(std::format("type_erased_array_t::operator[]: index {} exceeds element count {}", index, element_count()));
    }
    return static_cast<typed_storage_t<T>&>(*m_storage).m_values[index];
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
const T& type_erased_array_t::operator[](size_t index) const& {
    if (!m_storage || m_storage->element_type() != typeid(T)) {
        throw std::invalid_argument("type_erased_array_t::operator[]: element type does not match requested type");
    }
    if (element_count() <= index) {
        throw std::out_of_range(std::format("type_erased_array_t::operator[]: index {} exceeds element count {}", index, element_count()));
    }
    return static_cast<const typed_storage_t<T>&>(*m_storage).m_values[index];
}

template <typename T>
requires (std::is_trivially_copyable_v<T> && std::same_as<T, std::remove_cv_t<T>> && !std::same_as<T, bool>)
void type_erased_array_t::push_back(const T& value) {
    if (!m_storage) {
        m_storage = std::make_unique<typed_storage_t<T>>(std::vector<T> {});
    } else if (m_storage->element_type() != typeid(T)) {
        throw std::invalid_argument("type_erased_array_t::push_back: element type does not match stored type");
    }

    static_cast<typed_storage_t<T>&>(*m_storage).m_values.push_back(value);
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
