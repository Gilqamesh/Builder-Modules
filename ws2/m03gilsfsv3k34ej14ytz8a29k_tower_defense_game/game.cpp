#include "game.h"

#include <random>
#include <format>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <thread>

namespace {

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

}

namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game {

game_t::game_t():
    m_camera({{-400, 400}, {-300, 300}}, {{0, 400}, {0, 200}})
{
    std::vector<std::shared_ptr<texture_t>> tile_textures;
    std::vector<std::string> tile_texture_paths = {
        "assets/grass.png",
        "assets/road_four_way.png",
        "assets/road_straight.png",
        "assets/road_three_way.png",
        "assets/road_turn.png"
    };
    for (const auto& tile_texture_path : tile_texture_paths) {
        int tile_texture_width;
        int tile_texture_height;
        int tile_texture_channels;
        unsigned char* tile_texture_data = stbi_load(tile_texture_path.c_str(), &tile_texture_width, &tile_texture_height, &tile_texture_channels, 4);
        m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t tile_texture_bytes(std::span<const std::byte>(reinterpret_cast<const std::byte*>(tile_texture_data), tile_texture_width * tile_texture_height * 4));
        std::shared_ptr<texture_t> tile_texture = std::make_shared<texture_t>(texture_format_t::RGBA_U8_NORMALIZED, tile_texture_width, tile_texture_height, std::move(tile_texture_bytes));
        tile_textures.push_back(tile_texture);
    }
    std::shared_ptr<sampler_t> tile_sampler = std::make_shared<sampler_t>();
    std::shared_ptr<shader_t> tile_shader = std::make_shared<shader_t>();
    std::vector<std::shared_ptr<material_t>> materials;
    for (const auto& tile_texture : tile_textures) {
        std::shared_ptr<material_t> material = std::make_shared<material_t>();
        material->shader(tile_shader);
        texture_binding_t texture_binding = {tile_texture, tile_sampler};
        material->set_texture_binding(0, texture_binding);
        materials.push_back(material);
    }

    std::vector<vertex_attribute_t> vertex_attributes = {
        { vertex_attribute_type_t::R32, 2 }
    };

    std::vector<std::shared_ptr<mesh_t>> meshes;
    const auto number_of_meshes = 10;
    for (size_t i = 0; i < number_of_meshes; ++i) {
        const auto minimum_number_of_vertices = 3;
        const auto maximum_number_of_vertices = 15;
        const auto number_of_vertices = minimum_number_of_vertices + rand() % (maximum_number_of_vertices - minimum_number_of_vertices + 1);
        m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<std::array<float, 2>> vertex_streams;

        if (number_of_vertices == 1) {
            vertex_streams.push_back({0.0f, 0.0f});
        } else {
            const auto rotation = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.0f * std::numbers::pi_v<float>;
            const auto angle_step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(number_of_vertices);

            for (size_t j = 0; j < number_of_vertices; ++j) {
                const auto angle = rotation - static_cast<float>(j) * angle_step;
                const auto x = std::cos(angle);
                const auto y = std::sin(angle);
                vertex_streams.push_back({x, y});
            }
        }

        mesh_t mesh(std::move(vertex_streams), vertex_attributes);
        meshes.push_back(std::make_shared<mesh_t>(std::move(mesh)));
    }

    const auto number_of_entities = 1000;
    for (size_t i = 0; i < number_of_entities; ++i) {
        entity_t<float, 2> entity;

        const auto mesh_index = rand() % meshes.size();
        std::shared_ptr<mesh_t> mesh = meshes[mesh_index];
        entity.mesh(mesh);

        const auto number_of_vertices = mesh->number_of_vertices();

        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < number_of_vertices; ++i) {
            indices.push_back(i);
        }
        entity.indices(std::move(indices));

        entity.primitive_topology(vertex_primitive_topology_t::triangle_fan);
        // entity.primitive_topology(vertex_primitive_topology_t::triangle_strip);
        // entity.primitive_topology(vertex_primitive_topology_t::triangle);
        // entity.primitive_topology(vertex_primitive_topology_t::line_loop);
        // entity.primitive_topology(vertex_primitive_topology_t::line_strip);
        // entity.primitive_topology(vertex_primitive_topology_t::line);
        // entity.primitive_topology(vertex_primitive_topology_t::point);

        const auto material_index = rand() % materials.size();
        std::shared_ptr<material_t> material = materials[material_index];
        entity.material(material);

        const auto max_horizontal_translation = 5000;
        const auto max_vertical_translation = 3000;
        m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2> translation = {
            static_cast<float>(rand() % max_horizontal_translation - max_horizontal_translation / 2),
            static_cast<float>(rand() % max_vertical_translation - max_vertical_translation / 2)
        };
        entity.translation(translation);

        entity.rotation({0, 0});

        const auto max_horizontal_scale = 30;
        const auto max_vertical_scale = 20;
        m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2> scale = {
            static_cast<float>(rand() % max_horizontal_scale + 1),
            static_cast<float>(rand() % max_vertical_scale + 1)
        };
        entity.scale(scale);

        entity.finalize();

        m_entities.push_back(std::move(entity));
    }

    // create window with opengl core profile and renderer
    m03gkcdy62bnz808pmk4uzkjra_glfw::window_creation_settings_t window_settings;
    window_settings.opengl(4, 6, m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t::core);
    m_window = m03gkcdy62bnz808pmk4uzkjra_glfw::window_t::create("Tower Defense Game", m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4>{100, 100, 1600, 1200}, window_settings);
    m_renderer = std::make_shared<renderer3_t>(m_window);
}

game_t::~game_t() {
}

void game_t::run() {
    m_window->swap_interval(1);

    auto previous_frame_time = std::chrono::steady_clock::now();
    while (!m_window->should_close()) {
        const auto frame_time = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration<float>(frame_time - previous_frame_time).count();

        m03gkcdy62bnz808pmk4uzkjra_glfw::poll_events();

        update(dt);
        render();

        const auto frame_time_ms = std::chrono::duration<double, std::milli>(frame_time - previous_frame_time);
        previous_frame_time = frame_time;
        std::cout << std::format("frame: {:.2f} ms, framebuffer: {}x{}\n", frame_time_ms.count(), m_renderer->width(), m_renderer->height());
    }
}

void game_t::update(float dt) {
    auto camera_view_dp = m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>{0, 0};
    auto camera_world_dp = m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>{0, 0};
    auto camera_view_lengths_dp = m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>{0, 0};
    auto camera_world_lengths_dp = m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>{0, 0};

    const auto camera_world_rect = m_camera.world_rect();
    const auto& camera_world_rect_bounds = camera_world_rect.bounds();
    const auto camera_world_rect_horizontal_length = camera_world_rect_bounds[0].length();
    const auto camera_world_rect_vertical_length = camera_world_rect_bounds[1].length();
    const auto camera_world_horizontal_speed = std::max(1.0f, camera_world_rect_horizontal_length * dt);
    const auto camera_world_vertical_speed = std::max(1.0f, camera_world_rect_vertical_length * dt);

    const auto camera_view_rect = m_camera.view_rect();
    const auto camera_view_rect_bounds = camera_view_rect.bounds();
    const auto camera_view_rect_horizontal_length = camera_view_rect_bounds[0].length();
    const auto camera_view_rect_vertical_length = camera_view_rect_bounds[1].length();
    const auto camera_view_horizontal_speed = std::max(1, static_cast<int>(camera_view_rect_horizontal_length * dt));
    const auto camera_view_vertical_speed = std::max(1, static_cast<int>(camera_view_rect_vertical_length * dt));

    // if (IsKeyDown(KEY_W)) {
    //     camera_world_dp[1] -= camera_world_vertical_speed;
    // }
    // if (IsKeyDown(KEY_S)) {
    //     camera_world_dp[1] += camera_world_vertical_speed;
    // }
    // if (IsKeyDown(KEY_A)) {
    //     camera_world_dp[0] -= camera_world_horizontal_speed;
    // }
    // if (IsKeyDown(KEY_D)) {
    //     camera_world_dp[0] += camera_world_horizontal_speed;
    // }

    // if (IsKeyDown(KEY_UP)) {
    //     camera_view_dp[1] -= camera_view_vertical_speed;
    // }
    // if (IsKeyDown(KEY_DOWN)) {
    //     camera_view_dp[1] += camera_view_vertical_speed;
    // }
    // if (IsKeyDown(KEY_LEFT)) {
    //     camera_view_dp[0] -= camera_view_horizontal_speed;
    // }
    // if (IsKeyDown(KEY_RIGHT)) {
    //     camera_view_dp[0] += camera_view_horizontal_speed;
    // }

    // if (IsKeyDown(KEY_Q)) {
    //     camera_world_lengths_dp += m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>{std::max(1.0f, camera_world_rect_horizontal_length * dt), std::max(1.0f, camera_world_rect_vertical_length * dt)};
    // }
    // if (IsKeyDown(KEY_E)) {
    //     camera_world_lengths_dp -= m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>{std::max(1.0f, camera_world_rect_horizontal_length * dt), std::max(1.0f, camera_world_rect_vertical_length * dt)};
    // }
    // if (IsKeyDown(KEY_F)) {
    //     camera_view_lengths_dp += m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>{std::max(1, static_cast<int>(camera_view_rect_horizontal_length * dt)), std::max(1, static_cast<int>(camera_view_rect_vertical_length * dt))};
    // }
    // if (IsKeyDown(KEY_R)) {
    //     camera_view_lengths_dp -= m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>{std::max(1, static_cast<int>(camera_view_rect_horizontal_length * dt)), std::max(1, static_cast<int>(camera_view_rect_vertical_length * dt))};
    // }

    m_camera.world_rect() += camera_world_dp;
    m_camera.world_rect() = m_camera.world_rect().inflate(camera_world_lengths_dp);
    m_camera.view_rect() += camera_view_dp;
    m_camera.view_rect() = m_camera.view_rect().inflate(camera_view_lengths_dp);
}

void game_t::render() {
    // ClearBackground(RAYWHITE);

    // const auto camera_view_rect = m_camera.view_rect();
    // const auto camera_view_top_left = camera_view_rect.corner();
    // const auto camera_view_rect_bounds = camera_view_rect.bounds();
    // const auto camera_view_rect_horizontal_length = camera_view_rect_bounds[0].length();
    // const auto camera_view_rect_vertical_length = camera_view_rect_bounds[1].length();
    // DrawRectangleLines(camera_view_top_left[0], camera_view_top_left[1], camera_view_rect_horizontal_length, camera_view_rect_vertical_length, RED);

    if (m_renderer->begin_frame()) {
        for (const auto& entity : m_entities) {
            m_renderer->draw(m_camera, entity);
        }

        m_renderer->present();
    }
}

} // namespace m03gilsfsv3k34ej14ytz8a29k_tower_defense_game
