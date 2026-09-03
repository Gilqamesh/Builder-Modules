#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4hzaaehqgc59cj8f9q_rl_imgui/imgui_impl_raylib.h>
#include <m03ge9ij4hzaaehqgc59cj8f9q_rl_imgui/rlImGui.h>
#include <m03ge9ij4hzaaehqgc59cj8f9q_rl_imgui/rlImGuiColors.h>

#include <functional>
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        test::expect(std::identity(), true);
    });
}
