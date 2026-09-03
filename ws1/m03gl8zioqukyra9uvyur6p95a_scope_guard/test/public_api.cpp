#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gl8zioqukyra9uvyur6p95a_scope_guard/api.h>

#include <functional>

namespace api = m03gl8zioqukyra9uvyur6p95a_scope_guard;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        int cleanup_count = 0;
        {
            api::scope_guard_t guard([&] { ++cleanup_count; });
            test::expect(std::equal_to<>(), cleanup_count, 0);
        }
        test::expect(std::equal_to<>(), cleanup_count, 1);

        {
            api::scope_guard_t guard([&] { ++cleanup_count; });
            guard.release();
            guard.release();
        }
        test::expect(std::equal_to<>(), cleanup_count, 1);

        test::expect_no_throw([] {
            api::scope_guard_t guard { std::function<void()>() };
        });
    });
}
