#include "api.h"

#include <m03gagbhsp2drqq3gkop8pzfrm_workspace_graph/workspace_graph.h>
#include <m03gkcdy62bnz808pmk4uzkjra_glfw/glfw.h>
#include <m03gkcdy62bnz808pmk4uzkjra_glfw/window.h>
#include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace m03gm4hnyxwh4vcy1l7xonx52c_module_editor {

namespace {

namespace workspace_graph = m03gagbhsp2drqq3gkop8pzfrm_workspace_graph;
namespace glfw_api = m03gkcdy62bnz808pmk4uzkjra_glfw;
namespace opengl_renderer_api = m03gl22hn0dqmosreqjie9tg5m_opengl_renderer;

using steady_clock_t = std::chrono::steady_clock;

struct rgba8_t {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;
};

struct rect_t {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool contains(int px, int py) const {
        return x <= px && px < x + width && y <= py && py < y + height;
    }
};

struct module_entry_t {
    workspace_graph::module_name_t name;
    std::string alias;
};

struct token_t {
    std::size_t start = 0;
    std::size_t end = 0;
    std::string text;
};

bool is_alias_character(char c) {
    return c == '_' || std::isalnum(static_cast<unsigned char>(c));
}

bool starts_with(std::string_view text, std::string_view prefix) {
    return prefix.size() <= text.size() && text.substr(0, prefix.size()) == prefix;
}

bool contains_insensitive(std::string_view text, std::string_view filter) {
    if (filter.empty()) {
        return true;
    }
    if (text.size() < filter.size()) {
        return false;
    }

    for (std::size_t pos = 0; pos <= text.size() - filter.size(); ++pos) {
        bool matches = true;
        for (std::size_t index = 0; index < filter.size(); ++index) {
            const auto lhs = static_cast<unsigned char>(text[pos + index]);
            const auto rhs = static_cast<unsigned char>(filter[index]);
            if (std::tolower(lhs) != std::tolower(rhs)) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return true;
        }
    }

    return false;
}

std::string common_prefix(const std::vector<const module_entry_t*>& entries) {
    if (entries.empty()) {
        return {};
    }

    std::string result = entries.front()->alias;
    for (const auto* entry : entries) {
        while (!starts_with(entry->alias, result)) {
            result.pop_back();
            if (result.empty()) {
                return {};
            }
        }
    }

    return result;
}

const std::array<std::uint8_t, 7>& glyph(char c) {
    static const std::array<std::uint8_t, 7> blank{0, 0, 0, 0, 0, 0, 0};
    static const std::array<std::uint8_t, 7> unknown{31, 17, 1, 2, 4, 0, 4};

    if ('A' <= c && c <= 'Z') {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    switch (c) {
        case ' ': return blank;
        case '!': { static const std::array<std::uint8_t, 7> v{4, 4, 4, 4, 4, 0, 4}; return v; }
        case '"': { static const std::array<std::uint8_t, 7> v{10, 10, 10, 0, 0, 0, 0}; return v; }
        case '#': { static const std::array<std::uint8_t, 7> v{10, 10, 31, 10, 31, 10, 10}; return v; }
        case '$': { static const std::array<std::uint8_t, 7> v{4, 15, 20, 14, 5, 30, 4}; return v; }
        case '%': { static const std::array<std::uint8_t, 7> v{24, 25, 2, 4, 8, 19, 3}; return v; }
        case '&': { static const std::array<std::uint8_t, 7> v{12, 18, 20, 8, 21, 18, 13}; return v; }
        case '\'': { static const std::array<std::uint8_t, 7> v{4, 4, 8, 0, 0, 0, 0}; return v; }
        case '(': { static const std::array<std::uint8_t, 7> v{2, 4, 8, 8, 8, 4, 2}; return v; }
        case ')': { static const std::array<std::uint8_t, 7> v{8, 4, 2, 2, 2, 4, 8}; return v; }
        case '*': { static const std::array<std::uint8_t, 7> v{0, 4, 21, 14, 21, 4, 0}; return v; }
        case '+': { static const std::array<std::uint8_t, 7> v{0, 4, 4, 31, 4, 4, 0}; return v; }
        case ',': { static const std::array<std::uint8_t, 7> v{0, 0, 0, 0, 4, 4, 8}; return v; }
        case '-': { static const std::array<std::uint8_t, 7> v{0, 0, 0, 31, 0, 0, 0}; return v; }
        case '.': { static const std::array<std::uint8_t, 7> v{0, 0, 0, 0, 0, 12, 12}; return v; }
        case '/': { static const std::array<std::uint8_t, 7> v{1, 2, 2, 4, 8, 8, 16}; return v; }
        case '0': { static const std::array<std::uint8_t, 7> v{14, 17, 19, 21, 25, 17, 14}; return v; }
        case '1': { static const std::array<std::uint8_t, 7> v{4, 12, 4, 4, 4, 4, 14}; return v; }
        case '2': { static const std::array<std::uint8_t, 7> v{14, 17, 1, 2, 4, 8, 31}; return v; }
        case '3': { static const std::array<std::uint8_t, 7> v{31, 2, 4, 2, 1, 17, 14}; return v; }
        case '4': { static const std::array<std::uint8_t, 7> v{2, 6, 10, 18, 31, 2, 2}; return v; }
        case '5': { static const std::array<std::uint8_t, 7> v{31, 16, 30, 1, 1, 17, 14}; return v; }
        case '6': { static const std::array<std::uint8_t, 7> v{6, 8, 16, 30, 17, 17, 14}; return v; }
        case '7': { static const std::array<std::uint8_t, 7> v{31, 1, 2, 4, 8, 8, 8}; return v; }
        case '8': { static const std::array<std::uint8_t, 7> v{14, 17, 17, 14, 17, 17, 14}; return v; }
        case '9': { static const std::array<std::uint8_t, 7> v{14, 17, 17, 15, 1, 2, 12}; return v; }
        case ':': { static const std::array<std::uint8_t, 7> v{0, 12, 12, 0, 12, 12, 0}; return v; }
        case ';': { static const std::array<std::uint8_t, 7> v{0, 12, 12, 0, 4, 4, 8}; return v; }
        case '<': { static const std::array<std::uint8_t, 7> v{2, 4, 8, 16, 8, 4, 2}; return v; }
        case '=': { static const std::array<std::uint8_t, 7> v{0, 0, 31, 0, 31, 0, 0}; return v; }
        case '>': { static const std::array<std::uint8_t, 7> v{8, 4, 2, 1, 2, 4, 8}; return v; }
        case '?': return unknown;
        case '@': { static const std::array<std::uint8_t, 7> v{14, 17, 23, 21, 23, 16, 14}; return v; }
        case '[': { static const std::array<std::uint8_t, 7> v{14, 8, 8, 8, 8, 8, 14}; return v; }
        case '\\': { static const std::array<std::uint8_t, 7> v{16, 8, 8, 4, 2, 2, 1}; return v; }
        case ']': { static const std::array<std::uint8_t, 7> v{14, 2, 2, 2, 2, 2, 14}; return v; }
        case '^': { static const std::array<std::uint8_t, 7> v{4, 10, 17, 0, 0, 0, 0}; return v; }
        case '_': { static const std::array<std::uint8_t, 7> v{0, 0, 0, 0, 0, 0, 31}; return v; }
        case '`': { static const std::array<std::uint8_t, 7> v{8, 4, 2, 0, 0, 0, 0}; return v; }
        case 'a': { static const std::array<std::uint8_t, 7> v{0, 0, 14, 1, 15, 17, 15}; return v; }
        case 'b': { static const std::array<std::uint8_t, 7> v{16, 16, 22, 25, 17, 17, 30}; return v; }
        case 'c': { static const std::array<std::uint8_t, 7> v{0, 0, 14, 16, 16, 17, 14}; return v; }
        case 'd': { static const std::array<std::uint8_t, 7> v{1, 1, 13, 19, 17, 17, 15}; return v; }
        case 'e': { static const std::array<std::uint8_t, 7> v{0, 0, 14, 17, 31, 16, 14}; return v; }
        case 'f': { static const std::array<std::uint8_t, 7> v{6, 8, 30, 8, 8, 8, 8}; return v; }
        case 'g': { static const std::array<std::uint8_t, 7> v{0, 15, 17, 17, 15, 1, 14}; return v; }
        case 'h': { static const std::array<std::uint8_t, 7> v{16, 16, 22, 25, 17, 17, 17}; return v; }
        case 'i': { static const std::array<std::uint8_t, 7> v{4, 0, 12, 4, 4, 4, 14}; return v; }
        case 'j': { static const std::array<std::uint8_t, 7> v{2, 0, 6, 2, 2, 18, 12}; return v; }
        case 'k': { static const std::array<std::uint8_t, 7> v{16, 16, 18, 20, 24, 20, 18}; return v; }
        case 'l': { static const std::array<std::uint8_t, 7> v{12, 4, 4, 4, 4, 4, 14}; return v; }
        case 'm': { static const std::array<std::uint8_t, 7> v{0, 0, 26, 21, 21, 21, 21}; return v; }
        case 'n': { static const std::array<std::uint8_t, 7> v{0, 0, 22, 25, 17, 17, 17}; return v; }
        case 'o': { static const std::array<std::uint8_t, 7> v{0, 0, 14, 17, 17, 17, 14}; return v; }
        case 'p': { static const std::array<std::uint8_t, 7> v{0, 0, 30, 17, 30, 16, 16}; return v; }
        case 'q': { static const std::array<std::uint8_t, 7> v{0, 0, 13, 19, 15, 1, 1}; return v; }
        case 'r': { static const std::array<std::uint8_t, 7> v{0, 0, 22, 25, 16, 16, 16}; return v; }
        case 's': { static const std::array<std::uint8_t, 7> v{0, 0, 15, 16, 14, 1, 30}; return v; }
        case 't': { static const std::array<std::uint8_t, 7> v{8, 8, 30, 8, 8, 9, 6}; return v; }
        case 'u': { static const std::array<std::uint8_t, 7> v{0, 0, 17, 17, 17, 19, 13}; return v; }
        case 'v': { static const std::array<std::uint8_t, 7> v{0, 0, 17, 17, 17, 10, 4}; return v; }
        case 'w': { static const std::array<std::uint8_t, 7> v{0, 0, 17, 17, 21, 21, 10}; return v; }
        case 'x': { static const std::array<std::uint8_t, 7> v{0, 0, 17, 10, 4, 10, 17}; return v; }
        case 'y': { static const std::array<std::uint8_t, 7> v{0, 0, 17, 17, 15, 1, 14}; return v; }
        case 'z': { static const std::array<std::uint8_t, 7> v{0, 0, 31, 2, 4, 8, 31}; return v; }
        case '{': { static const std::array<std::uint8_t, 7> v{2, 4, 4, 8, 4, 4, 2}; return v; }
        case '|': { static const std::array<std::uint8_t, 7> v{4, 4, 4, 0, 4, 4, 4}; return v; }
        case '}': { static const std::array<std::uint8_t, 7> v{8, 4, 4, 2, 4, 4, 8}; return v; }
        case '~': { static const std::array<std::uint8_t, 7> v{0, 0, 8, 21, 2, 0, 0}; return v; }
        default: return unknown;
    }
}

class opengl_framebuffer_presenter_t {
public:
    explicit opengl_framebuffer_presenter_t(const opengl_renderer_api::opengl_renderer_t& renderer):
        m_gl(&renderer.get_gl())
    {
    }

    void present(glfw_api::window_t& window, int width, int height, const std::vector<rgba8_t>& pixels) const {
        if (width <= 0 || height <= 0) {
            return;
        }
        if (!window.context_current(true)) {
            throw std::runtime_error("module_editor: failed to make the OpenGL context current");
        }

        m_gl->Viewport(0, 0, width, height);
        m_gl->ClearColor(0.08f, 0.09f, 0.10f, 1.0f);
        m_gl->Clear(GL_COLOR_BUFFER_BIT);
        m_gl->MatrixMode(GL_PROJECTION);
        m_gl->LoadIdentity();
        m_gl->Ortho(0.0, width, 0.0, height, -1.0, 1.0);
        m_gl->MatrixMode(GL_MODELVIEW);
        m_gl->LoadIdentity();
        m_gl->RasterPos2i(0, 0);
        m_gl->DrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        window.swap_buffers();
    }

private:
    const GladGLContext* m_gl;
};

class pixel_surface_t {
public:
    void resize(int width, int height) {
        if (width == m_width && height == m_height) {
            return;
        }
        m_width = std::max(0, width);
        m_height = std::max(0, height);
        m_pixels.assign(static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height), rgba8_t{0, 0, 0, 255});
    }

    int width() const {
        return m_width;
    }

    int height() const {
        return m_height;
    }

    const std::vector<rgba8_t>& pixels() const {
        return m_pixels;
    }

    void clear(rgba8_t color) {
        std::fill(m_pixels.begin(), m_pixels.end(), color);
    }

    void pixel(int x, int y, rgba8_t color) {
        if (x < 0 || y < 0 || m_width <= x || m_height <= y) {
            return;
        }

        const std::size_t index = static_cast<std::size_t>(m_height - 1 - y) * static_cast<std::size_t>(m_width) + static_cast<std::size_t>(x);
        m_pixels[index] = color;
    }

    void rect(rect_t rect, rgba8_t color) {
        const int left = std::clamp(rect.x, 0, m_width);
        const int top = std::clamp(rect.y, 0, m_height);
        const int right = std::clamp(rect.x + rect.width, 0, m_width);
        const int bottom = std::clamp(rect.y + rect.height, 0, m_height);

        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                pixel(x, y, color);
            }
        }
    }

    void frame(rect_t rect, rgba8_t color) {
        this->rect({rect.x, rect.y, rect.width, 1}, color);
        this->rect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
        this->rect({rect.x, rect.y, 1, rect.height}, color);
        this->rect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
    }

    void text(int x, int y, std::string_view text, rgba8_t color, int max_width = -1) {
        int cursor_x = x;
        for (const char c : text) {
            if (max_width >= 0 && x + max_width < cursor_x + char_width()) {
                break;
            }
            character(cursor_x, y, c, color);
            cursor_x += char_width();
        }
    }

    static int char_width() {
        return 8;
    }

    static int line_height() {
        return 12;
    }

private:
    void character(int x, int y, char c, rgba8_t color) {
        const auto& rows = glyph(c);
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if ((rows[static_cast<std::size_t>(row)] & (1u << (4 - column))) == 0) {
                    continue;
                }

                rect({x + column, y + row, 1, 1}, color);
            }
        }
    }

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<rgba8_t> m_pixels;
};

class module_editor_t {
public:
    module_editor_t():
        m_text("# module_editor\n"),
        m_cursor(m_text.size()),
        m_selected_module_index(0),
        m_cursor_visible_until(steady_clock_t::now())
    {
        load_modules();
        update_completion();
    }

    int run() {
        glfw_api::glfw_t glfw;

        glfw_api::window_creation_settings_t settings;
        settings.opengl(2, 1, glfw_api::opengl_profile_t::any);

        auto window = glfw_api::window_t::create("module_editor", {100, 100, 1200, 760}, settings);
        if (!window) {
            throw std::runtime_error("module_editor: failed to create GLFW window");
        }

        opengl_renderer_api::opengl_renderer_t opengl_renderer(window);
        window->swap_interval(1);

        m_window = window.get();
        active_editor_guard_t active_editor_guard(this);
        install_callbacks(*window);

        opengl_framebuffer_presenter_t presenter(opengl_renderer);

        while (!window->should_close()) {
            glfw_api::poll_events();

            const auto framebuffer_size = window->framebuffer_size();
            if (framebuffer_size[0] <= 0 || framebuffer_size[1] <= 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }

            m_surface.resize(framebuffer_size[0], framebuffer_size[1]);
            draw();
            presenter.present(*window, m_surface.width(), m_surface.height(), m_surface.pixels());
        }

        m_window = nullptr;
        return 0;
    }

private:
    class active_editor_guard_t {
    public:
        explicit active_editor_guard_t(module_editor_t* editor) {
            s_active_editor = editor;
        }

        ~active_editor_guard_t() {
            s_active_editor = nullptr;
        }
    };

    void load_modules() {
        const auto context = workspace_graph::invocation_context();
        workspace_graph::workspace_graph_t graph(context.workspace_root, context.artifact_root);
        const std::set<workspace_graph::module_name_t> module_names = graph.module_names();

        std::map<std::string, int> friendly_name_counts;
        for (const auto& module_name : module_names) {
            ++friendly_name_counts[module_name.friendly_name()];
        }

        for (const auto& module_name : module_names) {
            const std::string friendly_name = module_name.friendly_name();
            if (friendly_name_counts[friendly_name] == 1) {
                m_modules.push_back(module_entry_t{.name = module_name, .alias = friendly_name});
            }
            m_modules.push_back(module_entry_t{.name = module_name, .alias = module_name.unique_name()});
        }

        std::sort(m_modules.begin(), m_modules.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.alias != rhs.alias) {
                return lhs.alias < rhs.alias;
            }
            return lhs.name.unique_name() < rhs.name.unique_name();
        });
    }

    void install_callbacks(glfw_api::window_t& window) {
        window.char_callback(&module_editor_t::char_callback);
        window.key_callback(&module_editor_t::key_callback);
        window.cursor_position_callback(&module_editor_t::cursor_position_callback);
        window.mouse_button_callback(&module_editor_t::mouse_button_callback);
        window.scroll_callback(&module_editor_t::scroll_callback);
    }

    static void char_callback(glfw_api::window_t*, std::uint32_t codepoint) {
        if (s_active_editor && 32 <= codepoint && codepoint <= 126) {
            s_active_editor->insert_text(std::string(1, static_cast<char>(codepoint)));
        }
    }

    static void key_callback(glfw_api::window_t*, int key, int, int action, int) {
        if (s_active_editor && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            s_active_editor->handle_key(key);
        }
    }

    static void cursor_position_callback(glfw_api::window_t*, double x, double y) {
        if (!s_active_editor) {
            return;
        }
        s_active_editor->m_mouse_x = static_cast<int>(x * s_active_editor->framebuffer_scale_x());
        s_active_editor->m_mouse_y = static_cast<int>(y * s_active_editor->framebuffer_scale_y());
    }

    static void mouse_button_callback(glfw_api::window_t*, int button, int action, int) {
        if (s_active_editor && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            s_active_editor->handle_click(s_active_editor->m_mouse_x, s_active_editor->m_mouse_y);
        }
    }

    static void scroll_callback(glfw_api::window_t*, double, double yoffset) {
        if (s_active_editor) {
            s_active_editor->handle_scroll(s_active_editor->m_mouse_x, s_active_editor->m_mouse_y, yoffset);
        }
    }

    double framebuffer_scale_x() const {
        const auto size = m_window->size();
        if (size[0] <= 0) {
            return 1.0;
        }
        return static_cast<double>(m_surface.width()) / static_cast<double>(size[0]);
    }

    double framebuffer_scale_y() const {
        const auto size = m_window->size();
        if (size[1] <= 0) {
            return 1.0;
        }
        return static_cast<double>(m_surface.height()) / static_cast<double>(size[1]);
    }

    void handle_key(int key) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                m_window->should_close(true);
                break;
            case GLFW_KEY_TAB:
                complete_alias();
                break;
            case GLFW_KEY_ENTER:
                insert_text("\n");
                break;
            case GLFW_KEY_BACKSPACE:
                erase_before_cursor();
                break;
            case GLFW_KEY_DELETE:
                erase_after_cursor();
                break;
            case GLFW_KEY_LEFT:
                if (0 < m_cursor) {
                    --m_cursor;
                    reset_cursor_blink();
                }
                break;
            case GLFW_KEY_RIGHT:
                if (m_cursor < m_text.size()) {
                    ++m_cursor;
                    reset_cursor_blink();
                }
                break;
            case GLFW_KEY_UP:
                move_cursor_vertical(-1);
                break;
            case GLFW_KEY_DOWN:
                move_cursor_vertical(1);
                break;
            case GLFW_KEY_HOME:
                m_cursor = line_start(m_cursor);
                reset_cursor_blink();
                break;
            case GLFW_KEY_END:
                m_cursor = line_end(m_cursor);
                reset_cursor_blink();
                break;
            default:
                break;
        }

        ensure_cursor_visible();
        update_completion();
    }

    void handle_click(int x, int y) {
        if (m_modules_rect.contains(x, y)) {
            const int row = (y - (m_modules_rect.y + 26)) / pixel_surface_t::line_height();
            const int index = module_index_from_visible_row(row);
            if (0 <= index) {
                m_selected_module_index = index;
                insert_text(m_modules[static_cast<std::size_t>(index)].alias);
            }
        } else if (m_completion_rect.contains(x, y)) {
            const int row = (y - (m_completion_rect.y + 26)) / pixel_surface_t::line_height();
            if (0 <= row && row < static_cast<int>(m_completion.size())) {
                replace_token_before_cursor(m_completion[static_cast<std::size_t>(row)]->alias);
            }
        } else if (m_editor_rect.contains(x, y)) {
            set_cursor_from_editor_position(x, y);
        }

        ensure_cursor_visible();
        update_completion();
    }

    void handle_scroll(int x, int y, double yoffset) {
        const int delta = yoffset > 0.0 ? -3 : 3;
        if (m_modules_rect.contains(x, y)) {
            m_module_scroll = std::max(0, m_module_scroll + delta);
        } else if (m_editor_rect.contains(x, y)) {
            m_line_scroll = std::max(0, m_line_scroll + delta);
        }
    }

    void insert_text(std::string_view value) {
        m_text.insert(m_cursor, value);
        m_cursor += value.size();
        reset_cursor_blink();
        ensure_cursor_visible();
        update_completion();
    }

    void erase_before_cursor() {
        if (m_cursor == 0) {
            return;
        }
        m_text.erase(m_cursor - 1, 1);
        --m_cursor;
        reset_cursor_blink();
    }

    void erase_after_cursor() {
        if (m_cursor == m_text.size()) {
            return;
        }
        m_text.erase(m_cursor, 1);
        reset_cursor_blink();
    }

    std::size_t line_start(std::size_t position) const {
        position = std::min(position, m_text.size());
        while (0 < position && m_text[position - 1] != '\n') {
            --position;
        }
        return position;
    }

    std::size_t line_end(std::size_t position) const {
        position = std::min(position, m_text.size());
        while (position < m_text.size() && m_text[position] != '\n') {
            ++position;
        }
        return position;
    }

    int line_number(std::size_t position) const {
        int line = 0;
        for (std::size_t index = 0; index < std::min(position, m_text.size()); ++index) {
            if (m_text[index] == '\n') {
                ++line;
            }
        }
        return line;
    }

    int column_number(std::size_t position) const {
        return static_cast<int>(std::min(position, m_text.size()) - line_start(position));
    }

    std::size_t position_for_line_column(int target_line, int target_column) const {
        std::size_t position = 0;
        int line = 0;
        while (line < target_line && position < m_text.size()) {
            if (m_text[position++] == '\n') {
                ++line;
            }
        }

        const std::size_t end = line_end(position);
        return std::min(end, position + static_cast<std::size_t>(std::max(0, target_column)));
    }

    void move_cursor_vertical(int delta) {
        const int current_line = line_number(m_cursor);
        const int current_column = column_number(m_cursor);
        m_cursor = position_for_line_column(std::max(0, current_line + delta), current_column);
        reset_cursor_blink();
    }

    token_t token_before_cursor() const {
        std::size_t start = m_cursor;
        while (0 < start && is_alias_character(m_text[start - 1])) {
            --start;
        }
        return token_t{.start = start, .end = m_cursor, .text = m_text.substr(start, m_cursor - start)};
    }

    std::vector<const module_entry_t*> matching_modules(std::string_view prefix) const {
        std::vector<const module_entry_t*> result;
        if (prefix.empty()) {
            return result;
        }

        for (const auto& module : m_modules) {
            if (starts_with(module.alias, prefix)) {
                result.push_back(&module);
            }
        }

        return result;
    }

    void update_completion() {
        const token_t token = token_before_cursor();
        m_completion_token = token.text;
        m_completion = matching_modules(token.text);
    }

    void replace_token_before_cursor(std::string_view replacement) {
        const token_t token = token_before_cursor();
        m_text.replace(token.start, token.end - token.start, replacement);
        m_cursor = token.start + replacement.size();
        reset_cursor_blink();
        update_completion();
    }

    void complete_alias() {
        const token_t token = token_before_cursor();
        const auto matches = matching_modules(token.text);
        if (matches.empty()) {
            return;
        }

        if (matches.size() == 1) {
            replace_token_before_cursor(matches.front()->alias);
            return;
        }

        const std::string prefix = common_prefix(matches);
        if (token.text.size() < prefix.size()) {
            replace_token_before_cursor(prefix);
        }
    }

    int visible_module_count() const {
        int result = 0;
        for (const auto& module : m_modules) {
            if (module_matches_filter(module)) {
                ++result;
            }
        }
        return result;
    }

    int module_index_from_visible_row(int target_row) const {
        if (target_row < 0) {
            return -1;
        }

        int visible_row = 0;
        for (int index = 0; index < static_cast<int>(m_modules.size()); ++index) {
            const auto& module = m_modules[static_cast<std::size_t>(index)];
            if (!module_matches_filter(module)) {
                continue;
            }
            if (visible_row == target_row + m_module_scroll) {
                return index;
            }
            ++visible_row;
        }

        return -1;
    }

    bool module_matches_filter(const module_entry_t& module) const {
        return contains_insensitive(module.alias, m_module_filter) ||
            contains_insensitive(module.name.unique_name(), m_module_filter) ||
            contains_insensitive(module.name.friendly_name(), m_module_filter);
    }

    void set_cursor_from_editor_position(int x, int y) {
        const int line = m_line_scroll + std::max(0, (y - m_editor_rect.y - 8) / pixel_surface_t::line_height());
        const int column = std::max(0, (x - m_editor_rect.x - 8) / pixel_surface_t::char_width());
        m_cursor = position_for_line_column(line, column);
        reset_cursor_blink();
    }

    void ensure_cursor_visible() {
        const int line = line_number(m_cursor);
        const int visible_lines = std::max(1, (m_editor_rect.height - 16) / pixel_surface_t::line_height());
        if (line < m_line_scroll) {
            m_line_scroll = line;
        } else if (m_line_scroll + visible_lines <= line) {
            m_line_scroll = line - visible_lines + 1;
        }
    }

    void reset_cursor_blink() {
        m_cursor_visible_until = steady_clock_t::now() + std::chrono::milliseconds(650);
    }

    void draw() {
        m_surface.clear(rgba8_t{24, 26, 29, 255});
        compute_layout();

        draw_header();
        draw_modules();
        draw_editor();
        draw_completions();
    }

    void compute_layout() {
        const int width = m_surface.width();
        const int height = m_surface.height();
        const int header_height = 30;
        const int left_width = std::min(360, std::max(260, width / 3));
        const int completion_height = std::min(150, std::max(96, height / 5));

        m_header_rect = {0, 0, width, header_height};
        m_modules_rect = {8, header_height + 8, left_width - 16, height - header_height - 16};
        m_editor_rect = {left_width + 8, header_height + 8, width - left_width - 16, height - header_height - completion_height - 18};
        m_completion_rect = {left_width + 8, m_editor_rect.y + m_editor_rect.height + 8, width - left_width - 16, completion_height};
    }

    void draw_header() {
        m_surface.rect(m_header_rect, rgba8_t{35, 39, 44, 255});
        m_surface.text(10, 10, "module_editor", rgba8_t{231, 235, 240, 255});
        m_surface.text(145, 10, std::format("{} aliases", m_modules.size()), rgba8_t{149, 160, 174, 255});
    }

    void draw_modules() {
        m_surface.rect(m_modules_rect, rgba8_t{30, 34, 39, 255});
        m_surface.frame(m_modules_rect, rgba8_t{69, 78, 90, 255});
        m_surface.text(m_modules_rect.x + 8, m_modules_rect.y + 8, "modules", rgba8_t{231, 235, 240, 255});

        const int list_y = m_modules_rect.y + 26;
        const int row_height = pixel_surface_t::line_height();
        const int visible_rows = std::max(0, (m_modules_rect.height - 34) / row_height);
        const int total_visible = visible_module_count();
        m_module_scroll = std::clamp(m_module_scroll, 0, std::max(0, total_visible - visible_rows));

        for (int row = 0; row < visible_rows; ++row) {
            const int index = module_index_from_visible_row(row);
            if (index < 0) {
                break;
            }

            const auto& module = m_modules[static_cast<std::size_t>(index)];
            const int y = list_y + row * row_height;
            if (index == m_selected_module_index) {
                m_surface.rect({m_modules_rect.x + 4, y - 2, m_modules_rect.width - 8, row_height}, rgba8_t{52, 78, 103, 255});
            }

            m_surface.text(m_modules_rect.x + 8, y, module.alias, rgba8_t{220, 226, 233, 255}, m_modules_rect.width - 16);
        }
    }

    void draw_editor() {
        m_surface.rect(m_editor_rect, rgba8_t{18, 21, 25, 255});
        m_surface.frame(m_editor_rect, rgba8_t{77, 88, 102, 255});
        draw_editor_text();
        draw_cursor();
    }

    void draw_editor_text() {
        const int x = m_editor_rect.x + 8;
        const int y = m_editor_rect.y + 8;
        const int max_width = m_editor_rect.width - 16;
        const int visible_lines = std::max(0, (m_editor_rect.height - 16) / pixel_surface_t::line_height());

        int current_line = 0;
        std::size_t line_start_pos = 0;
        while (line_start_pos <= m_text.size() && current_line < m_line_scroll + visible_lines) {
            const std::size_t end = line_end(line_start_pos);
            if (current_line >= m_line_scroll) {
                const int draw_y = y + (current_line - m_line_scroll) * pixel_surface_t::line_height();
                m_surface.text(x, draw_y, std::string_view(m_text).substr(line_start_pos, end - line_start_pos), rgba8_t{220, 226, 233, 255}, max_width);
            }

            if (end == m_text.size()) {
                break;
            }
            line_start_pos = end + 1;
            ++current_line;
        }
    }

    void draw_cursor() {
        const auto now = steady_clock_t::now();
        const bool visible = now < m_cursor_visible_until || (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 500) % 2 == 0;
        if (!visible) {
            return;
        }

        const int line = line_number(m_cursor);
        if (line < m_line_scroll) {
            return;
        }

        const int column = column_number(m_cursor);
        const int x = m_editor_rect.x + 8 + column * pixel_surface_t::char_width();
        const int y = m_editor_rect.y + 8 + (line - m_line_scroll) * pixel_surface_t::line_height();
        if (m_editor_rect.contains(x, y)) {
            m_surface.rect({x, y, 1, pixel_surface_t::line_height() - 2}, rgba8_t{255, 255, 255, 255});
        }
    }

    void draw_completions() {
        m_surface.rect(m_completion_rect, rgba8_t{27, 31, 36, 255});
        m_surface.frame(m_completion_rect, rgba8_t{69, 78, 90, 255});
        m_surface.text(m_completion_rect.x + 8, m_completion_rect.y + 8, std::format("completions: {}", m_completion_token), rgba8_t{231, 235, 240, 255}, m_completion_rect.width - 16);

        const int list_y = m_completion_rect.y + 26;
        const int row_height = pixel_surface_t::line_height();
        const int rows = std::max(0, (m_completion_rect.height - 34) / row_height);
        for (int row = 0; row < rows && row < static_cast<int>(m_completion.size()); ++row) {
            const auto* module = m_completion[static_cast<std::size_t>(row)];
            const int y = list_y + row * row_height;
            m_surface.text(m_completion_rect.x + 8, y, module->alias, rgba8_t{170, 220, 255, 255}, m_completion_rect.width - 16);
        }
    }

private:
    std::vector<module_entry_t> m_modules;
    std::vector<const module_entry_t*> m_completion;
    std::string m_completion_token;
    std::string m_module_filter;
    std::string m_text;
    std::size_t m_cursor;
    int m_selected_module_index;
    int m_module_scroll = 0;
    int m_line_scroll = 0;
    int m_mouse_x = 0;
    int m_mouse_y = 0;
    steady_clock_t::time_point m_cursor_visible_until;
    glfw_api::window_t* m_window = nullptr;
    pixel_surface_t m_surface;
    rect_t m_header_rect;
    rect_t m_modules_rect;
    rect_t m_editor_rect;
    rect_t m_completion_rect;
    inline static module_editor_t* s_active_editor = nullptr;
};

} // namespace

int run() {
    try {
        module_editor_t editor;
        return editor.run();
    } catch (const std::exception& exception) {
        std::cerr << std::format("module_editor: {}\n", exception.what());
        return 1;
    }
}

} // namespace m03gm4hnyxwh4vcy1l7xonx52c_module_editor
