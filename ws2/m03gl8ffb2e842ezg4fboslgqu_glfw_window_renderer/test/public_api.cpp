#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer/glfw_window_renderer.h>

#include <functional>
#include <memory>
#include <type_traits>

namespace api = m03gl8ffb2e842ezg4fboslgqu_glfw_window_renderer;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        static_assert(!std::is_copy_constructible_v<api::glfw_window_renderer_t>);
        static_assert(!std::is_copy_assignable_v<api::glfw_window_renderer_t>);
        static_assert(!std::is_move_constructible_v<api::glfw_window_renderer_t>);
        static_assert(!std::is_move_assignable_v<api::glfw_window_renderer_t>);
        static_assert(std::has_virtual_destructor_v<api::glfw_window_renderer_t>);

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::glfw_window_renderer_t renderer(nullptr);
        });
    });
}
