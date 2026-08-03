#include "graphics_representation_graph.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace example {

struct software_representation_t {};
struct opengl_representation_t {};

class software_mesh_t {};
class software_index_buffer_t {};
class software_texture_t {};
class software_sampler_t {};
class software_shader_t {};

class opengl_mesh_t {
public:
    std::vector<std::uint32_t> vertex_buffer_handles;
};

class opengl_index_buffer_t {
public:
    std::uint32_t handle = 0;
};

class opengl_texture_t {
public:
    std::uint32_t handle = 0;
};

class opengl_sampler_t {
public:
    std::uint32_t handle = 0;
};

class opengl_shader_t {
public:
    std::uint32_t handle = 0;
};

struct opengl_geometry_state_t {
    std::uint32_t vertex_array_handle = 0;
};

struct opengl_shader_program_state_t {
    std::uint32_t handle = 0;
};

} // namespace example

namespace graphics_representation_graph {

template <>
struct representation_type<example::software_representation_t, mesh_role_t> {
    using type = example::software_mesh_t;
};

template <>
struct representation_type<example::software_representation_t, index_buffer_role_t> {
    using type = example::software_index_buffer_t;
};

template <>
struct representation_type<example::software_representation_t, texture_role_t> {
    using type = example::software_texture_t;
};

template <>
struct representation_type<example::software_representation_t, sampler_role_t> {
    using type = example::software_sampler_t;
};

template <>
struct representation_type<example::software_representation_t, shader_role_t> {
    using type = example::software_shader_t;
};

template <>
struct representation_type<example::opengl_representation_t, mesh_role_t> {
    using type = example::opengl_mesh_t;
};

template <>
struct representation_type<example::opengl_representation_t, index_buffer_role_t> {
    using type = example::opengl_index_buffer_t;
};

template <>
struct representation_type<example::opengl_representation_t, texture_role_t> {
    using type = example::opengl_texture_t;
};

template <>
struct representation_type<example::opengl_representation_t, sampler_role_t> {
    using type = example::opengl_sampler_t;
};

template <>
struct representation_type<example::opengl_representation_t, shader_role_t> {
    using type = example::opengl_shader_t;
};

/* OpenGL reuses the generic graph shape but adds backend state. */
template <>
struct representation_type<example::opengl_representation_t, geometry_role_t> {
    using type = basic_geometry_t<example::opengl_representation_t, example::opengl_geometry_state_t>;
};

template <>
struct representation_type<example::opengl_representation_t, shader_program_role_t> {
    using type = basic_shader_program_t<example::opengl_representation_t, example::opengl_shader_program_state_t>;
};

/**
 * The conversion belongs to the representation pair, not to either representation.
 */
template <>
class representation_compiler_t<example::software_representation_t, example::opengl_representation_t> {
public:
    using source_representation_t = example::software_representation_t;
    using target_representation_t = example::opengl_representation_t;

    std::shared_ptr<mesh_t<target_representation_t>> compile(
        const std::shared_ptr<mesh_t<source_representation_t>>& source);

    std::shared_ptr<index_buffer_t<target_representation_t>> compile(
        const std::shared_ptr<index_buffer_t<source_representation_t>>& source);

    std::shared_ptr<texture_t<target_representation_t>> compile(
        const std::shared_ptr<texture_t<source_representation_t>>& source);

    std::shared_ptr<sampler_t<target_representation_t>> compile(
        const std::shared_ptr<sampler_t<source_representation_t>>& source);

    std::shared_ptr<shader_t<target_representation_t>> compile(
        const std::shared_ptr<shader_t<source_representation_t>>& source);

    std::shared_ptr<shader_program_t<target_representation_t>> compile(
        const std::shared_ptr<shader_program_t<source_representation_t>>& source);

    texture_binding_t<target_representation_t> compile(
        const texture_binding_t<source_representation_t>& source);

    std::shared_ptr<material_t<target_representation_t>> compile(
        const std::shared_ptr<material_t<source_representation_t>>& source);

    std::shared_ptr<geometry_t<target_representation_t>> compile(
        const std::shared_ptr<geometry_t<source_representation_t>>& source);

    template <typename T, std::size_t N>
    std::shared_ptr<render_item_t<target_representation_t, T, N>> compile(
        const std::shared_ptr<render_item_t<source_representation_t, T, N>>& source);
};

} // namespace graphics_representation_graph

static_assert(std::same_as<
    graphics_representation_graph::geometry_t<example::software_representation_t>,
    graphics_representation_graph::basic_geometry_t<example::software_representation_t>>);

static_assert(std::same_as<
    typename graphics_representation_graph::geometry_t<example::opengl_representation_t>::state_t,
    example::opengl_geometry_state_t>);

static_assert(std::same_as<
    typename graphics_representation_graph::shader_program_t<example::opengl_representation_t>::state_t,
    example::opengl_shader_program_state_t>);

int main() {
    graphics_representation_graph::render_item_t<example::software_representation_t, float, 3> software_item;
    graphics_representation_graph::render_item_t<example::opengl_representation_t, float, 3> opengl_item;

    (void)software_item;
    (void)opengl_item;
}
