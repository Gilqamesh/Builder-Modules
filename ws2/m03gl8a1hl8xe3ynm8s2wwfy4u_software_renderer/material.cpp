#include "material.h"

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

material_t::material_t()
{
}

std::vector<texture_binding_t>& material_t::texture_bindings() {
    return m_texture_bindings;
}

const std::vector<texture_binding_t>& material_t::texture_bindings() const {
    return m_texture_bindings;
}

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer
