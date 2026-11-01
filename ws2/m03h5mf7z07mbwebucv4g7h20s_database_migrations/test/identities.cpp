#include <m03h5mf7z07mbwebucv4g7h20s_database_migrations/api.h>
#include <m03gagbhtft23yhjwpp881tfmc_uuid/uuid.h>
#include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

struct metadata_row_t { std::string version; };

void no_transform(database_t&) {}

std::string encode(std::span<const std::byte> bytes) {
    auto identifier = m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t(bytes).to_radix(36);
    identifier.insert(0, 25 - identifier.size(), '0');
    return identifier;
}

void reject_identity(std::string_view identifier) {
    database_t database;
    const std::array history = {migration_t{baseline_version, identifier, no_transform}};
    bool caught = false;
    try {
        migrate(database, identifier, history);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught && database.tables().empty());
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations

int main() {
    using namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations;
    using m03gagbhtft23yhjwpp881tfmc_uuid::uuid;
    using m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t;

    // Independently check the fixed baseline with Builder's public UUID parser.
    const auto decoded = byte_stream_t::from_radix(baseline_version, 36);
    std::array<std::byte, 16> baseline_bytes{};
    assert(decoded.size() <= baseline_bytes.size());
    for (std::size_t index = 0; index < decoded.size(); ++index) {
        baseline_bytes[baseline_bytes.size() - decoded.size() + index] = decoded.bytes()[index];
    }
    assert(uuid(std::span<const std::byte>(baseline_bytes)).version() == 7);
    assert(encode(baseline_bytes) == baseline_version);

    for (int attempt = 0; attempt < 16; ++attempt) {
        // Use exactly create_module's UUIDv7 -> big-endian base36 -> zero padding.
        const auto generated_uuid = uuid::generate(7);
        const auto identifier = encode(generated_uuid.bytes());
        database_t database;
        const std::array history = {migration_t{baseline_version, identifier, no_transform}};
        migrate(database, identifier, history);
        migrate(database, identifier, history);
        const auto records = database.create_table<metadata_row_t>(metadata_table_name).rows();
        assert(records.size() == 1 && records.front().version == identifier);

        std::vector<std::byte> bytes(generated_uuid.bytes().begin(), generated_uuid.bytes().end());
        for (unsigned version = 0; version != 16; ++version) {
            if (version == 7) continue;
            bytes[6] = (generated_uuid.bytes()[6] & std::byte{0x0f}) | static_cast<std::byte>(version << 4);
            reject_identity(encode(bytes));
        }
        bytes[6] = generated_uuid.bytes()[6];
        for (const auto variant : {std::byte{0x00}, std::byte{0x40}, std::byte{0xc0}}) {
            bytes[8] = (generated_uuid.bytes()[8] & std::byte{0x3f}) | variant;
            reject_identity(encode(bytes));
        }
    }
}
