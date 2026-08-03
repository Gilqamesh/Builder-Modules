#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_GLFW_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_GLFW_H

# include "glfw_external.h"
# include "monitor.h"
# include "input.h"
    
# include <vector>
# include <memory>
# include <unordered_map>
# include <optional>

/**
 * Treat this module as a single-threaded event loop: construct one glfw_t on the main thread, keep it alive longer than every other module object, and perform all operations on that thread.
 *
 * Callbacks run synchronously during event processing and must not throw or destroy GLFW resources before returning.
 */
namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

/**
 * @brief RAII owner of GLFW's process-wide initialization.
 *
 * Only one instance may exist.
 */
class glfw_t {
public:
    glfw_t();
    ~glfw_t();

    glfw_t(const glfw_t&) = delete;
    glfw_t& operator=(const glfw_t&) = delete;

    glfw_t(glfw_t&&) = delete;
    glfw_t& operator=(glfw_t&&) = delete;
};

/**
 * @brief Processes all pending events and invokes their callbacks.
 */
void poll_events();

/**
 * @brief Waits for an event and then processes pending events.
 */
void wait_events();

/**
 * @brief Waits for at most the specified number of seconds and then processes pending events.
 */
void wait_events_timeout(double timeout);

/**
 * @brief Posts an empty event that wakes an event wait.
 */
void post_empty_event();

/**
 * @brief Returns the current primary monitor, or nullptr if none is available.
 */
std::shared_ptr<monitor_t> primary_monitor();

/**
 * @brief Returns the currently connected monitors in unspecified order.
 */
std::vector<std::shared_ptr<monitor_t>> monitors();

/**
 * @brief Polls the current joystick state into its staging snapshot, or does nothing if it is disconnected.
 */
void poll_joystick(joystick_t& joystick);

/**
 * @brief Returns the currently connected joysticks in joystick-ID order.
 */
std::vector<std::shared_ptr<joystick_t>> joysticks();

/**
 * @brief Polls the current gamepad state into its staging snapshot, or does nothing if it is disconnected.
 */
void poll_gamepad(gamepad_t& gamepad);

/**
 * @brief Returns the currently connected mapped gamepads in joystick-ID order.
 */
std::vector<std::shared_ptr<gamepad_t>> gamepads();

/**
 * @brief Installs or replaces a gamepad mapping and refreshes matching gamepad wrappers.
 */
void update_joystick_to_gamepad_mapping(const joystick_to_gamepad_mapping_t& joystick_to_gamepad_mapping);

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_GLFW_H
