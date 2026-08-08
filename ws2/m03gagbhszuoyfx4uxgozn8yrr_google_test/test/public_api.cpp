#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "GoogleTest public headers compile");
    });
}
