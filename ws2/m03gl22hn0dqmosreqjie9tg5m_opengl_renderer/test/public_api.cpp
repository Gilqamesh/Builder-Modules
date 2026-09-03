#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gl22hn0dqmosreqjie9tg5m_opengl_renderer/api.h>

#include <functional>
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        test::expect(std::identity(), true);
    });
}
