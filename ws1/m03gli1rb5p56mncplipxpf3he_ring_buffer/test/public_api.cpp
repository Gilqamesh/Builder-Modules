#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gli1rb5p56mncplipxpf3he_ring_buffer/api.h>

#include <functional>
#include <format>
#include <stdexcept>
#include <string>

namespace api = m03gli1rb5p56mncplipxpf3he_ring_buffer;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        using overlapping_advance_t = api::ring_buffer_t<
            int,
            api::staging_policy_t::overlapping,
            api::commit_policy_t::advance
        >;
        using overlapping_copy_t = api::ring_buffer_t<
            int,
            api::staging_policy_t::overlapping,
            api::commit_policy_t::copy_with_advance
        >;
        using dedicated_advance_t = api::ring_buffer_t<
            int,
            api::staging_policy_t::dedicated,
            api::commit_policy_t::advance
        >;
        using dedicated_copy_t = api::ring_buffer_t<
            int,
            api::staging_policy_t::dedicated,
            api::commit_policy_t::copy_with_advance
        >;

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const overlapping_advance_t invalid(0);
        });
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const dedicated_advance_t invalid(0);
        });

        overlapping_advance_t overlapping(2);
        test::expect(std::equal_to<>(), overlapping.history_capacity(), std::size_t(2));
        test::expect(std::equal_to<>(), overlapping.history_size(), std::size_t(0));
        test::expect(std::equal_to<>(), overlapping.stage(), 0);
        test::expect_throws<std::out_of_range>([&] {
            [[maybe_unused]] const auto value = overlapping.history(0);
        });

        overlapping.stage() = 1;
        overlapping.commit();
        test::expect(std::equal_to<>(), overlapping.history_size(), std::size_t(1));
        test::expect(std::equal_to<>(), overlapping.history(0), 1);

        overlapping.stage() = 2;
        overlapping.commit();
        test::expect(std::equal_to<>(), overlapping.history_size(), std::size_t(2));
        test::expect(std::equal_to<>(), overlapping.history(0), 2);
        test::expect(std::equal_to<>(), overlapping.history(1), 1);
        test::expect(std::equal_to<>(), overlapping.stage(), 1);

        overlapping.stage() = 3;
        overlapping.commit();
        test::expect(std::equal_to<>(), overlapping.history_size(), std::size_t(2));
        test::expect(std::equal_to<>(), overlapping.history(0), 3);
        test::expect(std::equal_to<>(), overlapping.history(1), 2);
        test::expect_throws<std::out_of_range>([&] {
            [[maybe_unused]] const auto value = overlapping.history(2);
        });

        overlapping.history(0) = 30;
        const auto& const_overlapping = overlapping;
        test::expect(std::equal_to<>(), const_overlapping.history(0), 30);
        test::expect_throws<std::out_of_range>([&] {
            [[maybe_unused]] const auto value = const_overlapping.history(2);
        });

        const auto formatted = std::format("{}", const_overlapping);
        test::expect(std::identity(), formatted.find("history capacity: 2") != std::string::npos);
        test::expect(std::identity(), formatted.find("history size: 2") != std::string::npos);
        test::expect(std::identity(), formatted.find("0: 30") != std::string::npos);
        test::expect(std::identity(), formatted.find("1: 2") != std::string::npos);

        overlapping_copy_t overlapping_copy(2);
        overlapping_copy.stage() = 4;
        overlapping_copy.commit();
        test::expect(std::equal_to<>(), overlapping_copy.stage(), 4);
        overlapping_copy.stage() = 5;
        overlapping_copy.commit();
        test::expect(std::equal_to<>(), overlapping_copy.history(0), 5);
        test::expect(std::equal_to<>(), overlapping_copy.history(1), 5);
        test::expect(std::equal_to<>(), overlapping_copy.stage(), 5);

        dedicated_advance_t dedicated(2);
        dedicated.stage() = 7;
        dedicated.commit();
        dedicated.stage() = 8;
        dedicated.commit();
        test::expect(std::equal_to<>(), dedicated.history_capacity(), std::size_t(2));
        test::expect(std::equal_to<>(), dedicated.history_size(), std::size_t(2));
        test::expect(std::equal_to<>(), dedicated.history(0), 8);
        test::expect(std::equal_to<>(), dedicated.history(1), 7);
        test::expect(std::equal_to<>(), dedicated.stage(), 0);
        dedicated.stage() = 9;
        dedicated.commit();
        test::expect(std::equal_to<>(), dedicated.history(0), 9);
        test::expect(std::equal_to<>(), dedicated.history(1), 8);

        dedicated_copy_t dedicated_copy(2);
        dedicated_copy.stage() = 11;
        dedicated_copy.commit();
        test::expect(std::equal_to<>(), dedicated_copy.stage(), 11);
        dedicated_copy.stage() = 12;
        dedicated_copy.commit();
        test::expect(std::equal_to<>(), dedicated_copy.history(0), 12);
        test::expect(std::equal_to<>(), dedicated_copy.history(1), 11);
        test::expect(std::equal_to<>(), dedicated_copy.stage(), 12);
        dedicated_copy.commit();
        test::expect(std::equal_to<>(), dedicated_copy.history(0), 12);
        test::expect(std::equal_to<>(), dedicated_copy.history(1), 12);
    });
}
