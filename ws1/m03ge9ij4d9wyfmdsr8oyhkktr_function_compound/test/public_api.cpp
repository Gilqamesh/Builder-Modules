#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4d9wyfmdsr8oyhkktr_function_compound/function_compound.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace api = m03ge9ij4d9wyfmdsr8oyhkktr_function_compound;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace runtime_api = m03ge9ij49xkr5obofujoj7ltw_function_runtime;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace typesystem_api = m03ge9ij43jyxy821pda20jhwh_typesystem;

namespace {

typesystem_api::typesystem_t* g_typesystem = nullptr;
int g_sink_call_count = 0;
std::uint8_t g_last_sink_argument = 0;

void no_op_call(runtime_api::function_t&, std::uint8_t) {
}

void sink_call(runtime_api::function_t&, std::uint8_t argument_index) {
    ++g_sink_call_count;
    g_last_sink_argument = argument_index;
}

runtime_api::function_t* create_child(id_api::function_id_t id) {
    auto* child = new runtime_api::function_t(
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
    child->arguments().resize(1);
    return child;
}

id_api::function_id_t make_id(std::string name, std::chrono::seconds seconds) {
    return id_api::function_id_t {
        .ns = "compound",
        .name = std::move(name),
        .creation_time = std::chrono::system_clock::time_point(seconds)
    };
}

} // namespace

int main() {
    return test::run([] {
        typesystem_api::typesystem_t typesystem;
        g_typesystem = &typesystem;
        typesystem.register_type<id_api::function_id_t>();
        typesystem.register_type<runtime_api::function_t*>();
        typesystem.register_coercion<id_api::function_id_t, runtime_api::function_t*>(
            &create_child
        );

        const auto root_id = make_id("root", std::chrono::seconds(10));
        const auto child_id = make_id("child", std::chrono::seconds(11));
        ir_api::function_ir_t ir {
            .function_id = root_id,
            .left = 1,
            .right = 2,
            .top = 3,
            .bottom = 4,
            .children = {
                ir_api::function_ir_t::child_t {
                    .function_id = child_id,
                    .left = 10,
                    .right = 110,
                    .top = 20,
                    .bottom = 120
                }
            },
            .connections = {}
        };

        std::unique_ptr<runtime_api::function_t> compound(
            api::function_compound_t::function(typesystem, ir)
        );
        test::expect(std::identity(), compound != nullptr);
        test::expect(std::equal_to<>(), compound->function_ir().function_id, root_id);
        test::expect(std::equal_to<>(), compound->function_ir().left, 1);
        test::expect(std::equal_to<>(), compound->function_ir().right, 2);
        test::expect(std::equal_to<>(), compound->function_ir().top, 3);
        test::expect(std::equal_to<>(), compound->function_ir().bottom, 4);
        test::expect(std::equal_to<>(), compound->function_ir().children.size(), std::size_t(1));
        test::expect(std::identity(), compound->children().empty());
        test::expect(std::identity(), compound->arguments().empty());

        ir.function_id = make_id("mutated-source", std::chrono::seconds(12));
        ir.children.clear();
        test::expect(std::equal_to<>(), compound->function_ir().function_id, root_id);
        test::expect(std::equal_to<>(), compound->function_ir().children.size(), std::size_t(1));

        compound->arguments().resize(1);
        runtime_api::function_t sink(
            typesystem,
            ir_api::function_ir_t {},
            &sink_call
        );
        sink.arguments().resize(1);
        compound->connect(&sink, 0, 0);

        g_sink_call_count = 0;
        compound->write(0, 42);
        test::expect(std::equal_to<>(), g_sink_call_count, 1);
        test::expect(std::equal_to<>(), static_cast<int>(sink.read(0)), 42);
        test::expect(std::identity(), compound->children().empty());

        compound->call(0);
        test::expect(std::equal_to<>(), g_sink_call_count, 2);
        test::expect(std::equal_to<>(), g_last_sink_argument, std::uint8_t(0));
        test::expect(std::equal_to<>(), compound->children().size(), std::size_t(1));
        auto* child = compound->children()[0];
        test::expect(std::equal_to<>(), child->function_ir().function_id, child_id);
        test::expect(std::equal_to<>(), child->left(), 10);
        test::expect(std::equal_to<>(), child->right(), 110);
        test::expect(std::equal_to<>(), child->top(), 20);
        test::expect(std::equal_to<>(), child->bottom(), 120);

        compound->call(0);
        test::expect(std::equal_to<>(), g_sink_call_count, 3);
        test::expect(std::equal_to<>(), compound->children().size(), std::size_t(1));
        test::expect(std::equal_to<>(), compound->children()[0], child);

        delete child;
        compound->children().clear();
        compound->disconnect(0);

        ir_api::function_ir_t empty_ir {
            .function_id = make_id("empty", std::chrono::seconds(20)),
            .left = 0,
            .right = 0,
            .top = 0,
            .bottom = 0,
            .children = {},
            .connections = {}
        };
        std::unique_ptr<runtime_api::function_t> empty(
            api::function_compound_t::function(typesystem, empty_ir)
        );
        empty->arguments().resize(1);
        empty->write(0, 7);
        test::expect_no_throw([&] { empty->call(0); });
        test::expect(std::identity(), empty->children().empty());
        test::expect(std::equal_to<>(), static_cast<int>(empty->read(0)), 7);
    });
}
