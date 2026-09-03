#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4ec2ss9jnrfdsrjm1q_function_repository/function_repository.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace api = m03ge9ij4ec2ss9jnrfdsrjm1q_function_repository;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace runtime_api = m03ge9ij49xkr5obofujoj7ltw_function_runtime;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace typesystem_api = m03ge9ij43jyxy821pda20jhwh_typesystem;

namespace {

int g_first_call_count = 0;
int g_second_call_count = 0;
std::uint8_t g_last_argument = 0;

void first_call(runtime_api::function_t&, std::uint8_t argument_index) {
    ++g_first_call_count;
    g_last_argument = argument_index;
}

void second_call(runtime_api::function_t&, std::uint8_t argument_index) {
    ++g_second_call_count;
    g_last_argument = argument_index;
}

id_api::function_id_t make_id(std::string name, std::chrono::seconds seconds) {
    return id_api::function_id_t {
        .ns = "repository",
        .name = std::move(name),
        .creation_time = std::chrono::system_clock::time_point(seconds)
    };
}

ir_api::function_ir_t make_ir(
    const id_api::function_id_t& id,
    int coordinate
) {
    return ir_api::function_ir_t {
        .function_id = id,
        .left = coordinate,
        .right = coordinate + 10,
        .top = coordinate + 20,
        .bottom = coordinate + 30,
        .children = {
            ir_api::function_ir_t::child_t {
                .function_id = make_id("child", std::chrono::seconds(99)),
                .left = 1,
                .right = 2,
                .top = 3,
                .bottom = 4
            }
        },
        .connections = {
            ir_api::function_ir_t::connection_info_t {
                .from_function_index = 0,
                .from_argument_index = 1,
                .to_function_index = 0,
                .to_argument_index = 2
            }
        }
    };
}

} // namespace

int main() {
    return test::run([] {
        api::function_repository_t repository;
        const auto first_id = make_id("first", std::chrono::seconds(10));
        const auto second_id = make_id("second", std::chrono::seconds(11));
        const auto missing_id = make_id("missing", std::chrono::seconds(12));

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto entry = repository.load(missing_id);
        });

        const auto first_ir = make_ir(first_id, 10);
        repository.save(first_ir, &first_call);
        const auto first_entry = repository.load(first_id);
        test::expect(std::identity(), first_entry.call == &first_call);
        test::expect(std::equal_to<>(), first_entry.ir.function_id, first_id);
        test::expect(std::equal_to<>(), first_entry.ir.left, 10);
        test::expect(std::equal_to<>(), first_entry.ir.right, 20);
        test::expect(std::equal_to<>(), first_entry.ir.top, 30);
        test::expect(std::equal_to<>(), first_entry.ir.bottom, 40);
        test::expect(std::equal_to<>(), first_entry.ir.children.size(), std::size_t(1));
        test::expect(std::equal_to<>(), first_entry.ir.connections.size(), std::size_t(1));

        typesystem_api::typesystem_t typesystem;
        runtime_api::function_t function(
            typesystem,
            first_entry.ir,
            first_entry.call
        );
        g_first_call_count = 0;
        first_entry.call(function, 7);
        test::expect(std::equal_to<>(), g_first_call_count, 1);
        test::expect(std::equal_to<>(), g_last_argument, std::uint8_t(7));

        auto modified_entry = repository.load(first_id);
        modified_entry.ir.left = 999;
        modified_entry.call = &second_call;
        const auto unchanged_entry = repository.load(first_id);
        test::expect(std::equal_to<>(), unchanged_entry.ir.left, 10);
        test::expect(std::identity(), unchanged_entry.call == &first_call);

        const auto replacement_ir = make_ir(first_id, 100);
        repository.save(replacement_ir, &second_call);
        const auto duplicate_entry = repository.load(first_id);
        test::expect(std::equal_to<>(), duplicate_entry.ir.left, 10);
        test::expect(std::identity(), duplicate_entry.call == &first_call);

        repository.save(make_ir(second_id, 50), &second_call);
        const auto second_entry = repository.load(second_id);
        test::expect(std::identity(), second_entry.call == &second_call);
        test::expect(std::equal_to<>(), second_entry.ir.function_id, second_id);
        test::expect(std::equal_to<>(), second_entry.ir.left, 50);
        g_second_call_count = 0;
        second_entry.call(function, 9);
        test::expect(std::equal_to<>(), g_second_call_count, 1);
        test::expect(std::equal_to<>(), g_last_argument, std::uint8_t(9));

        const api::function_repository_t::entry_t aggregate_entry {
            .call = &first_call,
            .ir = first_ir
        };
        test::expect(std::identity(), aggregate_entry.call == &first_call);
        test::expect(std::equal_to<>(), aggregate_entry.ir.function_id, first_id);

        const id_api::function_id_t empty_id;
        const ir_api::function_ir_t empty_ir {
            .function_id = empty_id,
            .left = 1,
            .right = 2,
            .top = 3,
            .bottom = 4,
            .children = {},
            .connections = {}
        };
        repository.save(empty_ir, &second_call);
        const auto empty_entry = repository.load(empty_id);
        test::expect(std::identity(), empty_entry.call == &second_call);
        test::expect(std::identity(), !static_cast<bool>(empty_entry.ir.function_id));
        test::expect(std::equal_to<>(), empty_entry.ir.left, 1);
    });
}
