#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_INDEX_BUFFER_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_INDEX_BUFFER_H

# include <vector>
# include <cstdint>
# include <format>
# include <stdexcept>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

class index_buffer_t {
public:
    using index_t = std::uint32_t;
    using indices_t = std::vector<index_t>;

public:
    indices_t& indices();
    const indices_t& indices() const;

    indices_t::iterator begin();
    indices_t::const_iterator begin() const;

    indices_t::iterator end();
    indices_t::const_iterator end() const;

    index_t& operator[](size_t index);
    const index_t& operator[](size_t index) const;

private:
    indices_t m_indices;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::index_buffer_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::index_buffer_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid index_buffer_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::index_buffer_t& index_buffer, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        const auto& indices = index_buffer.indices();
        out = std::format_to(out, "indices: [");
        for (size_t i = 0; i < indices.size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "{}", indices[i]);
        }
        out = std::format_to(out, "]");

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_INDEX_BUFFER_H
