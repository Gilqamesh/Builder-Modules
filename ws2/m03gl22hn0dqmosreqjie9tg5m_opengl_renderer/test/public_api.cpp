#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>

#include <cstddef>
#include <functional>
#include <span>

namespace api = m03gl22hn0dqmosreqjie9tg5m_opengl_renderer;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        using present_rgba8_t = void (api::opengl_renderer_t::*)(std::span<const std::byte>, int, int);
        [[maybe_unused]] const present_rgba8_t present_rgba8 = &api::opengl_renderer_t::present_rgba8;

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::opengl_renderer_t renderer(nullptr);
        });
    });
}
