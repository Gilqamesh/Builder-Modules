#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03glv28yaiwc5hbnvz43r14zr_matrix/api.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "public API headers compile");
    });
}
