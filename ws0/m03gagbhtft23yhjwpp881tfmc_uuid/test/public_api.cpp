#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gagbhtft23yhjwpp881tfmc_uuid/uuid.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

namespace api = m03gagbhtft23yhjwpp881tfmc_uuid;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        const std::array<std::byte, 16> bytes {
            std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
            std::byte { 0x00 }, std::byte { 0x2a }, std::byte { 0x70 }, std::byte { 0x00 },
            std::byte { 0x80 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 },
            std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }, std::byte { 0x00 }
        };

        const api::uuid value(bytes);
        test::expect_equal(value.bytes().size(), std::size_t(16));
        test::expect(std::equal(value.bytes().begin(), value.bytes().end(), bytes.begin()));
        test::expect_equal(value.version(), 7U);
        test::expect_equal(
            value.timestamp(),
            std::chrono::system_clock::time_point(std::chrono::milliseconds(42))
        );
        test::expect_equal(
            std::format("{}", value),
            std::string("00000000-002a-7000-8000-000000000000")
        );

        const api::uuid copied = value;
        const api::uuid moved = std::move(api::uuid(bytes));
        test::expect_equal(std::format("{}", copied), std::format("{}", value));
        test::expect_equal(std::format("{}", moved), std::format("{}", value));

        test::expect_throws<std::invalid_argument>([] {
            const std::array<std::byte, 15> too_short {};
            [[maybe_unused]] const api::uuid invalid(too_short);
        });

        test::expect_throws<std::invalid_argument>([&] {
            auto invalid_version = bytes;
            invalid_version[6] = std::byte { 0x40 };
            [[maybe_unused]] const api::uuid invalid(invalid_version);
        });

        test::expect_throws<std::invalid_argument>([&] {
            auto invalid_variant = bytes;
            invalid_variant[8] = std::byte { 0x00 };
            [[maybe_unused]] const api::uuid invalid(invalid_variant);
        });

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const auto invalid = api::uuid::generate(0);
        });
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const auto invalid = api::uuid::generate(8);
        });

        const auto before = std::chrono::system_clock::now();
        const auto first = api::uuid::generate(7);
        const auto second = api::uuid::generate(7);
        const auto after = std::chrono::system_clock::now();

        test::expect_equal(first.version(), 7U);
        test::expect_equal(second.version(), 7U);
        test::expect_equal(first.bytes().size(), std::size_t(16));
        test::expect_equal(
            std::to_integer<unsigned>(first.bytes()[8]) >> 6,
            2U
        );
        test::expect(
            std::lexicographical_compare(
                first.bytes().begin(), first.bytes().end(),
                second.bytes().begin(), second.bytes().end()
            ),
            "UUIDv7 values must increase byte-wise in generation order"
        );

        test::expect(before <= first.timestamp());
        test::expect(first.timestamp() <= after);
        test::expect(first.timestamp() <= second.timestamp());

        const auto formatted = std::format("{}", first);
        test::expect_equal(formatted.size(), std::size_t(36));
        test::expect_equal(formatted[8], '-');
        test::expect_equal(formatted[13], '-');
        test::expect_equal(formatted[18], '-');
        test::expect_equal(formatted[23], '-');
    });
}
