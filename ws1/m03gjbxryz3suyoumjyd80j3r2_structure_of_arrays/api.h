#ifndef M03GJBXRYZ3SUYOUMJYD80J3R2_API_H
# define M03GJBXRYZ3SUYOUMJYD80J3R2_API_H

# include <vector>
# include <tuple>
# include <format>
# include <span>

# include <m03gjfvd6i5jzbmngb2ldoooza_type_erased_array/api.h>

namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays {

template <typename... Ts>
class structure_of_arrays_t {
    static_assert(0 < sizeof...(Ts));
    static_assert((std::same_as<Ts, std::remove_cvref_t<Ts>> && ...), "structure_of_arrays_t only supports plain value types (no references, no const, no volatile).");
public:
    using data_t = std::tuple<std::vector<Ts>...>;

public:
    void push_back(Ts... values);

    const data_t& data() const&;

    data_t data()&&;

private:
    std::tuple<std::vector<Ts>...> m_data;
};

class erased_structure_of_arrays_t {
public:
    using data_t = std::vector<m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t>;

public:
    erased_structure_of_arrays_t() = default;

    template <typename... Ts>
    explicit erased_structure_of_arrays_t(structure_of_arrays_t<Ts...> soa);

    const data_t& data() const&;

    data_t data() &&;

    size_t size() const noexcept;

    const m03gjfvd6i5jzbmngb2ldoooza_type_erased_array::type_erased_array_t& operator[](size_t index) const&;

private:
    data_t m_data;
};

} // namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays

namespace std {

template <typename... Ts>
struct formatter<m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<Ts...>>;

template <>
struct formatter<m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t>;

} // namespace std

namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays {

template <typename... Ts>
void structure_of_arrays_t<Ts...>::push_back(Ts... values) {
    std::apply([&](auto&... vector) {
        (vector.push_back(std::move(values)), ...);
    }, m_data);
}

template <typename... Ts>
const structure_of_arrays_t<Ts...>::data_t& structure_of_arrays_t<Ts...>::data() const& {
    return m_data;
}

template <typename... Ts>
typename structure_of_arrays_t<Ts...>::data_t structure_of_arrays_t<Ts...>::data() && {
    return std::move(m_data);
}


template <typename... Ts>
erased_structure_of_arrays_t::erased_structure_of_arrays_t(structure_of_arrays_t<Ts...> soa) {
    static_assert((std::is_trivially_copyable_v<Ts> && ...), "erased_structure_of_arrays_t does not support non-trivially copyable types.");

    m_data.reserve(sizeof...(Ts));
    std::apply([this](auto&&... stream) {
        (m_data.emplace_back(std::move(stream)), ...);
    }, std::move(soa.data()));
}

} // namespace m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays

namespace std {

template <typename... Ts>
struct formatter<m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<Ts...>> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<Ts...>& soa, auto& ctx) const {
        auto out = ctx.out();

        out = format_to(out, "{{ ");

        size_t stream_index = 0;
        bool first_stream = true;

        apply(
            [&](const auto&... streams) {
                (
                    [&] {
                        if (!first_stream) {
                            out = format_to(out, ", ");
                        }
                        first_stream = false;

                        out = format_to(out, "[{}]: {{ element_size: {}, size: {}, data: [ ", stream_index++, sizeof(typename std::remove_cvref_t<decltype(streams)>::value_type), streams.size());

                        bool first_value = true;

                        for (const auto& value : streams) {
                            if (!first_value) {
                                out = format_to(out, ", ");
                            }
                            first_value = false;

                            out = format_to(out, "{}", value);
                        }

                        out = format_to(out, " ] }}");
                    }(),
                    ...
                );
            },
            soa.data()
        );

        out = format_to(out, " }}");

        return out;
    }
};

template <>
struct formatter<m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t& esoa, auto& ctx) const {
        auto out = ctx.out();

        out = format_to(out, "{{ ");

        for (size_t i = 0; i < esoa.data().size(); ++i) {
            if (0 < i) {
                out = format_to(out, ", ");
            }
            out = format_to(out, "[{}]: {{ {} }}", i, esoa.data()[i]);
        }

        out = format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GJBXRYZ3SUYOUMJYD80J3R2_API_H
