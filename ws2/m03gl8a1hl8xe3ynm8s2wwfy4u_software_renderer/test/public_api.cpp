# include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>
# include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>
# include <m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_pipeline.h>
# include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
# include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>
# include <m03gt0l0q3l4b1k27eab5k7py1_texture/api.h>

# include <algorithm>
# include <array>
# include <cstddef>
# include <cstdint>
# include <functional>
# include <limits>
# include <memory>
# include <span>
# include <stdexcept>
# include <utility>
# include <vector>

namespace api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace byte_stream = m03gagbht2l61mj6qitacwbmea_byte_stream;
namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace soa = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace software_shader = m03gt1djvvy5atia5evkbg6rqy_software_shader;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace texture = m03gt0l0q3l4b1k27eab5k7py1_texture;

namespace {

using clip_position_t = std::array<float, 4>;
using vector2f_t = shader::vector_t<float, 2>;
using vector4f_t = shader::vector_t<float, 4>;

constexpr api::rgba8_t clear_color {3, 5, 7, 11};
constexpr api::rgba8_t red {255, 0, 0, 255};
constexpr api::rgba8_t green {0, 255, 0, 255};

bool same_color(const api::rgba8_t& lhs, const api::rgba8_t& rhs) {
    return lhs.red == rhs.red
        && lhs.green == rhs.green
        && lhs.blue == rhs.blue
        && lhs.alpha == rhs.alpha;
}

void expect_color(const api::rgba8_t& actual, const api::rgba8_t& expected) {
    test::expect(std::identity(), same_color(actual, expected));
}

std::size_t colored_pixel_count(std::span<const api::rgba8_t> framebuffer) {
    return static_cast<std::size_t>(std::ranges::count_if(framebuffer, [](const auto& pixel) {
        return !same_color(pixel, clear_color);
    }));
}

std::shared_ptr<api::index_buffer_t> make_indices(api::index_buffer_t::indices_t values) {
    auto result = std::make_shared<api::index_buffer_t>();
    result->indices() = std::move(values);
    return result;
}

api::geometry_t make_geometry(
    std::vector<clip_position_t> positions,
    api::index_buffer_t::indices_t indices,
    api::vertex_primitive_topology_t topology
) {
    soa::structure_of_arrays_t<clip_position_t> streams;
    for (const auto& position : positions) {
        streams.push_back(position);
    }
    auto mesh = std::make_shared<api::mesh_t>(
        std::move(streams),
        std::vector<api::vertex_attribute_t> {
            api::vertex_attribute_t(api::vertex_attribute_type_t::R32, 4)
        }
    );
    api::geometry_t geometry(make_indices(std::move(indices)));
    geometry.mesh() = std::move(mesh);
    geometry.primitive_topology() = topology;
    geometry.finalize();
    return geometry;
}

software_shader::program_t solid_program(vector4f_t color) {
    shader::vertex_shader_ast_builder_t vertex;
    vertex.position(vertex.input<vector4f_t>(0));

    shader::fragment_shader_ast_builder_t fragment;
    fragment.color(color);

    return software_shader::program_t(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );
}

std::vector<api::rgba8_t> draw_solid(
    api::vertex_primitive_topology_t topology,
    std::vector<clip_position_t> positions,
    api::index_buffer_t::indices_t indices,
    int width = 16,
    int height = 16
) {
    auto geometry = make_geometry(std::move(positions), std::move(indices), topology);
    auto program = solid_program(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F}));
    const software_shader::bindings_t bindings;
    std::vector<api::rgba8_t> framebuffer(
        static_cast<std::size_t>(width * height),
        clear_color
    );
    api::draw_software_pipeline(program, bindings, geometry, width, height, framebuffer);
    return framebuffer;
}

texture::texture_t single_texel(std::array<std::uint8_t, 4> color) {
    std::vector<std::byte> bytes;
    bytes.reserve(color.size());
    for (const auto component : color) {
        bytes.push_back(static_cast<std::byte>(component));
    }
    return texture::texture_t(
        texture::format_t::rgba8_unorm,
        1,
        1,
        byte_stream::byte_stream_t(std::move(bytes))
    );
}

void test_resource_model() {
    soa::structure_of_arrays_t<float> vertices;
    vertices.push_back(0.0F);
    vertices.push_back(1.0F);
    vertices.push_back(2.0F);
    const auto mesh = std::make_shared<api::mesh_t>(
        std::move(vertices),
        std::vector<api::vertex_attribute_t> {
            api::vertex_attribute_t(api::vertex_attribute_type_t::R32, 1)
        }
    );
    test::expect(std::equal_to<>(), mesh->number_of_vertices(), std::size_t(3));

    const auto index_buffer = make_indices({0, 1, 2});
    api::geometry_t geometry(index_buffer);
    geometry.mesh() = mesh;
    test::expect_no_throw([&] { geometry.finalize(); });
    test::expect(std::equal_to<>(), geometry.indices().size(), std::size_t(3));

    test::expect_throws<std::invalid_argument>([] {
        [[maybe_unused]] const api::geometry_t invalid(nullptr);
    });
    test::expect_throws<std::out_of_range>([&] {
        [[maybe_unused]] const api::geometry_t invalid(
            index_buffer,
            api::index_range_t {.offset = 2, .count = 2}
        );
    });

    api::geometry_t missing_mesh(make_indices({0, 1, 2}));
    test::expect_throws<std::runtime_error>([&] { missing_mesh.finalize(); });

    api::geometry_t invalid_index(make_indices({0, 1, 3}));
    invalid_index.mesh() = mesh;
    test::expect_throws<std::runtime_error>([&] { invalid_index.finalize(); });

    api::geometry_t invalid_line(make_indices({0, 1, 2}));
    invalid_line.mesh() = mesh;
    invalid_line.primitive_topology() = api::vertex_primitive_topology_t::line;
    test::expect_throws<std::runtime_error>([&] { invalid_line.finalize(); });

    api::geometry_t mutable_range(index_buffer);
    index_buffer->indices().resize(1);
    test::expect_throws<std::out_of_range>([&] { (void)mutable_range.indices(); });

    soa::structure_of_arrays_t<float> overflow_vertices;
    overflow_vertices.push_back(0.0F);
    test::expect_throws<std::length_error>([&] {
        [[maybe_unused]] const api::mesh_t invalid(
            std::move(overflow_vertices),
            std::vector<api::vertex_attribute_t> {
                api::vertex_attribute_t(
                    api::vertex_attribute_type_t::R64,
                    std::numeric_limits<std::size_t>::max()
                )
            }
        );
    });
}

void test_topologies_and_clipping() {
    const auto point = draw_solid(
        api::vertex_primitive_topology_t::point,
        {{0.0F, 0.0F, 0.0F, 1.0F}},
        {0}
    );
    test::expect(std::equal_to<>(), colored_pixel_count(point), std::size_t(29));
    expect_color(point[8 * 16 + 8], red);

    const auto clipped_point = draw_solid(
        api::vertex_primitive_topology_t::point,
        {{2.0F, 0.0F, 0.0F, 1.0F}},
        {0}
    );
    test::expect(std::equal_to<>(), colored_pixel_count(clipped_point), std::size_t(0));

    const auto line = draw_solid(
        api::vertex_primitive_topology_t::line,
        {{-2.0F, 0.0F, 0.0F, 1.0F}, {0.5F, 0.0F, 0.0F, 1.0F}},
        {0, 1}
    );
    expect_color(line[8 * 16], red);
    expect_color(line[8 * 16 + 12], red);
    expect_color(line[8 * 16 + 15], clear_color);

    const std::vector<clip_position_t> line_vertices {
        {-0.75F, -0.5F, 0.0F, 1.0F},
        {0.0F, 0.5F, 0.0F, 1.0F},
        {0.75F, -0.5F, 0.0F, 1.0F}
    };
    const auto line_strip = draw_solid(
        api::vertex_primitive_topology_t::line_strip,
        line_vertices,
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(line_strip), std::size_t(12));

    const auto line_loop = draw_solid(
        api::vertex_primitive_topology_t::line_loop,
        line_vertices,
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(line_loop), colored_pixel_count(line_strip));
    expect_color(line_loop[12 * 16 + 8], red);

    const auto clipped_triangle = draw_solid(
        api::vertex_primitive_topology_t::triangle,
        {{-2.0F, -0.75F, 0.0F, 1.0F}, {0.75F, -0.75F, 0.0F, 1.0F}, {0.0F, 0.75F, 0.0F, 1.0F}},
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(clipped_triangle), std::size_t(0));
    expect_color(clipped_triangle[8 * 16 + 8], red);

    const auto triangle_strip = draw_solid(
        api::vertex_primitive_topology_t::triangle_strip,
        {{-0.75F, -0.75F, 0.0F, 1.0F}, {-0.75F, 0.75F, 0.0F, 1.0F}, {0.75F, -0.75F, 0.0F, 1.0F}, {0.75F, 0.75F, 0.0F, 1.0F}},
        {0, 1, 2, 3}
    );
    expect_color(triangle_strip[8 * 16 + 8], red);
    expect_color(triangle_strip[4 * 16 + 12], red);

    const auto triangle_fan = draw_solid(
        api::vertex_primitive_topology_t::triangle_fan,
        {{-0.75F, -0.75F, 0.0F, 1.0F}, {0.75F, -0.75F, 0.0F, 1.0F}, {0.75F, 0.75F, 0.0F, 1.0F}, {-0.75F, 0.75F, 0.0F, 1.0F}},
        {0, 1, 2, 3}
    );
    expect_color(triangle_fan[8 * 16 + 8], red);
    expect_color(triangle_fan[4 * 16 + 4], red);
}

void test_shared_edge_and_front_facing() {
    shader::vertex_shader_ast_builder_t vertex;
    vertex.position(vertex.input<vector4f_t>(0));

    shader::fragment_shader_ast_builder_t fragment;
    fragment.branch(
        fragment.front_facing(),
        [&] { fragment.color(vector4f_t({0.0F, 1.0F, 0.0F, 1.0F})); },
        [&] { fragment.color(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F})); }
    );
    software_shader::program_t program(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );
    const software_shader::bindings_t bindings;

    auto geometry = make_geometry(
        {{-1.0F, -1.0F, 0.0F, 1.0F}, {-1.0F, 1.0F, 0.0F, 1.0F}, {1.0F, -1.0F, 0.0F, 1.0F}, {1.0F, 1.0F, 0.0F, 1.0F}},
        {0, 1, 2, 3},
        api::vertex_primitive_topology_t::triangle_strip
    );
    std::vector<api::rgba8_t> framebuffer(16 * 16, clear_color);
    api::draw_software_pipeline(program, bindings, geometry, 16, 16, framebuffer);
    test::expect(std::equal_to<>(), colored_pixel_count(framebuffer), framebuffer.size());
    test::expect(std::identity(), std::ranges::all_of(framebuffer, [](const auto& pixel) {
        return same_color(pixel, green);
    }));
}

void test_interpolation_and_fragment_builtins() {
    soa::structure_of_arrays_t<clip_position_t, float> streams;
    streams.push_back({-0.8F, -0.8F, 0.0F, 1.0F}, 0.0F);
    streams.push_back({1.6F, -1.6F, 0.0F, 2.0F}, 1.0F);
    streams.push_back({0.0F, 1.6F, 0.0F, 2.0F}, 1.0F);
    auto mesh = std::make_shared<api::mesh_t>(
        std::move(streams),
        std::vector<api::vertex_attribute_t> {
            {api::vertex_attribute_type_t::R32, 4},
            {api::vertex_attribute_type_t::R32, 1}
        }
    );
    api::geometry_t geometry(make_indices({0, 1, 2}));
    geometry.mesh() = std::move(mesh);
    geometry.finalize();

    shader::vertex_shader_ast_builder_t vertex;
    vertex.position(vertex.input<vector4f_t>(0));
    vertex.output(0, vertex.input<float>(1));
    shader::fragment_shader_ast_builder_t fragment;
    fragment.color(fragment.construct<vector4f_t>(fragment.input<float>(0), 0.0F, 0.0F, 1.0F));
    software_shader::program_t program(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );
    const software_shader::bindings_t bindings;
    std::vector<api::rgba8_t> framebuffer(10 * 10, clear_color);
    api::draw_software_pipeline(program, bindings, geometry, 10, 10, framebuffer);
    test::expect(std::equal_to<>(), framebuffer[5 * 10 + 5].red, std::uint8_t(163));

    soa::structure_of_arrays_t<clip_position_t, float> line_streams;
    line_streams.push_back({-0.8F, 0.0F, 0.0F, 1.0F}, 0.0F);
    line_streams.push_back({1.6F, 0.0F, 0.0F, 2.0F}, 1.0F);
    auto line_mesh = std::make_shared<api::mesh_t>(
        std::move(line_streams),
        std::vector<api::vertex_attribute_t> {
            {api::vertex_attribute_type_t::R32, 4},
            {api::vertex_attribute_type_t::R32, 1}
        }
    );
    api::geometry_t line_geometry(make_indices({0, 1}));
    line_geometry.mesh() = std::move(line_mesh);
    line_geometry.primitive_topology() = api::vertex_primitive_topology_t::line;
    line_geometry.finalize();
    std::vector<api::rgba8_t> line_framebuffer(16 * 16, clear_color);
    api::draw_software_pipeline(program, bindings, line_geometry, 16, 16, line_framebuffer);
    test::expect(std::equal_to<>(), line_framebuffer[8 * 16 + 8].red, std::uint8_t(94));

    auto point = make_geometry(
        {{0.0F, 0.0F, 0.0F, 2.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );
    shader::vertex_shader_ast_builder_t builtin_vertex;
    builtin_vertex.position(builtin_vertex.input<vector4f_t>(0));
    shader::fragment_shader_ast_builder_t builtin_fragment;
    builtin_fragment.color(
        builtin_fragment.fragment_coordinate()
        / vector4f_t({8.0F, 8.0F, 1.0F, 1.0F})
    );
    software_shader::program_t builtin_program(
        std::move(builtin_vertex).finalize(),
        std::move(builtin_fragment).finalize()
    );
    std::vector<api::rgba8_t> builtin_framebuffer(8 * 8, clear_color);
    api::draw_software_pipeline(builtin_program, bindings, point, 8, 8, builtin_framebuffer);
    expect_color(builtin_framebuffer[4 * 8 + 4], {143, 143, 128, 128});
}

void test_color_discard_and_conversion() {
    const float infinity = std::numeric_limits<float>::infinity();
    auto geometry = make_geometry(
        {{0.0F, 0.0F, 0.0F, 1.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );
    auto program = solid_program(vector4f_t({
        std::numeric_limits<float>::quiet_NaN(),
        -infinity,
        infinity,
        0.5F
    }));
    const software_shader::bindings_t bindings;
    std::vector<api::rgba8_t> framebuffer(8 * 8, clear_color);
    api::draw_software_pipeline(program, bindings, geometry, 8, 8, framebuffer);
    expect_color(framebuffer[4 * 8 + 4], {0, 0, 255, 128});

    auto finite_program = solid_program(vector4f_t({-1.0F, 2.0F, 0.5F, 1.5F}));
    std::ranges::fill(framebuffer, clear_color);
    api::draw_software_pipeline(finite_program, bindings, geometry, 8, 8, framebuffer);
    expect_color(framebuffer[4 * 8 + 4], {0, 255, 128, 255});

    shader::vertex_shader_ast_builder_t no_color_vertex;
    no_color_vertex.position(no_color_vertex.input<vector4f_t>(0));
    shader::fragment_shader_ast_builder_t no_color_fragment;
    no_color_fragment.output(0, 1.0F);
    software_shader::program_t no_color_program(
        std::move(no_color_vertex).finalize(),
        std::move(no_color_fragment).finalize()
    );
    std::ranges::fill(framebuffer, clear_color);
    api::draw_software_pipeline(no_color_program, bindings, geometry, 8, 8, framebuffer);
    test::expect(std::equal_to<>(), colored_pixel_count(framebuffer), std::size_t(0));

    shader::vertex_shader_ast_builder_t discard_vertex;
    discard_vertex.position(discard_vertex.input<vector4f_t>(0));
    shader::fragment_shader_ast_builder_t discard_fragment;
    discard_fragment.color(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F}));
    discard_fragment.discard();
    software_shader::program_t discard_program(
        std::move(discard_vertex).finalize(),
        std::move(discard_fragment).finalize()
    );
    api::draw_software_pipeline(discard_program, bindings, geometry, 8, 8, framebuffer);
    test::expect(std::equal_to<>(), colored_pixel_count(framebuffer), std::size_t(0));
}

void test_vertex_layout_and_failures() {
    using int2_t = std::array<std::int32_t, 2>;
    using int3_t = std::array<std::int32_t, 3>;
    using int4_t = std::array<std::int32_t, 4>;
    using uint2_t = std::array<std::uint32_t, 2>;
    using uint3_t = std::array<std::uint32_t, 3>;
    using uint4_t = std::array<std::uint32_t, 4>;
    soa::structure_of_arrays_t<
        clip_position_t,
        float,
        std::array<float, 2>,
        std::array<float, 3>,
        std::int32_t,
        int2_t,
        int3_t,
        int4_t,
        std::uint32_t,
        uint2_t,
        uint3_t,
        uint4_t
    > streams;
    streams.push_back(
        {0.0F, 0.0F, 0.0F, 1.0F},
        1.0F,
        {2.0F, 3.0F},
        {4.0F, 5.0F, 6.0F},
        -1,
        {-2, -3},
        {-4, -5, -6},
        {-7, -8, -9, -10},
        1,
        {2, 3},
        {4, 5, 6},
        {7, 8, 9, 10}
    );
    auto mesh = std::make_shared<api::mesh_t>(
        std::move(streams),
        std::vector<api::vertex_attribute_t> {
            {api::vertex_attribute_type_t::R32, 4},
            {api::vertex_attribute_type_t::R32, 1},
            {api::vertex_attribute_type_t::R32, 2},
            {api::vertex_attribute_type_t::R32, 3},
            {api::vertex_attribute_type_t::I32, 1},
            {api::vertex_attribute_type_t::I32, 2},
            {api::vertex_attribute_type_t::I32, 3},
            {api::vertex_attribute_type_t::I32, 4},
            {api::vertex_attribute_type_t::U32, 1},
            {api::vertex_attribute_type_t::U32, 2},
            {api::vertex_attribute_type_t::U32, 3},
            {api::vertex_attribute_type_t::U32, 4}
        }
    );
    api::geometry_t geometry(make_indices({0}));
    geometry.mesh() = std::move(mesh);
    geometry.primitive_topology() = api::vertex_primitive_topology_t::point;
    geometry.finalize();

    shader::vertex_shader_ast_builder_t vertex;
    vertex.position(vertex.input<vector4f_t>(0));
    vertex.output(1, vertex.input<float>(1));
    vertex.output(2, vertex.input<vector2f_t>(2));
    vertex.output(3, vertex.input<shader::vector_t<float, 3>>(3));
    vertex.output(4, vertex.input<std::int32_t>(4));
    vertex.output(5, vertex.input<shader::vector_t<std::int32_t, 2>>(5));
    vertex.output(6, vertex.input<shader::vector_t<std::int32_t, 3>>(6));
    vertex.output(7, vertex.input<shader::vector_t<std::int32_t, 4>>(7));
    vertex.output(8, vertex.input<std::uint32_t>(8));
    vertex.output(9, vertex.input<shader::vector_t<std::uint32_t, 2>>(9));
    vertex.output(10, vertex.input<shader::vector_t<std::uint32_t, 3>>(10));
    vertex.output(11, vertex.input<shader::vector_t<std::uint32_t, 4>>(11));
    shader::fragment_shader_ast_builder_t fragment;
    fragment.color(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F}));
    software_shader::program_t program(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );
    const software_shader::bindings_t bindings;
    std::vector<api::rgba8_t> framebuffer(8 * 8, clear_color);
    test::expect_no_throw([&] {
        api::draw_software_pipeline(program, bindings, geometry, 8, 8, framebuffer);
    });

    soa::structure_of_arrays_t<std::array<double, 4>> wrong_stream;
    wrong_stream.push_back({0.0, 0.0, 0.0, 1.0});
    auto wrong_mesh = std::make_shared<api::mesh_t>(
        std::move(wrong_stream),
        std::vector<api::vertex_attribute_t> {{api::vertex_attribute_type_t::R64, 4}}
    );
    api::geometry_t wrong_geometry(make_indices({0}));
    wrong_geometry.mesh() = std::move(wrong_mesh);
    wrong_geometry.primitive_topology() = api::vertex_primitive_topology_t::point;
    wrong_geometry.finalize();
    auto wrong_program = solid_program(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F}));
    test::expect_throws<std::invalid_argument>([&] {
        api::draw_software_pipeline(wrong_program, bindings, wrong_geometry, 8, 8, framebuffer);
    });

    shader::vertex_shader_ast_builder_t integer_vertex;
    integer_vertex.position(integer_vertex.input<vector4f_t>(0));
    integer_vertex.output(0, std::int32_t(1));
    shader::fragment_shader_ast_builder_t integer_fragment;
    integer_fragment.output(0, integer_fragment.input<std::int32_t>(0));
    integer_fragment.color(vector4f_t({1.0F, 0.0F, 0.0F, 1.0F}));
    software_shader::program_t integer_program(
        std::move(integer_vertex).finalize(),
        std::move(integer_fragment).finalize()
    );
    auto integer_geometry = make_geometry(
        {{0.0F, 0.0F, 0.0F, 1.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );
    test::expect_throws<std::invalid_argument>([&] {
        api::draw_software_pipeline(integer_program, bindings, integer_geometry, 8, 8, framebuffer);
    });

    soa::structure_of_arrays_t<clip_position_t, float> conditional_streams;
    conditional_streams.push_back({0.0F, 0.0F, 0.0F, 1.0F}, 0.0F);
    auto conditional_mesh = std::make_shared<api::mesh_t>(
        std::move(conditional_streams),
        std::vector<api::vertex_attribute_t> {
            {api::vertex_attribute_type_t::R32, 4},
            {api::vertex_attribute_type_t::R32, 1}
        }
    );
    api::geometry_t conditional_geometry(make_indices({0}));
    conditional_geometry.mesh() = std::move(conditional_mesh);
    conditional_geometry.primitive_topology() = api::vertex_primitive_topology_t::point;
    conditional_geometry.finalize();
    shader::vertex_shader_ast_builder_t conditional_vertex;
    conditional_vertex.position(conditional_vertex.input<vector4f_t>(0));
    conditional_vertex.branch(conditional_vertex.input<float>(1) > 0.0F, [&] {
        conditional_vertex.output(0, 1.0F);
    });
    shader::fragment_shader_ast_builder_t conditional_fragment;
    conditional_fragment.color(conditional_fragment.construct<vector4f_t>(
        conditional_fragment.input<float>(0),
        0.0F,
        0.0F,
        1.0F
    ));
    software_shader::program_t conditional_program(
        std::move(conditional_vertex).finalize(),
        std::move(conditional_fragment).finalize()
    );
    test::expect_throws<std::runtime_error>([&] {
        api::draw_software_pipeline(conditional_program, bindings, conditional_geometry, 8, 8, framebuffer);
    });

    auto nonfinite_geometry = make_geometry(
        {{std::numeric_limits<float>::infinity(), 0.0F, 0.0F, 1.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );
    test::expect_throws<std::runtime_error>([&] {
        api::draw_software_pipeline(wrong_program, bindings, nonfinite_geometry, 8, 8, framebuffer);
    });
}

void test_material_resource_mapping() {
    shader::vertex_shader_ast_builder_t vertex;
    vertex.position(vertex.input<vector4f_t>(0));
    vertex.output(0, vector2f_t({0.5F, 0.5F}));
    shader::fragment_shader_ast_builder_t fragment;
    const auto image = fragment.resource<shader::shader_texture_2d_t>(1);
    const auto sampler = fragment.resource<shader::shader_sampler_t>(1);
    fragment.color(shader::sample(image, sampler, fragment.input<vector2f_t>(0)));
    software_shader::program_t program(
        std::move(vertex).finalize(),
        std::move(fragment).finalize()
    );

    auto image_resource = std::make_shared<texture::texture_t>(single_texel({17, 34, 51, 68}));
    auto sampler_resource = std::make_shared<texture::sampler_t>(
        texture::filter_t::nearest,
        texture::address_mode_t::clamp_to_edge,
        texture::address_mode_t::clamp_to_edge
    );
    api::material_t material;
    material.texture_bindings().resize(2);
    material.texture_bindings()[1] = {image_resource, sampler_resource};
    software_shader::bindings_t bindings;
    api::bind_material_resources(program, material, bindings);
    test::expect(std::identity(), &bindings.texture(1) == image_resource.get());
    test::expect(std::identity(), &bindings.sampler(1) == sampler_resource.get());

    auto geometry = make_geometry(
        {{0.0F, 0.0F, 0.0F, 1.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );
    std::vector<api::rgba8_t> framebuffer(8 * 8, clear_color);
    api::draw_software_pipeline(program, bindings, geometry, 8, 8, framebuffer);
    expect_color(framebuffer[4 * 8 + 4], {17, 34, 51, 68});

    api::material_t missing;
    software_shader::bindings_t missing_bindings;
    test::expect_throws<std::invalid_argument>([&] {
        api::bind_material_resources(program, missing, missing_bindings);
    });
    missing.texture_bindings().resize(2);
    missing.texture_bindings()[1].texture = image_resource;
    test::expect_throws<std::invalid_argument>([&] {
        api::bind_material_resources(program, missing, missing_bindings);
    });
}

} // namespace

int main() {
    return test::run([] {
        static_assert(sizeof(api::rgba8_t) == 4);
        test_resource_model();
        test_topologies_and_clipping();
        test_shared_edge_and_front_facing();
        test_interpolation_and_fragment_builtins();
        test_color_discard_and_conversion();
        test_vertex_layout_and_failures();
        test_material_resource_mapping();
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::software_renderer_t renderer(nullptr);
        });
    });
}
