#ifndef M03GILSFSV3K34EJ14YTZ8A29K_MATERIAL_H
# define M03GILSFSV3K34EJ14YTZ8A29K_MATERIAL_H

# include "texture.h"
# include "sampler.h"

# include <memory>
# include <vector>
# include <format>

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

struct texture_binding_t {
    std::shared_ptr<texture_t> texture;
    std::shared_ptr<sampler_t> sampler;
};

class material_t {
public:
    material_t();

    std::vector<texture_binding_t>& texture_bindings();
    const std::vector<texture_binding_t>& texture_bindings() const;

private:
    std::vector<texture_binding_t> m_texture_bindings;
};

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::material_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::material_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid material_t format specifier");
        }
        return it;
    }

    auto format(const m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::material_t& material, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "texture_bindings: [");
        const auto& texture_bindings = material.texture_bindings();
        for (size_t i = 0; i < texture_bindings.size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            const auto& binding = texture_bindings[i];
            out = std::format_to(out, "[{}]: ", i);

            if (binding.texture) {
                out = std::format_to(out, "texture: {}, ", *binding.texture);
            } else {
                out = std::format_to(out, "texture: -, ");
            }

            if (binding.sampler) {
                out = std::format_to(out, "sampler: {}", *binding.sampler);
            } else {
                out = std::format_to(out, "sampler: -");
            }
        }
        out = std::format_to(out, "]");

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GILSFSV3K34EJ14YTZ8A29K_MATERIAL_H
