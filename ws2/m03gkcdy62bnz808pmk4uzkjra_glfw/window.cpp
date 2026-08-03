#include "window.h"

#include <format>

namespace {

m03gkcdy62bnz808pmk4uzkjra_glfw::button_t glfw_mouse_to_button_type(int glfw_mouse_button) {
    using button_t = m03gkcdy62bnz808pmk4uzkjra_glfw::button_t;

    switch (glfw_mouse_button) {
        case GLFW_MOUSE_BUTTON_LEFT: return button_t::button_mouse_left;
        case GLFW_MOUSE_BUTTON_RIGHT: return button_t::button_mouse_right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return button_t::button_mouse_middle;
        case GLFW_MOUSE_BUTTON_4: return button_t::button_mouse_4;
        case GLFW_MOUSE_BUTTON_5: return button_t::button_mouse_5;
        case GLFW_MOUSE_BUTTON_6: return button_t::button_mouse_6;
        case GLFW_MOUSE_BUTTON_7: return button_t::button_mouse_7;
        case GLFW_MOUSE_BUTTON_8: return button_t::button_mouse_8;
        default: throw std::invalid_argument(std::format("glfw_to_button_type: invalid mouse button {}", glfw_mouse_button));
    }
}

m03gkcdy62bnz808pmk4uzkjra_glfw::button_t glfw_key_to_button_type(int glfw_key) {
    using button_t = m03gkcdy62bnz808pmk4uzkjra_glfw::button_t;

    switch (glfw_key) {
        case GLFW_KEY_SPACE: return button_t::button_space;
        case GLFW_KEY_APOSTROPHE: return button_t::button_apostrophe;
        case GLFW_KEY_COMMA: return button_t::button_comma;
        case GLFW_KEY_MINUS: return button_t::button_minus;
        case GLFW_KEY_PERIOD: return button_t::button_period;
        case GLFW_KEY_SLASH: return button_t::button_slash;
        case GLFW_KEY_0: return button_t::button_0;
        case GLFW_KEY_1: return button_t::button_1;
        case GLFW_KEY_2: return button_t::button_2;
        case GLFW_KEY_3: return button_t::button_3;
        case GLFW_KEY_4: return button_t::button_4;
        case GLFW_KEY_5: return button_t::button_5;
        case GLFW_KEY_6: return button_t::button_6;
        case GLFW_KEY_7: return button_t::button_7;
        case GLFW_KEY_8: return button_t::button_8;
        case GLFW_KEY_9: return button_t::button_9;
        case GLFW_KEY_SEMICOLON: return button_t::button_semicolon;
        case GLFW_KEY_EQUAL: return button_t::button_equal;
        case GLFW_KEY_A: return button_t::button_a;
        case GLFW_KEY_B: return button_t::button_b;
        case GLFW_KEY_C: return button_t::button_c;
        case GLFW_KEY_D: return button_t::button_d;
        case GLFW_KEY_E: return button_t::button_e;
        case GLFW_KEY_F: return button_t::button_f;
        case GLFW_KEY_G: return button_t::button_g;
        case GLFW_KEY_H: return button_t::button_h;
        case GLFW_KEY_I: return button_t::button_i;
        case GLFW_KEY_J: return button_t::button_j;
        case GLFW_KEY_K: return button_t::button_k;
        case GLFW_KEY_L: return button_t::button_l;
        case GLFW_KEY_M: return button_t::button_m;
        case GLFW_KEY_N: return button_t::button_n;
        case GLFW_KEY_O: return button_t::button_o;
        case GLFW_KEY_P: return button_t::button_p;
        case GLFW_KEY_Q: return button_t::button_q;
        case GLFW_KEY_R: return button_t::button_r;
        case GLFW_KEY_S: return button_t::button_s;
        case GLFW_KEY_T: return button_t::button_t;
        case GLFW_KEY_U: return button_t::button_u;
        case GLFW_KEY_V: return button_t::button_v;
        case GLFW_KEY_W: return button_t::button_w;
        case GLFW_KEY_X: return button_t::button_x;
        case GLFW_KEY_Y: return button_t::button_y;
        case GLFW_KEY_Z: return button_t::button_z;
        case GLFW_KEY_LEFT_BRACKET: return button_t::button_left_bracket;
        case GLFW_KEY_BACKSLASH: return button_t::button_backslash;
        case GLFW_KEY_RIGHT_BRACKET: return button_t::button_right_bracket;
        case GLFW_KEY_GRAVE_ACCENT: return button_t::button_grave_accent;
        case GLFW_KEY_WORLD_1: return button_t::button_world_1;
        case GLFW_KEY_WORLD_2: return button_t::button_world_2;
        case GLFW_KEY_ESCAPE: return button_t::button_escape;
        case GLFW_KEY_ENTER: return button_t::button_enter;
        case GLFW_KEY_TAB: return button_t::button_tab;
        case GLFW_KEY_BACKSPACE: return button_t::button_backspace;
        case GLFW_KEY_INSERT: return button_t::button_insert;
        case GLFW_KEY_DELETE: return button_t::button_delete;
        case GLFW_KEY_RIGHT: return button_t::button_right;
        case GLFW_KEY_LEFT: return button_t::button_left;
        case GLFW_KEY_DOWN: return button_t::button_down;
        case GLFW_KEY_UP: return button_t::button_up;
        case GLFW_KEY_PAGE_UP: return button_t::button_page_up;
        case GLFW_KEY_PAGE_DOWN: return button_t::button_page_down;
        case GLFW_KEY_HOME: return button_t::button_home;
        case GLFW_KEY_END: return button_t::button_end;
        case GLFW_KEY_CAPS_LOCK: return button_t::button_caps_lock;
        case GLFW_KEY_SCROLL_LOCK: return button_t::button_scroll_lock;
        case GLFW_KEY_NUM_LOCK: return button_t::button_num_lock;
        case GLFW_KEY_PRINT_SCREEN: return button_t::button_print_screen;
        case GLFW_KEY_PAUSE: return button_t::button_pause;
        case GLFW_KEY_F1: return button_t::button_f1;
        case GLFW_KEY_F2: return button_t::button_f2;
        case GLFW_KEY_F3: return button_t::button_f3;
        case GLFW_KEY_F4: return button_t::button_f4;
        case GLFW_KEY_F5: return button_t::button_f5;
        case GLFW_KEY_F6: return button_t::button_f6;
        case GLFW_KEY_F7: return button_t::button_f7;
        case GLFW_KEY_F8: return button_t::button_f8;
        case GLFW_KEY_F9: return button_t::button_f9;
        case GLFW_KEY_F10: return button_t::button_f10;
        case GLFW_KEY_F11: return button_t::button_f11;
        case GLFW_KEY_F12: return button_t::button_f12;
        case GLFW_KEY_F13: return button_t::button_f13;
        case GLFW_KEY_F14: return button_t::button_f14;
        case GLFW_KEY_F15: return button_t::button_f15;
        case GLFW_KEY_F16: return button_t::button_f16;
        case GLFW_KEY_F17: return button_t::button_f17;
        case GLFW_KEY_F18: return button_t::button_f18;
        case GLFW_KEY_F19: return button_t::button_f19;
        case GLFW_KEY_F20: return button_t::button_f20;
        case GLFW_KEY_F21: return button_t::button_f21;
        case GLFW_KEY_F22: return button_t::button_f22;
        case GLFW_KEY_F23: return button_t::button_f23;
        case GLFW_KEY_F24: return button_t::button_f24;
        case GLFW_KEY_F25: return button_t::button_f25;
        case GLFW_KEY_KP_0: return button_t::button_keypad_0;
        case GLFW_KEY_KP_1: return button_t::button_keypad_1;
        case GLFW_KEY_KP_2: return button_t::button_keypad_2;
        case GLFW_KEY_KP_3: return button_t::button_keypad_3;
        case GLFW_KEY_KP_4: return button_t::button_keypad_4;
        case GLFW_KEY_KP_5: return button_t::button_keypad_5;
        case GLFW_KEY_KP_6: return button_t::button_keypad_6;
        case GLFW_KEY_KP_7: return button_t::button_keypad_7;
        case GLFW_KEY_KP_8: return button_t::button_keypad_8;
        case GLFW_KEY_KP_9: return button_t::button_keypad_9;
        case GLFW_KEY_KP_DECIMAL: return button_t::button_keypad_decimal;
        case GLFW_KEY_KP_DIVIDE: return button_t::button_keypad_divide;
        case GLFW_KEY_KP_MULTIPLY: return button_t::button_keypad_multiply;
        case GLFW_KEY_KP_SUBTRACT: return button_t::button_keypad_subtract;
        case GLFW_KEY_KP_ADD: return button_t::button_keypad_add;
        case GLFW_KEY_KP_ENTER: return button_t::button_keypad_enter;
        case GLFW_KEY_KP_EQUAL: return button_t::button_keypad_equal;
        case GLFW_KEY_LEFT_SHIFT: return button_t::button_left_shift;
        case GLFW_KEY_LEFT_CONTROL: return button_t::button_left_control;
        case GLFW_KEY_LEFT_ALT: return button_t::button_left_alt;
        case GLFW_KEY_LEFT_SUPER: return button_t::button_left_super;
        case GLFW_KEY_RIGHT_SHIFT: return button_t::button_right_shift;
        case GLFW_KEY_RIGHT_CONTROL: return button_t::button_right_control;
        case GLFW_KEY_RIGHT_ALT: return button_t::button_right_alt;
        case GLFW_KEY_RIGHT_SUPER: return button_t::button_right_super;
        case GLFW_KEY_MENU: return button_t::button_menu;
        default: throw std::runtime_error(std::format("glfw_to_button: unknown GLFW key: {}", glfw_key));
    }
}

void window_position_callback(GLFWwindow* glfw_window, int xpos, int ypos) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    const auto& callback = window->position_callback();
    if (callback) {
        callback(window, xpos, ypos);
    }
}

void window_size_callback(GLFWwindow* glfw_window, int width, int height) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->size_callback(); callback) {
        callback(window, width, height);
    }
}

void window_close_callback(GLFWwindow* glfw_window) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    const auto& callback = window->close_callback();
    if (callback) {
        callback(window);
    }
}

void window_refresh_callback(GLFWwindow* glfw_window) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    const auto& callback = window->refresh_callback();
    if (callback) {
        callback(window);
    }
}

void window_focus_callback(GLFWwindow* glfw_window, int focused) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->focus_callback(); callback) {
        callback(window, focused == GLFW_TRUE);
    }
}

void window_iconify_callback(GLFWwindow* glfw_window, int iconified) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->iconify_callback(); callback) {
        callback(window, iconified == GLFW_TRUE);
    }
}

void window_maximize_callback(GLFWwindow* glfw_window, int maximized) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->maximize_callback(); callback) {
        callback(window, maximized == GLFW_TRUE);
    }
}

void window_framebuffer_size_callback(GLFWwindow* glfw_window, int width, int height) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->framebuffer_size_callback(); callback) {
        callback(window, width, height);
    }
}

void window_content_scale_callback(GLFWwindow* glfw_window, float xscale, float yscale) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->content_scale_callback(); callback) {
        callback(window, xscale, yscale);
    }
}

void window_mouse_button_callback(GLFWwindow* glfw_window, int button, int action, int mods) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);

    auto& input_states = window->input_states();
    auto& input_state = input_states.stage();
    const auto button_type = glfw_mouse_to_button_type(button);
    auto& button_state = input_state.button_state(button_type);
    switch (action) {
    case GLFW_PRESS: {
        button_state.is_down() = true;
        ++button_state.transition_count();
    } break;
    case GLFW_RELEASE: {
        button_state.is_down() = false;
        ++button_state.transition_count();
    } break;
    default: throw std::runtime_error(std::format("window_mouse_button_callback: unknown GLFW mouse button action: {}", action));
    }

    if (const auto& callback = window->mouse_button_callback(); callback) {
        callback(window, button, action, mods);
    }
}

void window_cursor_position_callback(GLFWwindow* glfw_window, double xpos, double ypos) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);

    auto& input_states = window->input_states();
    auto& input_state = input_states.stage();
    input_state.cursor_position() = {xpos, ypos};

    if (const auto& callback = window->cursor_position_callback(); callback) {
        callback(window, xpos, ypos);
    }
}

void window_cursor_enter_callback(GLFWwindow* glfw_window, int entered) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->cursor_enter_callback(); callback) {
        callback(window, entered == GLFW_TRUE);
    }
}

void window_scroll_callback(GLFWwindow* glfw_window, double xoffset, double yoffset) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);

    auto& input_states = window->input_states();
    auto& input_state = input_states.stage();
    input_state.scroll_offset() += {xoffset, yoffset};

    if (const auto& callback = window->scroll_callback(); callback) {
        callback(window, xoffset, yoffset);
    }
}

void window_key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);

    if (key != GLFW_KEY_UNKNOWN) {
        auto& input_states = window->input_states();
        auto& input_state = input_states.stage();
        const auto button_type = glfw_key_to_button_type(key);
        auto& button_state = input_state.button_state(button_type);
        switch (action) {
        case GLFW_PRESS: {
            button_state.is_down() = true;
            ++button_state.transition_count();
        } break;
        case GLFW_RELEASE: {
            button_state.is_down() = false;
            ++button_state.transition_count();
        } break;
        case GLFW_REPEAT: {
            ++button_state.repeat_count();
        } break;
        default: throw std::runtime_error(std::format("window_key_callback: unknown GLFW key action: {}", action));
        }
    }

    if (const auto& callback = window->key_callback(); callback) {
        callback(window, key, scancode, action, mods);
    }
}

void window_char_callback(GLFWwindow* glfw_window, unsigned int codepoint) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->char_callback(); callback) {
        callback(window, codepoint);
    }
}

void window_drop_callback(GLFWwindow* glfw_window, int count, const char** paths) {
    void* user_pointer = glfwGetWindowUserPointer(glfw_window);
    if (!user_pointer) {
        return;
    }

    m03gkcdy62bnz808pmk4uzkjra_glfw::window_t* window = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::window_t*>(user_pointer);
    if (const auto& callback = window->drop_callback(); callback) {
        std::vector<std::string> path_vector(paths, paths + count);
        callback(window, path_vector);
    }
}

} // namespace

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

GLFWglproc get_proc_address(const char* proc_name) {
    return glfwGetProcAddress(proc_name);
}

std::shared_ptr<window_t> window_t::create(const std::string& title, const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4>& rect, const window_creation_settings_t& settings) {
    settings.apply();

    glfwWindowHint(GLFW_POSITION_X, rect[0]);
    glfwWindowHint(GLFW_POSITION_Y, rect[1]);

    GLFWwindow* handle = glfwCreateWindow(rect[2], rect[3], title.c_str(), nullptr, nullptr);

    return create_internal(handle);
}

std::shared_ptr<window_t> window_t::create(const std::string& title, const monitor_t& monitor, const video_mode_t& video_mode, const window_creation_settings_t& settings) {
    settings.apply();

    glfwWindowHint(GLFW_RED_BITS, video_mode.red_bits);
    glfwWindowHint(GLFW_GREEN_BITS, video_mode.green_bits);
    glfwWindowHint(GLFW_BLUE_BITS, video_mode.blue_bits);
    glfwWindowHint(GLFW_REFRESH_RATE, video_mode.refresh_rate);

    GLFWmonitor* glfw_monitor = monitor.handle();
    GLFWwindow* handle = glfwCreateWindow(video_mode.width, video_mode.height, title.c_str(), glfw_monitor, nullptr);

    return create_internal(handle);
}

window_t::~window_t() {
    if (m_cursor_image) {
        glfwDestroyCursor(m_cursor_image);
        m_cursor_image = nullptr;
    }

    glfwDestroyWindow(m_handle);
}

GLFWwindow* window_t::handle() const {
    return m_handle;
}

m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<input_state_t>& window_t::input_states() {
    return m_input_states;
}

const m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<input_state_t>& window_t::input_states() const {
    return m_input_states;
}

void window_t::cursor_image(const image_t& image, const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& hotspot) {
    if (m_cursor_image) {
        glfwDestroyCursor(m_cursor_image);
        m_cursor_image = nullptr;
    }

    GLFWimage glfw_image;
    glfw_image.width = image.width;
    glfw_image.height = image.height;
    glfw_image.pixels = image.data;

    m_cursor_image = glfwCreateCursor(&glfw_image, hotspot[0], hotspot[1]);
    glfwSetCursor(m_handle, m_cursor_image);
}

void window_t::reset_cursor_image() {
    if (m_cursor_image) {
        glfwDestroyCursor(m_cursor_image);
        m_cursor_image = nullptr;
    }
    glfwSetCursor(m_handle, nullptr);
}

void window_t::cursor_mode(bool visible, bool locked) {
    int glfw_cursor_mode = 0;
    if (visible) {
        if (locked) {
            glfw_cursor_mode = GLFW_CURSOR_CAPTURED;
        } else {
            glfw_cursor_mode = GLFW_CURSOR_NORMAL;
        }
    } else {
        if (locked) {
            glfw_cursor_mode = GLFW_CURSOR_DISABLED;
        } else {
            glfw_cursor_mode = GLFW_CURSOR_HIDDEN;
        }
    }

    glfwSetInputMode(m_handle, GLFW_CURSOR, glfw_cursor_mode);
}

bool window_t::cursor_visible() const {
    const auto glfw_cursor_mode = glfwGetInputMode(m_handle, GLFW_CURSOR);
    return glfw_cursor_mode == GLFW_CURSOR_NORMAL || glfw_cursor_mode == GLFW_CURSOR_CAPTURED;
}

bool window_t::cursor_locked() const {
    const auto glfw_cursor_mode = glfwGetInputMode(m_handle, GLFW_CURSOR);
    return glfw_cursor_mode == GLFW_CURSOR_DISABLED || glfw_cursor_mode == GLFW_CURSOR_CAPTURED;
}

bool window_t::cursor_raw_motion(bool value) {
    if (!glfwRawMouseMotionSupported()) {
        return false;
    }

    glfwSetInputMode(m_handle, GLFW_RAW_MOUSE_MOTION, value ? GLFW_TRUE : GLFW_FALSE);

    return true;
}

bool window_t::cursor_raw_motion() {
    return glfwGetInputMode(m_handle, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
}

bool window_t::cursor_is_in_content_area() const {
    return glfwGetWindowAttrib(m_handle, GLFW_HOVERED) == GLFW_TRUE;
}

client_api_t window_t::client_api() const {
    int api = glfwGetWindowAttrib(m_handle, GLFW_CLIENT_API);
    switch (api) {
        case GLFW_NO_API: return client_api_t::none;
        case GLFW_OPENGL_API: return client_api_t::opengl;
        case GLFW_OPENGL_ES_API: return client_api_t::opengl_es;
        default: throw std::runtime_error(std::format("Unknown client API: {}", api));
    }
}

bool window_t::should_close() const {
    return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
}

void window_t::should_close(bool value) const {
    glfwSetWindowShouldClose(m_handle, value ? GLFW_TRUE : GLFW_FALSE);
}

std::string window_t::title() const {
    const char* title = glfwGetWindowTitle(m_handle);
    return title;
}

void window_t::title(const std::string& title) {
    glfwSetWindowTitle(m_handle, title.c_str());
}

void window_t::fullscreen(const monitor_t& monitor) {
    GLFWmonitor* glfw_monitor = monitor.handle();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor.handle());
    glfwSetWindowMonitor(m_handle, glfw_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

bool window_t::fullscreen() const {
    return glfwGetWindowMonitor(m_handle) != nullptr;
}

void window_t::windowed(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4>& work_area) {
    glfwSetWindowMonitor(m_handle, nullptr, work_area[0], work_area[1], work_area[2], work_area[3], 0);
}

bool window_t::windowed() const {
    return glfwGetWindowMonitor(m_handle) == nullptr;
}

void window_t::maximize() {
    glfwMaximizeWindow(m_handle);
}

bool window_t::maximized() const {
    return glfwGetWindowAttrib(m_handle, GLFW_MAXIMIZED) == GLFW_TRUE;
}

void window_t::minimize() {
    glfwIconifyWindow(m_handle);
}

bool window_t::minimized() const {
    return glfwGetWindowAttrib(m_handle, GLFW_ICONIFIED) == GLFW_TRUE;
}

void window_t::restore() {
    glfwRestoreWindow(m_handle);
}

void window_t::size(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& size) {
    glfwSetWindowSize(m_handle, size[0], size[1]);
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> window_t::size() const {
    int width, height;
    glfwGetWindowSize(m_handle, &width, &height);
    return {width, height};
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> window_t::framebuffer_size() const {
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return {width, height};
}

void window_t::size_limits(
    std::optional<int> min_width,
    std::optional<int> min_height,
    std::optional<int> max_width,
    std::optional<int> max_height
) {
    glfwSetWindowSizeLimits(
        m_handle,
        min_width.value_or(GLFW_DONT_CARE),
        min_height.value_or(GLFW_DONT_CARE),
        max_width.value_or(GLFW_DONT_CARE),
        max_height.value_or(GLFW_DONT_CARE)
    );
}

void window_t::aspect_ratio(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& ratio) {
    glfwSetWindowAspectRatio(m_handle, ratio[0], ratio[1]);
}

void window_t::clear_aspect_ratio() {
    glfwSetWindowAspectRatio(m_handle, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void window_t::visible(bool value) {
    if (value) {
        glfwShowWindow(m_handle);
    } else {
        glfwHideWindow(m_handle);
    }
}

bool window_t::visible() const {
    return glfwGetWindowAttrib(m_handle, GLFW_VISIBLE) == GLFW_TRUE;
}

void window_t::focus_on_visible(bool value) {
    glfwSetWindowAttrib(m_handle, GLFW_FOCUS_ON_SHOW, value ? GLFW_TRUE : GLFW_FALSE);
}

bool window_t::focus_on_visible() const {
    return glfwGetWindowAttrib(m_handle, GLFW_FOCUS_ON_SHOW) == GLFW_TRUE;
}

void window_t::focus() {
    glfwFocusWindow(m_handle);
}

bool window_t::focused() const {
    return glfwGetWindowAttrib(m_handle, GLFW_FOCUSED) == GLFW_TRUE;
}

void window_t::opacity(float value) {
    glfwSetWindowOpacity(m_handle, value);
}

float window_t::opacity() const {
    return glfwGetWindowOpacity(m_handle);
}

void window_t::request_attention() {
    glfwRequestWindowAttention(m_handle);
}

void window_t::icon(std::span<const image_t> images) {
    std::vector<GLFWimage> glfw_images;
    glfw_images.reserve(images.size());

    for (const auto& img : images) {
        GLFWimage image;
        image.width = img.width;
        image.height = img.height;
        image.pixels = img.data;
        glfw_images.push_back(image);
    }

    glfwSetWindowIcon(m_handle, static_cast<int>(glfw_images.size()), glfw_images.data());
}

bool window_t::transparent_framebuffer() const {
    return glfwGetWindowAttrib(m_handle, GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE;
}

bool window_t::context_current(bool value) const {
    return context_current_internal(value);
}

bool window_t::context_current() const {
    return glfwGetCurrentContext() == m_handle;
}

void window_t::swap_interval(int interval) {
    if (!context_current_internal(true)) {
        return;
    }

    glfwSwapInterval(interval);
}

void window_t::swap_buffers() {
    if (!context_current_internal(true)) {
        return;
    }

    glfwSwapBuffers(m_handle);
}

bool window_t::extension_supported(const std::string& extension_name) const {
    if (!context_current_internal(true)) {
        return false;
    }

    return glfwExtensionSupported(extension_name.c_str()) == GLFW_TRUE;
}

void window_t::position_callback(std::function<void(window_t*, int, int)> callback) {
    m_position_callback = std::move(callback);
}

const std::function<void(window_t*, int, int)>& window_t::position_callback() const {
    return m_position_callback;
}

void window_t::size_callback(std::function<void(window_t*, int, int)> callback) {
    m_size_callback = std::move(callback);
}

const std::function<void(window_t*, int, int)>& window_t::size_callback() const {
    return m_size_callback;
}

void window_t::close_callback(std::function<void(window_t*)> callback) {
    m_close_callback = std::move(callback);
}

const std::function<void(window_t*)>& window_t::close_callback() const {
    return m_close_callback;
}

void window_t::refresh_callback(std::function<void(window_t*)> callback) {
    m_refresh_callback = std::move(callback);
}

const std::function<void(window_t*)>& window_t::refresh_callback() const {
    return m_refresh_callback;
}

void window_t::focus_callback(std::function<void(window_t*, bool)> callback) {
    m_focus_callback = std::move(callback);
}

const std::function<void(window_t*, bool)>& window_t::focus_callback() const {
    return m_focus_callback;
}

void window_t::iconify_callback(std::function<void(window_t*, bool)> callback) {
    m_iconify_callback = std::move(callback);
}

const std::function<void(window_t*, bool)>& window_t::iconify_callback() const {
    return m_iconify_callback;
}

void window_t::maximize_callback(std::function<void(window_t*, bool)> callback) {
    m_maximize_callback = std::move(callback);
}

const std::function<void(window_t*, bool)>& window_t::maximize_callback() const {
    return m_maximize_callback;
}

void window_t::framebuffer_size_callback(std::function<void(window_t*, int, int)> callback) {
    m_framebuffer_size_callback = std::move(callback);
}

const std::function<void(window_t*, int, int)>& window_t::framebuffer_size_callback() const {
    return m_framebuffer_size_callback;
}

void window_t::content_scale_callback(std::function<void(window_t*, float, float)> callback) {
    m_content_scale_callback = std::move(callback);
}

const std::function<void(window_t*, float, float)>& window_t::content_scale_callback() const {
    return m_content_scale_callback;
}

void window_t::mouse_button_callback(std::function<void(window_t*, int, int, int)> callback) {
    m_mouse_button_callback = std::move(callback);
}

const std::function<void(window_t*, int, int, int)>& window_t::mouse_button_callback() const {
    return m_mouse_button_callback;
}

void window_t::cursor_position_callback(std::function<void(window_t*, double, double)> callback) {
    m_cursor_position_callback = std::move(callback);
}

const std::function<void(window_t*, double, double)>& window_t::cursor_position_callback() const {
    return m_cursor_position_callback;
}

void window_t::cursor_enter_callback(std::function<void(window_t*, bool)> callback) {
    m_cursor_enter_callback = std::move(callback);
}

const std::function<void(window_t*, bool)>& window_t::cursor_enter_callback() const {
    return m_cursor_enter_callback;
}

void window_t::scroll_callback(std::function<void(window_t*, double, double)> callback) {
    m_scroll_callback = std::move(callback);
}

const std::function<void(window_t*, double, double)>& window_t::scroll_callback() const {
    return m_scroll_callback;
}

void window_t::key_callback(std::function<void(window_t*, int, int, int, int)> callback) {
    m_key_callback = std::move(callback);
}

const std::function<void(window_t*, int, int, int, int)>& window_t::key_callback() const {
    return m_key_callback;
}

void window_t::char_callback(std::function<void(window_t*, std::uint32_t)> callback) {
    m_char_callback = std::move(callback);
}

const std::function<void(window_t*, std::uint32_t)>& window_t::char_callback() const {
    return m_char_callback;
}

void window_t::drop_callback(std::function<void(window_t*, const std::vector<std::string>&)> callback) {
    m_drop_callback = std::move(callback);
}

const std::function<void(window_t*, const std::vector<std::string>&)>& window_t::drop_callback() const {
    return m_drop_callback;
}

window_t::window_t(GLFWwindow* handle):
    m_handle(handle),
    m_input_states(4),
    m_cursor_image(nullptr)
{
}

std::shared_ptr<window_t> window_t::create_internal(GLFWwindow* handle) {
    if (!handle) {
        return nullptr;
    }
    auto result = std::shared_ptr<window_t>(new window_t(handle));

    glfwSetWindowUserPointer(handle, result.get());

    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> cursor_position;
    glfwGetCursorPos(handle, &cursor_position[0], &cursor_position[1]);
    auto& input_states = result->input_states();
    auto& input_state = input_states.stage();
    input_state.cursor_position() = cursor_position;
    input_state.scroll_offset() = {0.0, 0.0};
    for (std::size_t i = 0; i < static_cast<std::size_t>(m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::_button_count); ++i) {
        button_t button_type = static_cast<button_t>(i);
        auto& button_state = input_state.button_state(button_type);
        button_state.is_down() = false;
        button_state.transition_count() = 0;
        button_state.repeat_count() = 0;
    }

    glfwSetWindowPosCallback(result->m_handle, window_position_callback);
    glfwSetWindowSizeCallback(result->m_handle, window_size_callback);
    glfwSetWindowCloseCallback(result->m_handle, window_close_callback);
    glfwSetWindowRefreshCallback(result->m_handle, window_refresh_callback);
    glfwSetWindowFocusCallback(result->m_handle, window_focus_callback);
    glfwSetWindowIconifyCallback(result->m_handle, window_iconify_callback);
    glfwSetWindowMaximizeCallback(result->m_handle, window_maximize_callback);
    glfwSetFramebufferSizeCallback(result->m_handle, window_framebuffer_size_callback);
    glfwSetWindowContentScaleCallback(result->m_handle, window_content_scale_callback);

    glfwSetMouseButtonCallback(result->m_handle, window_mouse_button_callback);
    glfwSetCursorPosCallback(result->m_handle, window_cursor_position_callback);
    glfwSetCursorEnterCallback(result->m_handle, window_cursor_enter_callback);
    glfwSetScrollCallback(result->m_handle, window_scroll_callback);
    glfwSetKeyCallback(result->m_handle, window_key_callback);
    glfwSetCharCallback(result->m_handle, window_char_callback);
    glfwSetDropCallback(result->m_handle, window_drop_callback);

    return result;
}

bool window_t::context_current_internal(bool value) const {
    if (glfwGetWindowAttrib(m_handle, GLFW_CLIENT_API) == GLFW_NO_API) {
        return false;
    }

    auto current_context = glfwGetCurrentContext();

    if (value) {
        if (current_context != m_handle) {
            glfwMakeContextCurrent(m_handle);
        }

        return true;
    } else {
        if (current_context == m_handle) {
            glfwMakeContextCurrent(nullptr);
        }

        return false;
    }
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw
