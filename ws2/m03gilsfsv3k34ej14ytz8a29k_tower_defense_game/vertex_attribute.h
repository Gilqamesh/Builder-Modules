#ifndef M03GILSFSV3K34EJ14YTZ8A29K_VERTEX_ATTRIBUTE_H
# define M03GILSFSV3K34EJ14YTZ8A29K_VERTEX_ATTRIBUTE_H

# include <cstddef>

# include <format>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

enum class vertex_attribute_type_t {
    R32,
    R64,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
};

size_t vertex_attribute_type_size(vertex_attribute_type_t type);

class vertex_attribute_t {
public:
    vertex_attribute_t();
    vertex_attribute_t(vertex_attribute_type_t type, size_t component_count);

    void type(vertex_attribute_type_t type);
    vertex_attribute_type_t type() const;

    void component_count(size_t count);
    size_t component_count() const;

private:
    vertex_attribute_type_t m_type;
    size_t m_component_count;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid vertex_attribute_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_t& attribute, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{} x ", attribute.component_count());

        switch (attribute.type()) {
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::R32: out = std::format_to(out, "R32"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::R64: out = std::format_to(out, "R64"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::I8:  out = std::format_to(out, "I8");  break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::I16: out = std::format_to(out, "I16"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::I32: out = std::format_to(out, "I32"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::I64: out = std::format_to(out, "I64"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::U8:  out = std::format_to(out, "U8");  break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::U16: out = std::format_to(out, "U16"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::U32: out = std::format_to(out, "U32"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_attribute_type_t::U64: out = std::format_to(out, "U64"); break;
            default: throw std::runtime_error(std::format("formatter<vertex_attribute_t>::format: unknown vertex attribute type ({})", static_cast<int>(attribute.type())));
        }

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_VERTEX_ATTRIBUTE_H
