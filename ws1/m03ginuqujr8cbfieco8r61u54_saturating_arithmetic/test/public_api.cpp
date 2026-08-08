#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ginuqujr8cbfieco8r61u54_saturating_arithmetic/api.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "public API headers compile");
    });
}
