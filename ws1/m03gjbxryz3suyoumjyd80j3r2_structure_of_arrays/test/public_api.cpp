#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>

#include <cstddef>
#include <format>
#include <string>
#include <tuple>
#include <utility>

namespace api = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        using structure_t = api::structure_of_arrays_t<int, float>;

        structure_t structure;
        structure.push_back(1, 1.5F);
        structure.push_back(2, 2.5F);

        const auto& data = structure.data();
        test::expect_equal(std::get<0>(data).size(), std::size_t(2));
        test::expect_equal(std::get<0>(data)[0], 1);
        test::expect_equal(std::get<0>(data)[1], 2);
        test::expect_equal(std::get<1>(data).size(), std::size_t(2));
        test::expect_equal(std::get<1>(data)[0], 1.5F);
        test::expect_equal(std::get<1>(data)[1], 2.5F);

        const auto formatted_structure = std::format("{}", structure);
        test::expect(formatted_structure.find("[0]:") != std::string::npos);
        test::expect(formatted_structure.find("size: 2") != std::string::npos);
        test::expect(formatted_structure.find("1.5") != std::string::npos);

        auto moved_data = std::move(structure).data();
        test::expect_equal(std::get<0>(moved_data).size(), std::size_t(2));
        test::expect_equal(std::get<1>(moved_data)[1], 2.5F);

        api::erased_structure_of_arrays_t empty;
        test::expect_equal(empty.size(), std::size_t(0));
        test::expect(empty.data().empty());

        structure_t source;
        source.push_back(7, 3.25F);
        source.push_back(9, 4.5F);
        api::erased_structure_of_arrays_t erased(std::move(source));

        test::expect_equal(erased.size(), std::size_t(2));
        test::expect_equal(erased.data().size(), std::size_t(2));
        test::expect_equal(erased[0].element_size(), sizeof(int));
        test::expect_equal(erased[0].element_count(), std::size_t(2));
        test::expect_equal(erased[0].operator[]<int>(0), 7);
        test::expect_equal(erased[0].operator[]<int>(1), 9);
        test::expect_equal(erased[1].element_size(), sizeof(float));
        test::expect_equal(erased[1].element_count(), std::size_t(2));
        test::expect_equal(erased[1].operator[]<float>(0), 3.25F);
        test::expect_equal(erased[1].operator[]<float>(1), 4.5F);

        const auto formatted_erased = std::format("{}", erased);
        test::expect(formatted_erased.find("[0]:") != std::string::npos);
        test::expect(formatted_erased.find("element_count: 2") != std::string::npos);

        structure_t movable_source;
        movable_source.push_back(11, 6.5F);
        api::erased_structure_of_arrays_t movable(std::move(movable_source));
        auto erased_data = std::move(movable).data();
        test::expect_equal(erased_data.size(), std::size_t(2));
        test::expect_equal(erased_data[0].operator[]<int>(0), 11);
        test::expect_equal(erased_data[1].operator[]<float>(0), 6.5F);
    });
}
