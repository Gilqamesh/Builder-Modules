#include "cli_application.h"

#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace {

int parse_preference(std::string_view value, std::string_view name) {
    if (value == "none" || value == "no-preference" || value == "dont-care") {
        return glfw_api::window_creation_settings_t::no_preference;
    }
    const int result = parse_integer<int>(value, name);
    if (result < 0 && result != glfw_api::window_creation_settings_t::no_preference) {
        command_error(std::format(
            "{} must be non-negative or no-preference, got {}",
            name,
            result
        ));
    }
    return result;
}

glfw_api::opengl_profile_t parse_opengl_profile(std::string_view value) {
    if (value == "any") {
        return glfw_api::opengl_profile_t::any;
    }
    if (value == "compatibility") {
        return glfw_api::opengl_profile_t::compatibility;
    }
    if (value == "core") {
        return glfw_api::opengl_profile_t::core;
    }
    command_error(std::format("unknown OpenGL profile '{}'", value));
}

glfw_api::context_creation_api_t parse_context_creation_api(std::string_view value) {
    if (value == "native") {
        return glfw_api::context_creation_api_t::native;
    }
    if (value == "egl") {
        return glfw_api::context_creation_api_t::egl;
    }
    if (value == "osmesa") {
        return glfw_api::context_creation_api_t::osmesa;
    }
    command_error(std::format("unknown context creation API '{}'", value));
}

glfw_api::context_robustness_t parse_context_robustness(std::string_view value) {
    if (value == "none") {
        return glfw_api::context_robustness_t::none;
    }
    if (value == "no-reset-notification") {
        return glfw_api::context_robustness_t::no_reset_notification;
    }
    if (value == "lose-context-on-reset") {
        return glfw_api::context_robustness_t::lose_context_on_reset;
    }
    command_error(std::format("unknown context robustness '{}'", value));
}

glfw_api::context_release_behavior_t parse_context_release_behavior(std::string_view value) {
    if (value == "any") {
        return glfw_api::context_release_behavior_t::any;
    }
    if (value == "flush") {
        return glfw_api::context_release_behavior_t::flush;
    }
    if (value == "none") {
        return glfw_api::context_release_behavior_t::none;
    }
    command_error(std::format("unknown context release behavior '{}'", value));
}

} // namespace

void application_t::register_settings_commands() {
    const auto add_setting = [this](std::string name, std::string argument_description, std::string description, auto handler, std::vector<argument_spec_t> argument_specs = {}) {
        std::string usage = "settings set " + name;
        if (!argument_description.empty()) {
            usage += " " + argument_description;
        }

        add_command(
            {"settings", "set", name},
            usage,
            std::move(description),
            [this, handler, usage, name](arguments_t& arguments) {
                handler(arguments);
                arguments.expect_end(usage);
                std::cout << std::format("{} updated.\n", name);
            },
            std::move(argument_specs),
            false
        );
    };

    const auto add_bool_setting = [add_setting](std::string name, std::string description, auto setter) {
        add_setting(
            std::move(name),
            "<bool>",
            std::move(description),
            [setter](arguments_t& arguments) {
                setter(arguments.pop<bool>("value"));
            },
            {
                choice_argument(
                    "bool",
                    {"true", "false", "on", "off", "yes", "no", "1", "0"}
                )
            }
        );
    };

    const auto add_string_setting = [add_setting](std::string name, std::string description, auto setter) {
        add_setting(
            std::move(name),
            "<value>",
            std::move(description),
            [setter](arguments_t& arguments) {
                setter(arguments.pop<std::string>("value"));
            },
            {
                argument("value")
            }
        );
    };

    add_command(
        {"settings", "reset"},
        "settings reset",
        "Restore every creation setting to its API default.",
        [this](arguments_t& arguments) {
            arguments.expect_end("settings reset");
            m_creation_settings.reset();
            std::cout << "Creation settings reset.\n";
        },
        false
    );

    add_command(
        {"settings", "apply"},
        "settings apply",
        "Validate the current settings and apply them as GLFW window hints.",
        [this](arguments_t& arguments) {
            arguments.expect_end("settings apply");
            m_creation_settings.apply();
            std::cout << "Creation settings validated and applied.\n";
        },
        false
    );

    add_command(
        {"settings", "show"},
        "settings show",
        "Show every current creation setting through its public getters.",
        [this](arguments_t& arguments) {
            arguments.expect_end("settings show");
            std::cout << std::format("{}\n", m_creation_settings);
        },
        false
    );

    add_command(
        {"settings", "preset"},
        "settings preset default|no-api|hidden-no-api|gl33-core|transparent",
        "Reset settings and select a useful preset.",
        [this](arguments_t& arguments) {
            const std::string_view preset = arguments.pop("preset");
            arguments.expect_end(
                "settings preset default|no-api|hidden-no-api|gl33-core|transparent"
            );

            m_creation_settings.reset();
            if (preset == "default") {
                // reset() already applied the preset.
            } else if (preset == "no-api") {
                m_creation_settings.no_client_api();
            } else if (preset == "hidden-no-api") {
                m_creation_settings.visible(false).no_client_api();
            } else if (preset == "gl33-core") {
                m_creation_settings.opengl(
                    3,
                    3,
                    glfw_api::opengl_profile_t::core
                );
            } else if (preset == "transparent") {
                m_creation_settings.transparent_framebuffer(true);
            } else {
                command_error(std::format("unknown settings preset '{}'", preset));
            }

            std::cout << std::format("Applied '{}' creation-settings preset.\n", preset);
        },
        {
            choice_argument(
                "preset",
                {"default", "no-api", "hidden-no-api", "gl33-core", "transparent"}
            )
        },
        false
    );

    add_bool_setting(
        "resizable",
        "Set whether a windowed window is user-resizable.",
        [this](bool value) {
            m_creation_settings.resizable(value);
        }
    );
    add_bool_setting(
        "visible",
        "Set whether a new window is initially visible.",
        [this](bool value) {
            m_creation_settings.visible(value);
        }
    );
    add_bool_setting(
        "decorated",
        "Set whether a windowed window has decorations.",
        [this](bool value) {
            m_creation_settings.decorated(value);
        }
    );
    add_bool_setting(
        "focused",
        "Set whether a new window is initially focused.",
        [this](bool value) {
            m_creation_settings.focused(value);
        }
    );
    add_bool_setting(
        "maximized",
        "Set whether a new windowed window is initially maximized.",
        [this](bool value) {
            m_creation_settings.maximized(value);
        }
    );
    add_bool_setting(
        "auto-minimize-on-focus-loss",
        "Set whether a fullscreen window minimizes when focus is lost.",
        [this](bool value) {
            m_creation_settings.auto_minimize_on_focus_loss(value);
        }
    );
    add_bool_setting(
        "always-on-top",
        "Set whether a windowed window stays above regular windows.",
        [this](bool value) {
            m_creation_settings.always_on_top(value);
        }
    );
    add_bool_setting(
        "center-cursor-in-fullscreen",
        "Set whether a new fullscreen window centers the cursor.",
        [this](bool value) {
            m_creation_settings.center_cursor_in_fullscreen(value);
        }
    );
    add_bool_setting(
        "transparent-framebuffer",
        "Request a transparent framebuffer.",
        [this](bool value) {
            m_creation_settings.transparent_framebuffer(value);
        }
    );
    add_bool_setting(
        "focus-on-show",
        "Set whether showing a window requests input focus.",
        [this](bool value) {
            m_creation_settings.focus_on_show(value);
        }
    );
    add_bool_setting(
        "scale-to-monitor",
        "Set whether the content area follows monitor scale changes.",
        [this](bool value) {
            m_creation_settings.scale_to_monitor(value);
        }
    );
    add_bool_setting(
        "scale-framebuffer",
        "Set whether the framebuffer follows content-scale changes.",
        [this](bool value) {
            m_creation_settings.scale_framebuffer(value);
        }
    );
    add_bool_setting(
        "mouse-passthrough",
        "Set whether mouse input passes through the window.",
        [this](bool value) {
            m_creation_settings.mouse_passthrough(value);
        }
    );
    add_bool_setting(
        "stereo",
        "Request a stereoscopic framebuffer.",
        [this](bool value) {
            m_creation_settings.stereo(value);
        }
    );
    add_bool_setting(
        "srgb-capable",
        "Request an sRGB-capable framebuffer.",
        [this](bool value) {
            m_creation_settings.srgb_capable(value);
        }
    );
    add_bool_setting(
        "double-buffered",
        "Request a double-buffered framebuffer.",
        [this](bool value) {
            m_creation_settings.double_buffered(value);
        }
    );
    add_bool_setting(
        "forward-compatible",
        "Request a forward-compatible OpenGL context.",
        [this](bool value) {
            m_creation_settings.forward_compatible(value);
        }
    );
    add_bool_setting(
        "debug-context",
        "Request a debug OpenGL or OpenGL ES context.",
        [this](bool value) {
            m_creation_settings.debug_context(value);
        }
    );
    add_bool_setting(
        "win32-keyboard-menu",
        "Enable keyboard access to the Win32 window menu.",
        [this](bool value) {
            m_creation_settings.win32_keyboard_menu(value);
        }
    );
    add_bool_setting(
        "win32-show-default",
        "Use Win32 STARTUPINFO when the window is first shown.",
        [this](bool value) {
            m_creation_settings.win32_show_default(value);
        }
    );
    add_bool_setting(
        "cocoa-graphics-switching",
        "Enable automatic graphics switching on macOS.",
        [this](bool value) {
            m_creation_settings.cocoa_graphics_switching(value);
        }
    );

    add_string_setting(
        "cocoa-frame-name",
        "Set the UTF-8 macOS frame autosave name.",
        [this](std::string value) {
            m_creation_settings.cocoa_frame_name(std::move(value));
        }
    );
    add_string_setting(
        "wayland-application-id",
        "Set the ASCII Wayland application identifier.",
        [this](std::string value) {
            m_creation_settings.wayland_application_id(std::move(value));
        }
    );
    add_string_setting(
        "x11-class-name",
        "Set the ASCII X11 WM_CLASS class name.",
        [this](std::string value) {
            m_creation_settings.x11_class_name(std::move(value));
        }
    );
    add_string_setting(
        "x11-instance-name",
        "Set the ASCII X11 WM_CLASS instance name.",
        [this](std::string value) {
            m_creation_settings.x11_instance_name(std::move(value));
        }
    );

    add_setting(
        "color-bits",
        "<red> <green> <blue> <alpha>",
        "Set desired RGBA bit depths; none means no preference.",
        [this](arguments_t& arguments) {
            const int red = parse_preference(arguments.pop("red"), "red");
            const int green = parse_preference(arguments.pop("green"), "green");
            const int blue = parse_preference(arguments.pop("blue"), "blue");
            const int alpha = parse_preference(arguments.pop("alpha"), "alpha");
            m_creation_settings.color_bits(red, green, blue, alpha);
        },
        {
            suggested_values_argument("red", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("green", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("blue", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("alpha", {"none", "no-preference", "dont-care"})
        }
    );

    add_setting(
        "depth-stencil-bits",
        "<depth> <stencil>",
        "Set desired depth and stencil bit depths; none means no preference.",
        [this](arguments_t& arguments) {
            const int depth = parse_preference(arguments.pop("depth"), "depth");
            const int stencil = parse_preference(arguments.pop("stencil"), "stencil");
            m_creation_settings.depth_stencil_bits(depth, stencil);
        },
        {
            suggested_values_argument("depth", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("stencil", {"none", "no-preference", "dont-care"})
        }
    );

    add_setting(
        "accumulation-bits",
        "<red> <green> <blue> <alpha>",
        "Set desired accumulation-buffer bit depths.",
        [this](arguments_t& arguments) {
            const int red = parse_preference(arguments.pop("red"), "red");
            const int green = parse_preference(arguments.pop("green"), "green");
            const int blue = parse_preference(arguments.pop("blue"), "blue");
            const int alpha = parse_preference(arguments.pop("alpha"), "alpha");
            m_creation_settings.accumulation_bits(red, green, blue, alpha);
        },
        {
            suggested_values_argument("red", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("green", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("blue", {"none", "no-preference", "dont-care"}),
            suggested_values_argument("alpha", {"none", "no-preference", "dont-care"})
        }
    );

    add_setting(
        "auxiliary-buffers",
        "<count>",
        "Set the desired number of auxiliary buffers.",
        [this](arguments_t& arguments) {
            m_creation_settings.auxiliary_buffers(
                parse_preference(arguments.pop("count"), "count")
            );
        },
        {
            suggested_values_argument("count", {"none", "no-preference", "dont-care"})
        }
    );

    add_setting(
        "sample-count",
        "<count>",
        "Set the desired multisample count.",
        [this](arguments_t& arguments) {
            m_creation_settings.sample_count(
                parse_preference(arguments.pop("count"), "count")
            );
        },
        {
            suggested_values_argument("count", {"none", "no-preference", "dont-care"})
        }
    );

    add_setting(
        "no-client-api",
        "",
        "Create windows without an OpenGL or OpenGL ES context.",
        [this](arguments_t&) {
            m_creation_settings.no_client_api();
        },
        {}
    );

    add_setting(
        "opengl",
        "<major> <minor> any|compatibility|core",
        "Select a desktop OpenGL version and profile.",
        [this](arguments_t& arguments) {
            const int major = arguments.pop<int>("major");
            const int minor = arguments.pop<int>("minor");
            const auto profile = parse_opengl_profile(arguments.pop("profile"));
            m_creation_settings.opengl(major, minor, profile);
        },
        {
            argument("major"),
            argument("minor"),
            choice_argument("profile", {"any", "compatibility", "core"})
        }
    );

    add_setting(
        "opengl-es",
        "<major> <minor>",
        "Select an OpenGL ES version.",
        [this](arguments_t& arguments) {
            const int major = arguments.pop<int>("major");
            const int minor = arguments.pop<int>("minor");
            m_creation_settings.opengl_es(major, minor);
        },
        {
            argument("major"),
            argument("minor")
        }
    );

    add_setting(
        "context-creation-api",
        "native|egl|osmesa",
        "Select the context creation API.",
        [this](arguments_t& arguments) {
            m_creation_settings.context_creation_api(
                parse_context_creation_api(arguments.pop("value"))
            );
        },
        {
            choice_argument("value", {"native", "egl", "osmesa"})
        }
    );

    add_setting(
        "context-robustness",
        "none|no-reset-notification|lose-context-on-reset",
        "Select the context-reset robustness strategy.",
        [this](arguments_t& arguments) {
            m_creation_settings.context_robustness(
                parse_context_robustness(arguments.pop("value"))
            );
        },
        {
            choice_argument(
                "value",
                {"none", "no-reset-notification", "lose-context-on-reset"}
            )
        }
    );

    add_setting(
        "context-release-behavior",
        "any|flush|none",
        "Select behavior when a context stops being current.",
        [this](arguments_t& arguments) {
            m_creation_settings.context_release_behavior(
                parse_context_release_behavior(arguments.pop("value"))
            );
        },
        {
            choice_argument("value", {"any", "flush", "none"})
        }
    );

    add_command(
        {"settings", "set"},
        "settings set <name> <arguments...>",
        "Set one creation setting; use 'settings set help' for names.",
        [](arguments_t& arguments) {
            if (arguments.empty()) {
                command_error("usage: settings set <name> <arguments...>");
            }
            command_error(std::format(
                "unknown creation setting '{}'; use 'settings set help'",
                arguments.pop("name")
            ));
        },
        false
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
