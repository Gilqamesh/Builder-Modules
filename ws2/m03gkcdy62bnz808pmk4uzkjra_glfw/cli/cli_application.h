#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H

# include "../glfw.h"
# include "../window.h"

# include "cli_object_registry.h"
# include "cli_support.h"

# include <chrono>
# include <cstddef>
# include <cstdint>
# include <filesystem>
# include <format>
# include <functional>
# include <initializer_list>
# include <map>
# include <memory>
# include <span>
# include <stdexcept>
# include <string>
# include <string_view>
# include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;

enum class watch_target_t {
    window_input,
    joystick,
    gamepad
};

class application_t {
public:
    application_t();

    bool repl();
    bool run_command(std::string_view command);
    bool run_script(const std::filesystem::path& path, bool echo_commands = true);

    std::vector<std::shared_ptr<glfw_api::monitor_t>> retained_monitors() const;

private:
    using monitor_registry_t = object_registry_t<glfw_api::monitor_t, std::uintptr_t>;
    using joystick_registry_t = object_registry_t<glfw_api::joystick_t, int>;
    using gamepad_registry_t = object_registry_t<glfw_api::gamepad_t, int>;
    using arguments_t = cli::arguments_t;
    using argument_spec_t = cli::argument_t;
    using handler_t = std::function<void(arguments_t&)>;
    using completion_handler_t = std::function<std::vector<std::string>(std::span<const std::string>, std::string_view)>;

    struct watch_t {
        watch_t();
        watch_t(id_t id, watch_target_t target, id_t object_id, std::chrono::steady_clock::time_point deadline, std::chrono::steady_clock::time_point next_poll_time, std::chrono::milliseconds interval, std::size_t poll_count);

        id_t id;
        watch_target_t target;
        id_t object_id;
        std::chrono::steady_clock::time_point deadline;
        std::chrono::steady_clock::time_point next_poll_time;
        std::chrono::milliseconds interval;
        std::size_t poll_count;
    };

private:
    argument_spec_t window_id_argument(std::string name = "window-id") const;
    argument_spec_t connected_monitor_id_argument(std::string name = "monitor-id") const;
    argument_spec_t monitor_id_argument(std::string name = "monitor-id") const;
    argument_spec_t connected_joystick_id_argument(std::string name = "joystick-id") const;
    argument_spec_t joystick_id_argument(std::string name = "joystick-id") const;
    argument_spec_t connected_gamepad_id_argument(std::string name = "gamepad-id") const;
    argument_spec_t gamepad_id_argument(std::string name = "gamepad-id") const;
    argument_spec_t watch_id_or_all_argument(std::string name = "watch-id or all") const;

    std::vector<std::string> complete_window_ids(std::string_view partial) const;
    std::vector<std::string> complete_monitor_ids(std::string_view partial, bool require_connected) const;
    std::vector<std::string> complete_joystick_ids(std::string_view partial, bool require_connected) const;
    std::vector<std::string> complete_gamepad_ids(std::string_view partial, bool require_connected) const;
    std::vector<std::string> complete_watch_ids(std::string_view partial, bool include_all) const;
    std::vector<std::string> complete_monitor_video_mode_indices(id_t monitor_id, std::string_view partial) const;

    static argument_spec_t argument(std::string name);
    static argument_spec_t argument(std::string name, completion_handler_t complete);
    static argument_spec_t choice_argument(std::string name, std::initializer_list<std::string_view> values);
    static argument_spec_t suggested_values_argument(std::string name, std::initializer_list<std::string_view> values);
    static argument_spec_t files_argument(std::string name);

    void add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, bool poll_after = true);
    void add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, std::vector<argument_spec_t> arguments, bool poll_after = true);
    void add_command(std::initializer_list<std::string_view> path, std::string usage, std::string description, handler_t handler, completion_handler_t complete_arguments, bool poll_after = true);

    void register_commands();
    void register_core_commands();
    void register_monitor_commands();
    void register_device_commands();
    void register_window_commands();
    void register_settings_commands();

    void refresh_monitors(bool announce_changes);
    void refresh_all(bool announce_changes);

    void poll_once();
    void wait_once();
    void wait_timeout_once(double timeout);
    void after_event_dispatch();
    void idle_once();
    void service_watches();

    void sample_all_devices();
    void advance_window_inputs();

    id_t start_watch(watch_target_t target, id_t object_id, std::chrono::milliseconds duration, std::chrono::milliseconds interval);
    bool print_watch_sample(watch_t& watch);
    static bool watch_target_uses_interval(watch_target_t target);

    static void require_history_size(std::size_t actual_size, std::size_t required_size, std::string_view owner);

    std::shared_ptr<glfw_api::window_t> require_window(id_t id) const;

    static void install_window_callbacks(id_t id, glfw_api::window_t& window);
    static void clear_window_callbacks(glfw_api::window_t& window);
    static void print_callback_status(const glfw_api::window_t& window);
    static void print_window_status(id_t id, glfw_api::window_t& window);

    static std::vector<unsigned char> make_test_pixels(int size);
    static void set_test_icons(glfw_api::window_t& window);
    static void set_test_cursor(glfw_api::window_t& window);

private:
    cli::application_t m_commands;
    std::chrono::milliseconds m_event_pump_interval;
    std::chrono::steady_clock::time_point m_next_idle_time;

    glfw_api::window_creation_settings_t m_creation_settings;

    monitor_registry_t m_monitors;
    joystick_registry_t m_joysticks;
    gamepad_registry_t m_gamepads;

    std::map<id_t, std::shared_ptr<glfw_api::window_t>> m_windows;
    id_t m_next_window_id;

    std::map<id_t, watch_t> m_watches;
    id_t m_next_watch_id;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t& target, auto& ctx) const {
        switch (target) {
        case m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t::window_input: return std::format_to(ctx.out(), "window");
        case m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t::joystick: return std::format_to(ctx.out(), "joystick");
        case m03gkcdy62bnz808pmk4uzkjra_glfw_cli::watch_target_t::gamepad: return std::format_to(ctx.out(), "gamepad");
        default: throw std::runtime_error(std::format("unknown GLFW CLI watch target {}", static_cast<int>(target)));
        }
    }
};

} // namespace std

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
