#ifndef M03GN97N4IUSBTL7UTHB01WU9M_TEST_FRAMEWORK_TEST_FRAMEWORK_H
# define M03GN97N4IUSBTL7UTHB01WU9M_TEST_FRAMEWORK_TEST_FRAMEWORK_H

# include <exception>
# include <iostream>
# include <stdexcept>
# include <string>
# include <string_view>
# include <utility>

namespace m03gn97n4iusbtl7uthb01wu9m_test_framework {

class assertion_error_t : public std::runtime_error {
public:
    explicit assertion_error_t(std::string_view message):
        std::runtime_error(std::string(message))
    {
    }
};

[[noreturn]] inline void fail(std::string_view message) {
    throw assertion_error_t(message);
}

inline void expect(bool condition, std::string_view message = "expectation failed") {
    if (!condition) {
        fail(message);
    }
}

template <class actual_t, class expected_t>
void expect_equal(
    const actual_t& actual,
    const expected_t& expected,
    std::string_view message = "values are not equal"
) {
    if (!(actual == expected)) {
        fail(message);
    }
}

template <class actual_t, class expected_t>
void expect_not_equal(
    const actual_t& actual,
    const expected_t& expected,
    std::string_view message = "values are equal"
) {
    if (actual == expected) {
        fail(message);
    }
}

template <class fn_t>
void expect_no_throw(fn_t&& fn, std::string_view message = "unexpected exception") {
    try {
        std::forward<fn_t>(fn)();
    } catch (...) {
        fail(message);
    }
}

template <class exception_t = std::exception, class fn_t>
void expect_throws(fn_t&& fn, std::string_view message = "expected exception was not thrown") {
    try {
        std::forward<fn_t>(fn)();
    } catch (const exception_t&) {
        return ;
    } catch (...) {
        fail("unexpected exception type");
    }

    fail(message);
}

template <class fn_t>
int run(fn_t&& fn) {
    try {
        std::forward<fn_t>(fn)();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "test failed: " << e.what() << '\n';
    } catch (...) {
        std::cerr << "test failed: unknown exception\n";
    }

    return 1;
}

} // namespace m03gn97n4iusbtl7uthb01wu9m_test_framework

#endif // M03GN97N4IUSBTL7UTHB01WU9M_TEST_FRAMEWORK_TEST_FRAMEWORK_H
