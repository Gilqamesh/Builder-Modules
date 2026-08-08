#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "public API headers compile");
    });
}
