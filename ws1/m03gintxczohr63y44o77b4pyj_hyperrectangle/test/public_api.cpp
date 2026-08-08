#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gintxczohr63y44o77b4pyj_hyperrectangle/api.h>

#include <array>
#include <format>
#include <stdexcept>
#include <string>

namespace api = m03gintxczohr63y44o77b4pyj_hyperrectangle;
namespace interval_api = m03gin6lte1az5kj36aj9suk6t_interval;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace vector_api = m03ginwy24ng8o487c4beoms6l_vector;

int main() {
    return test::run([] {
        using hyperrectangle_t = api::hyperrectangle_t<int, 2>;
        using interval_t = interval_api::interval_t<int>;
        using vector_t = vector_api::vector_t<int, 2>;

        const hyperrectangle_t empty;
        test::expect(empty.is_empty());
        test::expect_equal(empty.corner(), vector_t({ 0, 0 }));
        test::expect_equal(empty.opposite_corner(), vector_t({ 0, 0 }));

        const std::array<interval_t, 2> intervals { interval_t(1, 5), interval_t(2, 8) };
        const hyperrectangle_t from_array(intervals);
        const hyperrectangle_t from_list { interval_t(1, 5), interval_t(2, 8) };
        const hyperrectangle_t from_corners(vector_t({ 1, 2 }), vector_t({ 5, 8 }));
        test::expect_equal(from_array.corner(), from_list.corner());
        test::expect_equal(from_array.opposite_corner(), from_list.opposite_corner());
        test::expect_equal(from_array.corner(), from_corners.corner());
        test::expect_equal(from_array.opposite_corner(), from_corners.opposite_corner());

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const hyperrectangle_t invalid { interval_t(0, 1) };
        });
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const hyperrectangle_t invalid(vector_t({ 2, 0 }), vector_t({ 1, 1 }));
        });

        hyperrectangle_t mutable_rectangle;
        mutable_rectangle.bounds(intervals);
        test::expect_equal(mutable_rectangle.bounds()[0][0], 1);
        test::expect_equal(mutable_rectangle.bounds()[1][1], 8);
        mutable_rectangle[0].bounds(3, 6);
        mutable_rectangle.begin()->bounds(1, 5);
        (mutable_rectangle.end() - 1)->bounds(2, 8);
        test::expect_equal(mutable_rectangle.corner(), vector_t({ 1, 2 }));

        const hyperrectangle_t const_rectangle = mutable_rectangle;
        test::expect_equal(const_rectangle[0][0], 1);
        test::expect_equal(const_rectangle.begin()->length(), 4);
        test::expect_equal((const_rectangle.end() - 1)->length(), 6);

        const vector_t displacement { 10, -2 };
        test::expect_equal((from_array + displacement).corner(), vector_t({ 11, 0 }));
        test::expect_equal((from_array + displacement).opposite_corner(), vector_t({ 15, 6 }));
        test::expect_equal((from_array - displacement).corner(), vector_t({ -9, 4 }));
        test::expect_equal((from_array - displacement).opposite_corner(), vector_t({ -5, 10 }));

        auto compound = from_array;
        test::expect_equal(&(compound += displacement), &compound);
        test::expect_equal(compound.corner(), vector_t({ 11, 0 }));
        test::expect_equal(&(compound -= displacement), &compound);
        test::expect_equal(compound.corner(), from_array.corner());
        test::expect_equal(compound.opposite_corner(), from_array.opposite_corner());

        test::expect_equal(from_array.clamp(vector_t({ -5, 20 })), vector_t({ 1, 8 }));
        test::expect_equal(from_array.clamp(vector_t({ 3, 4 })), vector_t({ 3, 4 }));

        const auto scalar_inflated = from_array.inflate(1);
        test::expect_equal(scalar_inflated.corner(), vector_t({ 0, 1 }));
        test::expect_equal(scalar_inflated.opposite_corner(), vector_t({ 6, 9 }));

        const auto vector_inflated = from_array.inflate(vector_t({ 1, 2 }));
        test::expect_equal(vector_inflated.corner(), vector_t({ 0, 0 }));
        test::expect_equal(vector_inflated.opposite_corner(), vector_t({ 6, 10 }));

        const auto scalar_deflated = from_array.deflate(1);
        test::expect_equal(scalar_deflated.corner(), vector_t({ 2, 3 }));
        test::expect_equal(scalar_deflated.opposite_corner(), vector_t({ 4, 7 }));

        const auto vector_deflated = from_array.deflate(vector_t({ 1, 2 }));
        test::expect_equal(vector_deflated.corner(), vector_t({ 2, 4 }));
        test::expect_equal(vector_deflated.opposite_corner(), vector_t({ 4, 6 }));

        const hyperrectangle_t overlapping(vector_t({ 4, 7 }), vector_t({ 10, 12 }));
        const auto intersection = from_array.intersect(overlapping);
        test::expect_equal(intersection.corner(), vector_t({ 4, 7 }));
        test::expect_equal(intersection.opposite_corner(), vector_t({ 5, 8 }));

        const hyperrectangle_t disjoint(vector_t({ 5, 2 }), vector_t({ 7, 8 }));
        const auto empty_intersection = from_array.intersect(disjoint);
        test::expect(empty_intersection.is_empty());

        test::expect(from_array.contains(vector_t({ 1, 2 })));
        test::expect(from_array.contains(vector_t({ 4, 7 })));
        test::expect(!from_array.contains(vector_t({ 5, 7 })));
        test::expect(!from_array.contains(vector_t({ 4, 8 })));
        test::expect(from_array.overlaps(overlapping));
        test::expect(!from_array.overlaps(disjoint));
        test::expect(!from_array.overlaps(empty));

        test::expect_equal(
            std::format("{}", from_array),
            std::string("{ [1, 5), [2, 8) }")
        );
    });
}
