#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>

#include <functional>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace api = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

struct throwing_value_t {
    int value;
    static bool fail_on_move;

    throwing_value_t(int value): value(value) {}
    throwing_value_t(const throwing_value_t&) = default;
    throwing_value_t& operator=(const throwing_value_t&) = default;
    throwing_value_t(throwing_value_t&& other): value(other.value) {
        if (fail_on_move) {
            throw std::runtime_error("move failed");
        }
    }
    throwing_value_t& operator=(throwing_value_t&&) = default;
};

bool throwing_value_t::fail_on_move = false;

} // namespace


int main() {
    return test::run([] {
        using structure_t = api::structure_of_arrays_t<int, float>;

        structure_t structure;
        structure.push_back(1, 1.5F);
        structure.push_back(2, 2.5F);

        const auto& data = structure.data();
        test::expect(std::equal_to<>(), std::get<0>(data).size(), std::size_t(2));
        test::expect(std::equal_to<>(), std::get<0>(data)[0], 1);
        test::expect(std::equal_to<>(), std::get<0>(data)[1], 2);
        test::expect(std::equal_to<>(), std::get<1>(data).size(), std::size_t(2));
        test::expect(std::equal_to<>(), std::get<1>(data)[0], 1.5F);
        test::expect(std::equal_to<>(), std::get<1>(data)[1], 2.5F);

        api::structure_of_arrays_t<int, throwing_value_t> exception_safe;
        throwing_value_t::fail_on_move = true;
        test::expect_throws<std::runtime_error>([&] {
            exception_safe.push_back(1, throwing_value_t(2));
        });
        test::expect(std::identity(), std::get<0>(exception_safe.data()).empty());
        test::expect(std::identity(), std::get<1>(exception_safe.data()).empty());
        throwing_value_t::fail_on_move = false;
        exception_safe.push_back(3, throwing_value_t(4));
        test::expect(std::equal_to<>(), std::get<0>(exception_safe.data()).size(), std::size_t(1));
        test::expect(std::equal_to<>(), std::get<1>(exception_safe.data()).size(), std::size_t(1));

        const auto formatted_structure = std::format("{}", structure);
        test::expect(std::identity(), formatted_structure.find("[0]:") != std::string::npos);
        test::expect(std::identity(), formatted_structure.find("size: 2") != std::string::npos);
        test::expect(std::identity(), formatted_structure.find("1.5") != std::string::npos);

        auto moved_data = std::move(structure).data();
        test::expect(std::equal_to<>(), std::get<0>(moved_data).size(), std::size_t(2));
        test::expect(std::equal_to<>(), std::get<1>(moved_data)[1], 2.5F);

        api::erased_structure_of_arrays_t empty;
        test::expect(std::equal_to<>(), empty.size(), std::size_t(0));
        test::expect(std::identity(), empty.data().empty());

        structure_t source;
        source.push_back(7, 3.25F);
        source.push_back(9, 4.5F);
        api::erased_structure_of_arrays_t erased(std::move(source));

        test::expect(std::equal_to<>(), erased.size(), std::size_t(2));
        test::expect(std::equal_to<>(), erased.data().size(), std::size_t(2));
        test::expect(std::equal_to<>(), erased[0].element_size(), sizeof(int));
        test::expect(std::equal_to<>(), erased[0].element_count(), std::size_t(2));
        test::expect(std::equal_to<>(), erased[0].operator[]<int>(0), 7);
        test::expect(std::equal_to<>(), erased[0].operator[]<int>(1), 9);
        test::expect(std::equal_to<>(), erased[1].element_size(), sizeof(float));
        test::expect(std::equal_to<>(), erased[1].element_count(), std::size_t(2));
        test::expect(std::equal_to<>(), erased[1].operator[]<float>(0), 3.25F);
        test::expect(std::equal_to<>(), erased[1].operator[]<float>(1), 4.5F);

        const auto formatted_erased = std::format("{}", erased);
        test::expect(std::identity(), formatted_erased.find("[0]:") != std::string::npos);
        test::expect(std::identity(), formatted_erased.find("element_count: 2") != std::string::npos);

        structure_t movable_source;
        movable_source.push_back(11, 6.5F);
        api::erased_structure_of_arrays_t movable(std::move(movable_source));
        auto erased_data = std::move(movable).data();
        test::expect(std::equal_to<>(), erased_data.size(), std::size_t(2));
        test::expect(std::equal_to<>(), erased_data[0].operator[]<int>(0), 11);
        test::expect(std::equal_to<>(), erased_data[1].operator[]<float>(0), 6.5F);
    });
}
