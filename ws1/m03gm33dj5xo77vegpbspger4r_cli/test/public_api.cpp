# include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
# include <m03gm33dj5xo77vegpbspger4r_cli/api.h>

# include <filesystem>
# include <format>
# include <functional>
# include <fstream>
# include <sstream>
# include <string>
# include <type_traits>
# include <vector>

namespace api = m03gm33dj5xo77vegpbspger4r_cli;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        static_assert(!std::is_copy_constructible_v<api::application_t>);
        static_assert(!std::is_copy_assignable_v<api::application_t>);
        static_assert(!std::is_move_constructible_v<api::application_t>);
        static_assert(!std::is_move_assignable_v<api::application_t>);

        const std::vector<std::string> values { "42", "yes", "tail" };
        api::arguments_t arguments(values);
        test::expect(std::equal_to<>(), arguments.size(), std::size_t(3));
        test::expect(std::equal_to<>(), arguments.pop<int>("count"), 42);
        test::expect(std::identity(), arguments.pop<bool>("enabled"));
        test::expect(std::equal_to<>(), arguments.pop<std::string>("value"), std::string("tail"));
        test::expect(std::identity(), arguments.empty());
        test::expect_throws<std::invalid_argument>([&] {
            [[maybe_unused]] const auto missing = arguments.pop("missing");
        });

        auto optional = api::argument_t::token("optional").optional();
        auto variadic = api::argument_t::token("values").optional().variadic();
        test::expect(std::identity(), optional.is_optional());
        test::expect(std::identity(), variadic.is_optional());
        test::expect(std::identity(), variadic.is_variadic());
        test::expect(std::equal_to<>(), std::format("{}", optional), std::string("[<optional>]"));
        test::expect_no_throw([] { api::argument_t::integer("value").validate("-42"); });
        test::expect_throws<std::invalid_argument>([] { api::argument_t::integer("value").validate("4.2"); });
        test::expect_no_throw([] { api::argument_t::unsigned_integer("value").validate("42"); });
        test::expect_throws<std::invalid_argument>([] { api::argument_t::unsigned_integer("value").validate("-1"); });
        test::expect_no_throw([] { api::argument_t::number("value").validate("4.2"); });
        test::expect_throws<std::invalid_argument>([] { api::argument_t::number("value").validate("inf"); });
        test::expect_no_throw([] { api::argument_t::boolean("value").validate("off"); });
        test::expect_throws<std::invalid_argument>([] { api::argument_t::boolean("value").validate("maybe"); });
        const auto choice = api::argument_t::choice("mode", {"fast", "safe"});
        test::expect_no_throw([&] { choice.validate("safe"); });
        test::expect_throws<std::invalid_argument>([&] { choice.validate("other"); });
        const auto choices = choice.complete({}, "s");
        test::expect(std::equal_to<>(), choices.size(), std::size_t(1));
        test::expect(std::equal_to<>(), choices.front(), std::string("safe"));

        api::application_t application;
        int invocation_count = 0;
        application.add(api::command_t(
            {"math", "add"},
            "Add two integers.",
            {api::argument_t::integer("left"), api::argument_t::integer("right")},
            [&](api::context_t& context) {
                const int left = context.arguments.pop<int>("left");
                const int right = context.arguments.pop<int>("right");
                context.arguments.expect_end("math add <left> <right>");
                context.out << left + right;
                ++invocation_count;
            }
        ));
        application.install_help_command();

        std::ostringstream out;
        std::ostringstream err;
        test::expect(std::identity(), application.run_command("math add 19 23", out, err));
        test::expect(std::equal_to<>(), invocation_count, 1);
        test::expect(std::equal_to<>(), out.str(), std::string("42"));
        test::expect(std::identity(), err.str().empty());
        test::expect_throws<std::invalid_argument>([&] { application.run_command("math add 1", out, err); });
        test::expect_throws<std::invalid_argument>([&] { application.run_command("math add one 2", out, err); });
        test::expect_throws<std::invalid_argument>([&] { application.run_command("unknown", out, err); });

        out.str("");
        application.run_command("math help", out, err);
        test::expect(std::identity(), out.str().find("math add <left> <right>") != std::string::npos);
        const auto topic_completions = application.complete_line("ma");
        test::expect(std::equal_to<>(), topic_completions.size(), std::size_t(1));
        test::expect(std::equal_to<>(), topic_completions.front(), std::string("math"));

        bool fallback_called = false;
        application.fallback([&](api::context_t& context) {
            fallback_called = context.arguments.pop("fallback") == "fallback";
            return fallback_called;
        });
        const std::vector<std::string> fallback { "fallback" };
        test::expect(std::identity(), application.run_arguments(fallback, out, err));
        test::expect(std::identity(), fallback_called);

        api::application_t invalid_application;
        test::expect_throws<std::logic_error>([&] { invalid_application.add(api::command_t()); });
        test::expect_throws<std::logic_error>([&] {
            invalid_application.add(api::command_t({}, "invalid", [](api::context_t&) {}));
        });
        test::expect_throws<std::logic_error>([&] {
            invalid_application.add(api::command_t(
                {"invalid"},
                "invalid",
                {api::argument_t::token("rest").variadic(), api::argument_t::token("after")},
                [](api::context_t&) {}
            ));
        });

        api::application_t stopped_application;
        stopped_application.add(api::command_t({"stop"}, "Stop.", [](api::context_t& context) { context.stop(); }));
        test::expect(std::identity(), !stopped_application.run_command("stop", out, err));
        test::expect(std::identity(), !stopped_application.running());
    });
}
