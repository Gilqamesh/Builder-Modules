#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RENDER_ITEM_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RENDER_ITEM_H

# include "geometry.h"
# include "material.h"

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

# include <cstddef>
# include <format>
# include <memory>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

template <typename T, std::size_t N>
class render_item_t {
public:
    render_item_t();

    void geometry(std::shared_ptr<geometry_t> geometry);
    std::shared_ptr<geometry_t> geometry() const;

    void material(std::shared_ptr<material_t> material);
    std::shared_ptr<material_t> material() const;

    void translation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation() const;

    void rotation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation() const;

    void scale(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale);
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale() const;

private:
    std::shared_ptr<geometry_t> m_geometry;
    std::shared_ptr<material_t> m_material;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_translation;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_rotation;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> m_scale;
};

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace std {

template <typename T, std::size_t N>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::render_item_t<T, N>>;

} // namespace std

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

template <typename T, std::size_t N>
render_item_t<T, N>::render_item_t():
    m_translation(static_cast<T>(0)),
    m_rotation(static_cast<T>(0)),
    m_scale(static_cast<T>(1))
{
}

template <typename T, std::size_t N>
void render_item_t<T, N>::geometry(std::shared_ptr<geometry_t> geometry) {
    m_geometry = geometry;
}

template <typename T, std::size_t N>
std::shared_ptr<geometry_t> render_item_t<T, N>::geometry() const {
    return m_geometry;
}

template <typename T, std::size_t N>
void render_item_t<T, N>::material(std::shared_ptr<material_t> material) {
    m_material = material;
}

template <typename T, std::size_t N>
std::shared_ptr<material_t> render_item_t<T, N>::material() const {
    return m_material;
}

template <typename T, std::size_t N>
void render_item_t<T, N>::translation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& translation) {
    m_translation = translation;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& render_item_t<T, N>::translation() const {
    return m_translation;
}

template <typename T, std::size_t N>
void render_item_t<T, N>::rotation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& rotation) {
    m_rotation = rotation;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& render_item_t<T, N>::rotation() const {
    return m_rotation;
}

template <typename T, std::size_t N>
void render_item_t<T, N>::scale(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& scale) {
    m_scale = scale;
}

template <typename T, std::size_t N>
const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>& render_item_t<T, N>::scale() const {
    return m_scale;
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

namespace std {

template <typename T, std::size_t N>
struct formatter<m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::render_item_t<T, N>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid render_item_t format specifier");
        }
        return it;
    }

    auto format(const m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer::render_item_t<T, N>& render_item, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        const auto& geometry = render_item.geometry();
        if (geometry) {
            out = std::format_to(out, "geometry: {}, ", *geometry);
        } else {
            out = std::format_to(out, "geometry: -, ");
        }

        const auto& material = render_item.material();
        if (material) {
            out = std::format_to(out, "material: {}, ", *material);
        } else {
            out = std::format_to(out, "material: -, ");
        }

        out = std::format_to(out, "translation: {}, ", render_item.translation());

        out = std::format_to(out, "rotation: {}, ", render_item.rotation());

        out = std::format_to(out, "scale: {}", render_item.scale());

        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_RENDER_ITEM_H
