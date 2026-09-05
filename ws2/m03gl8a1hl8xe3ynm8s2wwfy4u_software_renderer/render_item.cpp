#include "render_item.h"

#include <utility>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

render_item_t::render_item_t():
    m_translation(0.0F),
    m_rotation(0.0F),
    m_scale(1.0F)
{
}

void render_item_t::geometry(std::shared_ptr<geometry_t> geometry) {
    m_geometry = std::move(geometry);
}

std::shared_ptr<geometry_t> render_item_t::geometry() const {
    return m_geometry;
}

void render_item_t::material(std::shared_ptr<material_t> material) {
    m_material = std::move(material);
}

std::shared_ptr<material_t> render_item_t::material() const {
    return m_material;
}

void render_item_t::translation(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& translation) {
    m_translation = translation;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& render_item_t::translation() const {
    return m_translation;
}

void render_item_t::rotation(float rotation) {
    m_rotation = rotation;
}

float render_item_t::rotation() const {
    return m_rotation;
}

void render_item_t::scale(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& scale) {
    m_scale = scale;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& render_item_t::scale() const {
    return m_scale;
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
