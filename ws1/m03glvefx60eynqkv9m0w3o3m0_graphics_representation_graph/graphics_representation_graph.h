#ifndef GRAPHICS_REPRESENTATION_GRAPH_H
# define GRAPHICS_REPRESENTATION_GRAPH_H

# include "vertex_primitive_topology.h"

# include <array>
# include <cstddef>
# include <memory>
# include <utility>
# include <vector>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

namespace graphics_representation_graph {

using vertex_primitive_topology_t = m03gilsfsv3k34ej14ytz8a29k_tower_defense_game::vertex_primitive_topology_t;

struct mesh_role_t {};
struct index_buffer_role_t {};
struct geometry_role_t {};
struct texture_role_t {};
struct sampler_role_t {};
struct shader_role_t {};
struct shader_program_role_t {};
struct texture_binding_role_t {};
struct material_role_t {};

template <typename T, std::size_t N>
struct render_item_role_t {};

/**
 * Maps a role in a representation to its concrete type.
 *
 * Leaf roles intentionally have no default mapping. A representation provides
 * only the mappings it supports. Composite roles have generic defaults below,
 * but any representation may replace them with a completely custom type.
 */
template <typename Representation, typename Role>
struct representation_type;

template <typename Representation, typename Role>
using representation_type_t = typename representation_type<Representation, Role>::type;

/**
 * Converts one representation graph into another.
 *
 * The primary template is intentionally undefined. A backend module provides a
 * specialization for each supported source/target representation pair.
 */
template <typename SourceRepresentation, typename TargetRepresentation>
class representation_compiler_t;

template <typename Representation>
using mesh_t = representation_type_t<Representation, mesh_role_t>;

template <typename Representation>
using index_buffer_t = representation_type_t<Representation, index_buffer_role_t>;

template <typename Representation>
using geometry_t = representation_type_t<Representation, geometry_role_t>;

template <typename Representation>
using texture_t = representation_type_t<Representation, texture_role_t>;

template <typename Representation>
using sampler_t = representation_type_t<Representation, sampler_role_t>;

template <typename Representation>
using shader_t = representation_type_t<Representation, shader_role_t>;

template <typename Representation>
using shader_program_t = representation_type_t<Representation, shader_program_role_t>;

template <typename Representation>
using texture_binding_t = representation_type_t<Representation, texture_binding_role_t>;

template <typename Representation>
using material_t = representation_type_t<Representation, material_role_t>;

template <typename Representation, typename T, std::size_t N>
using render_item_t = representation_type_t<Representation, render_item_role_t<T, N>>;

struct empty_state_t {};

enum class shader_type_t {
    vertex,
    fragment,
    geometry,

    _count
};

template <typename Representation, typename State = empty_state_t>
class basic_geometry_t;

template <typename Representation, typename State = empty_state_t>
class basic_shader_program_t;

template <typename Representation, typename State = empty_state_t>
struct basic_texture_binding_t;

template <typename Representation, typename State = empty_state_t>
class basic_material_t;

template <typename Representation, typename T, std::size_t N, typename State = empty_state_t>
class basic_render_item_t;

/* Generic mappings for graph nodes whose dependency structure is shared. */
template <typename Representation>
struct representation_type<Representation, geometry_role_t> {
    using type = basic_geometry_t<Representation>;
};

template <typename Representation>
struct representation_type<Representation, shader_program_role_t> {
    using type = basic_shader_program_t<Representation>;
};

template <typename Representation>
struct representation_type<Representation, texture_binding_role_t> {
    using type = basic_texture_binding_t<Representation>;
};

template <typename Representation>
struct representation_type<Representation, material_role_t> {
    using type = basic_material_t<Representation>;
};

template <typename Representation, typename T, std::size_t N>
struct representation_type<Representation, render_item_role_t<T, N>> {
    using type = basic_render_item_t<Representation, T, N>;
};

template <typename Representation, typename State>
class basic_geometry_t {
public:
    using representation_t = Representation;
    using state_t = State;
    using mesh_type = mesh_t<Representation>;
    using index_buffer_type = index_buffer_t<Representation>;

    basic_geometry_t() = default;

    basic_geometry_t(
        std::shared_ptr<mesh_type> mesh,
        std::shared_ptr<index_buffer_type> index_buffer,
        vertex_primitive_topology_t primitive_topology,
        state_t state = {}):
        m_mesh(std::move(mesh)),
        m_index_buffer(std::move(index_buffer)),
        m_primitive_topology(primitive_topology),
        m_state(std::move(state))
    {
    }

    std::shared_ptr<mesh_type>& mesh() {
        return m_mesh;
    }

    const std::shared_ptr<mesh_type>& mesh() const {
        return m_mesh;
    }

    std::shared_ptr<index_buffer_type>& index_buffer() {
        return m_index_buffer;
    }

    const std::shared_ptr<index_buffer_type>& index_buffer() const {
        return m_index_buffer;
    }

    vertex_primitive_topology_t& primitive_topology() {
        return m_primitive_topology;
    }

    vertex_primitive_topology_t primitive_topology() const {
        return m_primitive_topology;
    }

    state_t& state() {
        return m_state;
    }

    const state_t& state() const {
        return m_state;
    }

private:
    std::shared_ptr<mesh_type> m_mesh;
    std::shared_ptr<index_buffer_type> m_index_buffer;
    vertex_primitive_topology_t m_primitive_topology{};
    [[no_unique_address]] state_t m_state;
};

template <typename Representation, typename State>
class basic_shader_program_t {
public:
    using representation_t = Representation;
    using state_t = State;
    using shader_type = shader_t<Representation>;
    using shaders_t = std::array<std::shared_ptr<shader_type>, static_cast<std::size_t>(shader_type_t::_count)>;

    basic_shader_program_t() = default;

    explicit basic_shader_program_t(state_t state):
        m_state(std::move(state))
    {
    }

    std::shared_ptr<shader_type>& shader(shader_type_t type) {
        return m_shaders[static_cast<std::size_t>(type)];
    }

    const std::shared_ptr<shader_type>& shader(shader_type_t type) const {
        return m_shaders[static_cast<std::size_t>(type)];
    }

    shaders_t& shaders() {
        return m_shaders;
    }

    const shaders_t& shaders() const {
        return m_shaders;
    }

    state_t& state() {
        return m_state;
    }

    const state_t& state() const {
        return m_state;
    }

private:
    shaders_t m_shaders;
    [[no_unique_address]] state_t m_state;
};

template <typename Representation, typename State>
struct basic_texture_binding_t {
    using representation_t = Representation;
    using state_t = State;
    using texture_type = texture_t<Representation>;
    using sampler_type = sampler_t<Representation>;

    std::shared_ptr<texture_type> texture;
    std::shared_ptr<sampler_type> sampler;
    [[no_unique_address]] state_t state;
};

template <typename Representation, typename State>
class basic_material_t {
public:
    using representation_t = Representation;
    using state_t = State;
    using shader_program_type = shader_program_t<Representation>;
    using texture_binding_type = texture_binding_t<Representation>;
    using texture_bindings_t = std::vector<texture_binding_type>;

    basic_material_t() = default;

    explicit basic_material_t(
        std::shared_ptr<shader_program_type> shader_program,
        state_t state = {}):
        m_shader_program(std::move(shader_program)),
        m_state(std::move(state))
    {
    }

    std::shared_ptr<shader_program_type>& shader_program() {
        return m_shader_program;
    }

    const std::shared_ptr<shader_program_type>& shader_program() const {
        return m_shader_program;
    }

    texture_bindings_t& texture_bindings() {
        return m_texture_bindings;
    }

    const texture_bindings_t& texture_bindings() const {
        return m_texture_bindings;
    }

    state_t& state() {
        return m_state;
    }

    const state_t& state() const {
        return m_state;
    }

private:
    std::shared_ptr<shader_program_type> m_shader_program;
    texture_bindings_t m_texture_bindings;
    [[no_unique_address]] state_t m_state;
};

template <typename Representation, typename T, std::size_t N, typename State>
class basic_render_item_t {
public:
    using representation_t = Representation;
    using state_t = State;
    using geometry_type = geometry_t<Representation>;
    using material_type = material_t<Representation>;
    using vector_t = m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N>;

    basic_render_item_t():
        m_translation(static_cast<T>(0)),
        m_rotation(static_cast<T>(0)),
        m_scale(static_cast<T>(1))
    {
    }

    basic_render_item_t(
        std::shared_ptr<geometry_type> geometry,
        std::shared_ptr<material_type> material,
        state_t state = {}):
        m_geometry(std::move(geometry)),
        m_material(std::move(material)),
        m_translation(static_cast<T>(0)),
        m_rotation(static_cast<T>(0)),
        m_scale(static_cast<T>(1)),
        m_state(std::move(state))
    {
    }

    std::shared_ptr<geometry_type>& geometry() {
        return m_geometry;
    }

    const std::shared_ptr<geometry_type>& geometry() const {
        return m_geometry;
    }

    std::shared_ptr<material_type>& material() {
        return m_material;
    }

    const std::shared_ptr<material_type>& material() const {
        return m_material;
    }

    void translation(const vector_t& translation) {
        m_translation = translation;
    }

    const vector_t& translation() const {
        return m_translation;
    }

    void rotation(const vector_t& rotation) {
        m_rotation = rotation;
    }

    const vector_t& rotation() const {
        return m_rotation;
    }

    void scale(const vector_t& scale) {
        m_scale = scale;
    }

    const vector_t& scale() const {
        return m_scale;
    }

    state_t& state() {
        return m_state;
    }

    const state_t& state() const {
        return m_state;
    }

private:
    std::shared_ptr<geometry_type> m_geometry;
    std::shared_ptr<material_type> m_material;
    vector_t m_translation;
    vector_t m_rotation;
    vector_t m_scale;
    [[no_unique_address]] state_t m_state;
};

} // namespace graphics_representation_graph

#endif // GRAPHICS_REPRESENTATION_GRAPH_H
