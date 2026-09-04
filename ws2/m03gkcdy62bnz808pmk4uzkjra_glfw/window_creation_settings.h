#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_WINDOW_CREATION_SETTINGS_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_WINDOW_CREATION_SETTINGS_H

# include <string>
# include <format>
# include <stdexcept>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

/**
 * @brief Client API requested for a new window.
 *
 * Default: opengl.
 */
enum class client_api_t {
    none,
    opengl,
    opengl_es
};

/**
 * @brief API used to create the OpenGL or OpenGL ES context.
 *
 * Default: native.
 */
enum class context_creation_api_t {
    /**
     * @brief Uses the platform-native context creation API.
     */
    native,

    /**
     * @brief Uses EGL to create the context.
     */
    egl,

    /**
     * @brief Uses OSMesa to create an off-screen OpenGL context.
     *
     * Swapping buffers does not update the window contents.
     */
    osmesa
};

/**
 * @brief Strategy for handling graphics context resets.
 *
 * Default: none.
 */
enum class context_robustness_t {
    /**
     * @brief Does not request context-reset robustness.
     */
    none,

    /**
     * @brief Requests no notification when a graphics reset occurs.
     */
    no_reset_notification,

    /**
     * @brief Requests that the context be considered lost after a reset.
     */
    lose_context_on_reset
};

/**
 * @brief Behavior used when a context stops being current.
 *
 * Default: any.
 */
enum class context_release_behavior_t {
    /**
     * @brief Uses the default behavior of the context creation API.
     */
    any,

    /**
     * @brief Flushes pending commands when the context is released.
     */
    flush,

    /**
     * @brief Does not flush pending commands when the context is released.
     */
    none
};

/**
 * @brief Desktop OpenGL context profile.
 *
 * Default: any.
 *
 * A specific profile requires OpenGL 3.2 or later.
 * This setting is ignored for OpenGL ES.
 */
enum class opengl_profile_t {
    /**
     * @brief Does not request a specific OpenGL profile.
     */
    any,

    /**
     * @brief Requests the OpenGL compatibility profile.
     *
     * Includes compatibility functionality unavailable in the core profile.
     */
    compatibility,

    /**
     * @brief Requests the OpenGL core profile.
     *
     * Excludes functionality removed from the requested core version.
     */
    core
};

/**
 * @brief Stores a complete window-creation hint state for this module.
 *
 * Setters update only this object, while apply validates the complete state, resets GLFW's current hints and applies this state.
 */
class window_creation_settings_t {
public:
    /**
     * @brief Numeric framebuffer preference meaning no preference.
     */
    static constexpr int no_preference = -1;

public:
    /**
     * @brief Constructs the default creation state.
     *
     * The default client API is OpenGL 1.0 with no specific profile.
     */
    window_creation_settings_t();

    /**
     * @brief Restores this object to its default state.
     */
    window_creation_settings_t& reset();

    /**
     * @brief Validates and applies this complete state after resetting GLFW's current window hints.
     *
     * Call this from the main thread while glfw_t is alive and immediately before creating a window.
     *
     * Fails if this state is invalid.
     */
    void apply() const;

    // General window state.

    /**
     * @brief Requests whether a windowed window is user-resizable.
     *
     * Default: true.
     */
    window_creation_settings_t& resizable(bool value);
    bool resizable() const;

    /**
     * @brief Requests whether a windowed window is initially visible.
     *
     * Default: true.
     */
    window_creation_settings_t& visible(bool value);
    bool visible() const;

    /**
     * @brief Requests whether a windowed window has decorations.
     *
     * Default: true.
     */
    window_creation_settings_t& decorated(bool value);
    bool decorated() const;

    /**
     * @brief Requests whether a windowed window is initially focused.
     *
     * Default: true.
     */
    window_creation_settings_t& focused(bool value);
    bool focused() const;

    /**
     * @brief Requests whether a windowed window is initially maximized.
     *
     * Default: false.
     */
    window_creation_settings_t& maximized(bool value);
    bool maximized() const;
    
    /**
     * @brief Requests whether a full-screen window minimizes on focus loss.
     *
     * Default: true.
     */
    window_creation_settings_t& auto_minimize_on_focus_loss(bool value);
    bool auto_minimize_on_focus_loss() const;

    /**
     * @brief Requests whether a windowed window stays above regular windows.
     *
     * Default: false.
     */
    window_creation_settings_t& always_on_top(bool value);
    bool always_on_top() const;

    /**
     * @brief Requests whether the cursor is centered over a new full-screen window.
     *
     * Default: true.
     */
    window_creation_settings_t& center_cursor_in_fullscreen(bool value);
    bool center_cursor_in_fullscreen() const;

    /**
     * @brief Requests a transparent framebuffer.
     *
     * Default: false.
     */
    window_creation_settings_t& transparent_framebuffer(bool value);
    bool transparent_framebuffer() const;

    /**
     * @brief Requests whether showing the window gives it input focus.
     *
     * Default: true.
     */
    window_creation_settings_t& focus_on_show(bool value);
    bool focus_on_show() const;

    /**
     * @brief Requests whether the content area follows monitor scale changes.
     *
     * Default: false.
     */
    window_creation_settings_t& scale_to_monitor(bool value);
    bool scale_to_monitor() const;

    /**
     * @brief Requests whether the framebuffer follows content-scale changes.
     *
     * Default: true.
     */
    window_creation_settings_t& scale_framebuffer(bool value);
    bool scale_framebuffer() const;

    /**
     * @brief Requests whether mouse input passes through the window.
     *
     * Default: false.
     */
    window_creation_settings_t& mouse_passthrough(bool value);
    bool mouse_passthrough() const;

    // Default framebuffer.

    /**
     * @brief Requests the desired RGBA bit depths.
     *
     * Default: 8, 8, 8, 8.
     *
     * Each value must be non-negative or no_preference.
     */
    window_creation_settings_t& color_bits(int red, int green, int blue, int alpha);
    int color_bits_red() const;
    int color_bits_green() const;
    int color_bits_blue() const;
    int color_bits_alpha() const;

    /**
     * @brief Requests the desired depth and stencil bit depths.
     *
     * Default: 24, 8.
     *
     * Each value must be non-negative or no_preference.
     */
    window_creation_settings_t& depth_stencil_bits(int depth, int stencil);
    int depth_stencil_bits_depth() const;
    int depth_stencil_bits_stencil() const;

    /**
     * @brief Requests the desired accumulation-buffer bit depths.
     *
     * Default: 0, 0, 0, 0.
     *
     * Each value must be non-negative or no_preference.
     */
    window_creation_settings_t& accumulation_bits(int red, int green, int blue, int alpha);
    int accumulation_bits_red() const;
    int accumulation_bits_green() const;
    int accumulation_bits_blue() const;
    int accumulation_bits_alpha() const;

    /**
     * @brief Requests the desired number of auxiliary buffers.
     *
     * Default: 0.
     *
     * The value must be non-negative or no_preference.
     */
    window_creation_settings_t& auxiliary_buffers(int count);
    int auxiliary_buffers() const;

    /**
     * @brief Requests the desired multisample count.
     *
     * Default: 0.
     *
     * The value must be non-negative or no_preference.
     */
    window_creation_settings_t& sample_count(int count);
    int sample_count() const;

    /**
     * @brief Requests a stereoscopic framebuffer.
     *
     * Default: false.
     */
    window_creation_settings_t& stereo(bool value);
    bool stereo() const;

    /**
     * @brief Requests an sRGB-capable framebuffer.
     *
     * Default: false.
     */
    window_creation_settings_t& srgb_capable(bool value);
    bool srgb_capable() const;

    /**
     * @brief Requests a double-buffered framebuffer.
     *
     * Default: true.
     */
    window_creation_settings_t& double_buffered(bool value);
    bool double_buffered() const;

    // Client API and context.

    /**
     * @brief Returns the selected client API.
     */
    client_api_t client_api() const;

    /**
     * @brief Selects no client API for the new window.
     */
    window_creation_settings_t& no_client_api();

    /**
     * @brief Selects the requested desktop OpenGL version and profile.
     *
     * Default: OpenGL 1.0 with opengl_profile_t::any.
     */
    window_creation_settings_t& opengl(int major, int minor, opengl_profile_t profile);
    opengl_profile_t opengl_profile() const;

    /**
     * @brief Selects the requested OpenGL ES version.
     */
    window_creation_settings_t& opengl_es(int major, int minor);

    /**
     * @brief Selects the requested context creation API.
     *
     * Default: context_creation_api_t::native.
     */
    window_creation_settings_t& context_creation_api(context_creation_api_t value);
    context_creation_api_t context_creation_api() const;
    int context_version_major() const;
    int context_version_minor() const;

    /**
     * @brief Selects the requested context robustness strategy.
     *
     * Default: context_robustness_t::none.
     */
    window_creation_settings_t& context_robustness(context_robustness_t value);
    context_robustness_t context_robustness() const;

    /**
     * @brief Selects the requested context release behavior.
     *
     * Default: context_release_behavior_t::any.
     */
    window_creation_settings_t& context_release_behavior(context_release_behavior_t value);
    context_release_behavior_t context_release_behavior() const;

    /**
     * @brief Requests a forward-compatible OpenGL context.
     *
     * Default: false.
     */
    window_creation_settings_t& forward_compatible(bool value);
    bool forward_compatible() const;

    /**
     * @brief Requests a debug context.
     *
     * Default: false.
     */
    window_creation_settings_t& debug_context(bool value);
    bool debug_context() const;

    // Platform-specific features.

    /**
     * @brief Requests Win32 keyboard access to the window menu.
     *
     * Default: false.
     */
    window_creation_settings_t& win32_keyboard_menu(bool value);
    bool win32_keyboard_menu() const;

    /**
     * @brief Requests Win32 STARTUPINFO behavior when the window is first shown.
     *
     * Default: false.
     */
    window_creation_settings_t& win32_show_default(bool value);
    bool win32_show_default() const;

    /**
     * @brief Sets the requested UTF-8 macOS frame autosave name.
     *
     * Default: empty.
     */
    window_creation_settings_t& cocoa_frame_name(std::string value);
    const std::string& cocoa_frame_name() const;

    /**
     * @brief Requests macOS automatic graphics switching.
     *
     * Default: false.
     */
    window_creation_settings_t& cocoa_graphics_switching(bool value);
    bool cocoa_graphics_switching() const;

    /**
     * @brief Sets the requested ASCII Wayland application identifier.
     *
     * Default: empty.
     */
    window_creation_settings_t& wayland_application_id(std::string value);
    const std::string& wayland_application_id() const;

    /**
     * @brief Sets the requested ASCII X11 WM_CLASS class name.
     *
     * Default: empty.
     */
    window_creation_settings_t& x11_class_name(std::string value);
    const std::string& x11_class_name() const;

    /**
     * @brief Sets the requested ASCII X11 WM_CLASS instance name.
     *
     * Default: empty.
     */
    window_creation_settings_t& x11_instance_name(std::string value);
    const std::string& x11_instance_name() const;

private:
    void validate() const;
    void validate_preference(int value, std::string_view setting) const;
    void validate_opengl_version(int major, int minor) const;
    void validate_opengl_es_version(int major, int minor) const;
    void validate_no_embedded_null(const std::string& value, std::string_view setting) const;
    void validate_ascii(const std::string& value, std::string_view setting) const;

private:
    bool m_resizable;
    bool m_visible;
    bool m_decorated;
    bool m_focused;
    bool m_maximized;

    bool m_auto_minimize_on_focus_loss;
    bool m_always_on_top;
    bool m_center_cursor_in_fullscreen;
    bool m_transparent_framebuffer;
    bool m_focus_on_show;
    bool m_scale_to_monitor;
    bool m_scale_framebuffer;
    bool m_mouse_passthrough;

    int m_red_bits;
    int m_green_bits;
    int m_blue_bits;
    int m_alpha_bits;

    int m_depth_bits;
    int m_stencil_bits;

    int m_accumulation_red_bits;
    int m_accumulation_green_bits;
    int m_accumulation_blue_bits;
    int m_accumulation_alpha_bits;

    int m_auxiliary_buffers;
    int m_sample_count;

    bool m_stereo;
    bool m_srgb_capable;
    bool m_double_buffered;

    client_api_t m_client_api;
    context_creation_api_t m_context_creation_api;

    int m_context_version_major;
    int m_context_version_minor;

    context_robustness_t m_context_robustness;
    context_release_behavior_t m_context_release_behavior;

    bool m_forward_compatible;
    bool m_debug_context;

    opengl_profile_t m_opengl_profile;

    bool m_win32_keyboard_menu;
    bool m_win32_show_default;

    std::string m_cocoa_frame_name;
    bool m_cocoa_graphics_switching;

    std::string m_wayland_application_id;

    std::string m_x11_class_name;
    std::string m_x11_instance_name;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::window_creation_settings_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t& client_api, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        switch (client_api) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t::none: {
                out = std::format_to(out, "none");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t::opengl: {
                out = std::format_to(out, "OpenGL");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::client_api_t::opengl_es: {
                out = std::format_to(out, "OpenGL ES");
            } break;
            default: throw std::runtime_error(std::format("Unknown client API: {}", static_cast<int>(client_api)));
        }

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t& context_creation_api, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        switch (context_creation_api) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t::native: {
                out = std::format_to(out, "native");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t::egl: {
                out = std::format_to(out, "EGL");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_creation_api_t::osmesa: {
                out = std::format_to(out, "OSMesa");
            } break;
            default: throw std::runtime_error(std::format("Unknown context creation API: {}", static_cast<int>(context_creation_api)));
        }

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t& context_robustness, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        switch (context_robustness) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t::none: {
                out = std::format_to(out, "none");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t::no_reset_notification: {
                out = std::format_to(out, "no reset notification");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_robustness_t::lose_context_on_reset: {
                out = std::format_to(out, "lose context on reset");
            } break;
            default: throw std::runtime_error(std::format("Unknown context robustness: {}", static_cast<int>(context_robustness)));
        }

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t& context_release_behavior, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        switch (context_release_behavior) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t::any: {
                out = std::format_to(out, "any");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t::none: {
                out = std::format_to(out, "none");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::context_release_behavior_t::flush: {
                out = std::format_to(out, "flush");
            } break;
            default: throw std::runtime_error(std::format("Unknown context release behavior: {}", static_cast<int>(context_release_behavior)));
        }

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t& opengl_profile, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        switch (opengl_profile) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t::any: {
                out = std::format_to(out, "any");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t::compatibility: {
                out = std::format_to(out, "compatibility");
            } break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::opengl_profile_t::core: {
                out = std::format_to(out, "core");
            } break;
            default: throw std::runtime_error(std::format("Unknown OpenGL profile: {}", static_cast<int>(opengl_profile)));
        }

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::window_creation_settings_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::window_creation_settings_t& settings, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "resizable: {}, ", settings.resizable());
        out = std::format_to(out, "visible: {}, ", settings.visible());
        out = std::format_to(out, "decorated: {}, ", settings.decorated());
        out = std::format_to(out, "focused: {}, ", settings.focused());
        out = std::format_to(out, "maximized: {}, ", settings.maximized());

        out = std::format_to(out, "auto_minimize_on_focus_loss: {}, ", settings.auto_minimize_on_focus_loss());
        out = std::format_to(out, "always_on_top: {}, ", settings.always_on_top());
        out = std::format_to(out, "center_cursor_in_fullscreen: {}, ", settings.center_cursor_in_fullscreen());
        out = std::format_to(out, "transparent_framebuffer: {}, ", settings.transparent_framebuffer());
        out = std::format_to(out, "focus_on_show: {}, ", settings.focus_on_show());
        out = std::format_to(out, "scale_to_monitor: {}, ", settings.scale_to_monitor());
        out = std::format_to(out, "scale_framebuffer: {}, ", settings.scale_framebuffer());
        out = std::format_to(out, "mouse_passthrough: {}, ", settings.mouse_passthrough());

        out = std::format_to(out, "color_bits: ({}, {}, {}, {}), ", settings.color_bits_red(), settings.color_bits_green(), settings.color_bits_blue(), settings.color_bits_alpha());

        out = std::format_to(out, "depth_stencil_bits: ({}, {}), ", settings.depth_stencil_bits_depth(), settings.depth_stencil_bits_stencil());

        out = std::format_to(out, "accumulation_bits: ({}, {}, {}, {}), ", settings.accumulation_bits_red(), settings.accumulation_bits_green(), settings.accumulation_bits_blue(), settings.accumulation_bits_alpha());

        out = std::format_to(out, "auxiliary_buffers: {}, ", settings.auxiliary_buffers());
        out = std::format_to(out, "sample_count: {}, ", settings.sample_count());

        out = std::format_to(out, "stereo: {}, ", settings.stereo());
        out = std::format_to(out, "srgb_capable: {}, ", settings.srgb_capable());
        out = std::format_to(out, "double_buffered: {}, ", settings.double_buffered());

        out = std::format_to(out, "client_api: {}, ", settings.client_api());
        out = std::format_to(out, "context_creation_api: {}, ", settings.context_creation_api());

        out = std::format_to(out, "context_version: ({}.{}), ", settings.context_version_major(), settings.context_version_minor());

        out = std::format_to(out, "context_robustness: {}, ", settings.context_robustness());
        out = std::format_to(out, "context_release_behavior: {}, ", settings.context_release_behavior());

        out = std::format_to(out, "forward_compatible: {}, ", settings.forward_compatible());
        out = std::format_to(out, "debug_context: {}, ", settings.debug_context());

        out = std::format_to(out, "opengl_profile: {}, ", settings.opengl_profile());

        out = std::format_to(out, "win32_keyboard_menu: {}, ", settings.win32_keyboard_menu());
        out = std::format_to(out, "win32_show_default: {}, ", settings.win32_show_default());

        out = std::format_to(out, "cocoa_frame_name: \"{}\", ", settings.cocoa_frame_name());
        out = std::format_to(out, "cocoa_graphics_switching: {}, ", settings.cocoa_graphics_switching());

        out = std::format_to(out, "wayland_application_id: \"{}\", ", settings.wayland_application_id());
        out = std::format_to(out, "x11_class_name: \"{}\", ", settings.x11_class_name());
        out = std::format_to(out, "x11_instance_name: \"{}\"", settings.x11_instance_name());

        out = std::format_to(out, " }}");

        return out;
    };
};

} // namespace std

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_WINDOW_CREATION_SETTINGS_H
