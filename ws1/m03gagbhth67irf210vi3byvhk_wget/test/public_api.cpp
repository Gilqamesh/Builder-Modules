#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gagbhth67irf210vi3byvhk_wget/wget.h>

#include <arpa/inet.h>
#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace api = m03gagbhth67irf210vi3byvhk_wget;
namespace filesystem_api = m03gagbhsnusi43zogoacgj2ez_filesystem;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

class temporary_directory_t {
public:
    temporary_directory_t() {
        m_path = std::filesystem::temp_directory_path() / std::format(
            "builder-wget-public-api-{}-{}",
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

class environment_guard_t {
public:
    explicit environment_guard_t(const char* name):
        m_name(name)
    {
        if (const char* value = std::getenv(name); value != nullptr) {
            m_value = value;
        }
    }

    ~environment_guard_t() {
        if (m_value) {
            setenv(m_name.c_str(), m_value->c_str(), 1);
        } else {
            unsetenv(m_name.c_str());
        }
    }

private:
    std::string m_name;
    std::optional<std::string> m_value;
};

class http_server_t {
public:
    http_server_t(std::string body, bool success) {
        int listener = socket(AF_INET, SOCK_STREAM, 0);
        if (listener == -1) {
            throw std::runtime_error("socket failed");
        }

        int reuse = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        if (bind(listener, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == -1) {
            close(listener);
            throw std::runtime_error("bind failed");
        }
        if (listen(listener, 1) == -1) {
            close(listener);
            throw std::runtime_error("listen failed");
        }

        socklen_t address_size = sizeof(address);
        if (getsockname(listener, reinterpret_cast<sockaddr*>(&address), &address_size) == -1) {
            close(listener);
            throw std::runtime_error("getsockname failed");
        }
        m_port = ntohs(address.sin_port);

        m_pid = fork();
        if (m_pid == -1) {
            close(listener);
            throw std::runtime_error("fork failed");
        }
        if (m_pid == 0) {
            const int connection = accept(listener, nullptr, nullptr);
            if (connection == -1) {
                _exit(240);
            }

            char request[4096];
            while (recv(connection, request, sizeof(request), 0) > 0) {
                const std::string_view received(request, sizeof(request));
                if (received.find("\r\n\r\n") != std::string_view::npos) {
                    break;
                }
            }

            const std::string response = std::format(
                "HTTP/1.1 {}\r\n"
                "Content-Length: {}\r\n"
                "Connection: close\r\n"
                "Content-Type: application/octet-stream\r\n"
                "\r\n{}",
                success ? "200 OK" : "404 Not Found",
                body.size(),
                body
            );

            std::size_t sent = 0;
            while (sent < response.size()) {
                const ssize_t count = send(
                    connection,
                    response.data() + sent,
                    response.size() - sent,
                    0
                );
                if (count <= 0) {
                    close(connection);
                    close(listener);
                    _exit(241);
                }
                sent += static_cast<std::size_t>(count);
            }

            close(connection);
            close(listener);
            _exit(0);
        }

        close(listener);
    }

    ~http_server_t() {
        if (0 < m_pid) {
            int status = 0;
            waitpid(m_pid, &status, 0);
        }
    }

    std::string url() const {
        return std::format("http://127.0.0.1:{}/fixture", m_port);
    }

    void expect_success() {
        int status = 0;
        test::expect_equal(waitpid(m_pid, &status, 0), m_pid);
        m_pid = -1;
        test::expect(WIFEXITED(status));
        test::expect_equal(WEXITSTATUS(status), 0);
    }

private:
    pid_t m_pid = -1;
    std::uint16_t m_port = 0;
};

void write_file(const std::filesystem::path& path, std::string_view contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to create wget fixture");
    }
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("failed to read wget fixture");
    }
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
}

} // namespace

int main() {
    return test::run([] {
        temporary_directory_t temporary_directory;
        environment_guard_t lower_no_proxy_guard("no_proxy");
        environment_guard_t upper_no_proxy_guard("NO_PROXY");
        setenv("no_proxy", "127.0.0.1,localhost", 1);
        setenv("NO_PROXY", "127.0.0.1,localhost", 1);

        const std::string payload = "wget public API payload\n";
        http_server_t success_server(payload, true);
        const filesystem_api::path_t output(
            temporary_directory.path() / "downloads" / "payload.bin"
        );
        test::expect_equal(api::download(success_server.url(), output), output);
        success_server.expect_success();
        test::expect(filesystem_api::is_regular_file(output));
        test::expect_equal(read_file(output.to_native_path()), payload);

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::download("", output);
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::download(
                "http://127.0.0.1/unused",
                output
            );
        });

        const filesystem_api::path_t preexisting(
            temporary_directory.path() / "preexisting.bin"
        );
        write_file(preexisting.to_native_path(), "keep");
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::download(
                "http://127.0.0.1/unused",
                preexisting
            );
        });
        test::expect_equal(read_file(preexisting.to_native_path()), std::string("keep"));

        http_server_t failure_server("missing", false);
        const filesystem_api::path_t failed_output(
            temporary_directory.path() / "failed" / "payload.bin"
        );
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto value = api::download(
                failure_server.url(),
                failed_output
            );
        });
        failure_server.expect_success();
        test::expect(!filesystem_api::exists(failed_output));
    });
}
