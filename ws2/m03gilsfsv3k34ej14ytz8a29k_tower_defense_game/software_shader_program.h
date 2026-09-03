#ifndef M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_SHADER_PROGRAM_H
# define M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_SHADER_PROGRAM_H

# include "shader.h"

# include <memory>
# include <format>
# include <stdexcept>
# include <array>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

enum class shader_type_t {
    vertex,
    fragment,
    geometry,

    _count
};

class software_shader_program_t {
public:
    software_shader_program_t();

    void finalize();

    std::shared_ptr<shader_t>& shader(shader_type_t type);
    std::shared_ptr<shader_t> shader(shader_type_t type) const;

private:
    std::array<std::shared_ptr<shader_t>, static_cast<size_t>(shader_type_t::_count)> m_shaders;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t>;

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::software_shader_program_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid shader_type_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t& shader_type, auto& ctx) const {
        auto out = ctx.out();

        switch (shader_type) {
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t::vertex: out = std::format_to(out, "vertex"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t::fragment: out = std::format_to(out, "fragment"); break;
            case m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t::geometry: out = std::format_to(out, "geometry"); break;
            default: throw std::runtime_error(std::format("invalid shader_type_t value: {}", static_cast<int>(shader_type)));
        }

        return out;
    }
};

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::software_shader_program_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid software_shader_program_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::software_shader_program_t& shader_program, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        bool wrote_shader = false;
        for (size_t i = 0; i < static_cast<size_t>(m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t::_count); ++i) {
            const auto shader_type = static_cast<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::shader_type_t>(i);
            const auto& shader = shader_program.shader(shader_type);
            if (shader) {
                if (wrote_shader) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}: {}", shader_type, *shader);
                wrote_shader = true;
            }
        }

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_TOWER_DEFENSE_GAME_SHADER_PROGRAM_H
