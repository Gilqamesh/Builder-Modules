#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer/software_renderer.h>

#include <functional>

namespace api = m03gl8a1hl8xe3ynm8s2wwfy4u_software_renderer;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        const api::rgba8_t pixel {
            .red = 1,
            .green = 2,
            .blue = 3,
            .alpha = 4
        };
        test::expect(std::equal_to<>(), pixel.red, std::uint8_t(1));
        test::expect(std::equal_to<>(), pixel.green, std::uint8_t(2));
        test::expect(std::equal_to<>(), pixel.blue, std::uint8_t(3));
        test::expect(std::equal_to<>(), pixel.alpha, std::uint8_t(4));
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::software_renderer_t renderer(nullptr);
        });
    });
}
