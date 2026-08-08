#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>

#include <stdexcept>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true);
        test::expect_equal(3, 3);
        test::expect_not_equal(3, 4);
        test::expect_no_throw([] {});
        test::expect_throws<std::runtime_error>([] {
            throw std::runtime_error("expected");
        });
    });
}
