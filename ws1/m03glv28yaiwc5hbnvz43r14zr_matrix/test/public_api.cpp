#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03glv28yaiwc5hbnvz43r14zr_matrix/api.h>

#include <functional>
#include <array>
#include <concepts>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

namespace matrix_api = m03glv28yaiwc5hbnvz43r14zr_matrix;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace vector_api = m03ginwy24ng8o487c4beoms6l_vector;

int main() {
    return test::run([] {
        using matrix_t = matrix_api::matrix_t<int, 2, 3>;

        static_assert(std::same_as<matrix_t::value_type, int>);
        static_assert(matrix_t::row_count == 2);
        static_assert(matrix_t::column_count == 3);

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const matrix_t invalid { 1, 2, 3 };
        });

        const matrix_t filled(4);
        test::expect(std::equal_to<>(), filled, matrix_t({ 4, 4, 4, 4, 4, 4 }));

        const std::array<int, 6> array { 1, 2, 3, 4, 5, 6 };
        const matrix_t from_array(array);
        const matrix_t from_list { 1, 2, 3, 4, 5, 6 };
        test::expect(std::equal_to<>(), from_array, from_list);

        matrix_t copied = from_list;
        matrix_t moved = std::move(copied);
        test::expect(std::equal_to<>(), moved, from_list);

        matrix_t assigned(0);
        assigned = from_list;
        matrix_t move_assigned(0);
        move_assigned = std::move(assigned);
        test::expect(std::equal_to<>(), move_assigned, from_list);

        matrix_t indexed = from_list;
        indexed(1, 1) = 9;
        test::expect(std::equal_to<>(), indexed(1, 1), 9);
        test::expect(std::equal_to<>(), *indexed.begin(), 1);
        test::expect(std::equal_to<>(), *(indexed.end() - 1), 6);

        const matrix_t const_indexed = indexed;
        test::expect(std::equal_to<>(), const_indexed(1, 1), 9);
        test::expect(std::equal_to<>(), *const_indexed.begin(), 1);
        test::expect(std::equal_to<>(), *(const_indexed.end() - 1), 6);

        const matrix_t lhs { 1, 2, 3, 4, 5, 6 };
        const matrix_t rhs { 6, 5, 4, 3, 2, 1 };
        test::expect(std::equal_to<>(), lhs + rhs, matrix_t({ 7, 7, 7, 7, 7, 7 }));
        test::expect(std::equal_to<>(), lhs - rhs, matrix_t({ -5, -3, -1, 1, 3, 5 }));
        test::expect(std::equal_to<>(), -lhs, matrix_t({ -1, -2, -3, -4, -5, -6 }));

        auto compound = lhs;
        test::expect(std::identity(), &(compound += rhs) == &compound);
        test::expect(std::equal_to<>(), compound, matrix_t({ 7, 7, 7, 7, 7, 7 }));
        test::expect(std::identity(), &(compound -= rhs) == &compound);
        test::expect(std::equal_to<>(), compound, lhs);

        test::expect(std::equal_to<>(), lhs * 2, matrix_t({ 2, 4, 6, 8, 10, 12 }));
        test::expect(std::equal_to<>(), (lhs * 6) / 3, matrix_t({ 2, 4, 6, 8, 10, 12 }));

        auto scalar_compound = lhs;
        test::expect(std::identity(), &(scalar_compound *= 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, matrix_t({ 2, 4, 6, 8, 10, 12 }));
        test::expect(std::identity(), &(scalar_compound /= 2) == &scalar_compound);
        test::expect(std::equal_to<>(), scalar_compound, lhs);

        const vector_api::vector_t<int, 3> vector { 1, 2, 3 };
        test::expect(std::equal_to<>(), lhs * vector, vector_api::vector_t<int, 2>({ 14, 32 }));

        const matrix_api::matrix_t<int, 3, 2> other {
            7, 8,
            9, 10,
            11, 12
        };
        test::expect(std::equal_to<>(), lhs * other,
            matrix_api::matrix_t<int, 2, 2>({ 58, 64, 139, 154 })
        );

        test::expect(std::equal_to<>(), std::format("{}", lhs),
            std::string("{ { 1, 2, 3 }, { 4, 5, 6 } }")
        );
    });
}
