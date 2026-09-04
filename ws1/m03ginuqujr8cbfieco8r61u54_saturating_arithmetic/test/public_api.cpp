#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ginuqujr8cbfieco8r61u54_saturating_arithmetic/api.h>

#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <stdexcept>

namespace api = m03ginuqujr8cbfieco8r61u54_saturating_arithmetic;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        static_assert(std::same_as<decltype(api::add<int>(1, 2)), int>);
        static_assert(std::same_as<decltype(api::sub<int>(1, 2)), int>);
        static_assert(std::same_as<decltype(api::mul<int>(1, 2)), int>);
        static_assert(std::same_as<decltype(api::div<int>(1, 2)), int>);

        test::expect(std::equal_to<>(), api::add(20, 22), 42);
        test::expect(std::equal_to<>(), api::add(std::numeric_limits<int>::max(), 1), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::add(std::numeric_limits<int>::lowest(), -1), std::numeric_limits<int>::lowest());
        test::expect(std::equal_to<>(), api::add(std::numeric_limits<unsigned int>::max(), 1U), std::numeric_limits<unsigned int>::max());

        test::expect(std::equal_to<>(), api::sub(50, 8), 42);
        test::expect(std::equal_to<>(), api::sub(std::numeric_limits<int>::lowest(), 1), std::numeric_limits<int>::lowest());
        test::expect(std::equal_to<>(), api::sub(std::numeric_limits<int>::max(), -1), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::sub(0U, 1U), 0U);

        test::expect(std::equal_to<>(), api::mul(6, 7), 42);
        test::expect(std::equal_to<>(), api::mul(std::numeric_limits<int>::max(), 2), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::mul(std::numeric_limits<int>::lowest(), 2), std::numeric_limits<int>::lowest());
        test::expect(std::equal_to<>(), api::mul(std::numeric_limits<int>::lowest(), -1), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::mul(std::numeric_limits<unsigned int>::max(), 2U), std::numeric_limits<unsigned int>::max());
        test::expect(std::equal_to<>(), api::mul(std::numeric_limits<int>::lowest(), 0), 0);

        test::expect(std::equal_to<>(), api::div(84, 2), 42);
        test::expect(std::equal_to<>(), api::div(std::numeric_limits<int>::lowest(), -1), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::div(1, 0), std::numeric_limits<int>::max());
        test::expect(std::equal_to<>(), api::div(-1, 0), std::numeric_limits<int>::lowest());
        test::expect(std::equal_to<>(), api::div(1U, 0U), std::numeric_limits<unsigned int>::max());

        const auto floating_max = std::numeric_limits<double>::max();
        test::expect(std::equal_to<>(), api::add(floating_max, floating_max), floating_max);
        test::expect(std::equal_to<>(), api::sub(-floating_max, floating_max), std::numeric_limits<double>::lowest());
        test::expect(std::equal_to<>(), api::add(1.25, 2.75), 4.0);
        test::expect(std::equal_to<>(), api::sub(5.5, 1.5), 4.0);
        test::expect(std::equal_to<>(), api::mul(6.0, 7.0), 42.0);
        test::expect(std::equal_to<>(), api::mul(floating_max, 2.0), floating_max);
        test::expect(std::equal_to<>(), api::mul(-floating_max, 2.0), std::numeric_limits<double>::lowest());
        test::expect(std::equal_to<>(), api::div(84.0, 2.0), 42.0);
        test::expect(std::equal_to<>(), api::div(floating_max, 0.5), floating_max);
        test::expect(std::equal_to<>(), api::div(-floating_max, 0.5), std::numeric_limits<double>::lowest());
        test::expect(std::equal_to<>(), api::div(1.0, 0.0), floating_max);
        test::expect(std::equal_to<>(), api::div(-1.0, 0.0), std::numeric_limits<double>::lowest());

        const auto nan = std::numeric_limits<double>::quiet_NaN();
        const auto infinity = std::numeric_limits<double>::infinity();
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::add(nan, 1.0); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::add(1.0, infinity); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::sub(nan, 1.0); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::sub(1.0, infinity); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::mul(nan, 1.0); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::mul(1.0, infinity); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::div(nan, 1.0); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto result = api::div(1.0, infinity); });
    });
}
