#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gjfvd6i5jzbmngb2ldoooza_type_erased_array/api.h>

#include <array>
#include <cstddef>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace api = m03gjfvd6i5jzbmngb2ldoooza_type_erased_array;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        using array_t = api::type_erased_array_t;
        using mismatched_t = std::array<std::byte, sizeof(int) + 1>;

        array_t empty;
        test::expect_equal(empty.element_count(), std::size_t(0));
        test::expect_equal(empty.element_size(), std::size_t(0));
        test::expect_equal(empty.byte_size(), std::size_t(0));
        test::expect(empty.data().empty());

        array_t values(std::vector<int> { 10, 20, 30 });
        test::expect_equal(values.element_count(), std::size_t(3));
        test::expect_equal(values.element_size(), sizeof(int));
        test::expect_equal(values.byte_size(), sizeof(int) * 3);
        test::expect_equal(values.data().size(), sizeof(int) * 3);
        test::expect_equal(values.operator[]<int>(0), 10);
        test::expect_equal(values.operator[]<int>(2), 30);

        values.operator[]<int>(1) = 25;
        test::expect_equal(values.operator[]<int>(1), 25);

        auto writable_bytes = values.data();
        test::expect_equal(writable_bytes.size(), values.byte_size());
        const auto const_values = values;
        const std::span<const std::byte> readable_bytes = const_values.data();
        test::expect_equal(readable_bytes.size(), values.byte_size());
        test::expect_equal(const_values.operator[]<int>(1), 25);

        test::expect_throws<std::invalid_argument>([&] {
            [[maybe_unused]] const auto& value = values.operator[]<mismatched_t>(0);
        });

        array_t appended;
        appended.push_back(7);
        appended.push_back(9);
        test::expect_equal(appended.element_size(), sizeof(int));
        test::expect_equal(appended.element_count(), std::size_t(2));
        test::expect_equal(appended.operator[]<int>(0), 7);
        test::expect_equal(appended.operator[]<int>(1), 9);
        test::expect_throws<std::invalid_argument>([&] {
            appended.push_back(mismatched_t {});
        });

        array_t copied = appended;
        copied.operator[]<int>(0) = 11;
        test::expect_equal(appended.operator[]<int>(0), 7);
        test::expect_equal(copied.operator[]<int>(0), 11);

        array_t moved = std::move(copied);
        test::expect_equal(moved.element_count(), std::size_t(2));
        test::expect_equal(moved.operator[]<int>(0), 11);

        appended.clear();
        test::expect_equal(appended.element_count(), std::size_t(0));
        test::expect_equal(appended.byte_size(), std::size_t(0));
        test::expect_equal(appended.element_size(), sizeof(int));
        appended.push_back(42);
        test::expect_equal(appended.operator[]<int>(0), 42);

        const auto formatted = std::format("{}", values);
        test::expect(formatted.starts_with("element_size: "));
        test::expect(formatted.find("element_count: 3") != std::string::npos);
        test::expect(formatted.find("data: 0x") != std::string::npos);
    });
}
