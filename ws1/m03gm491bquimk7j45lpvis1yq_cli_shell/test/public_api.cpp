#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gm491bquimk7j45lpvis1yq_cli_shell/api.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "public API headers compile");
    });
}
