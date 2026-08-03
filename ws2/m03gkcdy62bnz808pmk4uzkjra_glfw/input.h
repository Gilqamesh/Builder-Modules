#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_INPUT_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_INPUT_H

# include <array>
# include <format>
# include <stdexcept>
# include <cstdint>
# include <unordered_map>
# include <map>
# include <optional>
# include <string>
# include <vector>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>
# include <m03gli1rb5p56mncplipxpf3he_ring_buffer/api.h>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

/**
 * @brief Identifies a supported input button.
 */
enum class button_t : std::uint32_t {
    // Printable keys.
    button_space, button_apostrophe, button_comma, button_minus, button_period, button_slash,
    button_0, button_1, button_2, button_3, button_4, button_5, button_6, button_7, button_8, button_9,
    button_semicolon, button_equal,
    button_a, button_b, button_c, button_d, button_e, button_f, button_g, button_h, button_i,
    button_j, button_k, button_l, button_m, button_n, button_o, button_p, button_q, button_r,
    button_s, button_t, button_u, button_v, button_w, button_x, button_y, button_z,
    button_left_bracket, button_backslash, button_right_bracket, button_grave_accent,

    // Non-US keys.
    button_world_1, button_world_2,

    // Navigation and editing.
    button_escape, button_enter, button_tab, button_backspace, button_insert, button_delete,
    button_right, button_left, button_down, button_up, button_page_up, button_page_down,
    button_home, button_end,

    // Lock and system keys.
    button_caps_lock, button_scroll_lock, button_num_lock, button_print_screen, button_pause,

    // Function keys.
    button_f1, button_f2, button_f3, button_f4, button_f5, button_f6, button_f7, button_f8, button_f9,
    button_f10, button_f11, button_f12, button_f13, button_f14, button_f15, button_f16, button_f17,
    button_f18, button_f19, button_f20, button_f21, button_f22, button_f23, button_f24, button_f25,

    // Keypad.
    button_keypad_0, button_keypad_1, button_keypad_2, button_keypad_3, button_keypad_4,
    button_keypad_5, button_keypad_6, button_keypad_7, button_keypad_8, button_keypad_9,
    button_keypad_decimal, button_keypad_divide, button_keypad_multiply,
    button_keypad_subtract, button_keypad_add, button_keypad_enter, button_keypad_equal,

    // Modifier keys.
    button_left_shift, button_left_control, button_left_alt, button_left_super,
    button_right_shift, button_right_control, button_right_alt, button_right_super,
    button_menu,

    // Mouse buttons.
    button_mouse_left, button_mouse_right, button_mouse_middle,
    button_mouse_4, button_mouse_5, button_mouse_6, button_mouse_7, button_mouse_8,

    _button_count // Number of button values; not a valid button.
};

/**
 * @brief Stores one button's position and cumulative transition and repeat counts.
 *
 * Comparable snapshots must share the same counter origin and implicit initial position.
 */
class button_state_t {
public:
    /**
     * @brief Returns mutable access to the cumulative number of transitions between the up and down positions.
     */
    std::int32_t& transition_count();

    /**
     * @brief Returns the cumulative number of transitions between the up and down positions.
     */
    std::int32_t transition_count() const;

    /**
     * @brief Returns mutable access to the cumulative number of repeat events.
     */
    std::int32_t& repeat_count();

    /**
     * @brief Returns the cumulative number of repeat events.
     */
    std::int32_t repeat_count() const;

    /**
     * @brief Returns mutable access to whether the button is in the down position.
     */
    bool& is_down();

    /**
     * @brief Returns whether the button is in the down position.
     */
    bool is_down() const;

private:
    std::int32_t m_transition_count;
    std::int32_t m_repeat_count;
    bool m_is_down;
};

/**
 * @brief Stores one cumulative keyboard and mouse input snapshot.
 */
class input_state_t {
public:
    using button_states_t = std::array<button_state_t, static_cast<std::size_t>(button_t::_button_count)>;

public:
    /**
     * @brief Returns the state of the specified button.
     */
    button_state_t& button_state(button_t button);
    const button_state_t& button_state(button_t button) const;
    
    /**
     * @brief Returns the absolute cursor position.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& cursor_position();
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& cursor_position() const;
    
    /**
     * @brief Returns the cumulative scroll offset.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& scroll_offset();
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& scroll_offset() const;
    
private:
    button_states_t m_button_states;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> m_cursor_position;
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> m_scroll_offset;
};

/**
 * @brief Provides signed input changes between two snapshots of the same input history.
 *
 * Both snapshots must remain unchanged and outlive this view.
 */
class input_state_change_t {
public:
    /**
     * @brief Constructs an input change view from previous and current snapshots in either chronological order.
     */
    explicit input_state_change_t(const input_state_t& previous_state, const input_state_t& current_state);

    /**
     * @brief Returns whether at least one press separates the snapshots.
     */
    bool was_pressed(button_t button) const;
    
    /**
     * @brief Returns whether at least one release separates the snapshots.
     */
    bool was_released(button_t button) const;

    /**
     * @brief Returns the signed number of presses from the previous snapshot to the current snapshot.
     */
    std::int32_t press_delta(button_t button) const;

    /**
     * @brief Returns the signed number of releases from the previous snapshot to the current snapshot.
     */
    std::int32_t release_delta(button_t button) const;

    /**
     * @brief Returns the signed number of repeat events from the previous snapshot to the current snapshot.
     */
    std::int32_t repeat_delta(button_t button) const;

    /**
     * @brief Returns the current cursor position minus the previous cursor position.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> cursor_position_delta() const;

    /**
     * @brief Returns the current cumulative scroll offset minus the previous cumulative scroll offset.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> scroll_offset_delta() const;

private:
    struct button_delta_t {
        std::int32_t press_delta;
        std::int32_t release_delta;
    };

private:
    button_delta_t button_delta(button_t button) const;

private:
    const input_state_t& m_previous_state;
    const input_state_t& m_current_state;
};

/**
 * @brief Stores one raw joystick snapshot.
 */
class joystick_state_t {
public:
    using hat_states_t = std::vector<m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>>;

public:
    /**
     * @brief Returns the axis positions in GLFW order.
     */
    std::vector<float>& axes();
    const std::vector<float>& axes() const;

    /**
     * @brief Returns the button positions in GLFW order.
     */
    std::vector<bool>& buttons();
    const std::vector<bool>& buttons() const;

    /**
     * @brief Returns the hat directions in GLFW order.
     */
    hat_states_t& hats();
    const hat_states_t& hats() const;

private:
    std::vector<float> m_axes;
    std::vector<bool> m_buttons;
    hat_states_t m_hats;
};

/**
 * @brief Represents one joystick connection and its state history.
 *
 * An empty ID means disconnected, and only the module runtime should modify the ID, name or GUID.
 */
class joystick_t {
public:
    using joystick_states_t = m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<joystick_state_t>;

public:
    /**
     * @brief Constructs a disconnected joystick with a state history of the specified size.
     */
    explicit joystick_t(size_t joystick_states_size);

    /**
     * @brief Returns the joystick ID, or an empty value if disconnected.
     */
    std::optional<int>& id();
    const std::optional<int>& id() const;

    /**
     * @brief Returns the last known human-readable joystick name.
     */
    std::string& name();
    const std::string& name() const;

    /**
     * @brief Returns the last known joystick GUID.
     */
    std::string& guid();
    const std::string& guid() const;

    /**
     * @brief Returns the joystick state history whose staging snapshot is written by poll_joystick.
     */
    joystick_states_t& joystick_states();
    const joystick_states_t& joystick_states() const;

private:
    std::optional<int> m_id;
    std::string m_name;
    std::string m_guid;
    joystick_states_t m_joystick_states;
};

/**
 * @brief Provides changes between two raw joystick snapshots with matching element counts.
 *
 * Both snapshots must remain unchanged and outlive this view.
 */
class joystick_state_change_t {
public:
    /**
     * @brief Constructs a non-owning change view from previous and current snapshots.
     */
    explicit joystick_state_change_t(const joystick_state_t& previous_state, const joystick_state_t& current_state);

    /**
     * @brief Returns the shared number of axes.
     */
    std::size_t axis_count() const;

    /**
     * @brief Returns the current axis position minus the previous position.
     */
    float axis_delta(std::size_t axis_index) const;

    /**
     * @brief Returns the shared number of buttons.
     */
    std::size_t button_count() const;

    /**
     * @brief Returns whether the specified button changed from released to pressed.
     */
    bool was_pressed(std::size_t button_index) const;

    /**
     * @brief Returns whether the specified button changed from pressed to released.
     */
    bool was_released(std::size_t button_index) const;

    /**
     * @brief Returns the shared number of hats.
     */
    std::size_t hat_count() const;

    /**
     * @brief Returns the current hat direction minus the previous direction.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> hat_delta(std::size_t hat_index) const;

private:
    const joystick_state_t& m_previous_state;
    const joystick_state_t& m_current_state;    
};

/**
 * @brief Identifies a standardized gamepad button.
 */
enum class gamepad_button_t : std::uint32_t {
    button_a, button_cross = button_a,
    button_b, button_circle = button_b,
    button_x, button_square = button_x,
    button_y, button_triangle = button_y,
    button_left_bumper, button_right_bumper,
    button_back, button_start,
    button_guide,
    button_left_thumb, button_right_thumb,
    button_dpad_up, button_dpad_right, button_dpad_down, button_dpad_left,

    _button_count // Number of gamepad_button_t values; not a valid gamepad_button_t.
};

/**
 * @brief Identifies a standardized gamepad axis.
 */
enum class gamepad_axis_t : std::uint32_t {
    axis_left_x, axis_left_y,
    axis_right_x, axis_right_y,
    axis_left_trigger, axis_right_trigger,

    _axis_count // Number of gamepad_axis_t values; not a valid gamepad_axis_t.
};

/**
 * @brief Stores one standardized gamepad snapshot.
 */
class gamepad_state_t {
public:
    /**
     * @brief Returns the position of the specified gamepad button.
     */
    bool& button_state(gamepad_button_t button);
    bool button_state(gamepad_button_t button) const;

    /**
     * @brief Returns the position of the specified gamepad axis.
     */
    float& axis_state(gamepad_axis_t axis);
    float axis_state(gamepad_axis_t axis) const;

private:
    std::array<bool, static_cast<std::size_t>(gamepad_button_t::_button_count)> m_button_states;
    std::array<float, static_cast<std::size_t>(gamepad_axis_t::_axis_count)> m_axis_states;
};

/**
 * @brief Represents one mapped gamepad connection and its state history.
 *
 * An empty ID means disconnected, and only the module runtime should modify the ID, name or GUID.
 */
class gamepad_t {
public:
    using gamepad_states_t = m03gli1rb5p56mncplipxpf3he_ring_buffer::ring_buffer_t<gamepad_state_t>;

public:
    /**
     * @brief Constructs a disconnected gamepad with a state history of the specified size.
     */
    explicit gamepad_t(std::size_t gamepad_states_size);

    /**
     * @brief Returns the joystick ID, or an empty value if disconnected.
     */
    std::optional<int>& id();
    const std::optional<int>& id() const;

    /**
     * @brief Returns the last known mapped gamepad name.
     */
    std::string& name();
    const std::string& name() const;

    /**
     * @brief Returns the last known joystick GUID.
     */
    std::string& guid();
    const std::string& guid() const;

    /**
     * @brief Returns the gamepad state history whose staging snapshot is written by poll_gamepad.
     */
    gamepad_states_t& gamepad_states();
    const gamepad_states_t& gamepad_states() const;

private:
    std::optional<int> m_id;
    std::string m_name;
    std::string m_guid;
    gamepad_states_t m_gamepad_states;
};

/**
 * @brief Provides changes between two standardized gamepad snapshots.
 *
 * Both snapshots must remain unchanged and outlive this view.
 */
class gamepad_state_change_t {
public:
    /**
     * @brief Constructs a non-owning change view from previous and current snapshots.
     */
    explicit gamepad_state_change_t(const gamepad_state_t& previous_state, const gamepad_state_t& current_state);

    /**
     * @brief Returns whether the specified button changed from released to pressed.
     */
    bool was_pressed(gamepad_button_t button) const;

    /**
     * @brief Returns whether the specified button changed from pressed to released.
     */
    bool was_released(gamepad_button_t button) const;

    /**
     * @brief Returns the current axis position minus the previous position.
     */
    float axis_delta(gamepad_axis_t axis) const;

private:
    const gamepad_state_t& m_previous_state;
    const gamepad_state_t& m_current_state;
};

/**
 * @brief Identifies which half of a joystick axis is mapped.
 */
enum class joystick_to_gamepad_axis_direction_t {
    positive,
    negative,
    both
};

/**
 * @brief Maps one joystick axis to one standardized gamepad axis.
 */
class joystick_to_gamepad_axis_t {
public:
    /**
     * @brief Constructs the default axis mapping.
     */
    joystick_to_gamepad_axis_t();

    /**
     * @brief Returns the destination gamepad axis.
     */
    gamepad_axis_t& gamepad_axis();
    const gamepad_axis_t& gamepad_axis() const;

    /**
     * @brief Returns the mapped half of the joystick axis.
     */
    joystick_to_gamepad_axis_direction_t& gamepad_axis_mapping_direction();
    const joystick_to_gamepad_axis_direction_t& gamepad_axis_mapping_direction() const;

    /**
     * @brief Returns whether the joystick axis is inverted.
     */
    bool& inverted();
    const bool& inverted() const;

private:
    gamepad_axis_t m_gamepad_axis;
    joystick_to_gamepad_axis_direction_t m_gamepad_axis_mapping_direction;
    bool m_inverted;
};

/**
 * @brief Identifies the platform to which a gamepad mapping applies.
 */
enum class joystick_to_gamepad_platform_t {
    windows,
    linux,
    macos,
    all
};

/**
 * @brief Maps one joystick hat direction to one standardized gamepad button.
 */
class joystick_to_gamepad_hat_t {
public:
    /**
     * @brief Constructs the default hat mapping.
     */
    joystick_to_gamepad_hat_t();

    /**
     * @brief Returns the source joystick hat direction.
     */
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& joystick_hat();
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& joystick_hat() const;

    /**
     * @brief Returns the destination gamepad button.
     */
    gamepad_button_t& gamepad_button();
    const gamepad_button_t& gamepad_button() const;

private:
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> m_joystick_hat;
    gamepad_button_t m_gamepad_button;
};

/**
 * @brief Represents one supported SDL-format joystick-to-gamepad mapping.
 *
 * The parsing constructor validates its input, while the mutable accessors may make the mapping invalid until corrected.
 */
class joystick_to_gamepad_mapping_t {
public:
    /**
     * @brief Constructs an empty mapping for all platforms.
     */
    joystick_to_gamepad_mapping_t();

    /**
     * @brief Parses one supported SDL-format gamepad mapping line.
     *
     * @throws std::invalid_argument if the mapping is malformed or unsupported.
     */
    explicit joystick_to_gamepad_mapping_t(const std::string& gamepad_mapping); // follows SDL gamepad mapping string format

    /**
     * @brief Serializes the current mapping as one SDL-format mapping line.
     */
    std::string gamepad_mapping() const;

    /**
     * @brief Returns the 32-digit hexadecimal joystick GUID.
     */
    std::string& gamepad_guid();
    const std::string& gamepad_guid() const;

    /**
     * @brief Returns the human-readable gamepad name.
     */
    std::string& gamepad_name();
    const std::string& gamepad_name() const;

    /**
     * @brief Returns the target platform.
     */
    joystick_to_gamepad_platform_t& platform();
    const joystick_to_gamepad_platform_t& platform() const;

    /**
     * @brief Returns gamepad-button mappings keyed by joystick button index.
     */
    std::unordered_multimap<unsigned int, gamepad_button_t>& gamepad_button_by_joystick_button();
    const std::unordered_multimap<unsigned int, gamepad_button_t>& gamepad_button_by_joystick_button() const;

    /**
     * @brief Returns gamepad-axis mappings keyed by joystick axis index.
     */
    std::unordered_multimap<unsigned int, joystick_to_gamepad_axis_t>& gamepad_axis_by_joystick_axis();
    const std::unordered_multimap<unsigned int, joystick_to_gamepad_axis_t>& gamepad_axis_by_joystick_axis() const;

    /**
     * @brief Returns gamepad-button mappings keyed by joystick hat index.
     */
    std::unordered_multimap<unsigned int, joystick_to_gamepad_hat_t>& gamepad_hat_by_joystick_hat();
    const std::unordered_multimap<unsigned int, joystick_to_gamepad_hat_t>& gamepad_hat_by_joystick_hat() const;

private:
    std::string m_gamepad_guid;
    std::string m_gamepad_name;
    joystick_to_gamepad_platform_t m_platform;
    std::unordered_multimap<unsigned int, gamepad_button_t> m_gamepad_button_by_joystick_button;
    std::unordered_multimap<unsigned int, joystick_to_gamepad_axis_t> m_gamepad_axis_by_joystick_axis;
    std::unordered_multimap<unsigned int, joystick_to_gamepad_hat_t> m_gamepad_hat_by_joystick_hat;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::button_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::button_state_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_change_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_change_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_button_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_axis_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_change_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_direction_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_platform_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_hat_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_mapping_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::button_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::button_t& button, auto& ctx) const {
        auto out = ctx.out();

        const char* name = nullptr;
        switch (button) {
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_space: name = "space"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_apostrophe: name = "apostrophe"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_comma: name = "comma"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_minus: name = "minus"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_period: name = "period"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_slash: name = "slash"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_0: name = "0"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_1: name = "1"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_2: name = "2"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_3: name = "3"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_4: name = "4"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_5: name = "5"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_6: name = "6"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_7: name = "7"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_8: name = "8"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_9: name = "9"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_semicolon: name = "semicolon"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_equal: name = "equal"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_a: name = "a"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_b: name = "b"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_c: name = "c"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_d: name = "d"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_e: name = "e"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f: name = "f"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_g: name = "g"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_h: name = "h"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_i: name = "i"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_j: name = "j"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_k: name = "k"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_l: name = "l"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_m: name = "m"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_n: name = "n"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_o: name = "o"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_p: name = "p"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_q: name = "q"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_r: name = "r"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_s: name = "s"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_t: name = "t"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_u: name = "u"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_v: name = "v"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_w: name = "w"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_x: name = "x"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_y: name = "y"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_z: name = "z"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left_bracket: name = "left bracket"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_backslash: name = "backslash"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right_bracket: name = "right bracket"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_grave_accent: name = "grave accent"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_world_1: name = "world 1"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_world_2: name = "world 2"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_escape: name = "escape"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_enter: name = "enter"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_tab: name = "tab"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_backspace: name = "backspace"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_insert: name = "insert"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_delete: name = "delete"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right: name = "right"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left: name = "left"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_down: name = "down"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_up: name = "up"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_page_up: name = "page up"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_page_down: name = "page down"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_home: name = "home"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_end: name = "end"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_caps_lock: name = "caps lock"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_scroll_lock: name = "scroll lock"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_num_lock: name = "num lock"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_print_screen: name = "print screen"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_pause: name = "pause"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f1: name = "F1"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f2: name = "F2"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f3: name = "F3"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f4: name = "F4"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f5: name = "F5"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f6: name = "F6"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f7: name = "F7"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f8: name = "F8"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f9: name = "F9"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f10: name = "F10"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f11: name = "F11"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f12: name = "F12"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f13: name = "F13"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f14: name = "F14"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f15: name = "F15"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f16: name = "F16"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f17: name = "F17"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f18: name = "F18"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f19: name = "F19"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f20: name = "F20"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f21: name = "F21"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f22: name = "F22"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f23: name = "F23"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f24: name = "F24"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_f25: name = "F25"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_0: name = "keypad 0"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_1: name = "keypad 1"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_2: name = "keypad 2"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_3: name = "keypad 3"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_4: name = "keypad 4"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_5: name = "keypad 5"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_6: name = "keypad 6"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_7: name = "keypad 7"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_8: name = "keypad 8"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_9: name = "keypad 9"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_decimal: name = "keypad decimal"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_divide: name = "keypad divide"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_multiply: name = "keypad multiply"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_subtract: name = "keypad subtract"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_add: name = "keypad add"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_enter: name = "keypad enter"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_keypad_equal: name = "keypad equal"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left_shift: name = "left shift"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left_control: name = "left control"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left_alt: name = "left alt"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_left_super: name = "left super"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right_shift: name = "right shift"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right_control: name = "right control"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right_alt: name = "right alt"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_right_super: name = "right super"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_menu: name = "menu"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_left: name = "mouse left"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_right: name = "mouse right"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_middle: name = "mouse middle"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_4: name = "mouse 4"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_5: name = "mouse 5"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_6: name = "mouse 6"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_7: name = "mouse 7"; break;
            case m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::button_mouse_8: name = "mouse 8"; break;
        }

        if (name == nullptr) {
            throw std::runtime_error(std::format("Unknown button: {}", static_cast<std::uint32_t>(button)));
        }

        out = std::format_to(out, "{}", name);

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::button_state_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::button_state_t& button_state, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "transition count: {}, ", button_state.transition_count());
        out = std::format_to(out, "repeat count: {}, ", button_state.repeat_count());
        out = std::format_to(out, "is down: {}", button_state.is_down());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_t& input_state, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "button states: {{ ");
        for (std::size_t i = 0; i < static_cast<std::size_t>(m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::_button_count); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            const auto button = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::button_t>(i);
            out = std::format_to(out, "{}: {}", button, input_state.button_state(button));
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "cursor position: {}, ", input_state.cursor_position());
        out = std::format_to(out, "scroll offset: {}", input_state.scroll_offset());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_change_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::input_state_change_t& input_state_change, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "button changes: {{ ");
        bool wrote_button_change = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(m03gkcdy62bnz808pmk4uzkjra_glfw::button_t::_button_count); ++i) {
            const auto button = static_cast<m03gkcdy62bnz808pmk4uzkjra_glfw::button_t>(i);
            const auto press_delta = input_state_change.press_delta(button);
            const auto release_delta = input_state_change.release_delta(button);
            const auto repeat_delta = input_state_change.repeat_delta(button);
            if (press_delta == 0 && release_delta == 0 && repeat_delta == 0) {
                continue;
            }

            if (wrote_button_change) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {{ ", button);
            out = std::format_to(out, "press delta: {}, ", press_delta);
            out = std::format_to(out, "release delta: {}, ", release_delta);
            out = std::format_to(out, "repeat delta: {}", repeat_delta);
            out = std::format_to(out, " }}");

            wrote_button_change = true;
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "cursor position delta: {}, ", input_state_change.cursor_position_delta());
        out = std::format_to(out, "scroll offset delta: {}", input_state_change.scroll_offset_delta());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_t& joystick_state, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "axes: {{ ");
        for (std::size_t i = 0; i < joystick_state.axes().size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", i, joystick_state.axes()[i]);
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "buttons: {{ ");
        for (std::size_t i = 0; i < joystick_state.buttons().size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", i, joystick_state.buttons()[i]);
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "hats: {{ ");
        for (std::size_t i = 0; i < joystick_state.hats().size(); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", i, joystick_state.hats()[i]);
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_t& joystick, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "id: {}, ", joystick.id().has_value() ? std::to_string(joystick.id().value()) : "null");
        out = std::format_to(out, "name: {}, ", joystick.name());
        out = std::format_to(out, "guid: {}, ", joystick.guid());
        out = std::format_to(out, "joystick states: {}", joystick.joystick_states());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_change_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_state_change_t& joystick_state_change, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "axis deltas: {{ ");
        bool wrote_axis_change = false;
        for (std::size_t i = 0; i < joystick_state_change.axis_count(); ++i) {
            if (wrote_axis_change) {
                out = std::format_to(out, ", ");
            }

            if (joystick_state_change.axis_delta(i) == 0.0f) {
                continue;
            }

            out = std::format_to(out, "{}: {}", i, joystick_state_change.axis_delta(i));
            wrote_axis_change = true;
        }

        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "buttons: {{ ");
        bool wrote_button_change = false;
        for (std::size_t i = 0; i < joystick_state_change.button_count(); ++i) {
            if (joystick_state_change.was_pressed(i)) {
                if (wrote_button_change) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}: was pressed", i);
                wrote_button_change = true;
            } else if (joystick_state_change.was_released(i)) {
                if (wrote_button_change) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}: was released", i);
                wrote_button_change = true;
            }
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "hats: {{ ");
        bool wrote_hat_change = false;
        for (std::size_t i = 0; i < joystick_state_change.hat_count(); ++i) {
            const auto hat_delta = joystick_state_change.hat_delta(i);
            if (hat_delta == m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>{0, 0}) {
                continue;
            }

            if (wrote_hat_change) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", i, hat_delta);
            wrote_hat_change = true;
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    };
};

template<>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_button_t> {
    using gamepad_button_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_button_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const gamepad_button_t& button, auto& ctx) const {
        auto out = ctx.out();

        const char* name = nullptr;
        switch (button) {
            case gamepad_button_t::button_a: name = "a/cross"; break;
            case gamepad_button_t::button_b: name = "b/circle"; break;
            case gamepad_button_t::button_x: name = "x/square"; break;
            case gamepad_button_t::button_y: name = "y/triangle"; break;
            case gamepad_button_t::button_left_bumper: name = "left bumper"; break;
            case gamepad_button_t::button_right_bumper: name = "right bumper"; break;
            case gamepad_button_t::button_back: name = "back"; break;
            case gamepad_button_t::button_start: name = "start"; break;
            case gamepad_button_t::button_guide: name = "guide"; break;
            case gamepad_button_t::button_left_thumb: name = "left thumb"; break;
            case gamepad_button_t::button_right_thumb: name = "right thumb"; break;
            case gamepad_button_t::button_dpad_up: name = "dpad up"; break;
            case gamepad_button_t::button_dpad_right: name = "dpad right"; break;
            case gamepad_button_t::button_dpad_down: name = "dpad down"; break;
            case gamepad_button_t::button_dpad_left: name = "dpad left"; break;
            case gamepad_button_t::_button_count: break;
        }

        if (name == nullptr) {
            throw std::runtime_error(std::format("Unknown gamepad button: {}", static_cast<std::uint32_t>(button)));
        }

        out = std::format_to(out, "{}", name);

        return out;
    };
};

template<>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_axis_t> {
    using gamepad_axis_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_axis_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const gamepad_axis_t& axis, auto& ctx) const {
        auto out = ctx.out();

        const char* name = nullptr;
        switch (axis) {
            case gamepad_axis_t::axis_left_x: name = "left x"; break;
            case gamepad_axis_t::axis_left_y: name = "left y"; break;
            case gamepad_axis_t::axis_right_x: name = "right x"; break;
            case gamepad_axis_t::axis_right_y: name = "right y"; break;
            case gamepad_axis_t::axis_left_trigger: name = "left trigger"; break;
            case gamepad_axis_t::axis_right_trigger: name = "right trigger"; break;
            case gamepad_axis_t::_axis_count: break;
        }

        if (name == nullptr) {
            throw std::runtime_error(std::format("Unknown gamepad axis: {}", static_cast<std::uint32_t>(axis)));
        }

        out = std::format_to(out, "{}", name);

        return out;
    };
};

template<>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_t> {
    using gamepad_button_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_button_t;
    using gamepad_axis_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_axis_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_t& gamepad_state, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "button states: {{ ");
        for (std::size_t i = 0; i < static_cast<std::size_t>(gamepad_button_t::_button_count); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            const auto gamepad_button = static_cast<gamepad_button_t>(i);
            out = std::format_to(out, "{}: {}", gamepad_button, gamepad_state.button_state(gamepad_button));
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "axis states: {{ ");
        for (std::size_t i = 0; i < static_cast<std::size_t>(gamepad_axis_t::_axis_count); ++i) {
            if (0 < i) {
                out = std::format_to(out, ", ");
            }

            const auto gamepad_axis = static_cast<gamepad_axis_t>(i);
            out = std::format_to(out, "{}: {}", gamepad_axis, gamepad_state.axis_state(gamepad_axis));
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    };
};

template<>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_t& gamepad, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "id: {}, ", gamepad.id().has_value() ? std::to_string(gamepad.id().value()) : "null");
        out = std::format_to(out, "name: {}, ", gamepad.name());
        out = std::format_to(out, "guid: {}, ", gamepad.guid());
        out = std::format_to(out, "gamepad states: {}", gamepad.gamepad_states());

        out = std::format_to(out, " }}");

        return out;
    };
};

template<>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_change_t> {
    using gamepad_button_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_button_t;
    using gamepad_axis_t = m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_axis_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::gamepad_state_change_t& gamepad_state_change, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "button changes: {{ ");
        bool wrote_button_change = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(gamepad_button_t::_button_count); ++i) {
            const auto gamepad_button = static_cast<gamepad_button_t>(i);
            if (gamepad_state_change.was_pressed(gamepad_button)) {
                if (wrote_button_change) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}: was pressed", gamepad_button);
                wrote_button_change = true;
            }
            
            if (gamepad_state_change.was_released(gamepad_button)) {
                if (wrote_button_change) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}: was released", gamepad_button);
                wrote_button_change = true;
            }
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "axis deltas: {{ ");
        bool wrote_axis_change = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(gamepad_axis_t::_axis_count); ++i) {
            const auto gamepad_axis = static_cast<gamepad_axis_t>(i);
            if (gamepad_state_change.axis_delta(gamepad_axis) == 0.0f) {
                continue;
            }

            if (wrote_axis_change) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "{}: {}", gamepad_axis, gamepad_state_change.axis_delta(gamepad_axis));
            wrote_axis_change = true;
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_direction_t> {
    using joystick_to_gamepad_axis_direction_t = m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_direction_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const joystick_to_gamepad_axis_direction_t& direction, auto& ctx) const {
        auto out = ctx.out();

        const char* name = nullptr;
        switch (direction) {
            case joystick_to_gamepad_axis_direction_t::positive: name = "positive"; break;
            case joystick_to_gamepad_axis_direction_t::negative: name = "negative"; break;
            case joystick_to_gamepad_axis_direction_t::both: name = "both"; break;
        }

        if (name == nullptr) {
            throw std::runtime_error(std::format("Unknown joystick to gamepad axis direction: {}", static_cast<std::uint32_t>(direction)));
        }

        out = std::format_to(out, "{}", name);

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_axis_t& axis, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "gamepad axis: {}, ", axis.gamepad_axis());
        out = std::format_to(out, "gamepad axis mapping direction: {}, ", axis.gamepad_axis_mapping_direction());
        out = std::format_to(out, "inverted: {}", axis.inverted());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_platform_t> {
    using joystick_to_gamepad_platform_t = m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_platform_t;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const joystick_to_gamepad_platform_t& platform, auto& ctx) const {
        auto out = ctx.out();

        const char* name = nullptr;
        switch (platform) {
            case joystick_to_gamepad_platform_t::windows: name = "windows"; break;
            case joystick_to_gamepad_platform_t::linux: name = "linux"; break;
            case joystick_to_gamepad_platform_t::macos: name = "macos"; break;
            case joystick_to_gamepad_platform_t::all: name = "all"; break;
        }

        if (name == nullptr) {
            throw std::runtime_error(std::format("Unknown joystick to gamepad platform: {}", static_cast<std::uint32_t>(platform)));
        }

        out = std::format_to(out, "{}", name);

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_hat_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_hat_t& hat, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "joystick hat: ({}, {}), ", hat.joystick_hat()[0], hat.joystick_hat()[1]);
        out = std::format_to(out, "gamepad button: {}", hat.gamepad_button());

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_mapping_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::joystick_to_gamepad_mapping_t& mapping, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "gamepad guid: {}, ", mapping.gamepad_guid());
        out = std::format_to(out, "gamepad name: {}, ", mapping.gamepad_name());
        out = std::format_to(out, "platform: {}, ", mapping.platform());

        out = std::format_to(out, "gamepad button by joystick button: {{ ");
        bool wrote_button_mapping = false;
        for (const auto& [joystick_button, gamepad_button] : mapping.gamepad_button_by_joystick_button()) {
            if (wrote_button_mapping) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", joystick_button, gamepad_button);
            wrote_button_mapping = true;
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "gamepad axis by joystick axis: {{ ");
        bool wrote_axis_mapping = false;
        for (const auto& [joystick_axis, gamepad_axis] : mapping.gamepad_axis_by_joystick_axis()) {
            if (wrote_axis_mapping) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", joystick_axis, gamepad_axis);
            wrote_axis_mapping = true;
        }
        out = std::format_to(out, " }}, ");

        out = std::format_to(out, "gamepad hat by joystick hat: {{ ");
        bool wrote_hat_mapping = false;
        for (const auto& [joystick_hat, gamepad_hat] : mapping.gamepad_hat_by_joystick_hat()) {
            if (wrote_hat_mapping) {
                out = std::format_to(out, ", ");
            }

            out = std::format_to(out, "{}: {}", joystick_hat, gamepad_hat);
            wrote_hat_mapping = true;
        }
        out = std::format_to(out, " }}");

        out = std::format_to(out, " }}");

        return out;
    };
};

} // namespace std

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_INPUT_H
