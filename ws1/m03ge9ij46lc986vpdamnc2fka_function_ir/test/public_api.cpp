#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij46lc986vpdamnc2fka_function_ir/function_ir.h>

#include <chrono>
#include <cstdint>
#include <string>

namespace api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        using namespace std::chrono_literals;

        const api::function_ir_t empty {};
        test::expect(!static_cast<bool>(empty.function_id));
        test::expect_equal(empty.left, 0);
        test::expect_equal(empty.right, 0);
        test::expect_equal(empty.top, 0);
        test::expect_equal(empty.bottom, 0);
        test::expect(empty.children.empty());
        test::expect(empty.connections.empty());

        const id_api::function_id_t root_id {
            .ns = "graph",
            .name = "root",
            .creation_time = std::chrono::system_clock::time_point(100s)
        };
        const id_api::function_id_t child_a_id {
            .ns = "math",
            .name = "add",
            .creation_time = std::chrono::system_clock::time_point(101s)
        };
        const id_api::function_id_t child_b_id {
            .ns = "math",
            .name = "multiply",
            .creation_time = std::chrono::system_clock::time_point(102s)
        };

        api::function_ir_t ir {
            .function_id = root_id,
            .left = -10,
            .right = 250,
            .top = 7,
            .bottom = 99,
            .children = {
                api::function_ir_t::child_t {
                    .function_id = child_a_id,
                    .left = 1,
                    .right = 2,
                    .top = 3,
                    .bottom = 4
                },
                api::function_ir_t::child_t {
                    .function_id = child_b_id,
                    .left = -5,
                    .right = 20,
                    .top = -30,
                    .bottom = 40
                }
            },
            .connections = {
                api::function_ir_t::connection_info_t {
                    .from_function_index = 0,
                    .from_argument_index = 1,
                    .to_function_index = 1,
                    .to_argument_index = 2
                },
                api::function_ir_t::connection_info_t {
                    .from_function_index = 1,
                    .from_argument_index = 0,
                    .to_function_index = 0,
                    .to_argument_index = 3
                }
            }
        };

        test::expect_equal(ir.function_id, root_id);
        test::expect_equal(ir.left, -10);
        test::expect_equal(ir.right, 250);
        test::expect_equal(ir.top, 7);
        test::expect_equal(ir.bottom, 99);
        test::expect_equal(ir.children.size(), std::size_t(2));
        test::expect_equal(ir.children[0].function_id, child_a_id);
        test::expect_equal(ir.children[0].left, 1);
        test::expect_equal(ir.children[0].right, 2);
        test::expect_equal(ir.children[0].top, 3);
        test::expect_equal(ir.children[0].bottom, 4);
        test::expect_equal(ir.children[1].function_id, child_b_id);
        test::expect_equal(ir.children[1].left, -5);
        test::expect_equal(ir.children[1].right, 20);
        test::expect_equal(ir.children[1].top, -30);
        test::expect_equal(ir.children[1].bottom, 40);

        test::expect_equal(ir.connections.size(), std::size_t(2));
        test::expect_equal(ir.connections[0].from_function_index, std::uint16_t(0));
        test::expect_equal(ir.connections[0].from_argument_index, std::uint8_t(1));
        test::expect_equal(ir.connections[0].to_function_index, std::uint16_t(1));
        test::expect_equal(ir.connections[0].to_argument_index, std::uint8_t(2));
        test::expect_equal(ir.connections[1].from_function_index, std::uint16_t(1));
        test::expect_equal(ir.connections[1].to_argument_index, std::uint8_t(3));

        ir.children.push_back(api::function_ir_t::child_t {
            .function_id = root_id,
            .left = 50,
            .right = 60,
            .top = 70,
            .bottom = 80
        });
        ir.connections.push_back(api::function_ir_t::connection_info_t {
            .from_function_index = 2,
            .from_argument_index = 4,
            .to_function_index = 1,
            .to_argument_index = 5
        });
        test::expect_equal(ir.children.size(), std::size_t(3));
        test::expect_equal(ir.connections.size(), std::size_t(3));
    });
}
