#include "../glfw.h"
#include "../input.h"
#include "../monitor.h"
#include "../window.h"
#include "../window_creation_settings.h"

#include <array>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;
namespace vector_api = m03ginwy24ng8o487c4beoms6l_vector;

namespace {

template <typename T, std::size_t N>
using vector_t = vector_api::vector_t<T, N>;

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::logic_error(std::string(message));
    }
}

void initialize_input_state(glfw_api::input_state_t& state) {
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::button_t::_button_count); ++index) {
        auto& button = state.button_state(static_cast<glfw_api::button_t>(index));
        button.transition_count() = 0;
        button.repeat_count() = 0;
        button.is_down() = false;
    }

    state.cursor_position() = {0.0, 0.0};
    state.scroll_offset() = {0.0, 0.0};
}

void initialize_gamepad_state(glfw_api::gamepad_state_t& state) {
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::gamepad_button_t::_button_count); ++index) {
        state.button_state(static_cast<glfw_api::gamepad_button_t>(index)) = false;
    }
    for (std::size_t index = 0; index < static_cast<std::size_t>(glfw_api::gamepad_axis_t::_axis_count); ++index) {
        state.axis_state(static_cast<glfw_api::gamepad_axis_t>(index)) = 0.0f;
    }
}

void run(std::string_view name, const std::function<void()>& test) {
    std::cout << std::format("running {}\n", name);
    test();
    std::cout << std::format("passed {}\n", name);
}

void test_settings_model() {
    glfw_api::window_creation_settings_t settings;

    require(settings.resizable(), "default resizable should be true");
    require(settings.visible(), "default visible should be true");
    require(settings.decorated(), "default decorated should be true");
    require(settings.focused(), "default focused should be true");
    require(!settings.maximized(), "default maximized should be false");
    require(settings.auto_minimize_on_focus_loss(), "default auto-minimize should be true");
    require(!settings.always_on_top(), "default always-on-top should be false");
    require(settings.center_cursor_in_fullscreen(), "default center cursor should be true");
    require(!settings.transparent_framebuffer(), "default transparent framebuffer should be false");
    require(settings.focus_on_show(), "default focus-on-show should be true");
    require(!settings.scale_to_monitor(), "default scale-to-monitor should be false");
    require(settings.scale_framebuffer(), "default scale-framebuffer should be true");
    require(!settings.mouse_passthrough(), "default mouse passthrough should be false");
    require(settings.color_bits_red() == 8 && settings.color_bits_green() == 8 && settings.color_bits_blue() == 8 && settings.color_bits_alpha() == 8, "default color bits should be 8:8:8:8");
    require(settings.depth_stencil_bits_depth() == 24 && settings.depth_stencil_bits_stencil() == 8, "default depth/stencil bits should be 24:8");
    require(settings.accumulation_bits_red() == 0 && settings.accumulation_bits_green() == 0 && settings.accumulation_bits_blue() == 0 && settings.accumulation_bits_alpha() == 0, "default accumulation bits should be zero");
    require(settings.auxiliary_buffers() == 0, "default auxiliary buffer count should be zero");
    require(settings.sample_count() == 0, "default sample count should be zero");
    require(!settings.stereo(), "default stereo should be false");
    require(!settings.srgb_capable(), "default sRGB capability should be false");
    require(settings.double_buffered(), "default double buffering should be true");
    require(settings.client_api() == glfw_api::client_api_t::opengl, "default client API should be OpenGL");
    require(settings.context_creation_api() == glfw_api::context_creation_api_t::native, "default context creation API should be native");
    require(settings.context_version_major() == 1 && settings.context_version_minor() == 0, "default context version should be OpenGL 1.0");
    require(settings.context_robustness() == glfw_api::context_robustness_t::none, "default context robustness should be none");
    require(settings.context_release_behavior() == glfw_api::context_release_behavior_t::any, "default context release behavior should be any");
    require(!settings.forward_compatible(), "default forward-compatible flag should be false");
    require(!settings.debug_context(), "default debug context should be false");
    require(settings.opengl_profile() == glfw_api::opengl_profile_t::any, "default OpenGL profile should be any");
    require(!settings.win32_keyboard_menu(), "default Win32 keyboard menu should be false");
    require(!settings.win32_show_default(), "default Win32 show-default should be false");
    require(settings.cocoa_frame_name().empty(), "default Cocoa frame name should be empty");
    require(!settings.cocoa_graphics_switching(), "default Cocoa graphics switching should be false");
    require(settings.wayland_application_id().empty(), "default Wayland application ID should be empty");
    require(settings.x11_class_name().empty(), "default X11 class name should be empty");
    require(settings.x11_instance_name().empty(), "default X11 instance name should be empty");

    settings
        .resizable(false)
        .visible(false)
        .decorated(false)
        .focused(false)
        .maximized(true)
        .auto_minimize_on_focus_loss(false)
        .always_on_top(true)
        .center_cursor_in_fullscreen(false)
        .transparent_framebuffer(true)
        .focus_on_show(false)
        .scale_to_monitor(true)
        .scale_framebuffer(false)
        .mouse_passthrough(true)
        .color_bits(5, 6, 5, 0)
        .depth_stencil_bits(16, 0)
        .accumulation_bits(1, 2, 3, 4)
        .auxiliary_buffers(1)
        .sample_count(4)
        .stereo(true)
        .srgb_capable(true)
        .double_buffered(false)
        .no_client_api()
        .opengl_es(2, 0)
        .opengl(3, 3, glfw_api::opengl_profile_t::core)
        .context_creation_api(glfw_api::context_creation_api_t::osmesa)
        .context_creation_api(glfw_api::context_creation_api_t::egl)
        .context_creation_api(glfw_api::context_creation_api_t::native)
        .context_robustness(glfw_api::context_robustness_t::no_reset_notification)
        .context_robustness(glfw_api::context_robustness_t::lose_context_on_reset)
        .context_robustness(glfw_api::context_robustness_t::none)
        .context_release_behavior(glfw_api::context_release_behavior_t::flush)
        .context_release_behavior(glfw_api::context_release_behavior_t::none)
        .context_release_behavior(glfw_api::context_release_behavior_t::any)
        .forward_compatible(true)
        .debug_context(true)
        .win32_keyboard_menu(true)
        .win32_show_default(true)
        .cocoa_frame_name("glfw-public-api-settings-test")
        .cocoa_graphics_switching(true)
        .wayland_application_id("glfw-public-api-settings-test")
        .x11_class_name("GlfwPublicApiSettingsTest")
        .x11_instance_name("glfw-public-api-settings-test");

    require(!settings.resizable(), "resizable setter failed");
    require(!settings.visible(), "visible setter failed");
    require(!settings.decorated(), "decorated setter failed");
    require(!settings.focused(), "focused setter failed");
    require(settings.maximized(), "maximized setter failed");
    require(!settings.auto_minimize_on_focus_loss(), "auto-minimize setter failed");
    require(settings.always_on_top(), "always-on-top setter failed");
    require(!settings.center_cursor_in_fullscreen(), "center cursor setter failed");
    require(settings.transparent_framebuffer(), "transparent framebuffer setter failed");
    require(!settings.focus_on_show(), "focus-on-show setter failed");
    require(settings.scale_to_monitor(), "scale-to-monitor setter failed");
    require(!settings.scale_framebuffer(), "scale-framebuffer setter failed");
    require(settings.mouse_passthrough(), "mouse passthrough setter failed");
    require(settings.color_bits_red() == 5 && settings.color_bits_green() == 6 && settings.color_bits_blue() == 5 && settings.color_bits_alpha() == 0, "color bits setter failed");
    require(settings.depth_stencil_bits_depth() == 16 && settings.depth_stencil_bits_stencil() == 0, "depth/stencil setter failed");
    require(settings.accumulation_bits_red() == 1 && settings.accumulation_bits_green() == 2 && settings.accumulation_bits_blue() == 3 && settings.accumulation_bits_alpha() == 4, "accumulation bits setter failed");
    require(settings.auxiliary_buffers() == 1, "auxiliary buffer setter failed");
    require(settings.sample_count() == 4, "sample count setter failed");
    require(settings.stereo(), "stereo setter failed");
    require(settings.srgb_capable(), "sRGB setter failed");
    require(!settings.double_buffered(), "double buffering setter failed");
    require(settings.client_api() == glfw_api::client_api_t::opengl, "OpenGL selector failed");
    require(settings.context_version_major() == 3 && settings.context_version_minor() == 3, "OpenGL version selector failed");
    require(settings.opengl_profile() == glfw_api::opengl_profile_t::core, "OpenGL profile selector failed");
    require(settings.context_creation_api() == glfw_api::context_creation_api_t::native, "context creation API setter failed");
    require(settings.context_robustness() == glfw_api::context_robustness_t::none, "context robustness setter failed");
    require(settings.context_release_behavior() == glfw_api::context_release_behavior_t::any, "context release behavior setter failed");
    require(settings.forward_compatible(), "forward-compatible setter failed");
    require(settings.debug_context(), "debug-context setter failed");
    require(settings.win32_keyboard_menu(), "Win32 keyboard menu setter failed");
    require(settings.win32_show_default(), "Win32 show-default setter failed");
    require(settings.cocoa_frame_name() == "glfw-public-api-settings-test", "Cocoa frame name setter failed");
    require(settings.cocoa_graphics_switching(), "Cocoa graphics switching setter failed");
    require(settings.wayland_application_id() == "glfw-public-api-settings-test", "Wayland application ID setter failed");
    require(settings.x11_class_name() == "GlfwPublicApiSettingsTest", "X11 class name setter failed");
    require(settings.x11_instance_name() == "glfw-public-api-settings-test", "X11 instance name setter failed");

    settings.reset();
    require(settings.resizable(), "reset should restore defaults");
    require(settings.client_api() == glfw_api::client_api_t::opengl, "reset should restore OpenGL client API");
    require(settings.context_version_major() == 1 && settings.context_version_minor() == 0, "reset should restore OpenGL 1.0");
}

void test_input_changes() {
    glfw_api::input_state_t previous_input;
    glfw_api::input_state_t current_input;
    initialize_input_state(previous_input);
    initialize_input_state(current_input);

    auto& current_key = current_input.button_state(glfw_api::button_t::button_a);
    current_key.transition_count() = 1;
    current_key.repeat_count() = 2;
    current_key.is_down() = true;
    current_input.cursor_position() = {3.0, 4.0};
    current_input.scroll_offset() = {1.0, -2.0};

    const glfw_api::input_state_change_t input_change(previous_input, current_input);
    require(input_change.was_pressed(glfw_api::button_t::button_a), "input press detection failed");
    require(!input_change.was_released(glfw_api::button_t::button_a), "input release detection should be false");
    require(input_change.press_delta(glfw_api::button_t::button_a) == 1, "input press delta failed");
    require(input_change.release_delta(glfw_api::button_t::button_a) == 0, "input release delta failed");
    require(input_change.repeat_delta(glfw_api::button_t::button_a) == 2, "input repeat delta failed");
    require(input_change.cursor_position_delta() == vector_t<double, 2>{3.0, 4.0}, "cursor delta failed");
    require(input_change.scroll_offset_delta() == vector_t<double, 2>{1.0, -2.0}, "scroll delta failed");

    glfw_api::joystick_state_t previous_joystick;
    previous_joystick.axes() = {0.0f};
    previous_joystick.buttons() = {false};
    previous_joystick.hats() = {{0, 0}};

    glfw_api::joystick_state_t current_joystick;
    current_joystick.axes() = {0.5f};
    current_joystick.buttons() = {true};
    current_joystick.hats() = {{1, 0}};

    const glfw_api::joystick_state_change_t joystick_change(previous_joystick, current_joystick);
    require(joystick_change.axis_count() == 1, "joystick axis count failed");
    require(joystick_change.button_count() == 1, "joystick button count failed");
    require(joystick_change.hat_count() == 1, "joystick hat count failed");
    require(joystick_change.axis_delta(0) == 0.5f, "joystick axis delta failed");
    require(joystick_change.was_pressed(0), "joystick button press failed");
    require(!joystick_change.was_released(0), "joystick button release should be false");
    require(joystick_change.hat_delta(0) == vector_t<int, 2>{1, 0}, "joystick hat delta failed");

    glfw_api::gamepad_state_t previous_gamepad;
    glfw_api::gamepad_state_t current_gamepad;
    initialize_gamepad_state(previous_gamepad);
    initialize_gamepad_state(current_gamepad);

    current_gamepad.button_state(glfw_api::gamepad_button_t::button_a) = true;
    current_gamepad.axis_state(glfw_api::gamepad_axis_t::axis_left_x) = 0.5f;

    const glfw_api::gamepad_state_change_t gamepad_change(previous_gamepad, current_gamepad);
    require(gamepad_change.was_pressed(glfw_api::gamepad_button_t::button_a), "gamepad button press failed");
    require(!gamepad_change.was_released(glfw_api::gamepad_button_t::button_a), "gamepad button release should be false");
    require(gamepad_change.axis_delta(glfw_api::gamepad_axis_t::axis_left_x) == 0.5f, "gamepad axis delta failed");
}

void test_mapping_model() {
    const glfw_api::joystick_to_gamepad_mapping_t mapping(
        "030000005e0400008e02000014010000,Public API Test Gamepad,"
        "a:b0,b:b1,leftx:a0,lefty:a1~,righttrigger:+a2,dpup:h0.1,platform:Linux,"
    );

    require(mapping.gamepad_guid() == "030000005e0400008e02000014010000", "mapping GUID parse failed");
    require(mapping.gamepad_name() == "Public API Test Gamepad", "mapping name parse failed");
    require(mapping.platform() == glfw_api::joystick_to_gamepad_platform_t::linux, "mapping platform parse failed");
    require(mapping.gamepad_button_by_joystick_button().size() == 2, "mapping button count failed");
    require(mapping.gamepad_axis_by_joystick_axis().size() == 3, "mapping axis count failed");
    require(mapping.gamepad_hat_by_joystick_hat().size() == 1, "mapping hat count failed");

    const auto serialized = mapping.gamepad_mapping();
    require(serialized.find("030000005e0400008e02000014010000,Public API Test Gamepad,") == 0, "mapping serialization header failed");
    require(serialized.find("platform:Linux,") != std::string::npos, "mapping serialization platform failed");
}

void exercise_runtime_devices() {
    const auto primary = glfw_api::primary_monitor();
    const auto monitor_list = glfw_api::monitors();
    if (primary) {
        const auto mode = primary->video_mode();
        require(0 < mode.width && 0 < mode.height, "primary monitor video mode should have positive size");
        (void)primary->name();
        (void)primary->video_modes();
        (void)primary->virtual_position();
        (void)primary->physical_size();
        (void)primary->content_scale();
        (void)primary->work_area();
    }
    for (const auto& monitor : monitor_list) {
        require(monitor != nullptr, "monitor list contains null monitor");
        (void)monitor->name();
        (void)monitor->video_mode();
        (void)monitor->video_modes();
        (void)monitor->virtual_position();
        (void)monitor->physical_size();
        (void)monitor->content_scale();
        (void)monitor->work_area();
    }

    for (const auto& joystick : glfw_api::joysticks()) {
        require(joystick != nullptr, "joystick list contains null joystick");
        glfw_api::poll_joystick(*joystick);
    }
    for (const auto& gamepad : glfw_api::gamepads()) {
        require(gamepad != nullptr, "gamepad list contains null gamepad");
        glfw_api::poll_gamepad(*gamepad);
    }

    glfw_api::update_joystick_to_gamepad_mapping(glfw_api::joystick_to_gamepad_mapping_t(
        "030000005e0400008e02000014010000,Public API Test Gamepad,a:b0,b:b1,leftx:a0,lefty:a1,"
    ));
}

void test_no_api_window() {
    glfw_api::window_creation_settings_t settings;
    settings
        .no_client_api()
        .visible(false)
        .decorated(true)
        .resizable(true)
        .focus_on_show(false);

    auto window = glfw_api::window_t::create(
        "GLFW public API smoke test",
        vector_t<int, 4>{100, 100, 640, 480},
        settings
    );
    require(window != nullptr, "no-client-API window creation failed");

    require(window->client_api() == glfw_api::client_api_t::none, "window client API should be none");
    require(window->windowed(), "window should be windowed");
    require(!window->fullscreen(), "window should not be fullscreen");
    require(window->title() == "GLFW public API smoke test", "window title getter failed");
    require(!window->should_close(), "new window should not request close");
    require(!window->visible(), "hidden creation setting failed");
    require(!window->context_current(true), "no-client-API window should not make a context current");
    require(!window->extension_supported("GL_ARB_vertex_array_object"), "no-client-API window should report no extension support");

    window->size_limits(320, 240, 1280, 960);
    window->aspect_ratio({16, 9});
    window->clear_aspect_ratio();
    window->size_limits(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    int size_callback_count = 0;
    window->size_callback([&size_callback_count](glfw_api::window_t*, int, int) {
        ++size_callback_count;
    });
    require(static_cast<bool>(window->size_callback()), "size callback setter failed");
    window->size({800, 600});
    glfw_api::poll_events();
    (void)size_callback_count;

    window->title("Renamed GLFW public API smoke test");
    require(window->title() == "Renamed GLFW public API smoke test", "window title setter failed");
    window->focus_on_visible(false);
    require(!window->focus_on_visible(), "focus-on-visible setter failed");

    window->cursor_mode(true, false);
    require(window->cursor_visible(), "cursor visible mode failed");
    require(!window->cursor_locked(), "cursor unlocked mode failed");
    window->cursor_raw_motion(false);
    require(!window->cursor_raw_motion(), "cursor raw motion should be disabled");

    std::array<unsigned char, 16> image_pixels = {
        255, 0, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        255, 255, 255, 255
    };
    glfw_api::image_t image {
        .data = image_pixels.data(),
        .width = 2,
        .height = 2
    };
    window->cursor_image(image, {0, 0});
    window->reset_cursor_image();
    std::array<glfw_api::image_t, 1> icons = { image };
    window->icon(std::span<const glfw_api::image_t>(icons.data(), icons.size()));
    window->icon(std::span<const glfw_api::image_t>());

    window->visible(true);
    glfw_api::poll_events();
    window->visible(false);
    glfw_api::poll_events();

    window->should_close(true);
    require(window->should_close(), "window should-close setter failed");
}

void test_opengl_context() {
    glfw_api::window_creation_settings_t settings;
    settings
        .opengl(3, 3, glfw_api::opengl_profile_t::core)
        .visible(false)
        .resizable(true);

    auto window = glfw_api::window_t::create(
        "GLFW public API context test",
        vector_t<int, 4>{100, 100, 640, 480},
        settings
    );
    if (!window) {
        std::cout << "skipped OpenGL context test: window creation failed\n";
        return;
    }

    require(window->client_api() == glfw_api::client_api_t::opengl, "OpenGL window client API failed");
    require(window->context_current(true), "OpenGL context could not be made current");
    require(glfw_api::get_proc_address("glClear") != nullptr, "glClear procedure lookup failed");
    (void)window->extension_supported("GL_ARB_vertex_array_object");
    window->swap_interval(1);
    window->swap_buffers();
    require(window->context_current(false) == false, "OpenGL context detach failed");
}

void test_runtime() {
    glfw_api::glfw_t glfw;

    glfw_api::post_empty_event();
    glfw_api::poll_events();
    glfw_api::wait_events_timeout(0.0);

    exercise_runtime_devices();
    test_no_api_window();
    test_opengl_context();
}

} // namespace

int main() {
    try {
        run("settings model", test_settings_model);
        run("input changes", test_input_changes);
        run("mapping model", test_mapping_model);
        run("runtime API", test_runtime);
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << std::format("error: {}\n", exception.what());
        return 1;
    }
}
