#include "window_creation_settings.h"

#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>

#include <m03gkcdy62bnz808pmk4uzkjra_glfw/GLFW/glfw3.h>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

namespace {

int to_glfw(bool value) noexcept {
    return value ? GLFW_TRUE : GLFW_FALSE;
}

int to_glfw_preference(int value) noexcept {
    return value == window_creation_settings_t::no_preference ? GLFW_DONT_CARE : value;
}

bool version_at_least(int major, int minor, int required_major, int required_minor) noexcept {
    return required_major < major || (major == required_major && required_minor <= minor);
}

int to_glfw(context_creation_api_t value) {
    switch (value) {
    case context_creation_api_t::native: return GLFW_NATIVE_CONTEXT_API;
    case context_creation_api_t::egl: return GLFW_EGL_CONTEXT_API;
    case context_creation_api_t::osmesa: return GLFW_OSMESA_CONTEXT_API;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid context creation API value {}", static_cast<int>(value)));
    }
}

int to_glfw(context_robustness_t value) {
    switch (value) {
    case context_robustness_t::none: return GLFW_NO_ROBUSTNESS;
    case context_robustness_t::no_reset_notification: return GLFW_NO_RESET_NOTIFICATION;
    case context_robustness_t::lose_context_on_reset: return GLFW_LOSE_CONTEXT_ON_RESET;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid context robustness value {}", static_cast<int>(value)));
    }
}

int to_glfw(context_release_behavior_t value) {
    switch (value) {
    case context_release_behavior_t::any: return GLFW_ANY_RELEASE_BEHAVIOR;
    case context_release_behavior_t::flush: return GLFW_RELEASE_BEHAVIOR_FLUSH;
    case context_release_behavior_t::none: return GLFW_RELEASE_BEHAVIOR_NONE;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid context release behavior value {}", static_cast<int>(value)));
    }
}

int to_glfw(opengl_profile_t value) {
    switch (value) {
    case opengl_profile_t::any: return GLFW_OPENGL_ANY_PROFILE;
    case opengl_profile_t::compatibility: return GLFW_OPENGL_COMPAT_PROFILE;
    case opengl_profile_t::core: return GLFW_OPENGL_CORE_PROFILE;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid OpenGL profile value {}", static_cast<int>(value)));
    }
}

} // namespace

window_creation_settings_t::window_creation_settings_t() {
    reset();
}

window_creation_settings_t& window_creation_settings_t::reset() {
    m_resizable = true;
    m_visible = true;
    m_decorated = true;
    m_focused = true;
    m_maximized = false;

    m_auto_minimize_on_focus_loss = true;
    m_always_on_top = false;
    m_center_cursor_in_fullscreen = true;
    m_transparent_framebuffer = false;
    m_focus_on_show = true;
    m_scale_to_monitor = false;
    m_scale_framebuffer = true;
    m_mouse_passthrough = false;

    m_red_bits = 8;
    m_green_bits = 8;
    m_blue_bits = 8;
    m_alpha_bits = 8;

    m_depth_bits = 24;
    m_stencil_bits = 8;

    m_accumulation_red_bits = 0;
    m_accumulation_green_bits = 0;
    m_accumulation_blue_bits = 0;
    m_accumulation_alpha_bits = 0;

    m_auxiliary_buffers = 0;
    m_sample_count = 0;

    m_stereo = false;
    m_srgb_capable = false;
    m_double_buffered = true;

    m_client_api = client_api_t::opengl;
    m_context_creation_api = context_creation_api_t::native;

    m_context_version_major = 1;
    m_context_version_minor = 0;

    m_context_robustness = context_robustness_t::none;
    m_context_release_behavior = context_release_behavior_t::any;

    m_forward_compatible = false;
    m_debug_context = false;

    m_opengl_profile = opengl_profile_t::any;

    m_win32_keyboard_menu = false;
    m_win32_show_default = false;

    m_cocoa_frame_name.clear();
    m_cocoa_graphics_switching = false;

    m_wayland_application_id.clear();

    m_x11_class_name.clear();
    m_x11_instance_name.clear();

    return *this;
}

void window_creation_settings_t::apply() const {
    validate();

    int glfw_client_api = GLFW_NO_API;

    switch (m_client_api) {
    case client_api_t::none: {
        glfw_client_api = GLFW_NO_API;
    } break ;
    case client_api_t::opengl: {
        glfw_client_api = GLFW_OPENGL_API;
    } break ;
    case client_api_t::opengl_es: {
        glfw_client_api = GLFW_OPENGL_ES_API;
    } break ;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid client API value {}", static_cast<int>(m_client_api)));
    }

    const int glfw_context_creation_api = to_glfw(m_context_creation_api);
    const int glfw_context_robustness = to_glfw(m_context_robustness);
    const int glfw_context_release_behavior = to_glfw(m_context_release_behavior);
    const int glfw_opengl_profile = to_glfw(m_opengl_profile);

    glfwDefaultWindowHints();

    glfwWindowHint(GLFW_RESIZABLE, to_glfw(m_resizable));
    glfwWindowHint(GLFW_VISIBLE, to_glfw(m_visible));
    glfwWindowHint(GLFW_DECORATED, to_glfw(m_decorated));
    glfwWindowHint(GLFW_FOCUSED, to_glfw(m_focused));
    glfwWindowHint(GLFW_MAXIMIZED, to_glfw(m_maximized));
    glfwWindowHint(GLFW_AUTO_ICONIFY, to_glfw(m_auto_minimize_on_focus_loss));
    glfwWindowHint(GLFW_FLOATING, to_glfw(m_always_on_top));
    glfwWindowHint(GLFW_CENTER_CURSOR, to_glfw(m_center_cursor_in_fullscreen));
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, to_glfw(m_transparent_framebuffer));
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, to_glfw(m_focus_on_show));
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, to_glfw(m_scale_to_monitor));
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, to_glfw(m_scale_framebuffer));
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, to_glfw(m_mouse_passthrough));

    glfwWindowHint(GLFW_RED_BITS, to_glfw_preference(m_red_bits));
    glfwWindowHint(GLFW_GREEN_BITS, to_glfw_preference(m_green_bits));
    glfwWindowHint(GLFW_BLUE_BITS, to_glfw_preference(m_blue_bits));
    glfwWindowHint(GLFW_ALPHA_BITS, to_glfw_preference(m_alpha_bits));
    glfwWindowHint(GLFW_DEPTH_BITS, to_glfw_preference(m_depth_bits));
    glfwWindowHint(GLFW_STENCIL_BITS, to_glfw_preference(m_stencil_bits));
    glfwWindowHint(GLFW_ACCUM_RED_BITS, to_glfw_preference(m_accumulation_red_bits));
    glfwWindowHint(GLFW_ACCUM_GREEN_BITS, to_glfw_preference(m_accumulation_green_bits));
    glfwWindowHint(GLFW_ACCUM_BLUE_BITS, to_glfw_preference(m_accumulation_blue_bits));
    glfwWindowHint(GLFW_ACCUM_ALPHA_BITS, to_glfw_preference(m_accumulation_alpha_bits));
    glfwWindowHint(GLFW_AUX_BUFFERS, to_glfw_preference(m_auxiliary_buffers));
    glfwWindowHint(GLFW_SAMPLES, to_glfw_preference(m_sample_count));
    glfwWindowHint(GLFW_STEREO, to_glfw(m_stereo));
    glfwWindowHint(GLFW_SRGB_CAPABLE, to_glfw(m_srgb_capable));
    glfwWindowHint(GLFW_DOUBLEBUFFER, to_glfw(m_double_buffered));

    glfwWindowHint(GLFW_CLIENT_API, glfw_client_api);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, glfw_context_creation_api);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_context_version_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_context_version_minor);
    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, glfw_context_robustness);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, glfw_context_release_behavior);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, to_glfw(m_forward_compatible));
    glfwWindowHint(GLFW_CONTEXT_DEBUG, to_glfw(m_debug_context));
    glfwWindowHint(GLFW_OPENGL_PROFILE, glfw_opengl_profile);

    glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, to_glfw(m_win32_keyboard_menu));
    glfwWindowHint(GLFW_WIN32_SHOWDEFAULT, to_glfw(m_win32_show_default));
    glfwWindowHintString(GLFW_COCOA_FRAME_NAME, m_cocoa_frame_name.c_str());
    glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, to_glfw(m_cocoa_graphics_switching));
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, m_wayland_application_id.c_str());
    glfwWindowHintString(GLFW_X11_CLASS_NAME, m_x11_class_name.c_str());
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, m_x11_instance_name.c_str());
}

window_creation_settings_t& window_creation_settings_t::resizable(bool value) {
    m_resizable = value;
    return *this;
}

bool window_creation_settings_t::resizable() const {
    return m_resizable;
}

window_creation_settings_t& window_creation_settings_t::visible(bool value) {
    m_visible = value;
    return *this;
}

bool window_creation_settings_t::visible() const {
    return m_visible;
}

window_creation_settings_t& window_creation_settings_t::decorated(bool value) {
    m_decorated = value;
    return *this;
}

bool window_creation_settings_t::decorated() const {
    return m_decorated;
}

window_creation_settings_t& window_creation_settings_t::focused(bool value) {
    m_focused = value;
    return *this;
}

bool window_creation_settings_t::focused() const {
    return m_focused;
}

window_creation_settings_t& window_creation_settings_t::maximized(bool value) {
    m_maximized = value;
    return *this;
}

bool window_creation_settings_t::maximized() const {
    return m_maximized;
}

window_creation_settings_t& window_creation_settings_t::auto_minimize_on_focus_loss(bool value) {
    m_auto_minimize_on_focus_loss = value;
    return *this;
}

bool window_creation_settings_t::auto_minimize_on_focus_loss() const {
    return m_auto_minimize_on_focus_loss;
}

window_creation_settings_t& window_creation_settings_t::always_on_top(bool value) {
    m_always_on_top = value;
    return *this;
}

bool window_creation_settings_t::always_on_top() const {
    return m_always_on_top;
}

window_creation_settings_t& window_creation_settings_t::center_cursor_in_fullscreen(bool value) {
    m_center_cursor_in_fullscreen = value;
    return *this;
}

bool window_creation_settings_t::center_cursor_in_fullscreen() const {
    return m_center_cursor_in_fullscreen;
}

window_creation_settings_t& window_creation_settings_t::transparent_framebuffer(bool value) {
    m_transparent_framebuffer = value;
    return *this;
}

bool window_creation_settings_t::transparent_framebuffer() const {
    return m_transparent_framebuffer;
}

window_creation_settings_t& window_creation_settings_t::focus_on_show(bool value) {
    m_focus_on_show = value;
    return *this;
}

bool window_creation_settings_t::focus_on_show() const {
    return m_focus_on_show;
}

window_creation_settings_t& window_creation_settings_t::scale_to_monitor(bool value) {
    m_scale_to_monitor = value;
    return *this;
}

bool window_creation_settings_t::scale_to_monitor() const {
    return m_scale_to_monitor;
}

window_creation_settings_t& window_creation_settings_t::scale_framebuffer(bool value) {
    m_scale_framebuffer = value;
    return *this;
}

bool window_creation_settings_t::scale_framebuffer() const {
    return m_scale_framebuffer;
}

window_creation_settings_t& window_creation_settings_t::mouse_passthrough(bool value) {
    m_mouse_passthrough = value;
    return *this;
}

bool window_creation_settings_t::mouse_passthrough() const {
    return m_mouse_passthrough;
}

window_creation_settings_t& window_creation_settings_t::color_bits(int red, int green, int blue, int alpha) {
    m_red_bits = red;
    m_green_bits = green;
    m_blue_bits = blue;
    m_alpha_bits = alpha;

    return *this;
}

int window_creation_settings_t::color_bits_red() const {
    return m_red_bits;
}

int window_creation_settings_t::color_bits_green() const {
    return m_green_bits;
}

int window_creation_settings_t::color_bits_blue() const {
    return m_blue_bits;
}

int window_creation_settings_t::color_bits_alpha() const {
    return m_alpha_bits;
}

window_creation_settings_t& window_creation_settings_t::depth_stencil_bits(int depth, int stencil) {
    m_depth_bits = depth;
    m_stencil_bits = stencil;

    return *this;
}

int window_creation_settings_t::depth_stencil_bits_depth() const {
    return m_depth_bits;
}

int window_creation_settings_t::depth_stencil_bits_stencil() const {
    return m_stencil_bits;
}

window_creation_settings_t& window_creation_settings_t::accumulation_bits(int red, int green, int blue, int alpha) {
    m_accumulation_red_bits = red;
    m_accumulation_green_bits = green;
    m_accumulation_blue_bits = blue;
    m_accumulation_alpha_bits = alpha;

    return *this;
}

int window_creation_settings_t::accumulation_bits_red() const {
    return m_accumulation_red_bits;
}

int window_creation_settings_t::accumulation_bits_green() const {
    return m_accumulation_green_bits;
}

int window_creation_settings_t::accumulation_bits_blue() const {
    return m_accumulation_blue_bits;
}

int window_creation_settings_t::accumulation_bits_alpha() const {
    return m_accumulation_alpha_bits;
}

window_creation_settings_t& window_creation_settings_t::auxiliary_buffers(int count) {
    m_auxiliary_buffers = count;
    return *this;
}

int window_creation_settings_t::auxiliary_buffers() const {
    return m_auxiliary_buffers;
}

window_creation_settings_t& window_creation_settings_t::sample_count(int count) {
    m_sample_count = count;
    return *this;
}

int window_creation_settings_t::sample_count() const {
    return m_sample_count;
}

window_creation_settings_t& window_creation_settings_t::stereo(bool value) {
    m_stereo = value;
    return *this;
}

bool window_creation_settings_t::stereo() const {
    return m_stereo;
}

window_creation_settings_t& window_creation_settings_t::srgb_capable(bool value) {
    m_srgb_capable = value;
    return *this;
}

bool window_creation_settings_t::srgb_capable() const {
    return m_srgb_capable;
}

window_creation_settings_t& window_creation_settings_t::double_buffered(bool value) {
    m_double_buffered = value;
    return *this;
}

bool window_creation_settings_t::double_buffered() const {
    return m_double_buffered;
}

client_api_t window_creation_settings_t::client_api() const {
    return m_client_api;
}

window_creation_settings_t& window_creation_settings_t::no_client_api() {
    m_client_api = client_api_t::none;
    return *this;
}

window_creation_settings_t& window_creation_settings_t::opengl(int major, int minor, opengl_profile_t profile) {
    m_client_api = client_api_t::opengl;
    m_context_version_major = major;
    m_context_version_minor = minor;
    m_opengl_profile = profile;

    return *this;
}

opengl_profile_t window_creation_settings_t::opengl_profile() const {
    return m_opengl_profile;
}

window_creation_settings_t& window_creation_settings_t::opengl_es(int major, int minor) {
    m_client_api = client_api_t::opengl_es;
    m_context_version_major = major;
    m_context_version_minor = minor;
    m_opengl_profile = opengl_profile_t::any;

    return *this;
}

window_creation_settings_t& window_creation_settings_t::context_creation_api(context_creation_api_t value) {
    m_context_creation_api = value;
    return *this;
}

context_creation_api_t window_creation_settings_t::context_creation_api() const {
    return m_context_creation_api;
}

int window_creation_settings_t::context_version_major() const {
    return m_context_version_major;
}

int window_creation_settings_t::context_version_minor() const {
    return m_context_version_minor;
}

window_creation_settings_t& window_creation_settings_t::context_robustness(context_robustness_t value) {
    m_context_robustness = value;
    return *this;
}

context_robustness_t window_creation_settings_t::context_robustness() const {
    return m_context_robustness;
}

window_creation_settings_t& window_creation_settings_t::context_release_behavior(context_release_behavior_t value) {
    m_context_release_behavior = value;
    return *this;
}

context_release_behavior_t window_creation_settings_t::context_release_behavior() const {
    return m_context_release_behavior;
}

window_creation_settings_t& window_creation_settings_t::forward_compatible(bool value) {
    m_forward_compatible = value;
    return *this;
}

bool window_creation_settings_t::forward_compatible() const {
    return m_forward_compatible;
}

window_creation_settings_t& window_creation_settings_t::debug_context(bool value) {
    m_debug_context = value;
    return *this;
}

bool window_creation_settings_t::debug_context() const {
    return m_debug_context;
}

window_creation_settings_t& window_creation_settings_t::win32_keyboard_menu(bool value) {
    m_win32_keyboard_menu = value;
    return *this;
}

bool window_creation_settings_t::win32_keyboard_menu() const {
    return m_win32_keyboard_menu;
}

window_creation_settings_t& window_creation_settings_t::win32_show_default(bool value) {
    m_win32_show_default = value;
    return *this;
}

bool window_creation_settings_t::win32_show_default() const {
    return m_win32_show_default;
}

window_creation_settings_t& window_creation_settings_t::cocoa_frame_name(std::string value) {
    m_cocoa_frame_name = std::move(value);
    return *this;
}

const std::string& window_creation_settings_t::cocoa_frame_name() const {
    return m_cocoa_frame_name;
}

window_creation_settings_t& window_creation_settings_t::cocoa_graphics_switching(bool value) {
    m_cocoa_graphics_switching = value;
    return *this;
}

bool window_creation_settings_t::cocoa_graphics_switching() const {
    return m_cocoa_graphics_switching;
}

window_creation_settings_t& window_creation_settings_t::wayland_application_id(std::string value) {
    m_wayland_application_id = std::move(value);
    return *this;
}

const std::string& window_creation_settings_t::wayland_application_id() const {
    return m_wayland_application_id;
}

window_creation_settings_t& window_creation_settings_t::x11_class_name(std::string value) {
    m_x11_class_name = std::move(value);
    return *this;
}

const std::string& window_creation_settings_t::x11_class_name() const {
    return m_x11_class_name;
}

window_creation_settings_t& window_creation_settings_t::x11_instance_name(std::string value) {
    m_x11_instance_name = std::move(value);
    return *this;
}

const std::string& window_creation_settings_t::x11_instance_name() const {
    return m_x11_instance_name;
}

void window_creation_settings_t::validate() const {
    validate_preference(m_red_bits, "red_bits");
    validate_preference(m_green_bits, "green_bits");
    validate_preference(m_blue_bits, "blue_bits");
    validate_preference(m_alpha_bits, "alpha_bits");

    validate_preference(m_depth_bits, "depth_bits");
    validate_preference(m_stencil_bits, "stencil_bits");

    validate_preference(m_accumulation_red_bits, "accumulation_red_bits");
    validate_preference(m_accumulation_green_bits, "accumulation_green_bits");
    validate_preference(m_accumulation_blue_bits, "accumulation_blue_bits");
    validate_preference(m_accumulation_alpha_bits, "accumulation_alpha_bits");

    validate_preference(m_auxiliary_buffers, "auxiliary_buffers");
    validate_preference(m_sample_count, "sample_count");

    static_cast<void>(to_glfw(m_context_creation_api));
    static_cast<void>(to_glfw(m_context_robustness));
    static_cast<void>(to_glfw(m_context_release_behavior));
    static_cast<void>(to_glfw(m_opengl_profile));

    if (m_context_version_major < 1 || m_context_version_minor < 0) {
        throw std::invalid_argument(std::format(
            "window_creation_settings_t::validate: context version must have a positive major number and a non-negative minor number, but version {}.{} was requested",
            m_context_version_major,
            m_context_version_minor
        ));
    }

    switch (m_client_api) {
    case client_api_t::none: {
    } break;
    case client_api_t::opengl: {
        validate_opengl_version(m_context_version_major, m_context_version_minor);

        const auto required_major = 3;
        const auto required_minor = 2;
        if (m_opengl_profile != opengl_profile_t::any && !version_at_least(m_context_version_major, m_context_version_minor, required_major, required_minor)) {
            throw std::invalid_argument(std::format(
                "window_creation_settings_t::validate: OpenGL profiles require OpenGL {}.{} or later, but version {}.{} was requested",
                required_major,
                required_minor,
                m_context_version_major,
                m_context_version_minor
            ));
        }

        const auto required_forward_compatible_major = 3;
        const auto required_forward_compatible_minor = 0;
        if (m_forward_compatible && !version_at_least(m_context_version_major, m_context_version_minor, required_forward_compatible_major, required_forward_compatible_minor)) {
            throw std::invalid_argument(std::format(
                "window_creation_settings_t::validate: forward compatibility requires OpenGL {}.{} or later, but version {}.{} was requested",
                required_forward_compatible_major,
                required_forward_compatible_minor,
                m_context_version_major,
                m_context_version_minor
            ));
        }
    } break ;
    case client_api_t::opengl_es: {
        validate_opengl_es_version(m_context_version_major, m_context_version_minor);
    } break ;
    default: throw std::invalid_argument(std::format("window_creation_settings_t::validate: invalid client API value {}", static_cast<int>(m_client_api)));
    }

    validate_no_embedded_null(m_cocoa_frame_name, "cocoa_frame_name");

    validate_ascii(m_wayland_application_id, "wayland_application_id");

    validate_ascii(m_x11_class_name, "x11_class_name");

    validate_ascii(m_x11_instance_name, "x11_instance_name");

    if (m_x11_class_name.empty() != m_x11_instance_name.empty()) {
        throw std::invalid_argument(std::format(
            "window_creation_settings_t::validate: x11_class_name {}, and x11_instance_name {} must either both be empty or both be non-empty",
            m_x11_class_name,
            m_x11_instance_name
        ));
    }
}

void window_creation_settings_t::validate_preference(int value, std::string_view setting) const {
    if (value < 0 && value != window_creation_settings_t::no_preference) {
        throw std::invalid_argument(std::format("window_creation_settings_t::apply: {} must be non-negative or no_preference", setting));
    }
}

void window_creation_settings_t::validate_opengl_version(int major, int minor) const {
    if (major < 1 || minor < 0 || (major == 1 && 5 < minor) || (major == 2 && 1 < minor) || (major == 3 && 3 < minor)) {
        throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid OpenGL version {}.{}", major, minor));
    }
}

void window_creation_settings_t::validate_opengl_es_version(int major, int minor) const {
    if (major < 1 || minor < 0 || (major == 1 && 1 < minor) || (major == 2 && 0 < minor)) {
        throw std::invalid_argument(std::format("window_creation_settings_t::apply: invalid OpenGL ES version {}.{}", major, minor));
    }
}

void window_creation_settings_t::validate_no_embedded_null(const std::string& value, std::string_view setting) const {
    if (value.find('\0') != std::string::npos) {
        throw std::invalid_argument(std::format("window_creation_settings_t::apply: {} must not contain an embedded null character", setting));
    }
}

void window_creation_settings_t::validate_ascii(const std::string& value, std::string_view setting) const {
    validate_no_embedded_null(value, setting);

    for (const unsigned char character : value) {
        if (0x7f < character) {
            throw std::invalid_argument(std::format("window_creation_settings_t::apply: {} must contain only ASCII characters", setting));
        }
    }
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw