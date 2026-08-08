#include "input.h"
#include "glfw_external.h"

#include <charconv>
#include <algorithm>
#include <cctype>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

std::int32_t& button_state_t::transition_count() {
    return m_transition_count;
}

std::int32_t button_state_t::transition_count() const {
    return m_transition_count;
}

std::int32_t& button_state_t::repeat_count() {
    return m_repeat_count;
}

std::int32_t button_state_t::repeat_count() const {
    return m_repeat_count;
}

bool& button_state_t::is_down() {
    return m_is_down;
}

bool button_state_t::is_down() const {
    return m_is_down;
}

button_state_t& input_state_t::button_state(button_t button) {
    return m_button_states[static_cast<std::size_t>(button)];
}

const button_state_t& input_state_t::button_state(button_t button) const {
    return m_button_states[static_cast<std::size_t>(button)];
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& input_state_t::cursor_position() {
    return m_cursor_position;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& input_state_t::cursor_position() const {
    return m_cursor_position;
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& input_state_t::scroll_offset() {
    return m_scroll_offset;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2>& input_state_t::scroll_offset() const {
    return m_scroll_offset;
}

input_state_change_t::input_state_change_t(const input_state_t& previous_state, const input_state_t& current_state):
    m_previous_state(previous_state),
    m_current_state(current_state)
{
}

bool input_state_change_t::was_pressed(button_t button) const {
    return press_delta(button) != 0;
}

bool input_state_change_t::was_released(button_t button) const {
    return release_delta(button) != 0;
}

std::int32_t input_state_change_t::press_delta(button_t button) const {
    return button_delta(button).press_delta;
}

std::int32_t input_state_change_t::release_delta(button_t button) const {
    return button_delta(button).release_delta;
}

std::int32_t input_state_change_t::repeat_delta(button_t button) const {
    const auto previous_repeat_count = m_previous_state.button_state(button).repeat_count();
    const auto current_repeat_count = m_current_state.button_state(button).repeat_count();
    return current_repeat_count - previous_repeat_count;
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> input_state_change_t::cursor_position_delta() const {
    const auto previous_cursor_position = m_previous_state.cursor_position();
    const auto current_cursor_position = m_current_state.cursor_position();
    return current_cursor_position - previous_cursor_position;
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<double, 2> input_state_change_t::scroll_offset_delta() const {
    const auto previous_scroll_offset = m_previous_state.scroll_offset();
    const auto current_scroll_offset = m_current_state.scroll_offset();
    return current_scroll_offset - previous_scroll_offset;
}

input_state_change_t::button_delta_t input_state_change_t::button_delta(button_t button) const {
    const auto& previous_button_state = m_previous_state.button_state(button);
    const auto& current_button_state = m_current_state.button_state(button);
    const auto previous_is_down = previous_button_state.is_down();
    const auto current_is_down = current_button_state.is_down();
    const auto previous_transition_count = previous_button_state.transition_count();
    const auto current_transition_count = current_button_state.transition_count();
    const auto transitions = current_transition_count - previous_transition_count;
    const auto previous_initial_down = previous_button_state.is_down() ^ (previous_transition_count % 2 != 0);
    const auto current_initial_down = current_button_state.is_down() ^ (current_transition_count % 2 != 0);
    if (previous_initial_down != current_initial_down) {
        throw std::logic_error(std::format("input_state_change_t::button_delta: invalid transition between button {} and {}", previous_button_state, current_button_state));
    }
    return {
        static_cast<std::int32_t>((transitions + current_is_down - previous_is_down) / 2),
        static_cast<std::int32_t>((transitions - current_is_down + previous_is_down) / 2)
    };
}

std::vector<float>& joystick_state_t::axes() {
    return m_axes;
}

const std::vector<float>& joystick_state_t::axes() const {
    return m_axes;
}

std::vector<bool>& joystick_state_t::buttons() {
    return m_buttons;
}

const std::vector<bool>& joystick_state_t::buttons() const {
    return m_buttons;
}

std::vector<m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>>& joystick_state_t::hats() {
    return m_hats;
}

const std::vector<m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>>& joystick_state_t::hats() const {
    return m_hats;
}

joystick_t::joystick_t(size_t joystick_states_size):
    m_joystick_states(joystick_states_size)
{
}

std::optional<int>& joystick_t::id() {
    return m_id;
}

const std::optional<int>& joystick_t::id() const {
    return m_id;
}

std::string& joystick_t::name() {
    return m_name;
}

const std::string& joystick_t::name() const {
    return m_name;
}

std::string& joystick_t::guid() {
    return m_guid;
}

const std::string& joystick_t::guid() const {
    return m_guid;
}

joystick_t::joystick_states_t& joystick_t::joystick_states() {
    return m_joystick_states;
}

const joystick_t::joystick_states_t& joystick_t::joystick_states() const {
    return m_joystick_states;
}

joystick_state_change_t::joystick_state_change_t(const joystick_state_t& previous_state, const joystick_state_t& current_state):
    m_previous_state(previous_state),
    m_current_state(current_state)
{
    if (previous_state.axes().size() != current_state.axes().size()) {
        throw std::logic_error(std::format("joystick_state_change_t: previous_state and current_state have different axis counts ({} vs {})", previous_state.axes().size(), current_state.axes().size()));
    }

    if (previous_state.buttons().size() != current_state.buttons().size()) {
        throw std::logic_error(std::format("joystick_state_change_t: previous_state and current_state have different button counts ({} vs {})", previous_state.buttons().size(), current_state.buttons().size()));
    }

    if (previous_state.hats().size() != current_state.hats().size()) {
        throw std::logic_error(std::format("joystick_state_change_t: previous_state and current_state have different hat counts ({} vs {})", previous_state.hats().size(), current_state.hats().size()));
    }
}

std::size_t joystick_state_change_t::axis_count() const {
    return m_current_state.axes().size();
}

float joystick_state_change_t::axis_delta(std::size_t axis_index) const {
    const auto& previous_axes = m_previous_state.axes();
    const auto& current_axes = m_current_state.axes();
    if (previous_axes.size() <= axis_index || current_axes.size() <= axis_index) {
        throw std::out_of_range(std::format("joystick_state_change_t::axis_delta: axis_index {} is out of range", axis_index));
    }
    return current_axes[axis_index] - previous_axes[axis_index];
}

std::size_t joystick_state_change_t::button_count() const {
    return m_current_state.buttons().size();
}

bool joystick_state_change_t::was_pressed(std::size_t button_index) const {
    const auto& previous_buttons = m_previous_state.buttons();
    const auto& current_buttons = m_current_state.buttons();
    if (previous_buttons.size() <= button_index) {
        throw std::out_of_range(std::format("joystick_state_change_t::was_pressed: button_index {} is out of range", button_index));
    }
    return current_buttons[button_index] && !previous_buttons[button_index];
}

bool joystick_state_change_t::was_released(std::size_t button_index) const {
    const auto& previous_buttons = m_previous_state.buttons();
    const auto& current_buttons = m_current_state.buttons();
    if (previous_buttons.size() <= button_index) {
        throw std::out_of_range(std::format("joystick_state_change_t::was_released: button_index {} is out of range", button_index));
    }
    return !current_buttons[button_index] && previous_buttons[button_index];
}

std::size_t joystick_state_change_t::hat_count() const {
    return m_current_state.hats().size();
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> joystick_state_change_t::hat_delta(std::size_t hat_index) const {
    const auto& previous_hats = m_previous_state.hats();
    const auto& current_hats = m_current_state.hats();
    if (previous_hats.size() <= hat_index) {
        throw std::out_of_range(std::format("joystick_state_change_t::hat_delta: hat_index {} is out of range", hat_index));
    }
    return current_hats[hat_index] - previous_hats[hat_index];
}

bool& gamepad_state_t::button_state(gamepad_button_t button) {
    return m_button_states[static_cast<std::size_t>(button)];
}

bool gamepad_state_t::button_state(gamepad_button_t button) const {
    return m_button_states[static_cast<std::size_t>(button)];
}

float& gamepad_state_t::axis_state(gamepad_axis_t axis) {
    return m_axis_states[static_cast<std::size_t>(axis)];
}

float gamepad_state_t::axis_state(gamepad_axis_t axis) const {
    return m_axis_states[static_cast<std::size_t>(axis)];
}

gamepad_t::gamepad_t(std::size_t gamepad_states_size):
    m_gamepad_states(gamepad_states_size)
{
}

std::optional<int>& gamepad_t::id() {
    return m_id;
}

const std::optional<int>& gamepad_t::id() const {
    return m_id;
}

std::string& gamepad_t::name() {
    return m_name;
}

const std::string& gamepad_t::name() const {
    return m_name;
}

std::string& gamepad_t::guid() {
    return m_guid;
}

const std::string& gamepad_t::guid() const {
    return m_guid;
}

gamepad_t::gamepad_states_t& gamepad_t::gamepad_states() {
    return m_gamepad_states;
}

const gamepad_t::gamepad_states_t& gamepad_t::gamepad_states() const {
    return m_gamepad_states;
}

gamepad_state_change_t::gamepad_state_change_t(const gamepad_state_t& previous_state, const gamepad_state_t& current_state):
    m_previous_state(previous_state),
    m_current_state(current_state)
{
}

bool gamepad_state_change_t::was_pressed(gamepad_button_t button) const {
    const auto& previous_button_state = m_previous_state.button_state(button);
    const auto& current_button_state = m_current_state.button_state(button);
    return current_button_state && !previous_button_state;
}

bool gamepad_state_change_t::was_released(gamepad_button_t button) const {
    const auto& previous_button_state = m_previous_state.button_state(button);
    const auto& current_button_state = m_current_state.button_state(button);
    return !current_button_state && previous_button_state;
}

float gamepad_state_change_t::axis_delta(gamepad_axis_t axis) const {
    const auto& previous_axis_state = m_previous_state.axis_state(axis);
    const auto& current_axis_state = m_current_state.axis_state(axis);
    return current_axis_state - previous_axis_state;
}

joystick_to_gamepad_axis_t::joystick_to_gamepad_axis_t():
    m_gamepad_axis(gamepad_axis_t::axis_left_x),
    m_gamepad_axis_mapping_direction(joystick_to_gamepad_axis_direction_t::both),
    m_inverted(false)
{
}

gamepad_axis_t& joystick_to_gamepad_axis_t::gamepad_axis() {
    return m_gamepad_axis;
}

const gamepad_axis_t& joystick_to_gamepad_axis_t::gamepad_axis() const {
    return m_gamepad_axis;
}

joystick_to_gamepad_axis_direction_t& joystick_to_gamepad_axis_t::gamepad_axis_mapping_direction() {
    return m_gamepad_axis_mapping_direction;
}

const joystick_to_gamepad_axis_direction_t& joystick_to_gamepad_axis_t::gamepad_axis_mapping_direction() const {
    return m_gamepad_axis_mapping_direction;
}

bool& joystick_to_gamepad_axis_t::inverted() {
    return m_inverted;
}

const bool& joystick_to_gamepad_axis_t::inverted() const {
    return m_inverted;
}

joystick_to_gamepad_hat_t::joystick_to_gamepad_hat_t():
    m_joystick_hat({0, 0}),
    m_gamepad_button(gamepad_button_t::button_a)
{
}

m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& joystick_to_gamepad_hat_t::joystick_hat() {
    return m_joystick_hat;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& joystick_to_gamepad_hat_t::joystick_hat() const {
    return m_joystick_hat;
}

gamepad_button_t& joystick_to_gamepad_hat_t::gamepad_button() {
    return m_gamepad_button;
}

const gamepad_button_t& joystick_to_gamepad_hat_t::gamepad_button() const {
    return m_gamepad_button;
}

joystick_to_gamepad_mapping_t::joystick_to_gamepad_mapping_t():
    m_platform(joystick_to_gamepad_platform_t::all)
{
}

joystick_to_gamepad_mapping_t::joystick_to_gamepad_mapping_t(const std::string& gamepad_mapping): joystick_to_gamepad_mapping_t()
{
    const auto mapping_to_gamepad_button = [](std::string_view mapping) -> std::optional<gamepad_button_t> {
        if (mapping == "a") return gamepad_button_t::button_a;
        if (mapping == "b") return gamepad_button_t::button_b;
        if (mapping == "x") return gamepad_button_t::button_x;
        if (mapping == "y") return gamepad_button_t::button_y;
        if (mapping == "leftshoulder") return gamepad_button_t::button_left_bumper;
        if (mapping == "rightshoulder") return gamepad_button_t::button_right_bumper;
        if (mapping == "back") return gamepad_button_t::button_back;
        if (mapping == "start") return gamepad_button_t::button_start;
        if (mapping == "guide") return gamepad_button_t::button_guide;
        if (mapping == "leftstick") return gamepad_button_t::button_left_thumb;
        if (mapping == "rightstick") return gamepad_button_t::button_right_thumb;
        if (mapping == "dpup") return gamepad_button_t::button_dpad_up;
        if (mapping == "dpright") return gamepad_button_t::button_dpad_right;
        if (mapping == "dpdown") return gamepad_button_t::button_dpad_down;
        if (mapping == "dpleft") return gamepad_button_t::button_dpad_left;
        return std::nullopt;
    };
    const auto mapping_to_gamepad_axis = [](std::string_view mapping) -> std::optional<gamepad_axis_t> {
        if (mapping == "leftx") return gamepad_axis_t::axis_left_x;
        if (mapping == "lefty") return gamepad_axis_t::axis_left_y;
        if (mapping == "rightx") return gamepad_axis_t::axis_right_x;
        if (mapping == "righty") return gamepad_axis_t::axis_right_y;
        if (mapping == "lefttrigger") return gamepad_axis_t::axis_left_trigger;
        if (mapping == "righttrigger") return gamepad_axis_t::axis_right_trigger;
        return std::nullopt;
    };
    const auto mapping_to_joystick_hat = [](unsigned int bitmask) -> m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> {
        constexpr unsigned int valid_bits = GLFW_HAT_UP | GLFW_HAT_RIGHT | GLFW_HAT_DOWN | GLFW_HAT_LEFT;
        if ((bitmask & ~valid_bits) != 0 ||
            ((bitmask & GLFW_HAT_LEFT) && (bitmask & GLFW_HAT_RIGHT)) ||
            ((bitmask & GLFW_HAT_UP) && (bitmask & GLFW_HAT_DOWN))) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: invalid joystick hat bitmask {}", bitmask));
        }
        return {
            static_cast<int>((bitmask & GLFW_HAT_RIGHT) != 0) - static_cast<int>((bitmask & GLFW_HAT_LEFT) != 0),
            static_cast<int>((bitmask & GLFW_HAT_UP) != 0) - static_cast<int>((bitmask & GLFW_HAT_DOWN) != 0)
        };
    };
    if (gamepad_mapping.find('\0') != std::string::npos) {
        throw std::invalid_argument("joystick_to_gamepad_mapping_t: mapping contains a null character");
    }

    std::string_view remaining = gamepad_mapping;
    while (!remaining.empty() && (remaining.back() == '\r' || remaining.back() == '\n')) {
        remaining.remove_suffix(1);
    }
    if (remaining.find_first_of("\r\n") != std::string_view::npos) {
        throw std::invalid_argument("joystick_to_gamepad_mapping_t: expected one mapping line");
    }

    const auto take = [&remaining](bool comma_required) {
        const std::size_t comma = remaining.find(',');
        if (comma == std::string_view::npos) {
            if (comma_required) {
                throw std::invalid_argument("joystick_to_gamepad_mapping_t: mapping must contain a GUID and name");
            }
            const std::string_view value = remaining;
            remaining = {};
            return value;
        }
        const std::string_view value = remaining.substr(0, comma);
        remaining.remove_prefix(comma + 1);
        return value;
    };
    const auto parse_unsigned = [](std::string_view value, std::string_view description) {
        unsigned int result = 0;
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
        if (value.empty() || error != std::errc{} || end != value.data() + value.size()) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: invalid {} '{}'", description, value));
        }
        return result;
    };

    m_gamepad_guid = take(true);
    m_gamepad_name = take(false);

    if (m_gamepad_guid.size() != 32) {
        throw std::invalid_argument("joystick_to_gamepad_mapping_t: gamepad GUID must contain 32 hexadecimal digits");
    }
    for (const char character : m_gamepad_guid) {
        if (!std::isxdigit(static_cast<unsigned char>(character))) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: invalid gamepad GUID '{}'", m_gamepad_guid));
        }
    }

    bool platform_seen = false;
    while (!remaining.empty()) {
        const std::string_view entry = take(false);
        if (entry.empty()) {
            continue;
        }

        const std::size_t colon = entry.find(':');
        if (colon == std::string_view::npos || entry.find(':', colon + 1) != std::string_view::npos) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: invalid mapping entry '{}'", entry));
        }

        const std::string_view field = entry.substr(0, colon);
        std::string_view value = entry.substr(colon + 1);

        if (field == "platform") {
            if (platform_seen) {
                throw std::invalid_argument("joystick_to_gamepad_mapping_t: duplicate platform mapping");
            }
            platform_seen = true;
            if (value == "Windows") m_platform = joystick_to_gamepad_platform_t::windows;
            else if (value == "Linux") m_platform = joystick_to_gamepad_platform_t::linux;
            else if (value == "Mac OS X") m_platform = joystick_to_gamepad_platform_t::macos;
            else throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unknown platform '{}'", value));
            continue;
        }

        const auto gamepad_button = mapping_to_gamepad_button(field);
        const auto gamepad_axis = mapping_to_gamepad_axis(field);
        if (!gamepad_button && !gamepad_axis) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unknown gamepad output '{}'", field));
        }
        if (value.empty()) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: mapping entry '{}' has no joystick input", entry));
        }

        joystick_to_gamepad_axis_direction_t direction = joystick_to_gamepad_axis_direction_t::both;
        if (value.front() == '+' || value.front() == '-') {
            direction = value.front() == '+' ? joystick_to_gamepad_axis_direction_t::positive : joystick_to_gamepad_axis_direction_t::negative;
            value.remove_prefix(1);
        }

        bool inverted = false;
        if (!value.empty() && value.back() == '~') {
            inverted = true;
            value.remove_suffix(1);
        }
        if (value.empty()) {
            throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: mapping entry '{}' has no joystick input", entry));
        }

        switch (value.front()) {
            case 'b': {
                if (!gamepad_button || direction != joystick_to_gamepad_axis_direction_t::both || inverted) {
                    throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unsupported mapping entry '{}'", entry));
                }
                m_gamepad_button_by_joystick_button.emplace(parse_unsigned(value.substr(1), "joystick button index"), *gamepad_button);
                break;
            }
            case 'a': {
                if (!gamepad_axis) {
                    throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unsupported mapping entry '{}'", entry));
                }
                joystick_to_gamepad_axis_t mapping;
                mapping.gamepad_axis() = *gamepad_axis;
                mapping.gamepad_axis_mapping_direction() = direction;
                mapping.inverted() = inverted;
                m_gamepad_axis_by_joystick_axis.emplace(parse_unsigned(value.substr(1), "joystick axis index"), std::move(mapping));
                break;
            }
            case 'h': {
                if (!gamepad_button || direction != joystick_to_gamepad_axis_direction_t::both || inverted) {
                    throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unsupported mapping entry '{}'", entry));
                }
                const std::string_view hat = value.substr(1);
                const std::size_t dot = hat.find('.');
                if (dot == std::string_view::npos || hat.find('.', dot + 1) != std::string_view::npos) {
                    throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: invalid hat mapping '{}'", entry));
                }
                joystick_to_gamepad_hat_t mapping;
                mapping.joystick_hat() = mapping_to_joystick_hat(parse_unsigned(hat.substr(dot + 1), "joystick hat bitmask"));
                mapping.gamepad_button() = *gamepad_button;
                m_gamepad_hat_by_joystick_hat.emplace(parse_unsigned(hat.substr(0, dot), "joystick hat index"), std::move(mapping));
                break;
            }
            default:
                throw std::invalid_argument(std::format("joystick_to_gamepad_mapping_t: unknown joystick input '{}'", value));
        }
    }
}

std::string joystick_to_gamepad_mapping_t::gamepad_mapping() const {
    const auto platform_to_mapping = [](joystick_to_gamepad_platform_t platform) -> std::string {
        switch (platform) {
        case joystick_to_gamepad_platform_t::windows: return "Windows";
        case joystick_to_gamepad_platform_t::linux: return "Linux";
        case joystick_to_gamepad_platform_t::macos: return "Mac OS X";
        case joystick_to_gamepad_platform_t::all: return "";
        default: throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: unknown platform {}", static_cast<std::uint32_t>(platform)));
        }
    };

    const auto gamepad_button_to_mapping = [](gamepad_button_t gamepad_button) -> std::string {
        switch (gamepad_button) {
            case gamepad_button_t::button_a: return "a";
            case gamepad_button_t::button_b: return "b";
            case gamepad_button_t::button_x: return"x";
            case gamepad_button_t::button_y: return"y";
            case gamepad_button_t::button_left_bumper: return"leftshoulder";
            case gamepad_button_t::button_right_bumper: return"rightshoulder";
            case gamepad_button_t::button_back: return"back";
            case gamepad_button_t::button_start: return"start";
            case gamepad_button_t::button_guide: return"guide";
            case gamepad_button_t::button_left_thumb: return"leftstick";
            case gamepad_button_t::button_right_thumb: return"rightstick";
            case gamepad_button_t::button_dpad_up: return"dpup";
            case gamepad_button_t::button_dpad_right: return"dpright";
            case gamepad_button_t::button_dpad_down: return"dpdown";
            case gamepad_button_t::button_dpad_left: return"dpleft";
            default: throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: unknown gamepad button {}", static_cast<std::uint32_t>(gamepad_button)));
        }
    };

    const auto gamepad_axis_to_mapping = [](gamepad_axis_t gamepad_axis) -> std::string {
        switch (gamepad_axis) {
            case gamepad_axis_t::axis_left_x: return "leftx";
            case gamepad_axis_t::axis_left_y: return "lefty";
            case gamepad_axis_t::axis_right_x: return "rightx";
            case gamepad_axis_t::axis_right_y: return "righty";
            case gamepad_axis_t::axis_left_trigger: return "lefttrigger";
            case gamepad_axis_t::axis_right_trigger: return "righttrigger";
            default: throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: unknown gamepad axis {}", static_cast<std::uint32_t>(gamepad_axis)));
        }
    };

    const auto gamepad_axis_mapping_direction_to_mapping = [](joystick_to_gamepad_axis_direction_t direction) -> std::string {
        switch (direction) {
            case joystick_to_gamepad_axis_direction_t::positive: return "+";
            case joystick_to_gamepad_axis_direction_t::negative: return "-";
            case joystick_to_gamepad_axis_direction_t::both: return "";
            default: throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: unknown gamepad axis mapping direction {}", static_cast<std::uint32_t>(direction)));
        }
    };

    const auto gamepad_hat_bitmask_to_mapping = [](const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& hat) -> std::string {
        unsigned int bitmask = 0;
        if (hat[0] == -1) {
            bitmask |= GLFW_HAT_LEFT;
        } else if (hat[0] == 1) {
            bitmask |= GLFW_HAT_RIGHT;
        } else if (hat[0] != 0) {
            throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: invalid joystick hat x value {}", hat[0]));
        }

        if (hat[1] == -1) {
            bitmask |= GLFW_HAT_DOWN;
        } else if (hat[1] == 1) {
            bitmask |= GLFW_HAT_UP;
        } else if (hat[1] != 0) {
            throw std::logic_error(std::format("m03gkcdy62bnz808pmk4uzkjra_glfw::update_gamepad_mapping: invalid joystick hat y value {}", hat[1]));
        }

        return std::format("{}", bitmask);
    };

    std::string mapping = std::format("{},", m_gamepad_guid);

    mapping += std::format("{},", m_gamepad_name);
    if (m_platform != joystick_to_gamepad_platform_t::all) {
        mapping += std::format("platform:{},", platform_to_mapping(m_platform));
    }

    for (const auto& [joystick_button, gamepad_button] : m_gamepad_button_by_joystick_button) {
        mapping += std::format("{}:b{},", gamepad_button_to_mapping(gamepad_button), joystick_button);
    }

    for (const auto& [joystick_axis, gamepad_axis_mapping] : m_gamepad_axis_by_joystick_axis) {
        mapping += std::format("{}:{}a{}{},", gamepad_axis_to_mapping(gamepad_axis_mapping.gamepad_axis()), gamepad_axis_mapping_direction_to_mapping(gamepad_axis_mapping.gamepad_axis_mapping_direction()), joystick_axis, gamepad_axis_mapping.inverted() ? "~" : "");
    }

    for (const auto& [joystick_hat, gamepad_hat_mapping] : m_gamepad_hat_by_joystick_hat) {
        mapping += std::format("{}:h{}.{},", gamepad_button_to_mapping(gamepad_hat_mapping.gamepad_button()), joystick_hat, gamepad_hat_bitmask_to_mapping(gamepad_hat_mapping.joystick_hat()));
    }

    return mapping;
}

std::string& joystick_to_gamepad_mapping_t::gamepad_guid() {
    return m_gamepad_guid;
}

const std::string& joystick_to_gamepad_mapping_t::gamepad_guid() const {
    return m_gamepad_guid;
}

std::string& joystick_to_gamepad_mapping_t::gamepad_name() {
    return m_gamepad_name;
}

const std::string& joystick_to_gamepad_mapping_t::gamepad_name() const {
    return m_gamepad_name;
}

joystick_to_gamepad_platform_t& joystick_to_gamepad_mapping_t::platform() {
    return m_platform;
}

const joystick_to_gamepad_platform_t& joystick_to_gamepad_mapping_t::platform() const {
    return m_platform;
}

std::unordered_multimap<unsigned int, gamepad_button_t>& joystick_to_gamepad_mapping_t::gamepad_button_by_joystick_button() {
    return m_gamepad_button_by_joystick_button;
}

const std::unordered_multimap<unsigned int, gamepad_button_t>& joystick_to_gamepad_mapping_t::gamepad_button_by_joystick_button() const {
    return m_gamepad_button_by_joystick_button;
}

std::unordered_multimap<unsigned int, joystick_to_gamepad_axis_t>& joystick_to_gamepad_mapping_t::gamepad_axis_by_joystick_axis() {
    return m_gamepad_axis_by_joystick_axis;
}

const std::unordered_multimap<unsigned int, joystick_to_gamepad_axis_t>& joystick_to_gamepad_mapping_t::gamepad_axis_by_joystick_axis() const {
    return m_gamepad_axis_by_joystick_axis;
}

std::unordered_multimap<unsigned int, joystick_to_gamepad_hat_t>& joystick_to_gamepad_mapping_t::gamepad_hat_by_joystick_hat() {
    return m_gamepad_hat_by_joystick_hat;
}

const std::unordered_multimap<unsigned int, joystick_to_gamepad_hat_t>& joystick_to_gamepad_mapping_t::gamepad_hat_by_joystick_hat() const {
    return m_gamepad_hat_by_joystick_hat;
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw
