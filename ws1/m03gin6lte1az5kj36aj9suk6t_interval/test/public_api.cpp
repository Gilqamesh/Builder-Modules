#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gin6lte1az5kj36aj9suk6t_interval/api.h>

#include <format>
#include <limits>
#include <stdexcept>
#include <string>

namespace api = m03gin6lte1az5kj36aj9suk6t_interval;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        using interval_t = api::interval_t<int>;

        const interval_t empty;
        test::expect(empty.is_empty());
        test::expect_equal(empty[0], 0);
        test::expect_equal(empty[1], 0);
        test::expect_equal(*empty.begin(), 0);
        test::expect_equal(*(empty.end() - 1), 0);
        test::expect_equal(empty.length(), 0);

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const interval_t invalid(2, 1);
        });

        interval_t interval(2, 8);
        interval.bounds(3, 9);
        test::expect_equal(interval[0], 3);
        test::expect_equal(interval[1], 9);
        test::expect_throws<std::invalid_argument>([&] { interval.bounds(10, 9); });
        test::expect_equal(interval[0], 3);
        test::expect_equal(interval[1], 9);

        test::expect_equal((interval + 2)[0], 5);
        test::expect_equal((interval + 2)[1], 11);
        test::expect_equal((interval - 2)[0], 1);
        test::expect_equal((interval - 2)[1], 7);

        auto compound = interval;
        test::expect_equal(&(compound += 2), &compound);
        test::expect_equal(compound[0], 5);
        test::expect_equal(compound[1], 11);
        test::expect_equal(&(compound -= 2), &compound);
        test::expect_equal(compound[0], 3);
        test::expect_equal(compound[1], 9);

        const interval_t near_max(std::numeric_limits<int>::max() - 1, std::numeric_limits<int>::max());
        const auto saturated = near_max + 10;
        test::expect_equal(saturated[0], std::numeric_limits<int>::max());
        test::expect_equal(saturated[1], std::numeric_limits<int>::max());

        test::expect_equal(interval.clamp(1), 3);
        test::expect_equal(interval.clamp(5), 5);
        test::expect_equal(interval.clamp(9), 9);
        test::expect_equal(interval.clamp(20), 9);

        const auto inflated = interval.inflate(2);
        test::expect_equal(inflated[0], 1);
        test::expect_equal(inflated[1], 11);

        const auto deflated = interval.deflate(2);
        test::expect_equal(deflated[0], 5);
        test::expect_equal(deflated[1], 7);

        const auto collapsed = interval.deflate(20);
        test::expect(collapsed.is_empty());
        test::expect_equal(collapsed[0], 6);
        test::expect_equal(collapsed[1], 6);

        const auto intersection = interval.intersect(interval_t(5, 12));
        test::expect_equal(intersection[0], 5);
        test::expect_equal(intersection[1], 9);

        const auto disjoint_intersection = interval.intersect(interval_t(12, 15));
        test::expect(disjoint_intersection.is_empty());
        test::expect_equal(disjoint_intersection[0], 12);
        test::expect_equal(disjoint_intersection[1], 12);

        test::expect(interval.contains(3));
        test::expect(interval.contains(8));
        test::expect(!interval.contains(9));
        test::expect(!empty.contains(0));
        test::expect(interval.overlaps(interval_t(8, 12)));
        test::expect(!interval.overlaps(interval_t(9, 12)));
        test::expect(!interval.overlaps(empty));
        test::expect_equal(interval.length(), 6);
        test::expect_equal(std::format("{}", interval), std::string("[3, 9)"));

        using floating_interval_t = api::interval_t<double>;
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        const auto infinity = std::numeric_limits<double>::infinity();
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const floating_interval_t value(nan, 1.0); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const floating_interval_t value(0.0, infinity); });

        const floating_interval_t floating(0.0, 1.0);
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto value = floating + nan; });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto value = floating - infinity; });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto value = floating.clamp(nan); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto value = floating.inflate(infinity); });
        test::expect_throws<std::invalid_argument>([&] { [[maybe_unused]] const auto value = floating.deflate(nan); });
    });
}
