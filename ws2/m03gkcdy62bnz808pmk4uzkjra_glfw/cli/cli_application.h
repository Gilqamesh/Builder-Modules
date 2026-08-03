#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H

// monitor.h declares std::vector before including <vector>.
# include <vector>

# include "../glfw.h"
# include "../window.h"

# include "cli_arguments.h"
# include "cli_command_table.h"
# include "cli_object_registry.h"

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

private:
    void execute_safely(std::string_view line);
    void execute(std::string_view line);
    void execute_tokens(std::span<const std::string> tokens);

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

    void sample_all_devices();
    static void sample_joystick(glfw_api::joystick_t& joystick);
    static void sample_gamepad(glfw_api::gamepad_t& gamepad);
    void advance_window_inputs();

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
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_APPLICATION_H
