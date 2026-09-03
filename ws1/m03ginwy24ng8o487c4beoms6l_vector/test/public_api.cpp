#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

#include <array>
#include <cmath>
#include <concepts>
#include <format>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

namespace api = m03ginwy24ng8o487c4beoms6l_vector;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        using integer_vector_t = api::vector_t<int, 3>;
        using floating_vector_t = api::vector_t<double, 2>;

        static_assert(std::same_as<integer_vector_t::length_t, double>);
        static_assert(std::same_as<floating_vector_t::length_t, double>);

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const integer_vector_t invalid { 1, 2 };
        });

        const integer_vector_t filled(7);
        test::expect(std::equal_to<>(), filled, integer_vector_t({ 7, 7, 7 }));

        const std::array<int, 3> array { 1, 2, 3 };
        const integer_vector_t from_array(array);
        test::expect(std::equal_to<>(), from_array, integer_vector_t({ 1, 2, 3 }));

        integer_vector_t copied = from_array;
        integer_vector_t moved = std::move(copied);
        test::expect(std::equal_to<>(), moved, from_array);

        integer_vector_t assigned(0);
        assigned = from_array;
        integer_vector_t move_assigned(0);
        move_assigned = std::move(assigned);
        test::expect(std::equal_to<>(), move_assigned, from_array);

        integer_vector_t indexed { 1, 2, 3 };
        indexed[1] = 9;
        test::expect(std::equal_to<>(), indexed[1], 9);
        test::expect(std::equal_to<>(), *indexed.begin(), 1);
        test::expect(std::equal_to<>(), *(indexed.end() - 1), 3);

        const integer_vector_t const_indexed = indexed;
        test::expect(std::equal_to<>(), const_indexed[1], 9);
        test::expect(std::equal_to<>(), *const_indexed.begin(), 1);
        test::expect(std::equal_to<>(), *(const_indexed.end() - 1), 3);

        test::expect(std::identity(), integer_vector_t(0).is_zero());
        test::expect(std::identity(), !integer_vector_t({ 0, 1, 0 }).is_zero());

        const floating_vector_t lengths { 3.0, -4.0 };
        test::expect(std::equal_to<>(), lengths.chebyshev_length(), 4.0);
        test::expect(std::equal_to<>(), lengths.euclidean_length(), 5.0);
        test::expect(std::equal_to<>(), lengths.euclidean_length_squared(), 25.0);
        test::expect(std::equal_to<>(), lengths.manhattan_length(), 7.0);
        test::expect(std::equal_to<>(), lengths.taxicab_length(), 7.0);

        const auto unit = lengths.unit();
        test::expect(std::identity(), std::abs(unit[0] - 0.6) < 1e-12);
        test::expect(std::identity(), std::abs(unit[1] + 0.8) < 1e-12);
        test::expect(std::identity(), std::abs(unit.euclidean_length() - 1.0) < 1e-12);

        const auto nan = std::numeric_limits<double>::quiet_NaN();
        test::expect(std::identity(), !(floating_vector_t({ nan, 0.0 }) == floating_vector_t({ nan, 0.0 })));

        const integer_vector_t lhs { 8, 12, 16 };
        const integer_vector_t rhs { 2, 3, 4 };
        test::expect(std::equal_to<>(), lhs + rhs, integer_vector_t({ 10, 15, 20 }));
        test::expect(std::equal_to<>(), lhs - rhs, integer_vector_t({ 6, 9, 12 }));
        test::expect(std::equal_to<>(), -rhs, integer_vector_t({ -2, -3, -4 }));
        test::expect(std::equal_to<>(), lhs * rhs, integer_vector_t({ 16, 36, 64 }));
        test::expect(std::equal_to<>(), lhs / rhs, integer_vector_t({ 4, 4, 4 }));

        auto vector_compound = lhs;
        test::expect(std::identity(), &(vector_compound += rhs) == &vector_compound);
        test::expect(std::equal_to<>(), vector_compound, integer_vector_t({ 10, 15, 20 }));
        test::expect(std::identity(), &(vector_compound -= rhs) == &vector_compound);
        test::expect(std::equal_to<>(), vector_compound, lhs);
        test::expect(std::identity(), &(vector_compound *= rhs) == &vector_compound);
        test::expect(std::equal_to<>(), vector_compound, integer_vector_t({ 16, 36, 64 }));
        test::expect(std::identity(), &(vector_compound /= rhs) == &vector_compound);
        test::expect(std::equal_to<>(), vector_compound, lhs);

        test::expect(std::equal_to<>(), lhs + 2, integer_vector_t({ 10, 14, 18 }));
        test::expect(std::equal_to<>(), lhs - 2, integer_vector_t({ 6, 10, 14 }));
        test::expect(std::equal_to<>(), lhs * 2, integer_vector_t({ 16, 24, 32 }));
        test::expect(std::equal_to<>(), lhs / 2, integer_vector_t({ 4, 6, 8 }));

        auto scalar_compound = lhs;
        test::expect(std::identity(), &(scalar_compound += 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, integer_vector_t({ 10, 14, 18 }));
        test::expect(std::identity(), &(scalar_compound -= 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, lhs);
        test::expect(std::identity(), &(scalar_compound *= 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, integer_vector_t({ 16, 24, 32 }));
        test::expect(std::identity(), &(scalar_compound /= 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, lhs);

        test::expect(std::equal_to<>(), std::format("{}", from_array), std::string("{ 1, 2, 3 }"));
        test::expect(std::equal_to<>(), std::hash<integer_vector_t>()(from_array), std::hash<integer_vector_t>()(integer_vector_t({ 1, 2, 3 })));
    });
}
