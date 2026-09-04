#include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>
#include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>
#include <m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_renderer.h>
#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gt0l0q3l4b1k27eab5k7py1_texture/api.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace byte_stream = m03gagbht2l61mj6qitacwbmea_byte_stream;
namespace soa = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace texture = m03gt0l0q3l4b1k27eab5k7py1_texture;

namespace {

using position_t = std::array<float, 2>;

constexpr api::rgba8_t clear_color {3, 5, 7, 11};
constexpr api::rgba8_t red {255, 0, 0, 255};
constexpr api::rgba8_t green {0, 255, 0, 255};
constexpr api::rgba8_t blue {0, 0, 255, 255};
constexpr api::rgba8_t white {255, 255, 255, 255};
constexpr api::rgba8_t texture_color {17, 34, 51, 68};

bool same_color(const api::rgba8_t& lhs, const api::rgba8_t& rhs) {
    return lhs.red == rhs.red
        && lhs.green == rhs.green
        && lhs.blue == rhs.blue
        && lhs.alpha == rhs.alpha;
}

void expect_color(const api::rgba8_t& actual, const api::rgba8_t& expected) {
    test::expect(std::identity(), same_color(actual, expected));
}

std::size_t pixel_index(int x, int y, int width) {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
        + static_cast<std::size_t>(x);
}

std::size_t colored_pixel_count(std::span<const api::rgba8_t> framebuffer) {
    return static_cast<std::size_t>(std::ranges::count_if(framebuffer, [](const auto& pixel) {
        return !same_color(pixel, clear_color);
    }));
}

std::shared_ptr<texture::sampler_t> make_sampler() {
    return std::make_shared<texture::sampler_t>(
        texture::filter_t::nearest,
        texture::address_mode_t::clamp_to_edge,
        texture::address_mode_t::clamp_to_edge
    );
}

std::shared_ptr<texture::texture_t> make_unorm_texture(
    std::size_t width,
    std::size_t height,
    std::span<const api::rgba8_t> texels
) {
    std::vector<std::byte> bytes;
    bytes.reserve(texels.size() * sizeof(api::rgba8_t));
    for (const auto texel : texels) {
        bytes.push_back(static_cast<std::byte>(texel.red));
        bytes.push_back(static_cast<std::byte>(texel.green));
        bytes.push_back(static_cast<std::byte>(texel.blue));
        bytes.push_back(static_cast<std::byte>(texel.alpha));
    }
    return std::make_shared<texture::texture_t>(
        texture::format_t::rgba8_unorm,
        width,
        height,
        byte_stream::byte_stream_t(std::move(bytes))
    );
}

std::shared_ptr<texture::texture_t> make_unorm_texture(api::rgba8_t color) {
    const std::array texels {color};
    return make_unorm_texture(1, 1, texels);
}

std::shared_ptr<texture::texture_t> make_float_texture(std::array<float, 4> color) {
    return std::make_shared<texture::texture_t>(
        texture::format_t::rgba32_float,
        1,
        1,
        byte_stream::byte_stream_t(std::as_bytes(std::span<const float>(color)))
    );
}

std::shared_ptr<api::index_buffer_t> make_indices(api::index_buffer_t::indices_t values) {
    auto result = std::make_shared<api::index_buffer_t>();
    result->indices() = std::move(values);
    return result;
}

template <typename Position>
std::shared_ptr<api::geometry_t> make_typed_geometry(
    std::vector<Position> positions,
    api::vertex_attribute_t attribute,
    api::index_buffer_t::indices_t indices,
    api::vertex_primitive_topology_t topology
) {
    soa::structure_of_arrays_t<Position> streams;
    for (const auto& position : positions) {
        streams.push_back(position);
    }
    auto mesh = std::make_shared<api::mesh_t>(
        std::move(streams),
        std::vector<api::vertex_attribute_t> {attribute}
    );
    auto geometry = std::make_shared<api::geometry_t>(make_indices(std::move(indices)));
    geometry->mesh() = std::move(mesh);
    geometry->primitive_topology() = topology;
    geometry->finalize();
    return geometry;
}

std::shared_ptr<api::geometry_t> make_geometry(
    std::vector<position_t> clip_positions,
    api::index_buffer_t::indices_t indices,
    api::vertex_primitive_topology_t topology
) {
    for (auto& position : clip_positions) {
        position[1] = -position[1];
    }
    return make_typed_geometry(
        std::move(clip_positions),
        api::vertex_attribute_t(api::vertex_attribute_type_t::R32, 2),
        std::move(indices),
        topology
    );
}

std::shared_ptr<api::material_t> make_material(
    std::shared_ptr<texture::texture_t> image,
    std::shared_ptr<texture::sampler_t> sampler = make_sampler()
) {
    auto material = std::make_shared<api::material_t>();
    material->texture_bindings().push_back({
        .texture = std::move(image),
        .sampler = std::move(sampler)
    });
    return material;
}

api::render_item_t<float, 2> make_render_item(
    std::shared_ptr<api::geometry_t> geometry,
    std::shared_ptr<api::material_t> material
) {
    api::render_item_t<float, 2> render_item;
    render_item.geometry(std::move(geometry));
    render_item.material(std::move(material));
    render_item.translation({0.0F, 0.0F});
    render_item.scale({1.0F, 1.0F});
    return render_item;
}

api::camera_t<float, int, 2> make_camera(int width, int height) {
    return api::camera_t<float, int, 2>(
        {{-1.0F, 1.0F}, {-1.0F, 1.0F}},
        {{0, width}, {0, height}}
    );
}

std::vector<api::rgba8_t> draw_scene(
    api::vertex_primitive_topology_t topology,
    std::vector<position_t> positions,
    api::index_buffer_t::indices_t indices,
    int width = 16,
    int height = 16,
    std::shared_ptr<texture::texture_t> image = make_unorm_texture(red)
) {
    std::vector<api::rgba8_t> pixels(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height),
        clear_color
    );
    api::software_renderer_t renderer({
        .pixels = pixels,
        .width = width,
        .height = height
    });
    const auto camera = make_camera(width, height);
    const auto render_item = make_render_item(
        make_geometry(std::move(positions), std::move(indices), topology),
        make_material(std::move(image))
    );
    renderer.draw(camera, render_item);
    return pixels;
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

void test_framebuffer() {
    std::vector<api::rgba8_t> pixels(6);
    test::expect_throws<std::invalid_argument>([&] {
        [[maybe_unused]] const api::software_renderer_t invalid({
            .pixels = pixels,
            .width = -1,
            .height = 6
        });
    });
    test::expect_throws<std::invalid_argument>([&] {
        [[maybe_unused]] const api::software_renderer_t invalid({
            .pixels = pixels,
            .width = 2,
            .height = 2
        });
    });

    api::software_renderer_t renderer({
        .pixels = pixels,
        .width = 3,
        .height = 2
    });

    renderer.clear(clear_color);
    test::expect(std::identity(), std::ranges::all_of(pixels, [](const auto& pixel) {
        return same_color(pixel, clear_color);
    }));

    const auto framebuffer = renderer.framebuffer();
    test::expect(std::identity(), framebuffer.pixels.data() == pixels.data());
    test::expect(std::equal_to<>(), framebuffer.width, 3);
    test::expect(std::equal_to<>(), framebuffer.height, 2);

    test::expect_throws<std::invalid_argument>([&] {
        renderer.framebuffer({.pixels = pixels, .width = -1, .height = 6});
    });
    test::expect_throws<std::invalid_argument>([&] {
        renderer.framebuffer({.pixels = pixels, .width = 2, .height = 2});
    });
    test::expect(std::identity(), renderer.framebuffer().pixels.data() == pixels.data());

    std::vector<api::rgba8_t> replacement(4);
    renderer.framebuffer({.pixels = replacement, .width = 2, .height = 2});
    renderer.clear(texture_color);
    test::expect(std::identity(), std::ranges::all_of(replacement, [](const auto& pixel) {
        return same_color(pixel, texture_color);
    }));
}

void test_empty_framebuffer() {
    std::vector<api::rgba8_t> pixels;
    api::software_renderer_t renderer({
        .pixels = pixels,
        .width = 0,
        .height = 4
    });
    test::expect_no_throw([&] { renderer.clear(clear_color); });

    const auto camera = make_camera(8, 8);
    const auto render_item = make_render_item(
        make_geometry({{0.0F, 0.0F}}, {0}, api::vertex_primitive_topology_t::point),
        make_material(make_unorm_texture(red))
    );
    test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, render_item); });
}

void test_topologies_and_clipping() {
    const auto point = draw_scene(
        api::vertex_primitive_topology_t::point,
        {{0.0F, 0.0F}},
        {0}
    );
    test::expect(std::equal_to<>(), colored_pixel_count(point), std::size_t(29));
    expect_color(point[pixel_index(8, 8, 16)], red);

    const auto clipped_point = draw_scene(
        api::vertex_primitive_topology_t::point,
        {{2.0F, 0.0F}},
        {0}
    );
    test::expect(std::equal_to<>(), colored_pixel_count(clipped_point), std::size_t(0));

    const auto line = draw_scene(
        api::vertex_primitive_topology_t::line,
        {{-2.0F, 0.0F}, {0.5F, 0.0F}},
        {0, 1}
    );
    expect_color(line[pixel_index(0, 8, 16)], red);
    expect_color(line[pixel_index(12, 8, 16)], red);
    expect_color(line[pixel_index(15, 8, 16)], clear_color);

    const std::vector<position_t> line_vertices {
        {-0.75F, -0.5F},
        {0.0F, 0.5F},
        {0.75F, -0.5F}
    };
    const auto line_strip = draw_scene(
        api::vertex_primitive_topology_t::line_strip,
        line_vertices,
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(line_strip), std::size_t(12));

    const auto line_loop = draw_scene(
        api::vertex_primitive_topology_t::line_loop,
        line_vertices,
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(line_loop), colored_pixel_count(line_strip));
    expect_color(line_loop[pixel_index(8, 12, 16)], red);

    const auto clipped_triangle = draw_scene(
        api::vertex_primitive_topology_t::triangle,
        {{-2.0F, -0.75F}, {0.75F, -0.75F}, {0.0F, 0.75F}},
        {0, 1, 2}
    );
    test::expect(std::greater<>(), colored_pixel_count(clipped_triangle), std::size_t(0));
    expect_color(clipped_triangle[pixel_index(8, 8, 16)], red);

    const auto triangle_strip = draw_scene(
        api::vertex_primitive_topology_t::triangle_strip,
        {{-0.75F, -0.75F}, {-0.75F, 0.75F}, {0.75F, -0.75F}, {0.75F, 0.75F}},
        {0, 1, 2, 3}
    );
    expect_color(triangle_strip[pixel_index(8, 8, 16)], red);
    expect_color(triangle_strip[pixel_index(12, 4, 16)], red);

    const auto triangle_fan = draw_scene(
        api::vertex_primitive_topology_t::triangle_fan,
        {{-0.75F, -0.75F}, {0.75F, -0.75F}, {0.75F, 0.75F}, {-0.75F, 0.75F}},
        {0, 1, 2, 3}
    );
    expect_color(triangle_fan[pixel_index(8, 8, 16)], red);
    expect_color(triangle_fan[pixel_index(4, 4, 16)], red);
}

void test_shared_edge_coverage() {
    const auto framebuffer = draw_scene(
        api::vertex_primitive_topology_t::triangle_strip,
        {{-1.0F, -1.0F}, {-1.0F, 1.0F}, {1.0F, -1.0F}, {1.0F, 1.0F}},
        {0, 1, 2, 3}
    );
    test::expect(std::equal_to<>(), colored_pixel_count(framebuffer), framebuffer.size());
    test::expect(std::identity(), std::ranges::all_of(framebuffer, [](const auto& pixel) {
        return same_color(pixel, red);
    }));
}

void test_texture_coordinate_interpolation() {
    const std::array texels {red, green, blue, white};
    const auto framebuffer = draw_scene(
        api::vertex_primitive_topology_t::triangle_strip,
        {{-1.0F, -1.0F}, {-1.0F, 1.0F}, {1.0F, -1.0F}, {1.0F, 1.0F}},
        {0, 1, 2, 3},
        8,
        8,
        make_unorm_texture(2, 2, texels)
    );
    expect_color(framebuffer[pixel_index(1, 1, 8)], red);
    expect_color(framebuffer[pixel_index(6, 1, 8)], green);
    expect_color(framebuffer[pixel_index(1, 6, 8)], blue);
    expect_color(framebuffer[pixel_index(6, 6, 8)], white);
}

void test_explicit_color_and_rgba8_conversion() {
    const float infinity = std::numeric_limits<float>::infinity();
    const auto special = draw_scene(
        api::vertex_primitive_topology_t::point,
        {{0.0F, 0.0F}},
        {0},
        8,
        8,
        make_float_texture({
            std::numeric_limits<float>::quiet_NaN(),
            -infinity,
            infinity,
            0.5F
        })
    );
    expect_color(special[pixel_index(4, 4, 8)], {0, 0, 255, 128});

    const auto clamped = draw_scene(
        api::vertex_primitive_topology_t::point,
        {{0.0F, 0.0F}},
        {0},
        8,
        8,
        make_float_texture({-1.0F, 2.0F, 0.5F, 1.5F})
    );
    expect_color(clamped[pixel_index(4, 4, 8)], {0, 255, 128, 255});
}

void test_vertex_layout_rejection() {
    std::vector<api::rgba8_t> pixels(8 * 8, clear_color);
    api::software_renderer_t renderer({.pixels = pixels, .width = 8, .height = 8});
    const auto camera = make_camera(8, 8);
    const auto material = make_material(make_unorm_texture(red));

    const auto expect_rejected = [&](auto geometry) {
        const auto render_item = make_render_item(std::move(geometry), material);
        test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, render_item); });
    };

    expect_rejected(make_typed_geometry(
        std::vector<float> {0.0F},
        api::vertex_attribute_t(api::vertex_attribute_type_t::R32, 1),
        {0},
        api::vertex_primitive_topology_t::point
    ));
    expect_rejected(make_typed_geometry(
        std::vector<std::array<double, 2>> {{{0.0, 0.0}}},
        api::vertex_attribute_t(api::vertex_attribute_type_t::R64, 2),
        {0},
        api::vertex_primitive_topology_t::point
    ));
    expect_rejected(make_typed_geometry(
        std::vector<std::array<std::int32_t, 2>> {{{0, 0}}},
        api::vertex_attribute_t(api::vertex_attribute_type_t::I32, 2),
        {0},
        api::vertex_primitive_topology_t::point
    ));
    expect_rejected(make_typed_geometry(
        std::vector<std::array<std::uint32_t, 2>> {{{0, 0}}},
        api::vertex_attribute_t(api::vertex_attribute_type_t::U32, 2),
        {0},
        api::vertex_primitive_topology_t::point
    ));
}

void test_material_resource_mapping() {
    std::vector<api::rgba8_t> pixels(8 * 8, clear_color);
    api::software_renderer_t renderer({.pixels = pixels, .width = 8, .height = 8});
    const auto camera = make_camera(8, 8);
    const auto geometry = make_geometry(
        {{0.0F, 0.0F}},
        {0},
        api::vertex_primitive_topology_t::point
    );

    auto mapped_material = make_material(make_unorm_texture(texture_color));
    mapped_material->texture_bindings().push_back({
        .texture = make_unorm_texture(red),
        .sampler = make_sampler()
    });
    const auto mapped = make_render_item(geometry, std::move(mapped_material));
    renderer.draw(camera, mapped);
    expect_color(pixels[pixel_index(4, 4, 8)], texture_color);

    const auto missing = make_render_item(geometry, std::make_shared<api::material_t>());
    test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, missing); });

    auto null_texture_material = make_material(nullptr);
    const auto null_texture = make_render_item(geometry, std::move(null_texture_material));
    test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, null_texture); });

    auto null_sampler_material = make_material(make_unorm_texture(red), nullptr);
    const auto null_sampler = make_render_item(geometry, std::move(null_sampler_material));
    test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, null_sampler); });
}

void test_nonfinite_clip_position_rejection() {
    std::vector<api::rgba8_t> pixels(8 * 8, clear_color);
    api::software_renderer_t renderer({.pixels = pixels, .width = 8, .height = 8});
    const auto camera = make_camera(8, 8);
    const auto material = make_material(make_unorm_texture(red));

    const auto infinity = make_render_item(
        make_geometry(
            {{std::numeric_limits<float>::infinity(), 0.0F}},
            {0},
            api::vertex_primitive_topology_t::point
        ),
        material
    );
    test::expect_throws<std::runtime_error>([&] { renderer.draw(camera, infinity); });

    const auto nan = make_render_item(
        make_geometry(
            {{std::numeric_limits<float>::quiet_NaN(), 0.0F}},
            {0},
            api::vertex_primitive_topology_t::point
        ),
        material
    );
    test::expect_throws<std::runtime_error>([&] { renderer.draw(camera, nan); });
}

} // namespace

int main() {
    return test::run([] {
        static_assert(sizeof(api::rgba8_t) == 4);
        test_resource_model();
        test_framebuffer();
        test_empty_framebuffer();
        test_topologies_and_clipping();
        test_shared_edge_coverage();
        test_texture_coordinate_interpolation();
        test_explicit_color_and_rgba8_conversion();
        test_vertex_layout_rejection();
        test_material_resource_mapping();
        test_nonfinite_clip_position_rejection();
    });
}
