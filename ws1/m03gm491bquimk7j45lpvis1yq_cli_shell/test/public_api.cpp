#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gm491bquimk7j45lpvis1yq_cli_shell/api.h>

#include <chrono>
#include <functional>

namespace api = m03gm491bquimk7j45lpvis1yq_cli_shell;
namespace cli = m03gm33dj5xo77vegpbspger4r_cli;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        cli::application_t application;
        api::shell_t shell(application, "test> ");
        test::expect_no_throw([&] { shell.history_size(0); });
        test::expect_no_throw([&] { shell.history_file("history.txt"); });
        test::expect_no_throw([&] { shell.idle(std::chrono::milliseconds(0), [] {}); });
        test::expect_throws<std::invalid_argument>([&] {
            shell.idle(std::chrono::milliseconds(-1), [] {});
        });
        test::expect_throws<std::invalid_argument>([&] {
            shell.idle(std::chrono::milliseconds(1), {});
        });
    });
}
