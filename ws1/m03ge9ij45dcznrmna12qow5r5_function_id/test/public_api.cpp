#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij45dcznrmna12qow5r5_function_id/function_id.h>

#include <functional>
#include <chrono>
#include <format>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        using namespace std::chrono_literals;

        const api::function_id_t empty;
        test::expect(std::identity(), !static_cast<bool>(empty));
        test::expect(std::equal_to<>(), api::function_id_t::to_string(empty), std::string("::@0"));

        const api::function_id_t missing_namespace {
            .ns = "",
            .name = "function",
            .creation_time = std::chrono::system_clock::time_point(123s)
        };
        const api::function_id_t missing_name {
            .ns = "namespace",
            .name = "",
            .creation_time = std::chrono::system_clock::time_point(123s)
        };
        const api::function_id_t missing_time {
            .ns = "namespace",
            .name = "function",
            .creation_time = {}
        };
        test::expect(std::identity(), !static_cast<bool>(missing_namespace));
        test::expect(std::identity(), !static_cast<bool>(missing_name));
        test::expect(std::identity(), !static_cast<bool>(missing_time));

        const api::function_id_t value {
            .ns = "math",
            .name = "add",
            .creation_time = std::chrono::system_clock::time_point(1'700'000'123s)
        };
        test::expect(std::identity(), static_cast<bool>(value));
        test::expect(std::equal_to<>(), api::function_id_t::to_string(value),
            std::string("math::add@1700000123")
        );
        test::expect(std::equal_to<>(), std::format("{}", value), std::string("math::add@1700000123"));
        const api::function_id_t braces {
            .ns = "{namespace}",
            .name = "{name}",
            .creation_time = std::chrono::system_clock::time_point(1s)
        };
        test::expect(std::equal_to<>(), std::format("{}", braces), std::string("{namespace}::{name}@1"));

        const auto parsed = api::function_id_t::from_string("math::add@1700000123");
        test::expect(std::equal_to<>(), parsed, value);
        test::expect(std::equal_to<>(), parsed.ns, std::string("math"));
        test::expect(std::equal_to<>(), parsed.name, std::string("add"));
        test::expect(std::equal_to<>(), parsed.creation_time,
            std::chrono::system_clock::time_point(1'700'000'123s)
        );

        const api::function_id_t same = value;
        const api::function_id_t different_namespace {
            .ns = "other",
            .name = value.name,
            .creation_time = value.creation_time
        };
        const api::function_id_t different_name {
            .ns = value.ns,
            .name = "subtract",
            .creation_time = value.creation_time
        };
        const api::function_id_t different_time {
            .ns = value.ns,
            .name = value.name,
            .creation_time = value.creation_time + 1s
        };
        test::expect(std::identity(), value == same);
        test::expect(std::identity(), !(value == different_namespace));
        test::expect(std::identity(), !(value == different_name));
        test::expect(std::identity(), !(value == different_time));

        test::expect(std::equal_to<>(), std::hash<api::function_id_t>()(value),
            std::hash<api::function_id_t>()(same)
        );
        std::unordered_set<api::function_id_t> ids;
        ids.insert(value);
        ids.insert(same);
        ids.insert(different_time);
        test::expect(std::equal_to<>(), ids.size(), std::size_t(2));
        test::expect(std::identity(), ids.contains(value));

        const api::function_id_t subsecond {
            .ns = "timing",
            .name = "sample",
            .creation_time = std::chrono::system_clock::time_point(42s + 999ms)
        };
        test::expect(std::equal_to<>(), api::function_id_t::to_string(subsecond),
            std::string("timing::sample@42")
        );
        const auto subsecond_roundtrip = api::function_id_t::from_string(
            api::function_id_t::to_string(subsecond)
        );
        test::expect(std::equal_to<>(), subsecond_roundtrip.creation_time,
            std::chrono::system_clock::time_point(42s)
        );
        test::expect(std::identity(), !(subsecond_roundtrip == subsecond));

        const auto embedded_delimiter = api::function_id_t::from_string(
            "space::name@with@77"
        );
        test::expect(std::equal_to<>(), embedded_delimiter.ns, std::string("space"));
        test::expect(std::equal_to<>(), embedded_delimiter.name, std::string("name@with"));
        test::expect(std::equal_to<>(), embedded_delimiter.creation_time,
            std::chrono::system_clock::time_point(77s)
        );

        const auto empty_fields = api::function_id_t::from_string("::@0");
        test::expect(std::identity(), !static_cast<bool>(empty_fields));
        test::expect(std::equal_to<>(), empty_fields, empty);

        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "missing-namespace-separator@1"
            );
        });
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "namespace::missing-time-separator"
            );
        });
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "namespace::name@not-a-number"
            );
        });
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "namespace::name@1trailing"
            );
        });
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "namespace@1::name"
            );
        });
        test::expect_throws<std::out_of_range>([] {
            [[maybe_unused]] const auto invalid = api::function_id_t::from_string(
                "namespace::name@999999999999999999999999999999999999"
            );
        });
    });
}
