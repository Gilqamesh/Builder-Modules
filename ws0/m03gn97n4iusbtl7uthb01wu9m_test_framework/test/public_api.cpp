#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        const test::assertion_error_t error("message");
        test::expect_equal(std::string(error.what()), std::string("message"));

        test::expect_throws<test::assertion_error_t>([] {
            test::fail("failure");
        });

        test::expect(true);
        test::expect_throws<test::assertion_error_t>([] {
            test::expect(false, "false condition");
        });

        test::expect_equal(3, 3);
        test::expect_equal(3, 3L);
        test::expect_throws<test::assertion_error_t>([] {
            test::expect_equal(3, 4, "different values");
        });

        test::expect_not_equal(3, 4);
        test::expect_throws<test::assertion_error_t>([] {
            test::expect_not_equal(3, 3, "equal values");
        });

        test::expect_no_throw([] {});
        test::expect_throws<test::assertion_error_t>([] {
            test::expect_no_throw([] {
                throw std::runtime_error("unexpected");
            });
        });

        test::expect_throws<std::runtime_error>([] {
            throw std::runtime_error("expected");
        });
        test::expect_throws<std::exception>([] {
            throw std::runtime_error("derived");
        });
        test::expect_throws<test::assertion_error_t>([] {
            test::expect_throws<std::runtime_error>([] {});
        });
        test::expect_throws<test::assertion_error_t>([] {
            test::expect_throws<std::invalid_argument>([] {
                throw std::runtime_error("wrong type");
            });
        });

        test::expect_equal(test::run([] {}), 0);

        std::ostringstream captured_errors;
        auto* previous_buffer = std::cerr.rdbuf(captured_errors.rdbuf());
        const int standard_exception_result = test::run([] {
            throw std::runtime_error("nested failure");
        });
        const int unknown_exception_result = test::run([] {
            throw 42;
        });
        std::cerr.rdbuf(previous_buffer);

        test::expect_equal(standard_exception_result, 1);
        test::expect_equal(unknown_exception_result, 1);
        test::expect(
            captured_errors.str().find("nested failure") != std::string::npos
        );
        test::expect(
            captured_errors.str().find("unknown exception") != std::string::npos
        );
    });
}
