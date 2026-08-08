#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gn7qllwpi68ovctow4jrccj_lexer/lexer.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace lexer = m03gn7qllwpi68ovctow4jrccj_lexer;

int main() {
    return test::run([] {
        std::istringstream input(
            "#include \"local/header.h\"\n"
            "// #include \"ignored/comment.h\"\n"
            "const char* text = \"#include \\\"ignored/string.h\\\"\";\n"
            "#include <system/header.hpp>\n"
        );

        const std::vector<std::filesystem::path> paths = lexer::include_paths(input);
        test::expect_equal(paths.size(), std::size_t(2), "expected two include paths");
        test::expect_equal(paths[0].generic_string(), std::string("local/header.h"), "quoted include path was not parsed");
        test::expect_equal(paths[1].generic_string(), std::string("system/header.hpp"), "angle include path was not parsed");
    });
}
