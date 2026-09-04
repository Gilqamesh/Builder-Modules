#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_MATERIAL_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_MATERIAL_H

# include <m03gt0l0q3l4b1k27eab5k7py1_texture/api.h>

# include <cstddef>
# include <format>
# include <memory>
# include <vector>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

struct texture_binding_t {
    std::shared_ptr<m03gt0l0q3l4b1k27eab5k7py1_texture::texture_t> texture;
    std::shared_ptr<m03gt0l0q3l4b1k27eab5k7py1_texture::sampler_t> sampler;
};

class material_t {
public:
    material_t();

    std::vector<texture_binding_t>& texture_bindings();
    const std::vector<texture_binding_t>& texture_bindings() const;

private:
    std::vector<texture_binding_t> m_texture_bindings;
};

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::texture_binding_t>;

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::material_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::texture_binding_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid texture_binding_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::texture_binding_t& binding, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ texture: ");
        if (binding.texture) {
            out = std::format_to(out, "{}", *binding.texture);
        } else {
            out = std::format_to(out, "-");
        }
        out = std::format_to(out, ", sampler: ");
        if (binding.sampler) {
            out = std::format_to(out, "{}", *binding.sampler);
        } else {
            out = std::format_to(out, "-");
        }
        out = std::format_to(out, " }}");

        return out;
    }
};

template <>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::material_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid material_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::material_t& material, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "texture_bindings: [");
        const auto& texture_bindings = material.texture_bindings();
        for (std::size_t i = 0; i < texture_bindings.size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "[{}]: {}", i, texture_bindings[i]);
        }
        out = std::format_to(out, "]");

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_MATERIAL_H
