#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4c25hdgt1ohryq3fcp_function_alu/function_alu.h>

#include <functional>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace api = m03ge9ij4c25hdgt1ohryq3fcp_function_alu;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace runtime_api = m03ge9ij49xkr5obofujoj7ltw_function_runtime;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace typesystem_api = m03ge9ij43jyxy821pda20jhwh_typesystem;

namespace {

int g_sink_call_count = 0;
std::uint8_t g_last_sink_argument = 0;

void sink_call(runtime_api::function_t&, std::uint8_t argument_index) {
    ++g_sink_call_count;
    g_last_sink_argument = argument_index;
}

void expect_identity(
    runtime_api::function_t& function,
    const std::string& name
) {
    test::expect(std::equal_to<>(), function.function_ir().function_id.ns,
        std::string("function_alu")
    );
    test::expect(std::equal_to<>(), function.function_ir().function_id.name, name);
    test::expect(std::identity(), static_cast<bool>(function.function_ir().function_id));
    test::expect(std::identity(), function.function_ir().children.empty());
    test::expect(std::identity(), function.function_ir().connections.empty());
}

} // namespace

int main() {
    return test::run([] {
        typesystem_api::typesystem_t typesystem;

        std::unique_ptr<runtime_api::function_t> add(api::function_alu_t::add(typesystem));
        expect_identity(*add, "add");
        add->arguments().resize(3);
        add->write(0, 19);
        add->write(1, 23);
        add->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(add->read(2)), 42);
        add->write(0, -5);
        add->write(1, 2);
        add->call(1);
        test::expect(std::equal_to<>(), static_cast<int>(add->read(2)), -3);

        std::unique_ptr<runtime_api::function_t> sub(api::function_alu_t::sub(typesystem));
        expect_identity(*sub, "sub");
        sub->arguments().resize(3);
        sub->write(0, 50);
        sub->write(1, 8);
        sub->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(sub->read(2)), 42);

        std::unique_ptr<runtime_api::function_t> mul(api::function_alu_t::mul(typesystem));
        expect_identity(*mul, "mul");
        mul->arguments().resize(3);
        mul->write(0, 6);
        mul->write(1, 7);
        mul->call(1);
        test::expect(std::equal_to<>(), static_cast<int>(mul->read(2)), 42);

        std::unique_ptr<runtime_api::function_t> div(api::function_alu_t::div(typesystem));
        expect_identity(*div, "div");
        div->arguments().resize(3);
        div->write(0, 84);
        div->write(1, 2);
        div->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(div->read(2)), 42);
        div->write(0, -21);
        div->write(1, 3);
        div->call(1);
        test::expect(std::equal_to<>(), static_cast<int>(div->read(2)), -7);

        std::unique_ptr<runtime_api::function_t> cond(api::function_alu_t::cond(typesystem));
        expect_identity(*cond, "cond");
        cond->arguments().resize(4);
        cond->write(1, 11);
        cond->write(2, 22);
        cond->write(0, true);
        cond->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(cond->read(3)), 11);
        cond->write(0, false);
        cond->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(cond->read(3)), 22);
        cond->clear(1);
        cond->write(0, true);
        cond->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(cond->read(3)), 22);

        std::unique_ptr<runtime_api::function_t> is_zero(
            api::function_alu_t::is_zero(typesystem)
        );
        expect_identity(*is_zero, "is_zero");
        is_zero->arguments().resize(2);
        is_zero->write(0, 0);
        is_zero->call(0);
        test::expect(std::equal_to<>(), static_cast<bool>(is_zero->read(1)), true);
        is_zero->write(0, 9);
        is_zero->call(0);
        test::expect(std::equal_to<>(), static_cast<bool>(is_zero->read(1)), false);

        std::unique_ptr<runtime_api::function_t> integer(
            api::function_alu_t::integer(typesystem)
        );
        expect_identity(*integer, "integer");
        integer->arguments().resize(1);
        integer->write(0, 123);
        integer->call(0);
        test::expect(std::equal_to<>(), static_cast<int>(integer->read(0)), 0);

        std::unique_ptr<runtime_api::function_t> logger(
            api::function_alu_t::logger(typesystem)
        );
        expect_identity(*logger, "logger");
        logger->arguments().resize(2);
        std::ostringstream explicit_output;
        std::ostream* output_pointer = &explicit_output;
        logger->write(0, 42);
        logger->write(1, output_pointer);
        logger->call(0);
        test::expect(std::equal_to<>(), explicit_output.str(), std::string("42\n"));

        logger->clear(1);
        std::ostringstream fallback_output;
        auto* previous_buffer = std::cout.rdbuf(fallback_output.rdbuf());
        logger->write(0, -7);
        logger->call(0);
        std::cout.rdbuf(previous_buffer);
        test::expect(std::equal_to<>(), fallback_output.str(), std::string("-7\n"));

        std::unique_ptr<runtime_api::function_t> pin(api::function_alu_t::pin(typesystem));
        expect_identity(*pin, "pin");
        pin->arguments().resize(3);

        runtime_api::function_t first_sink(
            typesystem,
            ir_api::function_ir_t {},
            &sink_call
        );
        runtime_api::function_t second_sink(
            typesystem,
            ir_api::function_ir_t {},
            &sink_call
        );
        first_sink.arguments().resize(1);
        second_sink.arguments().resize(1);
        pin->connect(&first_sink, 0, 1);
        pin->connect(&second_sink, 0, 2);

        g_sink_call_count = 0;
        pin->write(0, 314);
        pin->call(0);
        test::expect(std::equal_to<>(), g_sink_call_count, 2);
        test::expect(std::equal_to<>(), g_last_sink_argument, std::uint8_t(0));
        test::expect(std::equal_to<>(), static_cast<int>(first_sink.read(0)), 314);
        test::expect(std::equal_to<>(), static_cast<int>(second_sink.read(0)), 314);
        test::expect(std::equal_to<>(), static_cast<int>(pin->read(1)), 314);
        test::expect(std::equal_to<>(), static_cast<int>(pin->read(2)), 314);

        g_sink_call_count = 0;
        pin->write(1, 271);
        test::expect(std::equal_to<>(), g_sink_call_count, 1);
        pin->call(1);
        test::expect(std::equal_to<>(), g_sink_call_count, 2);
        test::expect(std::equal_to<>(), static_cast<int>(second_sink.read(0)), 271);
        test::expect(std::equal_to<>(), static_cast<int>(pin->read(2)), 271);
    });
}
