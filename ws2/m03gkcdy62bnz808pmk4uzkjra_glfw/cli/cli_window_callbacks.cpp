#include "cli_application.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <syncstream>
#include <utility>
#include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

std::string_view action_name(int action) {
    switch (action) {
    case GLFW_PRESS: return "press";
    case GLFW_RELEASE: return "release";
    case GLFW_REPEAT: return "repeat";
    default: return "unknown";
    }
}

void print_event(std::string message) {
    std::osyncstream(std::cout) << std::move(message) << '\n';
}

} // namespace

void application_t::install_window_callbacks(id_t id, glfw_api::window_t& window) {
    window.position_callback([id](glfw_api::window_t*, int x, int y) {
        print_event(std::format("[event window {}] position: {}, {}", id, x, y));
    });

    window.size_callback([id](glfw_api::window_t*, int width, int height) {
        print_event(std::format("[event window {}] size: {}x{}", id, width, height));
    });

    window.close_callback([id](glfw_api::window_t*) {
        print_event(std::format("[event window {}] close requested", id));
    });

    window.refresh_callback([id](glfw_api::window_t*) {
        print_event(std::format("[event window {}] refresh", id));
    });

    window.focus_callback([id](glfw_api::window_t*, bool focused) {
        print_event(std::format("[event window {}] focused: {}", id, focused));
    });

    window.iconify_callback([id](glfw_api::window_t*, bool iconified) {
        print_event(std::format("[event window {}] iconified: {}", id, iconified));
    });

    window.maximize_callback([id](glfw_api::window_t*, bool maximized) {
        print_event(std::format("[event window {}] maximized: {}", id, maximized));
    });

    window.framebuffer_size_callback([id](glfw_api::window_t*, int width, int height) {
        print_event(std::format("[event window {}] framebuffer size: {}x{}", id, width, height));
    });

    window.content_scale_callback([id](glfw_api::window_t*, float xscale, float yscale) {
        print_event(std::format("[event window {}] content scale: {}, {}", id, xscale, yscale));
    });

    window.mouse_button_callback([id](glfw_api::window_t*, int button, int action, int mods) {
        print_event(std::format(
            "[event window {}] mouse button: button={}, action={} ({}), mods={}",
            id,
            button,
            action_name(action),
            action,
            mods
        ));
    });

    window.cursor_position_callback([id](glfw_api::window_t*, double x, double y) {
        print_event(std::format("[event window {}] cursor position: {}, {}", id, x, y));
    });

    window.cursor_enter_callback([id](glfw_api::window_t*, bool entered) {
        print_event(std::format("[event window {}] cursor entered: {}", id, entered));
    });

    window.scroll_callback([id](glfw_api::window_t*, double xoffset, double yoffset) {
        print_event(std::format("[event window {}] scroll: {}, {}", id, xoffset, yoffset));
    });

    window.key_callback([id](glfw_api::window_t*, int key, int scancode, int action, int mods) {
        print_event(std::format(
            "[event window {}] key: key={}, scancode={}, action={} ({}), mods={}",
            id,
            key,
            scancode,
            action_name(action),
            action,
            mods
        ));
    });

    window.char_callback([id](glfw_api::window_t*, std::uint32_t codepoint) {
        print_event(std::format("[event window {}] char: U+{:04X}", id, codepoint));
    });

    window.drop_callback([id](glfw_api::window_t*, const std::vector<std::string>& paths) {
        std::string message = std::format("[event window {}] drop:", id);
        for (const std::string& path : paths) {
            message += std::format(" {}", quote_token(path));
        }
        print_event(std::move(message));
    });
}

void application_t::clear_window_callbacks(glfw_api::window_t& window) {
    window.position_callback({});
    window.size_callback({});
    window.close_callback({});
    window.refresh_callback({});
    window.focus_callback({});
    window.iconify_callback({});
    window.maximize_callback({});
    window.framebuffer_size_callback({});
    window.content_scale_callback({});
    window.mouse_button_callback({});
    window.cursor_position_callback({});
    window.cursor_enter_callback({});
    window.scroll_callback({});
    window.key_callback({});
    window.char_callback({});
    window.drop_callback({});
}

void application_t::print_callback_status(const glfw_api::window_t& window) {
    std::cout << std::format(
        "callbacks: position={}, size={}, close={}, refresh={}, focus={}, "
        "iconify={}, maximize={}, framebuffer-size={}, content-scale={}, "
        "mouse-button={}, cursor-position={}, cursor-enter={}, scroll={}, "
        "key={}, char={}, drop={}\n",
        static_cast<bool>(window.position_callback()),
        static_cast<bool>(window.size_callback()),
        static_cast<bool>(window.close_callback()),
        static_cast<bool>(window.refresh_callback()),
        static_cast<bool>(window.focus_callback()),
        static_cast<bool>(window.iconify_callback()),
        static_cast<bool>(window.maximize_callback()),
        static_cast<bool>(window.framebuffer_size_callback()),
        static_cast<bool>(window.content_scale_callback()),
        static_cast<bool>(window.mouse_button_callback()),
        static_cast<bool>(window.cursor_position_callback()),
        static_cast<bool>(window.cursor_enter_callback()),
        static_cast<bool>(window.scroll_callback()),
        static_cast<bool>(window.key_callback()),
        static_cast<bool>(window.char_callback()),
        static_cast<bool>(window.drop_callback())
    );
}

void application_t::print_window_status(id_t id, glfw_api::window_t& window) {
    std::cout << std::format("window {}: {}\n", id, window);
    std::cout << std::format(
        "  handle={}, client_api={}, framebuffer_size={}, fullscreen={}, "
        "windowed={}, maximized={}, minimized={}, focused={}, opacity={}, "
        "transparent_framebuffer={}\n",
        static_cast<void*>(window.handle()),
        window.client_api(),
        window.framebuffer_size(),
        window.fullscreen(),
        window.windowed(),
        window.maximized(),
        window.minimized(),
        window.focused(),
        window.opacity(),
        window.transparent_framebuffer()
    );
    std::cout << std::format(
        "  cursor_visible={}, cursor_locked={}, cursor_raw_motion={}, cursor_hovered={}\n",
        window.cursor_visible(),
        window.cursor_locked(),
        window.cursor_raw_motion(),
        window.cursor_is_in_content_area()
    );

    const auto& input_history = window.input_states();
    if (input_history.history_size() != 0) {
        std::cout << std::format(
            "  input history[0]: {}\n",
            input_history.history(0)
        );
    } else {
        std::cout << "  input: no committed samples\n";
    }

    std::cout << "  ";
    print_callback_status(window);
}

std::vector<unsigned char> application_t::make_test_pixels(int size) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(size) * static_cast<std::size_t>(size) * 4);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const bool alternate = ((x / std::max(1, size / 4)) + (y / std::max(1, size / 4))) % 2 != 0;
            const std::size_t offset = static_cast<std::size_t>(y * size + x) * 4;

            pixels[offset + 0] = alternate ? 235 : 35;
            pixels[offset + 1] = alternate ? 90 : 190;
            pixels[offset + 2] = alternate ? 45 : 235;
            pixels[offset + 3] = 255;
        }
    }

    return pixels;
}

void application_t::set_test_icons(glfw_api::window_t& window) {
    constexpr std::array<int, 3> sizes{16, 32, 48};
    std::array<std::vector<unsigned char>, sizes.size()> pixel_storage;
    std::array<glfw_api::image_t, sizes.size()> images;

    for (std::size_t index = 0; index < sizes.size(); ++index) {
        pixel_storage[index] = make_test_pixels(sizes[index]);
        images[index] = glfw_api::image_t{
            .data = pixel_storage[index].data(),
            .width = sizes[index],
            .height = sizes[index]
        };
    }

    window.icon(std::span<const glfw_api::image_t>(images));
}

void application_t::set_test_cursor(glfw_api::window_t& window) {
    constexpr int size = 32;
    std::vector<unsigned char> pixel_storage = make_test_pixels(size);
    const glfw_api::image_t image{
        .data = pixel_storage.data(),
        .width = size,
        .height = size
    };

    window.cursor_image(image, {0, 0});
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
