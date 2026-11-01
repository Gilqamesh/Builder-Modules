#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>

#include <array>
#include <cassert>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

struct metadata_row_t { std::string version; };

void no_transform(database_t&) {}

const unique_id_t reference_id{"000000006g2dvmnfh66jop340"};
const std::array reference_history = {migration_t{baseline_version, reference_id, no_transform}};

void reject_stored_version(std::string_view encoded_version) {
    bool caught = false;
    try {
        [[maybe_unused]] const unique_id_t invalid_id(encoded_version);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);

    database_t database;
    const auto metadata = database.create_table<metadata_row_t>(metadata_table_name);
    metadata.insert(metadata_row_t{std::string(encoded_version)});
    const auto names = database.tables();
    caught = false;
    try {
        migrate(database, baseline_version, {});
    } catch (const std::runtime_error& error) {
        caught = std::string_view(error.what()).contains("malformed persisted version");
    }
    assert(caught && database.tables() == names);
    const auto records = metadata.rows();
    assert(records.size() == 1 && records.front().version == encoded_version);
}

void existing_metadata(const std::filesystem::path& directory) {
    {
        database_t database(directory);
        const auto metadata = database.create_table<metadata_row_t>(metadata_table_name);
        // Reproduce metadata written by the former string-based API.
        metadata.insert(metadata_row_t{"03h5mgnvq9zym7wvkijwwaobj"});
        migrate(database, baseline_version, {});
        assert(metadata.rows().front().version == "03h5mgnvq9zym7wvkijwwaobj");
        migrate(database, reference_id, reference_history);
        const auto records = metadata.rows();
        assert(records.size() == 1 && records.front().version == "000000006g2dvmnfh66jop340");
    }
    {
        database_t reopened_database(directory);
        migrate(reopened_database, reference_id, reference_history);
        const auto records = reopened_database.create_table<metadata_row_t>(metadata_table_name).rows();
        assert(records.size() == 1 && records.front().version == "000000006g2dvmnfh66jop340");
    }
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;

    assert(baseline_version.uuid().version() == 7);
    assert(baseline_version.string() == "03h5mgnvq9zym7wvkijwwaobj");
    assert(reference_history.front().from == baseline_version);
    assert(std::format("{}", reference_id.uuid()) == "00000000-002a-7000-8000-000000000000");
    assert(std::format("{}", reference_history.front()) == "03h5mgnvq9zym7wvkijwwaobj -> 000000006g2dvmnfh66jop340");
    {
        std::string source_text = "000000006g2dvmnfh66jop340";
        const std::array history = {migration_t{baseline_version, unique_id_t(source_text), no_transform}};
        source_text.assign("replaced source text");
        database_t database;
        migrate(database, unique_id_t(std::string("000000006g2dvmnfh66jop340")), history);
        const auto records = database.create_table<metadata_row_t>(metadata_table_name).rows();
        assert(records.size() == 1 && records.front().version == "000000006g2dvmnfh66jop340");
    }

    for (int attempt = 0; attempt < 16; ++attempt) {
        const auto unique_id = unique_id_t::generate();
        database_t database;
        const std::array history = {migration_t{baseline_version, unique_id, no_transform}};
        migrate(database, unique_id, history);
        migrate(database, unique_id_t(unique_id.string()), history);
        const auto records = database.create_table<metadata_row_t>(metadata_table_name).rows();
        assert(records.size() == 1 && records.front().version == unique_id.string());
    }

    for (const auto encoded_version : {
        "", "00000006g2dvmnfh66jop340", "0000000006g2dvmnfh66jop340",
        "000000006g2Dvmnfh66jop340", "000000006g2dvmnfh66jop34!",
        "f5lxx1zz5pnorynqglhzmsp34", "0000000000000000000000000",
        "000000006f1h1uz039d4g622o", "000000006g2dtokqgnngmaups"
    }) {
        reject_stored_version(encoded_version);
    }
    auto embedded_nul = reference_id.string();
    embedded_nul[10] = '\0';
    reject_stored_version(embedded_nul);

    const auto root = std::filesystem::temp_directory_path() / ("typed-migration-identities-" + unique_id_t::generate().string());
    std::filesystem::create_directories(root);
    try {
        existing_metadata(root);
    } catch (...) {
        std::filesystem::remove_all(root);
        throw;
    }
    std::filesystem::remove_all(root);
}
