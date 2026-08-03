#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_MONITOR_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_MONITOR_H

# include "glfw_external.h"

# include <string>
# include <format>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

/**
 * @brief Describes a monitor video mode in pixels, channel bits and hertz.
 */
struct video_mode_t {
    int width;
    int height;
    int red_bits;
    int green_bits;
    int blue_bits;
    int refresh_rate;
};

/**
 * @brief Represents one monitor connection through a non-owning GLFW handle.
 *
 * Property getters update their cached values while connected and otherwise return the last successfully queried value; video_modes returns an empty vector when disconnected.
 */
class monitor_t {
public:
    monitor_t();

    /**
     * @brief Returns the non-owning GLFW handle, or nullptr after disconnection.
     */
    GLFWmonitor* handle() const;

    /**
     * @brief Associates the wrapper with a non-owning GLFW handle without clearing cached properties; only the module runtime should call this.
     */
    void handle(GLFWmonitor* handle);

    /**
     * @brief Returns the current UTF-8 name, or the last successfully queried name if disconnected.
     */
    const std::string& name() const;

    /**
     * @brief Returns the current video mode, or the last successfully queried mode if disconnected.
     */
    video_mode_t video_mode() const;

    /**
     * @brief Returns the currently advertised video modes, or an empty vector if disconnected.
     */
    std::vector<video_mode_t> video_modes() const;

    /**
     * @brief Returns the virtual position in screen coordinates, or the last successfully queried position if disconnected.
     */
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& virtual_position() const;

    /**
     * @brief Returns the physical size in millimeters, or the last successfully queried size if disconnected.
     */
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& physical_size() const;

    /**
     * @brief Returns the DPI scale relative to the platform default, or the last successfully queried scale if disconnected.
     */
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& content_scale() const;

    /**
     * @brief Returns the screen-coordinate area not occupied by system UI, or the last successfully queried area if disconnected.
     */
    const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4>& work_area() const;

private:
    GLFWmonitor* m_handle;
    mutable std::string m_name;
    mutable video_mode_t m_video_mode;
    mutable m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> m_virtual_position; // [screen coordinates]
    mutable m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2> m_physical_size; // [mm]
    mutable m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2> m_content_scale; // [DPI / Platform DPI]
    mutable m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4> m_work_area; // [screen coordinates]
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::video_mode_t>;

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::monitor_t>;

} // namespace std

namespace std {

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::video_mode_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::video_mode_t& video_mode, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "width: {}, ", video_mode.width);
        out = std::format_to(out, "height: {}, ", video_mode.height);
        out = std::format_to(out, "red bits: {}, ", video_mode.red_bits);
        out = std::format_to(out, "green bits: {}, ", video_mode.green_bits);
        out = std::format_to(out, "blue bits: {}, ", video_mode.blue_bits);
        out = std::format_to(out, "refresh rate: {}", video_mode.refresh_rate);

        out = std::format_to(out, " }}");

        return out;
    };
};

template <>
struct formatter<m03gkcdy62bnz808pmk4uzkjra_glfw::monitor_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const m03gkcdy62bnz808pmk4uzkjra_glfw::monitor_t& monitor, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");

        out = std::format_to(out, "handle: {}, ", static_cast<void*>(monitor.handle()));

        out = std::format_to(out, "name: {}, ", monitor.name());

        out = std::format_to(out, "video mode: {}, ", monitor.video_mode());

        out = std::format_to(out, "virtual position [screen coordinates]: {}, ", monitor.virtual_position());

        out = std::format_to(out, "physical size [mm]: {}, ", monitor.physical_size());

        out = std::format_to(out, "content scale [DPI / platform default DPI]: {}, ", monitor.content_scale());

        out = std::format_to(out, "work area [screen coordinates]: {}", monitor.work_area());

        out = std::format_to(out, " }}");

        return out;
    };
};

} // namespace std

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_MONITOR_H
