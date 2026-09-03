#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer/api.h>

#include <functional>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace api = m03glhrcfhhqzxmllaxhicu5hc_spsc_ring_buffer;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;


int main() {
    return test::run([] {
        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::spsc_ring_buffer_t<int> buffer(
                std::numeric_limits<std::size_t>::digits
            );
        });

        api::spsc_ring_buffer_t<int> buffer(1);
        test::expect(std::equal_to<>(), buffer.buffer().size(), std::size_t(2));
        test::expect(std::equal_to<>(), buffer.head(), std::size_t(0));
        test::expect(std::equal_to<>(), buffer.tail(), std::size_t(0));

        int value = -1;
        test::expect(std::identity(), !buffer.try_read(value));
        test::expect(std::equal_to<>(), value, -1);

        const int copied = 10;
        test::expect(std::identity(), buffer.try_write(copied));
        test::expect(std::identity(), buffer.try_write(20));
        test::expect(std::identity(), !buffer.try_write(30));
        test::expect(std::equal_to<>(), buffer.head(), std::size_t(2));
        test::expect(std::equal_to<>(), buffer.tail(), std::size_t(0));

        test::expect(std::identity(), buffer.try_read(value));
        test::expect(std::equal_to<>(), value, 10);
        test::expect(std::equal_to<>(), buffer.tail(), std::size_t(1));

        test::expect(std::identity(), buffer.try_write(30));
        test::expect(std::equal_to<>(), buffer.head(), std::size_t(3));

        test::expect(std::identity(), buffer.try_read(value));
        test::expect(std::equal_to<>(), value, 20);
        test::expect(std::identity(), buffer.try_read(value));
        test::expect(std::equal_to<>(), value, 30);
        test::expect(std::identity(), !buffer.try_read(value));
        test::expect(std::equal_to<>(), buffer.head(), std::size_t(3));
        test::expect(std::equal_to<>(), buffer.tail(), std::size_t(3));

        const auto formatted = std::format("{}", buffer);
        test::expect(std::identity(), formatted.find("capacity: 2") != std::string::npos);
        test::expect(std::identity(), formatted.find("head: 3") != std::string::npos);
        test::expect(std::identity(), formatted.find("tail: 3") != std::string::npos);
        test::expect(std::identity(), formatted.find("values: [") != std::string::npos);

        api::spsc_ring_buffer_t<std::string> strings(0);
        std::string source = "copied";
        test::expect(std::identity(), strings.try_write(source));
        test::expect(std::equal_to<>(), source, std::string("copied"));
        test::expect(std::identity(), !strings.try_write(std::string("full")));

        std::string destination;
        test::expect(std::identity(), strings.try_read(destination));
        test::expect(std::equal_to<>(), destination, std::string("copied"));
        test::expect(std::identity(), strings.try_write(std::string("moved")));
        test::expect(std::identity(), strings.try_read(destination));
        test::expect(std::equal_to<>(), destination, std::string("moved"));
    });
}
