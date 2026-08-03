#include "cli_application.h"
#include "cli_history.h"

#include <chrono>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

void application_t::register_window_commands() {
    const auto add_query = [this](
        std::string name,
        std::string description,
        auto getter
    ) {
        const std::string usage = "window " + name + " <window-id>";
        m_commands.add(
            {"window", name},
            usage,
            std::move(description),
            [this, getter, usage](arguments_t& arguments) {
                const id_t id = arguments.pop_id("window-id");
                arguments.expect_end(usage);
                std::cout << std::format("{}\n", getter(*require_window(id)));
            },
            {
                window_id_argument()
            }
        );
    };

    const auto add_action = [this](
        std::string name,
        std::string description,
        auto action
    ) {
        const std::string usage = "window " + name + " <window-id>";
        m_commands.add(
            {"window", name},
            usage,
            std::move(description),
            [this, action, usage](arguments_t& arguments) {
                const id_t id = arguments.pop_id("window-id");
                arguments.expect_end(usage);
                action(*require_window(id));
                std::cout << "ok\n";
            },
            {
                window_id_argument()
            }
        );
    };

    const auto add_bool_property = [this](
        std::string name,
        std::string description,
        auto getter,
        auto setter
    ) {
        const std::string usage = "window " + name + " <window-id> [bool]";
        m_commands.add(
            {"window", name},
            usage,
            std::move(description),
            [this, getter, setter, usage](arguments_t& arguments) {
                const id_t id = arguments.pop_id("window-id");
                auto window = require_window(id);
                if (!arguments.empty()) {
                    setter(*window, arguments.pop_bool("value"));
                }
                arguments.expect_end(usage);
                std::cout << std::format("{}\n", getter(*window));
            },
            {
                window_id_argument(),
                command_table_t::values_argument(
                    "bool",
                    {"true", "false", "on", "off", "yes", "no", "1", "0"}
                )
            }
        );
    };

    const auto add_string_property = [this](
        std::string name,
        std::string description,
        auto getter,
        auto setter
    ) {
        const std::string usage = "window " + name + " <window-id> [value]";
        m_commands.add(
            {"window", name},
            usage,
            std::move(description),
            [this, getter, setter, usage](arguments_t& arguments) {
                const id_t id = arguments.pop_id("window-id");
                auto window = require_window(id);
                if (!arguments.empty()) {
                    setter(*window, std::string(arguments.pop("value")));
                }
                arguments.expect_end(usage);
                std::cout << quote_token(getter(*window)) << '\n';
            },
            {
                window_id_argument(),
                command_table_t::argument("value")
            }
        );
    };

    const auto make_input_change = [](
        const glfw_api::input_state_t& previous,
        const glfw_api::input_state_t& current
    ) {
        return glfw_api::input_state_change_t(previous, current);
    };

    const auto resize_window_input_history = [](
        glfw_api::window_t& window,
        std::size_t sample_count
    ) {
        if (sample_count == 0) {
            command_error("sample-count must be positive");
        }

        auto& history = window.input_states();
        const glfw_api::input_state_t snapshot = history.size() == 0
            ? history.stage()
            : history.history(0);

        using history_t = std::remove_reference_t<decltype(history)>;
        history = history_t(sample_count);
        history.stage() = snapshot;
        history.commit();
        history.stage() = history.history(0);
    };

    m_commands.add(
        {"window", "create"},
        "window create windowed <title> <x> <y> <width> <height> | "
        "window create fullscreen <title> <monitor-id> [video-mode-index]",
        "Create a window with the current creation settings.",
        [this](arguments_t& arguments) {
            const std::string mode(arguments.pop("windowed or fullscreen"));
            const std::string title(arguments.pop("title"));
            std::shared_ptr<glfw_api::window_t> window;

            if (mode == "windowed") {
                const int x = arguments.pop_int("x");
                const int y = arguments.pop_int("y");
                const int width = arguments.pop_int("width");
                const int height = arguments.pop_int("height");
                arguments.expect_end(
                    "window create windowed <title> <x> <y> <width> <height>"
                );

                if (width <= 0 || height <= 0) {
                    command_error(std::format(
                        "window dimensions must be positive, got {}x{}",
                        width,
                        height
                    ));
                }

                window = glfw_api::window_t::create(
                    title,
                    {x, y, width, height},
                    m_creation_settings
                );
            } else if (mode == "fullscreen") {
                const id_t monitor_id = arguments.pop_id("monitor-id");
                auto& monitor = *m_monitors.require(monitor_id, true).object;
                glfw_api::video_mode_t video_mode = monitor.video_mode();

                if (!arguments.empty()) {
                    const std::size_t mode_index = arguments.pop_size("video-mode-index");
                    const auto video_modes = monitor.video_modes();
                    if (mode_index >= video_modes.size()) {
                        command_error(std::format(
                            "video-mode-index {} is out of range; monitor has {} mode(s)",
                            mode_index,
                            video_modes.size()
                        ));
                    }
                    video_mode = video_modes[mode_index];
                }

                arguments.expect_end(
                    "window create fullscreen <title> <monitor-id> [video-mode-index]"
                );
                window = glfw_api::window_t::create(
                    title,
                    monitor,
                    video_mode,
                    m_creation_settings
                );
            } else {
                command_error("window creation mode must be 'windowed' or 'fullscreen'");
            }

            if (!window) {
                command_error("window creation returned nullptr");
            }

            const id_t id = m_next_window_id++;
            m_windows.emplace(id, window);
            install_window_callbacks(id, *window);
            std::cout << std::format(
                "Created {} window {} with title {}.\n",
                mode,
                id,
                quote_token(title)
            );
        },
        [this](const completion_context_t& context) {
            if (context.arguments.empty()) {
                return command_table_t::values_completion(
                    {"windowed", "fullscreen"}
                )(context);
            }

            if (context.arguments.front() != "fullscreen") {
                return completion_result_t{};
            }

            if (context.arguments.size() == 2) {
                return complete_monitor_ids(context.partial, true);
            }

            if (context.arguments.size() == 3) {
                try {
                    const id_t monitor_id = parse_integer<id_t>(
                        context.arguments[2],
                        "monitor-id"
                    );
                    return complete_monitor_video_mode_indices(
                        monitor_id,
                        context.partial
                    );
                } catch (const std::exception&) {
                    return completion_result_t{};
                }
            }

            return completion_result_t{};
        }
    );

    m_commands.add(
        {"window", "list"},
        "window list",
        "List every window currently owned by the CLI.",
        [this](arguments_t& arguments) {
            arguments.expect_end("window list");

            if (m_windows.empty()) {
                std::cout << "No windows.\n";
                return;
            }

            for (const auto& [id, window] : m_windows) {
                std::cout << std::format(
                    "{}: title={}, size={}, framebuffer_size={}, visible={}, "
                    "should_close={}, fullscreen={}, client_api={}\n",
                    id,
                    quote_token(window->title()),
                    window->size(),
                    window->framebuffer_size(),
                    window->visible(),
                    window->should_close(),
                    window->fullscreen(),
                    window->client_api()
                );
            }
        }
    );

    const auto add_status_command = [this](
        std::string_view name,
        std::string description
    ) {
        const std::string usage = std::format("window {} <window-id>", name);
        m_commands.add(
            {"window", name},
            usage,
            std::move(description),
            [this, usage](arguments_t& arguments) {
                const id_t id = arguments.pop_id("window-id");
                arguments.expect_end(usage);
                print_window_status(id, *require_window(id));
            },
            {
                window_id_argument()
            }
        );
    };

    add_status_command(
        "show",
        "Show the full window status, latest input and callback status."
    );
    add_status_command(
        "status",
        "Alias for window show."
    );

    m_commands.add(
        {"window", "destroy"},
        "window destroy <window-id>",
        "Destroy one window while GLFW is still initialized.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window destroy <window-id>");
            if (m_windows.erase(id) == 0) {
                command_error(std::format("no window with ID {}", id));
            }
            std::cout << std::format("Destroyed window {}.\n", id);
        },
        {
            window_id_argument()
        },
        false
    );

    m_commands.add(
        {"window", "destroy-all"},
        "window destroy-all",
        "Destroy every window owned by the CLI.",
        [this](arguments_t& arguments) {
            arguments.expect_end("window destroy-all");
            const std::size_t count = m_windows.size();
            m_windows.clear();
            std::cout << std::format("Destroyed {} window(s).\n", count);
        },
        false
    );

    add_bool_property(
        "should-close",
        "Get or set the window close flag.",
        [](glfw_api::window_t& window) {
            return window.should_close();
        },
        [](glfw_api::window_t& window, bool value) {
            window.should_close(value);
        }
    );

    add_string_property(
        "title",
        "Get or set the UTF-8 window title.",
        [](glfw_api::window_t& window) {
            return window.title();
        },
        [](glfw_api::window_t& window, const std::string& value) {
            window.title(value);
        }
    );

    add_bool_property(
        "visible",
        "Get or set window visibility.",
        [](glfw_api::window_t& window) {
            return window.visible();
        },
        [](glfw_api::window_t& window, bool value) {
            window.visible(value);
        }
    );

    add_bool_property(
        "focus-on-visible",
        "Get or set whether showing the window requests focus.",
        [](glfw_api::window_t& window) {
            return window.focus_on_visible();
        },
        [](glfw_api::window_t& window, bool value) {
            window.focus_on_visible(value);
        }
    );

    add_action(
        "maximize",
        "Request maximization.",
        [](glfw_api::window_t& window) {
            window.maximize();
        }
    );
    add_action(
        "minimize",
        "Request minimization.",
        [](glfw_api::window_t& window) {
            window.minimize();
        }
    );
    add_action(
        "restore",
        "Restore the window from minimized or maximized state.",
        [](glfw_api::window_t& window) {
            window.restore();
        }
    );
    add_action(
        "focus",
        "Request input focus.",
        [](glfw_api::window_t& window) {
            window.focus();
        }
    );
    add_action(
        "request-attention",
        "Request the user's attention.",
        [](glfw_api::window_t& window) {
            window.request_attention();
        }
    );

    add_query(
        "maximized",
        "Show whether the window is maximized.",
        [](glfw_api::window_t& window) {
            return window.maximized();
        }
    );
    add_query(
        "minimized",
        "Show whether the window is minimized.",
        [](glfw_api::window_t& window) {
            return window.minimized();
        }
    );
    add_query(
        "focused",
        "Show whether the window has input focus.",
        [](glfw_api::window_t& window) {
            return window.focused();
        }
    );
    add_query(
        "framebuffer-size",
        "Show framebuffer dimensions in pixels.",
        [](glfw_api::window_t& window) {
            return window.framebuffer_size();
        }
    );
    add_query(
        "cursor-visible",
        "Show whether the cursor is visible.",
        [](glfw_api::window_t& window) {
            return window.cursor_visible();
        }
    );
    add_query(
        "cursor-locked",
        "Show whether the cursor is locked to the window.",
        [](glfw_api::window_t& window) {
            return window.cursor_locked();
        }
    );
    add_query(
        "cursor-hovered",
        "Show whether the cursor is inside the content area.",
        [](glfw_api::window_t& window) {
            return window.cursor_is_in_content_area();
        }
    );
    add_query(
        "transparent-framebuffer",
        "Show whether the window has a transparent framebuffer.",
        [](glfw_api::window_t& window) {
            return window.transparent_framebuffer();
        }
    );

    m_commands.add(
        {"window", "handle"},
        "window handle <window-id>",
        "Show the native GLFWwindow pointer.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window handle <window-id>");
            std::cout << std::format(
                "window {} handle: {}\n",
                id,
                static_cast<void*>(require_window(id)->handle())
            );
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "client-api"},
        "window client-api <window-id>",
        "Show the window client API.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window client-api <window-id>");
            std::cout << std::format("{}\n", require_window(id)->client_api());
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "size"},
        "window size <window-id> [width height]",
        "Get or set the content-area size in screen coordinates.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);

            if (!arguments.empty()) {
                if (arguments.size() != 2) {
                    command_error("usage: window size <window-id> [width height]");
                }
                const int width = arguments.pop_int("width");
                const int height = arguments.pop_int("height");
                if (width <= 0 || height <= 0) {
                    command_error(std::format(
                        "window dimensions must be positive, got {}x{}",
                        width,
                        height
                    ));
                }
                window->size({width, height});
            }

            arguments.expect_end("window size <window-id> [width height]");
            std::cout << std::format("{}\n", window->size());
        },
        {
            window_id_argument(),
            command_table_t::argument("width"),
            command_table_t::argument("height")
        }
    );

    m_commands.add(
        {"window", "size-limits"},
        "window size-limits <window-id> clear | "
        "<min-width|none> <min-height|none> <max-width|none> <max-height|none>",
        "Set or clear window size limits.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);
            const std::string_view first = arguments.pop("clear or min-width");

            if (first == "clear") {
                arguments.expect_end("window size-limits <window-id> clear");
                window->size_limits(
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                );
                std::cout << "Cleared window size limits.\n";
                return;
            }

            const auto min_width = parse_optional_non_negative_integer(first, "min-width");
            const auto min_height = parse_optional_non_negative_integer(
                arguments.pop("min-height"),
                "min-height"
            );
            const auto max_width = parse_optional_non_negative_integer(
                arguments.pop("max-width"),
                "max-width"
            );
            const auto max_height = parse_optional_non_negative_integer(
                arguments.pop("max-height"),
                "max-height"
            );
            arguments.expect_end(
                "window size-limits <window-id> <min-width|none> <min-height|none> "
                "<max-width|none> <max-height|none>"
            );

            if (min_width && max_width && *max_width < *min_width) {
                command_error("max-width must be greater than or equal to min-width");
            }
            if (min_height && max_height && *max_height < *min_height) {
                command_error("max-height must be greater than or equal to min-height");
            }

            window->size_limits(min_width, min_height, max_width, max_height);
            std::cout << "Window size limits updated.\n";
        },
        {
            window_id_argument(),
            command_table_t::values_argument(
                "clear or min-width",
                {"clear", "none", "any", "no-limit", "no-preference", "dont-care"}
            ),
            command_table_t::values_argument(
                "min-height",
                {"none", "any", "no-limit", "no-preference", "dont-care"}
            ),
            command_table_t::values_argument(
                "max-width",
                {"none", "any", "no-limit", "no-preference", "dont-care"}
            ),
            command_table_t::values_argument(
                "max-height",
                {"none", "any", "no-limit", "no-preference", "dont-care"}
            )
        }
    );

    m_commands.add(
        {"window", "aspect-ratio"},
        "window aspect-ratio <window-id> clear | <numerator> <denominator>",
        "Set or clear the required content-area aspect ratio.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);
            const std::string_view first = arguments.pop("clear or numerator");

            if (first == "clear") {
                arguments.expect_end("window aspect-ratio <window-id> clear");
                window->clear_aspect_ratio();
                std::cout << "Cleared the window aspect ratio.\n";
                return;
            }

            const int numerator = parse_integer<int>(first, "numerator");
            const int denominator = arguments.pop_int("denominator");
            arguments.expect_end(
                "window aspect-ratio <window-id> <numerator> <denominator>"
            );

            if (numerator <= 0 || denominator <= 0) {
                command_error(std::format(
                    "aspect ratio components must be positive, got {}:{}",
                    numerator,
                    denominator
                ));
            }

            window->aspect_ratio({numerator, denominator});
            std::cout << std::format("Aspect ratio set to {}:{}.\n", numerator, denominator);
        },
        {
            window_id_argument(),
            command_table_t::values_argument("clear or numerator", {"clear"}),
            command_table_t::argument("denominator")
        }
    );

    m_commands.add(
        {"window", "fullscreen"},
        "window fullscreen <window-id> [monitor-id]",
        "Get fullscreen state or enter fullscreen mode on a monitor.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);

            if (!arguments.empty()) {
                const id_t monitor_id = arguments.pop_id("monitor-id");
                window->fullscreen(*m_monitors.require(monitor_id, true).object);
            }

            arguments.expect_end("window fullscreen <window-id> [monitor-id]");
            std::cout << std::format("{}\n", window->fullscreen());
        },
        {
            window_id_argument(),
            connected_monitor_id_argument()
        }
    );

    m_commands.add(
        {"window", "windowed"},
        "window windowed <window-id> | <window-id> <monitor-id> | "
        "<window-id> <x> <y> <width> <height>",
        "Get windowed state or enter windowed mode using a monitor work area or rectangle.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);

            if (arguments.size() == 1) {
                const id_t monitor_id = arguments.pop_id("monitor-id");
                window->windowed(m_monitors.require(monitor_id, true).object->work_area());
            } else if (arguments.size() == 4) {
                const int x = arguments.pop_int("x");
                const int y = arguments.pop_int("y");
                const int width = arguments.pop_int("width");
                const int height = arguments.pop_int("height");
                if (width <= 0 || height <= 0) {
                    command_error(std::format(
                        "window dimensions must be positive, got {}x{}",
                        width,
                        height
                    ));
                }
                window->windowed({x, y, width, height});
            } else if (!arguments.empty()) {
                command_error(
                    "usage: window windowed <window-id> | <window-id> <monitor-id> | "
                    "<window-id> <x> <y> <width> <height>"
                );
            }

            std::cout << std::format("{}\n", window->windowed());
        },
        [this](const completion_context_t& context) {
            if (context.arguments.empty()) {
                return complete_window_ids(context.partial);
            }
            if (context.arguments.size() == 1) {
                return complete_monitor_ids(context.partial, true);
            }
            return completion_result_t{};
        }
    );

    m_commands.add(
        {"window", "opacity"},
        "window opacity <window-id> [0..1]",
        "Get or set window opacity.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);

            if (!arguments.empty()) {
                const float opacity = arguments.pop_float("opacity");
                if (opacity < 0.0f || opacity > 1.0f) {
                    command_error(std::format(
                        "opacity must be between 0 and 1, got {}",
                        opacity
                    ));
                }
                window->opacity(opacity);
            }

            arguments.expect_end("window opacity <window-id> [0..1]");
            std::cout << std::format("{}\n", window->opacity());
        },
        {
            window_id_argument(),
            command_table_t::values_argument("opacity", {"0", "0.5", "1"})
        }
    );

    m_commands.add(
        {"window", "cursor-mode"},
        "window cursor-mode <window-id> <visible> <locked>",
        "Set cursor visibility and locking together.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const bool visible = arguments.pop_bool("visible");
            const bool locked = arguments.pop_bool("locked");
            arguments.expect_end(
                "window cursor-mode <window-id> <visible> <locked>"
            );

            auto window = require_window(id);
            window->cursor_mode(visible, locked);
            std::cout << std::format(
                "cursor_visible={}, cursor_locked={}\n",
                window->cursor_visible(),
                window->cursor_locked()
            );
        },
        {
            window_id_argument(),
            command_table_t::values_argument(
                "visible",
                {"true", "false", "on", "off", "yes", "no", "1", "0"}
            ),
            command_table_t::values_argument(
                "locked",
                {"true", "false", "on", "off", "yes", "no", "1", "0"}
            )
        }
    );

    m_commands.add(
        {"window", "cursor-raw-motion"},
        "window cursor-raw-motion <window-id> [bool]",
        "Get or request raw mouse motion.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            auto window = require_window(id);

            if (!arguments.empty()) {
                const bool supported = window->cursor_raw_motion(
                    arguments.pop_bool("value")
                );
                std::cout << std::format("request_supported={}\n", supported);
            }

            arguments.expect_end("window cursor-raw-motion <window-id> [bool]");
            std::cout << std::format("enabled={}\n", window->cursor_raw_motion());
        },
        {
            window_id_argument(),
            command_table_t::values_argument(
                "bool",
                {"true", "false", "on", "off", "yes", "no", "1", "0"}
            )
        }
    );

    m_commands.add(
        {"window", "cursor-image"},
        "window cursor-image <window-id> test|clear",
        "Install a generated cursor image or restore the default cursor.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::string_view operation = arguments.pop("test or clear");
            arguments.expect_end("window cursor-image <window-id> test|clear");

            auto window = require_window(id);
            if (operation == "test") {
                set_test_cursor(*window);
                std::cout << "Installed a generated 32x32 cursor.\n";
            } else if (operation == "clear") {
                window->reset_cursor_image();
                std::cout << "Restored the default cursor.\n";
            } else {
                command_error("cursor-image operation must be 'test' or 'clear'");
            }
        },
        {
            window_id_argument(),
            command_table_t::values_argument("operation", {"test", "clear"})
        }
    );

    m_commands.add(
        {"window", "input"},
        "window input <window-id>",
        "Show the newest committed input snapshot.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window input <window-id>");

            const auto window = require_window(id);
            const auto& history = window->input_states();
            require_history_size(history.size(), 1, std::format("window {} input", id));
            std::cout << std::format("{}\n", history.history(0));
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "input-delta"},
        "window input-delta <window-id>",
        "Show the change from input history[1] to history[0].",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window input-delta <window-id>");

            const auto window = require_window(id);
            const auto& history = window->input_states();
            require_history_size(history.size(), 2, std::format("window {} input", id));
            const glfw_api::input_state_change_t change(
                history.history(1),
                history.history(0)
            );
            std::cout << std::format("{}\n", change);
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "input-history"},
        "window input-history <window-id>",
        "Show every retained input snapshot and every adjacent change.",
        [this, make_input_change](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window input-history <window-id>");
            const auto window = require_window(id);
            const std::string heading = std::format("window {} input history", id);
            m03gkcdy62bnz808pmk4uzkjra_glfw_cli::print_history(
                heading,
                window->input_states(),
                make_input_change
            );
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "input-history-size"},
        "window input-history-size <window-id> <sample-count>",
        "Replace retained input history with a new sample capacity.",
        [this, resize_window_input_history](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::size_t sample_count = arguments.pop_size("sample-count");
            arguments.expect_end("window input-history-size <window-id> <sample-count>");

            auto window = require_window(id);
            resize_window_input_history(*window, sample_count);

            const auto& history = window->input_states();
            std::cout << std::format(
                "window {} input history capacity set to {}; samples={}\n",
                id,
                history.capacity(),
                history.size()
            );
        },
        {
            window_id_argument(),
            command_table_t::argument("sample-count")
        },
        false
    );

    m_commands.add(
        {"window", "input-watch"},
        "window input-watch <window-id> <milliseconds> [interval-ms]",
        "Start a non-blocking watch that prints retained input changes.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const auto duration = std::chrono::milliseconds(
                arguments.pop_long_long("milliseconds")
            );
            const auto interval = std::chrono::milliseconds(
                arguments.empty()
                    ? 16
                    : arguments.pop_long_long("interval-ms")
            );
            arguments.expect_end(
                "window input-watch <window-id> <milliseconds> [interval-ms]"
            );

            if (duration.count() < 0) {
                command_error("milliseconds must be non-negative");
            }
            if (interval.count() <= 0) {
                command_error("interval-ms must be positive");
            }

            const id_t watch_id = start_watch(
                watch_target_t::window_input,
                id,
                duration,
                interval
            );
            std::cout << std::format(
                "Started watch {} for window {} input for {} ms at {} ms intervals.\n",
                watch_id,
                id,
                duration.count(),
                interval.count()
            );
        },
        {
            window_id_argument(),
            command_table_t::argument("milliseconds"),
            command_table_t::argument("interval-ms")
        }
    );

    m_commands.add(
        {"window", "make-context-current"},
        "window make-context-current <window-id>",
        "Make this window's OpenGL or OpenGL ES context current.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            arguments.expect_end("window make-context-current <window-id>");
            std::cout << std::format("{}\n", require_window(id)->context_current(true));
        },
        {
            window_id_argument()
        }
    );

    m_commands.add(
        {"window", "swap-interval"},
        "window swap-interval <window-id> <interval>",
        "Set the swap interval for the window context.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const int interval = arguments.pop_int("interval");
            arguments.expect_end("window swap-interval <window-id> <interval>");
            require_window(id)->swap_interval(interval);
            std::cout << std::format("Swap interval requested: {}.\n", interval);
        },
        {
            window_id_argument(),
            command_table_t::argument("interval")
        }
    );

    m_commands.add(
        {"window", "swap-buffers"},
        "window swap-buffers <window-id> [count]",
        "Swap the window's buffers one or more times.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::size_t count = arguments.empty()
                ? 1
                : arguments.pop_size("count");
            arguments.expect_end("window swap-buffers <window-id> [count]");

            const auto window = require_window(id);
            for (std::size_t index = 0; index < count; ++index) {
                window->swap_buffers();
            }
            std::cout << std::format("Swapped buffers {} time(s).\n", count);
        },
        {
            window_id_argument(),
            command_table_t::argument("count")
        }
    );

    m_commands.add(
        {"window", "extension-supported"},
        "window extension-supported <window-id> <extension-name>",
        "Check an OpenGL or OpenGL ES extension on the window context.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::string extension_name(arguments.pop("extension-name"));
            arguments.expect_end(
                "window extension-supported <window-id> <extension-name>"
            );

            const auto window = require_window(id);
            if (window->client_api() == glfw_api::client_api_t::none) {
                command_error("window has no client API context");
            }
            std::cout << std::format(
                "{}\n",
                window->extension_supported(extension_name)
            );
        },
        {
            window_id_argument(),
            command_table_t::argument("extension-name")
        }
    );

    m_commands.add(
        {"window", "proc-address"},
        "window proc-address <window-id> <function-name>",
        "Check whether a context function address can be resolved.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::string function_name(arguments.pop("function-name"));
            arguments.expect_end(
                "window proc-address <window-id> <function-name>"
            );

            const auto window = require_window(id);
            if (!window->context_current(true)) {
                command_error("window has no client API context");
            }
            std::cout << std::format(
                "available={}\n",
                glfw_api::get_proc_address(function_name.c_str()) != nullptr
            );
        },
        {
            window_id_argument(),
            command_table_t::argument("function-name")
        }
    );

    m_commands.add(
        {"window", "icon"},
        "window icon <window-id> test|clear",
        "Install generated window icons or restore the default icon.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::string_view operation = arguments.pop("test or clear");
            arguments.expect_end("window icon <window-id> test|clear");

            auto window = require_window(id);
            if (operation == "test") {
                set_test_icons(*window);
                std::cout << "Installed generated 16x16, 32x32 and 48x48 icons.\n";
            } else if (operation == "clear") {
                window->icon(std::span<const glfw_api::image_t>{});
                std::cout << "Restored the default window icon.\n";
            } else {
                command_error("icon operation must be 'test' or 'clear'");
            }
        },
        {
            window_id_argument(),
            command_table_t::values_argument("operation", {"test", "clear"})
        }
    );

    m_commands.add(
        {"window", "callbacks"},
        "window callbacks <window-id> install|clear|status",
        "Install, clear or inspect logging callbacks for every callback API.",
        [this](arguments_t& arguments) {
            const id_t id = arguments.pop_id("window-id");
            const std::string_view operation = arguments.pop("operation");
            arguments.expect_end(
                "window callbacks <window-id> install|clear|status"
            );

            auto window = require_window(id);
            if (operation == "install") {
                install_window_callbacks(id, *window);
                std::cout << std::format("Installed callbacks for window {}.\n", id);
            } else if (operation == "clear") {
                clear_window_callbacks(*window);
                std::cout << std::format("Cleared callbacks for window {}.\n", id);
            } else if (operation == "status") {
                print_callback_status(*window);
            } else {
                command_error("callback operation must be install, clear or status");
            }
        },
        {
            window_id_argument(),
            command_table_t::values_argument(
                "operation",
                {"install", "clear", "status"}
            )
        }
    );
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli
