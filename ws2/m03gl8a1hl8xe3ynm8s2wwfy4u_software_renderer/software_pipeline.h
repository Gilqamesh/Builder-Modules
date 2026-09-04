#ifndef M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_PIPELINE_H
# define M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_PIPELINE_H

# include "geometry.h"
# include "material.h"
# include "software_renderer.h"

# include <m03gt1djvvy5atia5evkbg6rqy_software_shader/software_shader.h>

# include <span>

namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer {

void bind_material_resources(
    const m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t& program,
    const material_t& material,
    m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t& bindings
);

void draw_software_pipeline(
    const m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t& program,
    const m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t& bindings,
    const geometry_t& geometry,
    int width,
    int height,
    std::span<rgba8_t> framebuffer
);

} // namespace m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer

#endif // M03GL8A1HL8XE3YNM8S2WWFY4U_SOFTWARE_RENDERER_SOFTWARE_PIPELINE_H
