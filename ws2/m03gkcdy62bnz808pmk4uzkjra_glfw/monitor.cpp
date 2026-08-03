#include "monitor.h"

namespace m03gkcdy62bnz808pmk4uzkjra_glfw {

monitor_t::monitor_t():
    m_handle(nullptr),
    m_video_mode({0, 0, 0, 0, 0, 0}),
    m_virtual_position({0, 0}),
    m_physical_size({0, 0}),
    m_content_scale({1.0f, 1.0f}),
    m_work_area({0, 0, 0, 0})
{
}

GLFWmonitor* monitor_t::handle() const {
    return m_handle;
}

void monitor_t::handle(GLFWmonitor* handle) {
    m_handle = handle;
}

const std::string& monitor_t::name() const {
    if (m_handle) {
        const char* monitor_name = glfwGetMonitorName(m_handle);
        if (monitor_name) {
            m_name = monitor_name;
        }
    }

    return m_name;
}

video_mode_t monitor_t::video_mode() const {
    if (m_handle) {
        const GLFWvidmode* glfw_video_mode = glfwGetVideoMode(m_handle);
        if (glfw_video_mode) {
            m_video_mode = {
                .width = glfw_video_mode->width,
                .height = glfw_video_mode->height,
                .red_bits = glfw_video_mode->redBits,
                .green_bits = glfw_video_mode->greenBits,
                .blue_bits = glfw_video_mode->blueBits,
                .refresh_rate = glfw_video_mode->refreshRate
            };
        }
    }

    return m_video_mode;
}

std::vector<video_mode_t> monitor_t::video_modes() const {
    std::vector<video_mode_t> video_modes;

    if (m_handle) {
        int count;
        const GLFWvidmode* glfw_video_modes = glfwGetVideoModes(m_handle, &count);
        if (glfw_video_modes) {
            video_modes.reserve(count);
            for (int i = 0; i < count; ++i) {
                const GLFWvidmode& glfw_video_mode = glfw_video_modes[i];
                video_mode_t video_mode = {
                    .width = glfw_video_mode.width,
                    .height = glfw_video_mode.height,
                    .red_bits = glfw_video_mode.redBits,
                    .green_bits = glfw_video_mode.greenBits,
                    .blue_bits = glfw_video_mode.blueBits,
                    .refresh_rate = glfw_video_mode.refreshRate
                };
                video_modes.push_back(video_mode);
            }
        }
    }

    return video_modes;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& monitor_t::virtual_position() const {
    if (m_handle) {
        int x;
        int y;
        glfwGetMonitorPos(m_handle, &x, &y);
        m_virtual_position = { x, y };
    }

    return m_virtual_position;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 2>& monitor_t::physical_size() const {
    if (m_handle) {
        int physical_width;
        int physical_height;
        glfwGetMonitorPhysicalSize(m_handle, &physical_width, &physical_height);
        m_physical_size = { physical_width, physical_height };
    }

    return m_physical_size;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>& monitor_t::content_scale() const {
    if (m_handle) {
        float xscale;
        float yscale;
        glfwGetMonitorContentScale(m_handle, &xscale, &yscale);
        m_content_scale = { xscale, yscale };
    }

    return m_content_scale;
}

const m03ginwy24ng8o487c4beoms6l_vector::vector_t<int, 4>& monitor_t::work_area() const {
    if (m_handle) {
        int work_area_x;
        int work_area_y;
        int work_area_width;
        int work_area_height;
        glfwGetMonitorWorkarea(m_handle, &work_area_x, &work_area_y, &work_area_width, &work_area_height);
        m_work_area = { work_area_x, work_area_y, work_area_width, work_area_height };
    }

    return m_work_area;
}

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw
