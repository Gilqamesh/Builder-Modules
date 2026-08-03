#include "glfw.h"

#include <stdexcept>
#include <format>
#include <iostream>
#include <unordered_map>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

namespace {

static glfw_t* g_instance;
std::unordered_map<GLFWmonitor*, std::shared_ptr<monitor_t>> g_monitor_by_glfw_monitor;
std::vector<std::shared_ptr<joystick_t>> g_joysticks;
std::vector<std::shared_ptr<gamepad_t>> g_gamepads;

gamepad_button_t glfw_gamepad_button_to_gamepad_button(int glfw_gamepad_button) {
    switch (glfw_gamepad_button) {
        case GLFW_GAMEPAD_BUTTON_A: return gamepad_button_t::button_a; // or gamepad_button_t::button_cross
        case GLFW_GAMEPAD_BUTTON_B: return gamepad_button_t::button_b; // or gamepad_button_t::button_circle
        case GLFW_GAMEPAD_BUTTON_X: return gamepad_button_t::button_x; // or gamepad_button_t::button_square
        case GLFW_GAMEPAD_BUTTON_Y: return gamepad_button_t::button_y; // or gamepad_button_t::button_triangle
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return gamepad_button_t::button_left_bumper;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return gamepad_button_t::button_right_bumper;
        case GLFW_GAMEPAD_BUTTON_BACK: return gamepad_button_t::button_back;
        case GLFW_GAMEPAD_BUTTON_START: return gamepad_button_t::button_start;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return gamepad_button_t::button_guide;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return gamepad_button_t::button_left_thumb;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return gamepad_button_t::button_right_thumb;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return gamepad_button_t::button_dpad_up;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return gamepad_button_t::button_dpad_right;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return gamepad_button_t::button_dpad_down;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return gamepad_button_t::button_dpad_left;
        default: throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_gamepad_button_to_gamepad_button: glfw_gamepad_button {} is out of range", glfw_gamepad_button));
    }
}

gamepad_axis_t glfw_gamepad_axis_to_gamepad_axis(int glfw_gamepad_axis) {
    switch (glfw_gamepad_axis) {
        case GLFW_GAMEPAD_AXIS_LEFT_X: return gamepad_axis_t::axis_left_x;
        case GLFW_GAMEPAD_AXIS_LEFT_Y: return gamepad_axis_t::axis_left_y;
        case GLFW_GAMEPAD_AXIS_RIGHT_X: return gamepad_axis_t::axis_right_x;
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: return gamepad_axis_t::axis_right_y;
        case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return gamepad_axis_t::axis_left_trigger;
        case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return gamepad_axis_t::axis_right_trigger;
        default: throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_gamepad_axis_to_gamepad_axis: glfw_gamepad_axis {} is out of range", glfw_gamepad_axis));
    }
}

std::shared_ptr<monitor_t> connect_monitor(GLFWmonitor* monitor_handle) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_monitor: g_instance is null");
    }

    if (!monitor_handle) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_monitor: monitor_handle is null");
    }

    auto it = g_monitor_by_glfw_monitor.find(monitor_handle);
    if (it != g_monitor_by_glfw_monitor.end()) {
        return it->second;
    }

    auto monitor = std::make_shared<monitor_t>();
    monitor->handle(monitor_handle);

    g_monitor_by_glfw_monitor.insert({monitor_handle, monitor});

    return monitor;
}

void disconnect_monitor(GLFWmonitor* monitor_handle) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_monitor: g_instance is null");
    }

    if (!monitor_handle) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_monitor: monitor_handle is null");
    }

    auto it = g_monitor_by_glfw_monitor.find(monitor_handle);
    if (it == g_monitor_by_glfw_monitor.end()) {
        return;
    }

    it->second->handle(nullptr);
    g_monitor_by_glfw_monitor.erase(monitor_handle);
}

std::shared_ptr<joystick_t> connect_joystick(int joystick_id) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_joystick: g_instance is null");
    }

    if (joystick_id < 0 || GLFW_JOYSTICK_LAST < joystick_id) {
        throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_joystick: joystick_id {} is out of range", joystick_id));
    }

    const char* joystick_name = glfwGetJoystickName(joystick_id);
    if (!joystick_name) {
        throw std::runtime_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_joystick: failed to get name for joystick {}", joystick_id));
    }

    const char* joystick_guid = glfwGetJoystickGUID(joystick_id);
    if (!joystick_guid) {
        throw std::runtime_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_joystick: failed to get GUID for joystick {}", joystick_id));
    }

    if (g_joysticks.size() <= static_cast<std::size_t>(joystick_id)) {
        g_joysticks.resize(joystick_id + 1);
    }

    auto joystick = std::make_shared<joystick_t>(2);

    joystick->id() = joystick_id;
    joystick->name() = joystick_name;
    joystick->guid() = joystick_guid;
    
    g_joysticks[joystick_id] = joystick;

    return joystick;
}

void disconnect_joystick(int joystick_id) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_joystick: g_instance is null");
    }

    if (joystick_id < 0 || GLFW_JOYSTICK_LAST < joystick_id) {
        throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_joystick: joystick_id {} is out of range", joystick_id));
    }

    if (g_joysticks.size() <= static_cast<std::size_t>(joystick_id)) {
        g_joysticks.resize(joystick_id + 1);
    }
    
    auto& joystick = g_joysticks[joystick_id];
    if (joystick) {
        joystick->id() = std::nullopt;
        joystick.reset();
    }
}

std::shared_ptr<gamepad_t> connect_gamepad(int joystick_id) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_gamepad: g_instance is null");
    }

    if (joystick_id < 0 || GLFW_JOYSTICK_LAST < joystick_id) {
        throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_gamepad: joystick_id {} is out of range", joystick_id));
    }

    const char* gamepad_name = glfwGetGamepadName(joystick_id);
    if (!gamepad_name) {
        throw std::runtime_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_gamepad: failed to get name for gamepad {}", joystick_id));
    }

    const char* gamepad_guid = glfwGetJoystickGUID(joystick_id);
    if (!gamepad_guid) {
        throw std::runtime_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::connect_gamepad: failed to get GUID for gamepad {}", joystick_id));
    }

    if (g_gamepads.size() <= static_cast<std::size_t>(joystick_id)) {
        g_gamepads.resize(joystick_id + 1);
    }

    auto gamepad = std::make_shared<gamepad_t>(2);

    gamepad->id() = joystick_id;
    gamepad->name() = gamepad_name;
    gamepad->guid() = gamepad_guid;
    
    g_gamepads[joystick_id] = gamepad;

    return gamepad;
}

void disconnect_gamepad(int joystick_id) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_gamepad: g_instance is null");
    }

    if (joystick_id < 0 || GLFW_JOYSTICK_LAST < joystick_id) {
        throw std::out_of_range(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::disconnect_gamepad: joystick_id {} is out of range", joystick_id));
    }

    if (g_gamepads.size() <= static_cast<std::size_t>(joystick_id)) {
        g_gamepads.resize(joystick_id + 1);
    }
    
    auto& gamepad = g_gamepads[joystick_id];
    if (gamepad) {
        gamepad->id() = std::nullopt;
        gamepad.reset();
    }
}

void glfw_monitor_callback(GLFWmonitor* monitor_handle, int event) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_monitor_callback: g_instance is null");
    }

    std::cout << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_monitor_callback: monitor: {}, event: {}", static_cast<void*>(monitor_handle), event) << std::endl;

    try {
        if (event == GLFW_CONNECTED) {
            connect_monitor(monitor_handle);
        } else if (event == GLFW_DISCONNECTED) {
            disconnect_monitor(monitor_handle);
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_monitor_callback: exception: {}", e.what()) << std::endl;
    }
}

void glfw_error_callback(int code, const char* description) {
    std::cerr << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_error_callback: code: {}, description: {}", code, description) << std::endl;
}

void glfw_joystick_callback(int jid, int event) {
    if (!g_instance) {
        throw std::logic_error("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_joystick_callback: g_instance is null");
    }

    std::cout << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_joystick_callback: jid: {}, event: {}", jid, event) << std::endl;

    try {
        if (event == GLFW_CONNECTED) {
            if (glfwJoystickIsGamepad(jid)) {
                connect_gamepad(jid);
            }
            connect_joystick(jid);
        } else if (event == GLFW_DISCONNECTED) {
            disconnect_gamepad(jid);
            disconnect_joystick(jid);
        }
    } catch (const std::exception& e) {
        std::cerr << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_joystick_callback: exception: {}", e.what()) << std::endl;
    }
}

} // namespace

glfw_t::glfw_t() {
    if (g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_t: only one instance of glfw_t is allowed");
    }
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_t: failed to initialize GLFW");
    }
    g_instance = this;

    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    std::cout << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_t: compile version: {}.{}.{}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION) << std::endl;
    std::cout << std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::glfw_t: runtime version: {}.{}.{}", major, minor, revision) << std::endl;

    glfwSetMonitorCallback(glfw_monitor_callback);
    glfwSetJoystickCallback(glfw_joystick_callback);

    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    for (int i = 0; i < count; ++i) {
        connect_monitor(monitors[i]);
    }

    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
        if (glfwJoystickPresent(jid)) {
            if (glfwJoystickIsGamepad(jid)) {
                connect_gamepad(jid);
            }
            connect_joystick(jid);
        }
    }
}

glfw_t::~glfw_t() {
    if (g_instance) {
        for (auto& joystick : g_joysticks) {
            if (joystick) {
                joystick->id() = std::nullopt;
                joystick.reset();
            }
        }
        g_joysticks.clear();

        for (auto& gamepad : g_gamepads) {
            if (gamepad) {
                gamepad->id() = std::nullopt;
                gamepad.reset();
            }
        }
        g_gamepads.clear();

        for (auto& [_, monitor] : g_monitor_by_glfw_monitor) {
            monitor->handle(nullptr);
        }
        g_monitor_by_glfw_monitor.clear();

        glfwSetJoystickCallback(nullptr);
        glfwSetMonitorCallback(nullptr);

        g_instance = nullptr;
        glfwTerminate();
        glfwSetErrorCallback(nullptr);
    }
}

void poll_events() {
    glfwPollEvents();
}

void wait_events() {
    glfwWaitEvents();
}

void wait_events_timeout(double timeout) {
    if (timeout < 0.0) {
        throw std::invalid_argument("m03gkcdy62bnz808pmk4uzkjra_glfw::wait_events_timeout: timeout must be non-negative");
    }

    glfwWaitEventsTimeout(timeout);
}

void post_empty_event() {
    glfwPostEmptyEvent();
}

std::shared_ptr<monitor_t> primary_monitor() {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::primary_monitor: GLFW is not initialized");
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        return nullptr;
    }

    return connect_monitor(monitor);
}

std::vector<std::shared_ptr<monitor_t>> monitors() {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::monitors: GLFW is not initialized");
    }

    std::vector<std::shared_ptr<monitor_t>> result;
    for (const auto& [_, monitor] : g_monitor_by_glfw_monitor) {
        result.push_back(monitor);
    }
    return result;
}

void poll_joystick(joystick_t& joystick) {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::poll_joystick: GLFW is not initialized");
    }

    auto joystick_id_opt = joystick.id();
    if (!joystick_id_opt) {
        return;
    }

    int joystick_id = *joystick_id_opt;

    auto& joystick_states = joystick.joystick_states();
    auto& joystick_state = joystick_states.stage();

    int axis_count = 0;
    const float* axes = glfwGetJoystickAxes(joystick_id, &axis_count);
    auto& axes_vector = joystick_state.axes();
    axes_vector.resize(axis_count);
    for (int i = 0; i < axis_count; ++i) {
        axes_vector[i] = axes[i];
    }

    int button_count = 0;
    const unsigned char* buttons = glfwGetJoystickButtons(joystick_id, &button_count);
    auto& button_states = joystick_state.buttons();
    button_states.resize(button_count);
    for (int i = 0; i < button_count; ++i) {
        button_states[i] = (buttons[i] == GLFW_PRESS);
    }

    int hat_count = 0;
    const unsigned char* hats = glfwGetJoystickHats(joystick_id, &hat_count);
    auto& hat_states = joystick_state.hats();
    hat_states.resize(hat_count);
    for (int i = 0; i < hat_count; ++i) {
        int hat_value = hats[i];
        int x = 0;
        int y = 0;
        if (hat_value & GLFW_HAT_UP) {
            y = 1;
        } else if (hat_value & GLFW_HAT_DOWN) {
            y = -1;
        }
        if (hat_value & GLFW_HAT_RIGHT) {
            x = 1;
        } else if (hat_value & GLFW_HAT_LEFT) {
            x = -1;
        }
        hat_states[i] = {x, y};
    }
}

std::vector<std::shared_ptr<joystick_t>> joysticks() {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::joysticks: GLFW is not initialized");
    }

    std::vector<std::shared_ptr<joystick_t>> result;

    for (const auto& joystick : g_joysticks) {
        if (joystick) {
            result.push_back(joystick);
        }
    }

    return result;
}

void poll_gamepad(gamepad_t& gamepad) {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::poll_gamepad: GLFW is not initialized");
    }

    auto gamepad_id_opt = gamepad.id();
    if (!gamepad_id_opt) {
        return;
    }

    int gamepad_id = *gamepad_id_opt;

    auto& gamepad_states = gamepad.gamepad_states();
    auto& gamepad_state = gamepad_states.stage();

    GLFWgamepadstate state;
    if (!glfwGetGamepadState(gamepad_id, &state)) {
        throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::poll_gamepad: failed to get gamepad state for gamepad {}", gamepad_id));
    }

    for (std::size_t i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
        const auto gamepad_button = glfw_gamepad_button_to_gamepad_button(static_cast<int>(i));
        gamepad_state.button_state(gamepad_button) = (state.buttons[i] == GLFW_PRESS);
    }

    for (std::size_t i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i) {
        const auto gamepad_axis = glfw_gamepad_axis_to_gamepad_axis(static_cast<int>(i));
        gamepad_state.axis_state(gamepad_axis) = state.axes[i];
    }
}

std::vector<std::shared_ptr<gamepad_t>> gamepads() {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::gamepads: GLFW is not initialized");
    }

    std::vector<std::shared_ptr<gamepad_t>> result;

    for (const auto& gamepad : g_gamepads) {
        if (gamepad) {
            result.push_back(gamepad);
        }
    }

    return result;
}

void update_joystick_to_gamepad_mapping(const joystick_to_gamepad_mapping_t& joystick_to_gamepad_mapping) {
    if (!g_instance) {
        throw std::runtime_error("m03gkcdy62bnz808pmk4uzkjra_glfw::update_joystick_to_gamepad_mapping: GLFW is not initialized");
    }
    
    std::string gamepad_mapping = joystick_to_gamepad_mapping.gamepad_mapping();
    if (glfwUpdateGamepadMappings(gamepad_mapping.c_str()) == GLFW_FALSE) {
        throw std::runtime_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_joystick_to_gamepad_mapping: failed to update gamepad mapping with mapping '{}'", gamepad_mapping));
    }

    for (const auto& joystick : g_joysticks) {
        if (!joystick) {
            continue;
        }

        if (!joystick->id()) {
            continue;
        }

        if (joystick->guid() != joystick_to_gamepad_mapping.gamepad_guid()) {
            continue;
        }

        int joystick_id = *joystick->id();

        disconnect_gamepad(joystick_id);

        if (glfwJoystickIsGamepad(joystick_id)) {
            connect_gamepad(joystick_id);
        }
    }
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw
