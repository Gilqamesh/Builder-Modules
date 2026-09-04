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
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace byte_stream = m03gagbht2l61mj6qitacwbmea_byte_stream;
namespace soa = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace texture = m03gt0l0q3l4b1k27eab5k7py1_texture;

namespace {

constexpr api::rgba8_t clear_color {3, 5, 7, 11};
constexpr api::rgba8_t texture_color {17, 34, 51, 68};

bool same_color(const api::rgba8_t& lhs, const api::rgba8_t& rhs) {
    return lhs.red == rhs.red
        && lhs.green == rhs.green
        && lhs.blue == rhs.blue
        && lhs.alpha == rhs.alpha;
}

std::shared_ptr<texture::texture_t> make_texture(api::rgba8_t color) {
    std::vector<std::byte> bytes {
        static_cast<std::byte>(color.red),
        static_cast<std::byte>(color.green),
        static_cast<std::byte>(color.blue),
        static_cast<std::byte>(color.alpha)
    };
    return std::make_shared<texture::texture_t>(
        texture::format_t::rgba8_unorm,
        1,
        1,
        byte_stream::byte_stream_t(std::move(bytes))
    );
}

api::render_item_t<float, 2> make_render_item() {
    soa::structure_of_arrays_t<std::array<float, 2>> streams;
    streams.push_back({-1.0F, -1.0F});
    streams.push_back({-1.0F, 1.0F});
    streams.push_back({1.0F, -1.0F});
    streams.push_back({1.0F, 1.0F});
    auto mesh = std::make_shared<api::mesh_t>(
        std::move(streams),
        std::vector<api::vertex_attribute_t> {
            {api::vertex_attribute_type_t::R32, 2}
        }
    );

    auto index_buffer = std::make_shared<api::index_buffer_t>();
    index_buffer->indices() = {0, 1, 2, 3};
    auto geometry = std::make_shared<api::geometry_t>(index_buffer);
    geometry->mesh() = std::move(mesh);
    geometry->primitive_topology() = api::vertex_primitive_topology_t::triangle_strip;
    geometry->finalize();

    auto material = std::make_shared<api::material_t>();
    material->texture_bindings().push_back({
        .texture = make_texture(texture_color),
        .sampler = std::make_shared<texture::sampler_t>(
            texture::filter_t::nearest,
            texture::address_mode_t::clamp_to_edge,
            texture::address_mode_t::clamp_to_edge
        )
    });

    api::render_item_t<float, 2> render_item;
    render_item.geometry(std::move(geometry));
    render_item.material(std::move(material));
    render_item.translation({4.0F, 4.0F});
    render_item.scale({4.0F, 4.0F});
    return render_item;
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

    const api::camera_t<float, int, 2> camera({{0.0F, 8.0F}, {0.0F, 8.0F}}, {{0, 8}, {0, 8}});
    const auto render_item = make_render_item();
    test::expect_throws<std::invalid_argument>([&] { renderer.draw(camera, render_item); });
}

void test_draw() {
    std::vector<api::rgba8_t> pixels(8 * 8);
    api::software_renderer_t renderer({
        .pixels = pixels,
        .width = 8,
        .height = 8
    });
    const api::camera_t<float, int, 2> camera({{0.0F, 8.0F}, {0.0F, 8.0F}}, {{0, 8}, {0, 8}});
    const auto render_item = make_render_item();

    renderer.clear(clear_color);
    renderer.draw(camera, render_item);

    test::expect(std::identity(), std::ranges::all_of(pixels, [](const auto& pixel) {
        return same_color(pixel, texture_color);
    }));
}

} // namespace

int main() {
    return test::run([] {
        static_assert(sizeof(api::rgba8_t) == 4);
        test_framebuffer();
        test_empty_framebuffer();
        test_draw();
    });
}
