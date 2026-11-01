#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>
#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <exception>
#include <filesystem>
#include <format>
#include <future>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

constexpr std::string_view v1 = "03h5mgnvq9zym7wvkijwwaobr";
constexpr std::string_view v2 = "03h5mgnvq9zym7wvkijwwaobk";

struct entry_t { int number; };
struct metadata_row_t { std::string version; };
std::atomic<int> callbacks{0};
std::promise<void>* entered_callback;
std::future<void>* rival_finished;

void initialize(database_t& database) {
    database.create_table<entry_t>("entries");
}

void upgrade(database_t& database) {
    ++callbacks;
    database.create_table<entry_t>("entries").insert(entry_t{42});
    entered_callback->set_value();
    assert(rival_finished->wait_for(std::chrono::seconds(20)) == std::future_status::ready);
    rival_finished->get();
}

void concurrent_upgrade(const std::filesystem::path& directory, bool newly_initialized) {
    database_t writer(directory);
    database_t rival(directory);
    if (!newly_initialized) {
        const std::array v1_history = {migration_t{baseline_version, v1, initialize}};
        migrate(writer, v1, v1_history);
    }
    const std::array history = {
        migration_t{baseline_version, v1, initialize},
        migration_t{v1, v2, upgrade}
    };
    const std::array first_history = {migration_t{baseline_version, v2, upgrade}};
    const auto migrations = newly_initialized ? std::span<const migration_t>(first_history) : std::span<const migration_t>(history);
    callbacks = 0;
    std::promise<void> entered;
    auto entered_future = entered.get_future();
    std::promise<void> finished;
    auto finished_future = finished.get_future();
    entered_callback = &entered;
    rival_finished = &finished_future;
    std::exception_ptr writer_error;
    std::thread worker([&] {
        try {
            migrate(writer, v2, migrations);
        } catch (...) {
            writer_error = std::current_exception();
            try { entered.set_value(); } catch (...) {}
        }
    });
    assert(entered_future.wait_for(std::chrono::seconds(20)) == std::future_status::ready);
    entered_future.get();
    bool conflict = false;
    try {
        migrate(rival, v2, migrations);
    } catch (const std::runtime_error&) {
        conflict = true;
    }
    finished.set_value();
    worker.join();
    if (writer_error) std::rethrow_exception(writer_error);
    assert(conflict && callbacks == 1);

    // After the winner commits, another explicit invocation is a no-op.
    migrate(rival, v2, migrations);
    assert(callbacks == 1);
    const auto entries = rival.create_table<entry_t>("entries").rows();
    assert(entries.size() == 1 && entries.front().number == 42);
    const auto records = rival.create_table<metadata_row_t>(metadata_table_name).rows();
    assert(records.size() == 1 && records.front().version == v2);
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    const auto root = std::filesystem::temp_directory_path() / std::format("concurrent-migrations-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::create_directories(root);
    for (const bool newly_initialized : {false, true}) {
        const auto directory = root / (newly_initialized ? "new" : "existing");
        concurrent_upgrade(directory, newly_initialized);
        database_t reopened_database(directory);
        const auto records = reopened_database.create_table<metadata_row_t>(metadata_table_name).rows();
        assert(records.size() == 1 && records.front().version == v2);
        const auto entries = reopened_database.create_table<entry_t>("entries").rows();
        assert(entries.size() == 1 && entries.front().number == 42);
    }
    std::filesystem::remove_all(root);
}
