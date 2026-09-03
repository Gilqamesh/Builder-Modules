#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4azvq47860wqjyzskd_function_primitive_lang/function_primitive_lang.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

namespace api = m03ge9ij4azvq47860wqjyzskd_function_primitive_lang;
namespace runtime_api = m03ge9ij49xkr5obofujoj7ltw_function_runtime;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace typesystem_api = m03ge9ij43jyxy821pda20jhwh_typesystem;

namespace {

int g_call_count = 0;
std::uint8_t g_last_argument = 0;

void callback(runtime_api::function_t&, std::uint8_t argument_index) {
    ++g_call_count;
    g_last_argument = argument_index;
}

} // namespace

int main() {
    return test::run([] {
        typesystem_api::typesystem_t typesystem;
        const auto before = std::chrono::system_clock::now();
        std::unique_ptr<runtime_api::function_t> function(
            api::function_primitive_lang_t::function(
                typesystem,
                "primitive_namespace",
                "primitive_name",
                &callback
            )
        );
        const auto after = std::chrono::system_clock::now();

        test::expect(std::identity(), function != nullptr);
        test::expect(std::identity(), function->parent() == nullptr);
        test::expect(std::identity(), function->function_call() == &callback);
        test::expect(std::equal_to<>(), function->function_ir().function_id.ns,
            std::string("primitive_namespace")
        );
        test::expect(std::equal_to<>(), function->function_ir().function_id.name,
            std::string("primitive_name")
        );
        test::expect(std::identity(), before <= function->function_ir().function_id.creation_time
        );
        test::expect(std::identity(), function->function_ir().function_id.creation_time <= after
        );
        test::expect(std::identity(), static_cast<bool>(function->function_ir().function_id));
        test::expect(std::equal_to<>(), function->function_ir().left, 0);
        test::expect(std::equal_to<>(), function->function_ir().right, 0);
        test::expect(std::equal_to<>(), function->function_ir().top, 0);
        test::expect(std::equal_to<>(), function->function_ir().bottom, 0);
        test::expect(std::identity(), function->function_ir().children.empty());
        test::expect(std::identity(), function->function_ir().connections.empty());
        test::expect(std::identity(), function->children().empty());
        test::expect(std::identity(), function->arguments().empty());

        g_call_count = 0;
        function->call(13);
        test::expect(std::equal_to<>(), g_call_count, 1);
        test::expect(std::equal_to<>(), g_last_argument, std::uint8_t(13));

        std::unique_ptr<runtime_api::function_t> incomplete_id(
            api::function_primitive_lang_t::function(
                typesystem,
                "",
                "",
                &callback
            )
        );
        test::expect(std::identity(), !static_cast<bool>(incomplete_id->function_ir().function_id));
        test::expect(std::equal_to<>(), incomplete_id->function_ir().function_id.ns,
            std::string()
        );
        test::expect(std::equal_to<>(), incomplete_id->function_ir().function_id.name,
            std::string()
        );
    });
}
