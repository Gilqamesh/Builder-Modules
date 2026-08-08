#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4azvq47860wqjyzskd_function_primitive_lang/function_primitive_lang.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "public API headers compile");
    });
}
