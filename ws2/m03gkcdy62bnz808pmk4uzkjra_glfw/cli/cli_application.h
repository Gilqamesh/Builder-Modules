#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H

// monitor.h declares std::vector before including <vector>.
# include <vector>

# include "../glfw.h"
# include "../window.h"

# include "cli_arguments.h"
# include "cli_command_table.h"
# include "cli_object_registry.h"

# include <chrono>
# include <cstddef>
# include <cstdint>
# include <filesystem>
# include <map>
# include <memory>
# include <span>
# include <string>
# include <string_view>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;

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
    using argument_spec_t = command_table_t::argument_spec_t;
    using completion_context_t = command_table_t::completion_context_t;
    using completion_result_t = command_table_t::completion_result_t;

    enum class watch_target_t {
        window_input,
        joystick,
        gamepad
    };

    struct watch_t {
        id_t id = 0;
        watch_target_t target = watch_target_t::window_input;
        id_t object_id = 0;
        std::chrono::steady_clock::time_point deadline;
        std::chrono::steady_clock::time_point next_poll_time;
        std::chrono::milliseconds interval{0};
        std::size_t poll_count = 0;
    };

private:
    void execute_safely(std::string_view line);
    void execute(std::string_view line);
    void execute_tokens(std::span<const std::string> tokens);

#if defined(__unix__) || defined(__APPLE__)
    static void readline_line_handler(char* line);
    static char** readline_completion(const char* text, int start, int end);
    static char* readline_completion_generator(const char* text, int state);

    completion_result_t complete_command_line(
        std::string_view line,
        std::size_t word_start,
        std::string_view partial
    ) const;
#endif

    argument_spec_t window_id_argument(std::string name = "window-id") const;
    argument_spec_t connected_monitor_id_argument(std::string name = "monitor-id") const;
    argument_spec_t monitor_id_argument(std::string name = "monitor-id") const;
    argument_spec_t connected_joystick_id_argument(std::string name = "joystick-id") const;
    argument_spec_t joystick_id_argument(std::string name = "joystick-id") const;
    argument_spec_t connected_gamepad_id_argument(std::string name = "gamepad-id") const;
    argument_spec_t gamepad_id_argument(std::string name = "gamepad-id") const;
    argument_spec_t watch_id_or_all_argument(std::string name = "watch-id or all") const;

    completion_result_t complete_window_ids(std::string_view partial) const;
    completion_result_t complete_monitor_ids(std::string_view partial, bool require_connected) const;
    completion_result_t complete_joystick_ids(std::string_view partial, bool require_connected) const;
    completion_result_t complete_gamepad_ids(std::string_view partial, bool require_connected) const;
    completion_result_t complete_watch_ids(std::string_view partial, bool include_all) const;
    completion_result_t complete_monitor_video_mode_indices(
        id_t monitor_id,
        std::string_view partial
    ) const;

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
    void service_watches();

    void sample_all_devices();
    static void sample_joystick(glfw_api::joystick_t& joystick);
    static void sample_gamepad(glfw_api::gamepad_t& gamepad);
    void advance_window_inputs();

    id_t start_watch(
        watch_target_t target,
        id_t object_id,
        std::chrono::milliseconds duration,
        std::chrono::milliseconds interval
    );
    bool print_watch_sample(watch_t& watch);
    static std::string_view watch_target_label(watch_target_t target);

    static void require_history_size(
        std::size_t actual_size,
        std::size_t required_size,
        std::string_view owner
    );

    std::shared_ptr<glfw_api::window_t> require_window(id_t id) const;

    static void install_window_callbacks(id_t id, glfw_api::window_t& window);
    static void clear_window_callbacks(glfw_api::window_t& window);
    static void print_callback_status(const glfw_api::window_t& window);
    static void print_window_status(id_t id, glfw_api::window_t& window);

    static std::vector<unsigned char> make_test_pixels(int size);
    static void set_test_icons(glfw_api::window_t& window);
    static void set_test_cursor(glfw_api::window_t& window);

private:
    bool m_running = true;
    command_table_t m_commands;

    glfw_api::window_creation_settings_t m_creation_settings;

    monitor_registry_t m_monitors;
    joystick_registry_t m_joysticks;
    gamepad_registry_t m_gamepads;

    std::map<id_t, std::shared_ptr<glfw_api::window_t>> m_windows;
    id_t m_next_window_id = 1;

    std::map<id_t, watch_t> m_watches;
    id_t m_next_watch_id = 1;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
