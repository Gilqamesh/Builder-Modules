#include "software_renderer.h"

#include <m03gkcdy62bnz808pmk4uzkjra_glfw/glfw.h>
#include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
#include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;
namespace opengl_renderer_api = m03gl22hn0dqmosreqjie9tg5m_opengl_renderer;
namespace software_renderer_api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;

using steady_clock_t = std::chrono::steady_clock;
using rgba8_t = software_renderer_api::rgba8_t;

std::uint8_t color_channel(float value) {
    const float clamped = std::clamp(value, 0.0f, 1.0f);
    return static_cast<std::uint8_t>(clamped * 255.0f + 0.5f);
}

rgba8_t shade_pixel(int x, int y, int width, int height, float seconds) {
    const float fx = (static_cast<float>(x) + 0.5f) / static_cast<float>(width);
    const float fy = (static_cast<float>(y) + 0.5f) / static_cast<float>(height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    const float centered_x = (fx * 2.0f - 1.0f) * aspect;
    const float centered_y = fy * 2.0f - 1.0f;

    const float distance = std::sqrt(centered_x * centered_x + centered_y * centered_y);
    const float wave = 0.5f + 0.5f * std::sin(18.0f * distance - seconds * 4.0f);
    const bool checker = ((x / 32) + (y / 32)) % 2 == 0;

    const float red = checker ? 0.12f + wave * 0.55f : 0.08f + fx * 0.35f;
    const float green = 0.18f + fy * 0.55f;
    const float blue = checker ? 0.38f + fx * 0.45f : 0.20f + wave * 0.50f;

    return {
        .red = color_channel(red),
        .green = color_channel(green),
        .blue = color_channel(blue),
        .alpha = 255
    };
}

void render_frame(std::span<rgba8_t> pixels, int width, int height, float seconds) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
                + static_cast<std::size_t>(x);
            pixels[index] = shade_pixel(x, y, width, height, seconds);
        }
    }
}

std::size_t framebuffer_pixel_count(int width, int height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("software_renderer CLI framebuffer dimensions must be non-negative");
    }
    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);
    if (width_size != 0 && height_size > std::numeric_limits<std::size_t>::max() / width_size) {
        throw std::length_error("software_renderer CLI framebuffer size overflows size_t");
    }
    return width_size * height_size;
}

} // namespace

int main() {
    try {
        glfw_api::glfw_t glfw;

        glfw_api::window_creation_settings_t settings;
        settings.opengl(3, 3, glfw_api::opengl_profile_t::core);

        auto window = glfw_api::window_t::create("Software Renderer", {300, 200, 960, 540}, settings);
        if (!window) {
            throw std::runtime_error("software_renderer CLI failed to create its window");
        }
        window->swap_interval(1);

        std::vector<rgba8_t> pixels;
        software_renderer_api::software_renderer_t renderer({
            .pixels = pixels,
            .width = 0,
            .height = 0
        });
        opengl_renderer_api::opengl_renderer_t opengl_renderer(window);

        const auto started_at = steady_clock_t::now();
        auto previous_frame_started_at = started_at;

        while (!window->should_close()) {
            const auto frame_started_at = steady_clock_t::now();
            const auto seconds = std::chrono::duration<float>(frame_started_at - started_at).count();

            glfw_api::poll_events();

            const auto size = window->framebuffer_size();
            auto framebuffer = renderer.framebuffer();
            if (framebuffer.width != size[0] || framebuffer.height != size[1]) {
                pixels.resize(framebuffer_pixel_count(size[0], size[1]));
                renderer.framebuffer({
                    .pixels = pixels,
                    .width = size[0],
                    .height = size[1]
                });
                framebuffer = renderer.framebuffer();
            }

            if (framebuffer.width == 0 || framebuffer.height == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            } else {
                renderer.clear({0, 0, 0, 255});
                render_frame(pixels, framebuffer.width, framebuffer.height, seconds);
                opengl_renderer.present_rgba8(
                    std::as_bytes(std::span<const rgba8_t>(pixels)),
                    framebuffer.width,
                    framebuffer.height
                );
            }

            const auto frame_time = std::chrono::duration<double, std::milli>(frame_started_at - previous_frame_started_at);
            previous_frame_started_at = frame_started_at;
            std::cout << std::format("frame: {:.2f} ms, framebuffer: {}x{}\n", frame_time.count(), framebuffer.width, framebuffer.height);
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << std::format("software_renderer: {}\n", error.what());
        return 1;
    }
}
