#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij47sume9p7pg7nbvu88_function_ir_binary/function_ir_binary.h>

#include <functional>
#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace api = m03ge9ij47sume9p7pg7nbvu88_function_ir_binary;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


namespace {

id_api::function_id_t make_id(
    std::string ns,
    std::string name,
    std::chrono::system_clock::duration duration
) {
    return id_api::function_id_t {
        .ns = std::move(ns),
        .name = std::move(name),
        .creation_time = std::chrono::system_clock::time_point(duration)
    };
}

void expect_id_equal(
    const id_api::function_id_t& actual,
    const id_api::function_id_t& expected
) {
    test::expect(std::equal_to<>(), actual.ns, expected.ns);
    test::expect(std::equal_to<>(), actual.name, expected.name);
    test::expect(std::equal_to<>(), actual.creation_time, expected.creation_time);
}

} // namespace

int main() {
    return test::run([] {
        using namespace std::chrono_literals;

        const ir_api::function_ir_t empty_ir {};
        const api::function_ir_binary_t empty_binary(empty_ir);
        test::expect(std::equal_to<>(), empty_binary.bytes().size(), std::size_t(10));
        test::expect(std::equal_to<>(), empty_binary.bytes()[0], std::uint8_t(0));
        test::expect(std::equal_to<>(), empty_binary.bytes()[1], std::uint8_t(0));
        for (std::size_t i = 2; i < empty_binary.bytes().size(); ++i) {
            test::expect(std::equal_to<>(), empty_binary.bytes()[i], std::uint8_t(0));
        }
        const auto decoded_empty = empty_binary.function_ir();
        test::expect(std::identity(), !static_cast<bool>(decoded_empty.function_id));
        test::expect(std::identity(), decoded_empty.children.empty());
        test::expect(std::identity(), decoded_empty.connections.empty());

        const auto root_id = make_id("graph", "root", 100s + 999ms);
        const auto first_child_id = make_id("math", "add", 101s);
        const auto second_child_id = make_id("math", "multiply", 102s);
        const ir_api::function_ir_t ir {
            .function_id = root_id,
            .left = 500,
            .right = 600,
            .top = 700,
            .bottom = 800,
            .children = {
                ir_api::function_ir_t::child_t {
                    .function_id = first_child_id,
                    .left = -32768,
                    .right = 1,
                    .top = -255,
                    .bottom = 256
                },
                ir_api::function_ir_t::child_t {
                    .function_id = second_child_id,
                    .left = 1024,
                    .right = 4096,
                    .top = 16384,
                    .bottom = 32767
                }
            },
            .connections = {
                ir_api::function_ir_t::connection_info_t {
                    .from_function_index = 0,
                    .from_argument_index = 1,
                    .to_function_index = 1,
                    .to_argument_index = 2
                },
                ir_api::function_ir_t::connection_info_t {
                    .from_function_index = 1,
                    .from_argument_index = 3,
                    .to_function_index = 0,
                    .to_argument_index = 4
                }
            }
        };

        const api::function_ir_binary_t binary(ir);
        test::expect(std::identity(), !binary.bytes().empty());
        const auto decoded = binary.function_ir();
        expect_id_equal(
            decoded.function_id,
            make_id("graph", "root", 100s)
        );
        test::expect(std::equal_to<>(), decoded.children.size(), std::size_t(2));
        expect_id_equal(decoded.children[0].function_id, first_child_id);
        test::expect(std::equal_to<>(), decoded.children[0].left, -32768);
        test::expect(std::equal_to<>(), decoded.children[0].right, 1);
        test::expect(std::equal_to<>(), decoded.children[0].top, -255);
        test::expect(std::equal_to<>(), decoded.children[0].bottom, 256);
        expect_id_equal(decoded.children[1].function_id, second_child_id);
        test::expect(std::equal_to<>(), decoded.children[1].left, 1024);
        test::expect(std::equal_to<>(), decoded.children[1].right, 4096);
        test::expect(std::equal_to<>(), decoded.children[1].top, 16384);
        test::expect(std::equal_to<>(), decoded.children[1].bottom, 32767);

        test::expect(std::equal_to<>(), decoded.connections.size(), std::size_t(2));
        test::expect(std::equal_to<>(), decoded.connections[0].from_function_index, std::uint16_t(0));
        test::expect(std::equal_to<>(), decoded.connections[0].from_argument_index, std::uint8_t(1));
        test::expect(std::equal_to<>(), decoded.connections[0].to_function_index, std::uint16_t(1));
        test::expect(std::equal_to<>(), decoded.connections[0].to_argument_index, std::uint8_t(2));
        test::expect(std::equal_to<>(), decoded.connections[1].from_function_index, std::uint16_t(1));
        test::expect(std::equal_to<>(), decoded.connections[1].from_argument_index, std::uint8_t(3));
        test::expect(std::equal_to<>(), decoded.connections[1].to_function_index, std::uint16_t(0));
        test::expect(std::equal_to<>(), decoded.connections[1].to_argument_index, std::uint8_t(4));

        const api::function_ir_binary_t rebuilt(decoded);
        test::expect(std::identity(), rebuilt.bytes() == binary.bytes());

        auto root_connection_ir = ir;
        root_connection_ir.connections = {
            ir_api::function_ir_t::connection_info_t {
                .from_function_index = std::numeric_limits<std::uint16_t>::max(),
                .from_argument_index = 1,
                .to_function_index = 0,
                .to_argument_index = 2
            }
        };
        const auto decoded_root_connection = api::function_ir_binary_t(root_connection_ir).function_ir();
        test::expect(std::equal_to<>(), decoded_root_connection.connections[0].from_function_index,
            std::numeric_limits<std::uint16_t>::max()
        );
        test::expect(std::equal_to<>(), decoded_root_connection.connections[0].to_function_index, std::uint16_t(0));

        auto invalid_index_ir = ir;
        invalid_index_ir.connections[0].from_function_index = std::numeric_limits<std::uint8_t>::max();
        test::expect_throws<std::invalid_argument>([&] {
            [[maybe_unused]] const api::function_ir_binary_t invalid(invalid_index_ir);
        });

        auto invalid_coordinate_ir = ir;
        invalid_coordinate_ir.children[0].left = std::numeric_limits<std::int16_t>::max() + 1;
        test::expect_throws<std::invalid_argument>([&] {
            [[maybe_unused]] const api::function_ir_binary_t invalid(invalid_coordinate_ir);
        });

        auto invalid_id_ir = ir;
        invalid_id_ir.function_id.name = std::string("bad\0name", 8);
        test::expect_throws<std::invalid_argument>([&] {
            [[maybe_unused]] const api::function_ir_binary_t invalid(invalid_id_ir);
        });

        std::vector<std::uint8_t> raw { 1, 2, 3, 4 };
        const api::function_ir_binary_t from_bytes(raw);
        test::expect(std::identity(), from_bytes.bytes() == raw);
        raw[0] = 99;
        test::expect(std::equal_to<>(), from_bytes.bytes()[0], std::uint8_t(1));

        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                std::vector<std::uint8_t> {}
            ).function_ir();
        });
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                std::vector<std::uint8_t> { 'n', 's' }
            ).function_ir();
        });
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                std::vector<std::uint8_t> { 0, 'n', 'a', 'm', 'e' }
            ).function_ir();
        });
        test::expect_throws<std::runtime_error>([] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                std::vector<std::uint8_t> { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
            ).function_ir();
        });

        auto unknown_opcode = empty_binary.bytes();
        unknown_opcode.push_back(255);
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                unknown_opcode
            ).function_ir();
        });

        auto truncated_create = empty_binary.bytes();
        truncated_create.push_back(0);
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                truncated_create
            ).function_ir();
        });

        auto truncated_connection = empty_binary.bytes();
        truncated_connection.push_back(1);
        truncated_connection.push_back(0);
        truncated_connection.push_back(1);
        truncated_connection.push_back(2);
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto ir = api::function_ir_binary_t(
                truncated_connection
            ).function_ir();
        });
    });
}
