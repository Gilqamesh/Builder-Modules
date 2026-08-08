#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij4fjvyaf48asgpm6fdt_function_ir_file_repository/function_ir_file_repository.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

namespace api = m03ge9ij4fjvyaf48asgpm6fdt_function_ir_file_repository;
namespace id_api = m03ge9ij45dcznrmna12qow5r5_function_id;
namespace ir_api = m03ge9ij46lc986vpdamnc2fka_function_ir;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        m_path = std::filesystem::temp_directory_path() / std::format(
            "builder-function-ir-repository-public-api-{}-{}",
            std::chrono::steady_clock::now().time_since_epoch().count(),
            reinterpret_cast<std::uintptr_t>(this)
        );
        std::filesystem::create_directory(m_path);
    }

    ~temporary_directory_t() {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    const std::filesystem::path& path() const {
        return m_path;
    }

private:
    std::filesystem::path m_path;
};

id_api::function_id_t make_id(
    std::string ns,
    std::string name,
    std::chrono::seconds seconds
) {
    return id_api::function_id_t {
        .ns = std::move(ns),
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
                .function_id = make_id(
                    "repository",
                    "child",
                    std::chrono::seconds(5)
                ),
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

void expect_ir_equal(
    const ir_api::function_ir_t& actual,
    const ir_api::function_ir_t& expected
) {
    test::expect_equal(actual.function_id, expected.function_id);
    test::expect_equal(actual.left, expected.left);
    test::expect_equal(actual.right, expected.right);
    test::expect_equal(actual.top, expected.top);
    test::expect_equal(actual.bottom, expected.bottom);
    test::expect_equal(actual.children.size(), expected.children.size());
    test::expect_equal(actual.connections.size(), expected.connections.size());

    for (std::size_t i = 0; i < actual.children.size(); ++i) {
        test::expect_equal(
            actual.children[i].function_id,
            expected.children[i].function_id
        );
        test::expect_equal(actual.children[i].left, expected.children[i].left);
        test::expect_equal(actual.children[i].right, expected.children[i].right);
        test::expect_equal(actual.children[i].top, expected.children[i].top);
        test::expect_equal(actual.children[i].bottom, expected.children[i].bottom);
    }

    for (std::size_t i = 0; i < actual.connections.size(); ++i) {
        test::expect_equal(
            actual.connections[i].from_function_index,
            expected.connections[i].from_function_index
        );
        test::expect_equal(
            actual.connections[i].from_argument_index,
            expected.connections[i].from_argument_index
        );
        test::expect_equal(
            actual.connections[i].to_function_index,
            expected.connections[i].to_function_index
        );
        test::expect_equal(
            actual.connections[i].to_argument_index,
            expected.connections[i].to_argument_index
        );
    }
}

void write_bytes(const std::filesystem::path& path, std::string_view bytes) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to create file-repository fixture");
    }
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

} // namespace

int main() {
    return test::run([] {
        temporary_directory_t temporary_directory;
        const auto repository_path = temporary_directory.path()
            / "nested"
            / "function-ir";
        test::expect(!std::filesystem::exists(repository_path));
        api::function_ir_file_repository_t repository(repository_path);
        test::expect(std::filesystem::is_directory(repository_path));

        const auto first_id = make_id(
            "math",
            "add",
            std::chrono::seconds(10)
        );
        const auto second_id = make_id(
            "math",
            "add",
            std::chrono::seconds(20)
        );
        const auto other_id = make_id(
            "math",
            "subtract",
            std::chrono::seconds(30)
        );
        const auto first_ir = make_ir(first_id, 10);
        const auto second_ir = make_ir(second_id, 20);
        const auto other_ir = make_ir(other_id, 30);

        repository.save(first_ir);
        repository.save(second_ir);
        repository.save(other_ir);
        test::expect(std::filesystem::is_regular_file(
            repository_path / id_api::function_id_t::to_string(first_id)
        ));
        test::expect(std::filesystem::is_regular_file(
            repository_path / id_api::function_id_t::to_string(second_id)
        ));
        test::expect(std::filesystem::is_regular_file(
            repository_path / id_api::function_id_t::to_string(other_id)
        ));

        expect_ir_equal(repository.load(first_id), first_ir);
        expect_ir_equal(repository.load(second_id), second_ir);
        expect_ir_equal(repository.load(other_id), other_ir);
        expect_ir_equal(repository.load_latest("math", "add"), second_ir);
        expect_ir_equal(repository.load_latest("math", "subtract"), other_ir);

        const auto overwritten_first = make_ir(first_id, 100);
        repository.save(overwritten_first);
        expect_ir_equal(repository.load(first_id), overwritten_first);
        expect_ir_equal(repository.load_latest("math", "add"), second_ir);

        std::filesystem::create_directory(repository_path / "ignored-directory");
        expect_ir_equal(repository.load_latest("math", "add"), second_ir);

        const auto missing_id = make_id(
            "math",
            "missing",
            std::chrono::seconds(40)
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = repository.load(missing_id);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = repository.load_latest(
                "missing",
                "function"
            );
        });

        const auto corrupt_id = make_id(
            "math",
            "corrupt",
            std::chrono::seconds(50)
        );
        write_bytes(
            repository_path / id_api::function_id_t::to_string(corrupt_id),
            "bad"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = repository.load(corrupt_id);
        });

        write_bytes(repository_path / "not-a-function-id", "irrelevant");
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = repository.load_latest(
                "math",
                "add"
            );
        });

        const auto file_path = temporary_directory.path() / "repository-file";
        write_bytes(file_path, "not a directory");
        api::function_ir_file_repository_t file_repository(file_path);
        test::expect_throws<std::runtime_error>([&] {
            file_repository.save(first_ir);
        });
    });
}
