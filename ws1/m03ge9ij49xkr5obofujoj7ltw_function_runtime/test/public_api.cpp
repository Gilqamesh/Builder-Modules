#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij49xkr5obofujoj7ltw_function_runtime/function.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace api = m03ge9ij49xkr5obofujoj7ltw_function_runtime;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;
namespace typesystem_api = m03ge9ij43jyxy821pda20jhwh_typesystem;

namespace {

typesystem_api::typesystem_t* g_typesystem = nullptr;
int g_call_count = 0;
std::uint8_t g_last_argument = 0;
int g_alternate_call_count = 0;

void record_call(api::function_t&, std::uint8_t argument_index) {
    ++g_call_count;
    g_last_argument = argument_index;
}

void alternate_call(api::function_t&, std::uint8_t argument_index) {
    ++g_alternate_call_count;
    g_last_argument = argument_index;
}

void no_op_call(api::function_t&, std::uint8_t) {
}

double int_to_double(int value) {
    return static_cast<double>(value) + 0.5;
}

api::function_t* function_id_to_function(id_api::function_id_t id) {
    auto* result = new api::function_t(
        *g_typesystem,
        ir_api::function_ir_t {
            .function_id = std::move(id),
            .left = 0,
            .right = 0,
            .top = 0,
            .bottom = 0,
            .children = {},
            .connections = {}
        },
        &no_op_call
    );
    result->arguments().resize(2);
    return result;
}

id_api::function_id_t make_id(std::string name, std::chrono::seconds seconds) {
    return id_api::function_id_t {
        .ns = "runtime",
        .name = std::move(name),
        .creation_time = std::chrono::system_clock::time_point(seconds)
    };
}

} // namespace

int main() {
    return test::run([] {
        static_assert(std::has_virtual_destructor_v<api::function_t>);

        api::function_t::argument_t default_argument;
        test::expect_equal(default_argument.m_connection, nullptr);
        test::expect_equal(
            default_argument.m_connection_argument_index,
            std::numeric_limits<std::uint8_t>::max()
        );
        test::expect(default_argument.m_name.empty());
        test::expect_equal(default_argument.m_data_type_id, -1);
        test::expect(default_argument.m_data.empty());

        typesystem_api::typesystem_t typesystem;
        typesystem.register_type<int>();
        typesystem.register_type<double>();
        typesystem.register_coercion<int, double>(&int_to_double);

        const auto base_id = make_id("base", std::chrono::seconds(10));
        ir_api::function_ir_t base_ir {
            .function_id = base_id,
            .left = 1,
            .right = 2,
            .top = 3,
            .bottom = 4,
            .children = {},
            .connections = {}
        };
        api::function_t function(typesystem, base_ir, &record_call);
        function.arguments().resize(3);

        test::expect_equal(function.parent(), nullptr);
        api::function_t parent(typesystem, ir_api::function_ir_t {}, &no_op_call);
        function.parent(&parent);
        test::expect_equal(function.parent(), &parent);
        function.parent(nullptr);
        test::expect_equal(function.parent(), nullptr);

        test::expect_equal(function.function_ir().function_id, base_id);
        function.function_ir().left = 99;
        test::expect_equal(function.function_ir().left, 99);
        test::expect(function.function_call() == &record_call);

        g_call_count = 0;
        function.call(7);
        test::expect_equal(g_call_count, 1);
        test::expect_equal(g_last_argument, std::uint8_t(7));
        function.function_call() = &alternate_call;
        g_alternate_call_count = 0;
        function.call(8);
        test::expect_equal(g_alternate_call_count, 1);
        test::expect_equal(g_last_argument, std::uint8_t(8));
        function.function_call() = &record_call;

        function.argument_name(0, "input");
        function.argument_name(1, "copy");
        test::expect_equal(function.argument_name(0), std::string("input"));
        test::expect_equal(function.argument_name(1), std::string("copy"));
        test::expect_throws<std::runtime_error>([&] {
            function.argument_name(3, "invalid");
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto& name = function.argument_name(3);
        });

        function.write(0, 41);
        const int integer_value = function.read(0);
        const double coerced_value = function.read(0);
        test::expect_equal(integer_value, 41);
        test::expect_equal(coerced_value, 41.5);
        test::expect_equal(function.arguments()[0].m_data_type_id, typesystem.type_id<int>());
        test::expect_equal(function.arguments()[0].m_data.size(), sizeof(int));

        int raw_value = 17;
        function.write(1, &raw_value, typesystem.type_id<int>());
        test::expect_equal(static_cast<int>(function.read(1)), 17);
        test::expect_no_throw([&] {
            function.write(250, &raw_value, -1);
        });

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const int value = function.read(2);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const int value = function.read(9);
        });

        function.copy(0, 1);
        test::expect_equal(static_cast<int>(function.read(1)), 41);
        function.clear(2);
        function.copy(2, 1);
        test::expect_equal(static_cast<int>(function.read(1)), 41);
        test::expect_throws<std::runtime_error>([&] {
            function.copy(9, 0);
        });
        test::expect_throws<std::runtime_error>([&] {
            function.copy(0, 9);
        });

        api::function_t source(typesystem, ir_api::function_ir_t {}, &no_op_call);
        api::function_t target(typesystem, ir_api::function_ir_t {}, &record_call);
        source.arguments().resize(2);
        target.arguments().resize(2);

        source.write(0, 100);
        g_call_count = 0;
        source.connect(&target, 1, 0);
        test::expect(source.is_connected(0));
        test::expect_equal(source.connection(0), &target);
        test::expect_equal(g_call_count, 1);
        test::expect_equal(g_last_argument, std::uint8_t(1));
        test::expect_equal(static_cast<int>(target.read(1)), 100);

        source.write(0, 101);
        test::expect_equal(g_call_count, 2);
        test::expect_equal(static_cast<int>(target.read(1)), 101);
        source.send(0);
        test::expect_equal(g_call_count, 3);

        source.clear(0);
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const int value = source.read(0);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const int value = target.read(1);
        });

        source.disconnect(0);
        test::expect(!source.is_connected(0));
        test::expect_equal(source.connection(0), nullptr);
        source.write(0, 102);
        test::expect_equal(g_call_count, 3);

        test::expect_throws<std::runtime_error>([&] {
            source.connect(&target, 0, 9);
        });
        test::expect_throws<std::runtime_error>([&] {
            source.connect(&target, 9, 0);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const bool connected = source.is_connected(9);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] auto* connected = source.connection(9);
        });
        test::expect_throws<std::runtime_error>([&] {
            source.disconnect(9);
        });
        test::expect_throws<std::runtime_error>([&] {
            source.clear(9);
        });

        test::expect_equal(&function.children(), &function.children());
        test::expect_equal(&function.arguments(), &function.arguments());
        test::expect_equal(function.children().size(), std::size_t(0));
        test::expect_equal(function.arguments().size(), std::size_t(3));

        api::function_t geometry(typesystem, ir_api::function_ir_t {}, &no_op_call);
        geometry.left(10);
        geometry.right(110);
        geometry.top(20);
        geometry.bottom(220);
        test::expect_equal(geometry.left(), 10);
        test::expect_equal(geometry.right(), 110);
        test::expect_equal(geometry.top(), 20);
        test::expect_equal(geometry.bottom(), 220);
        geometry.finalize_dimensions();
        test::expect(std::abs(geometry.coordinate_system_width() - 16383.5F) < 0.01F);
        test::expect(std::abs(geometry.coordinate_system_height() - 32767.0F) < 0.01F);
        test::expect_equal(geometry.to_child_x(60), 0);
        test::expect_equal(geometry.to_child_y(120), 0);
        test::expect_equal(geometry.from_child_x(0), 60);
        test::expect_equal(geometry.from_child_y(0), 120);

        g_typesystem = &typesystem;
        typesystem.register_type<id_api::function_id_t>();
        typesystem.register_type<api::function_t*>();
        typesystem.register_coercion<id_api::function_id_t, api::function_t*>(
            &function_id_to_function
        );

        const auto child_id = make_id("child", std::chrono::seconds(20));
        const auto compound_id = make_id("compound", std::chrono::seconds(21));
        ir_api::function_ir_t compound_ir {
            .function_id = compound_id,
            .left = 0,
            .right = 0,
            .top = 0,
            .bottom = 0,
            .children = {
                ir_api::function_ir_t::child_t {
                    .function_id = child_id,
                    .left = 10,
                    .right = 110,
                    .top = 20,
                    .bottom = 120
                }
            },
            .connections = {
                ir_api::function_ir_t::connection_info_t {
                    .from_function_index = 0,
                    .from_argument_index = 0,
                    .to_function_index = std::numeric_limits<std::uint16_t>::max(),
                    .to_argument_index = 1
                }
            }
        };

        api::function_t compound(typesystem, compound_ir, &record_call);
        compound.arguments().resize(2);
        compound.expand();
        test::expect_equal(compound.children().size(), std::size_t(1));
        auto* child = compound.children()[0];
        test::expect_equal(child->function_ir().function_id, child_id);
        test::expect_equal(child->left(), 10);
        test::expect_equal(child->right(), 110);
        test::expect_equal(child->top(), 20);
        test::expect_equal(child->bottom(), 120);
        test::expect(std::abs(child->coordinate_system_width() - 32767.0F) < 0.01F);
        test::expect(std::abs(child->coordinate_system_height() - 32767.0F) < 0.01F);
        test::expect(child->is_connected(0));
        test::expect_equal(child->connection(0), &compound);

        g_call_count = 0;
        child->write(0, 77);
        test::expect_equal(g_call_count, 1);
        test::expect_equal(g_last_argument, std::uint8_t(1));
        test::expect_equal(static_cast<int>(compound.read(1)), 77);

        compound.expand();
        test::expect_equal(compound.children().size(), std::size_t(1));
        test::expect_throws<std::runtime_error>([&] {
            compound.shrink();
        });

        ir_api::function_ir_t same_identity_ir = compound_ir;
        same_identity_ir.left = 500;
        g_alternate_call_count = 0;
        compound.morph(typesystem, same_identity_ir, &alternate_call);
        test::expect_equal(compound.function_ir().left, 500);
        test::expect_equal(compound.children().size(), std::size_t(1));
        compound.call(4);
        test::expect_equal(g_alternate_call_count, 1);
        test::expect_equal(g_last_argument, std::uint8_t(4));

        ir_api::function_ir_t different_identity_ir = same_identity_ir;
        different_identity_ir.function_id = make_id(
            "different",
            std::chrono::seconds(22)
        );
        test::expect_throws<std::runtime_error>([&] {
            compound.morph(typesystem, different_identity_ir, &no_op_call);
        });
        test::expect_equal(compound.function_ir().function_id, compound_id);

        child->disconnect(0);
        delete child;
        compound.children().clear();

        api::function_t unexpanded(typesystem, ir_api::function_ir_t {}, &no_op_call);
        test::expect_no_throw([&] { unexpanded.shrink(); });

        ir_api::function_ir_t invalid_ir {
            .function_id = make_id("invalid", std::chrono::seconds(30)),
            .left = 0,
            .right = 0,
            .top = 0,
            .bottom = 0,
            .children = {},
            .connections = {
                ir_api::function_ir_t::connection_info_t {
                    .from_function_index = 0,
                    .from_argument_index = 0,
                    .to_function_index = std::numeric_limits<std::uint16_t>::max(),
                    .to_argument_index = 0
                }
            }
        };
        api::function_t invalid(typesystem, invalid_ir, &no_op_call);
        invalid.arguments().resize(1);
        test::expect_throws<std::runtime_error>([&] {
            invalid.expand();
        });
    });
}
