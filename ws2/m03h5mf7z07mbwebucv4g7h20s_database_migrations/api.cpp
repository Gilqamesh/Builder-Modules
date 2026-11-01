#include "api.h"
#include "helpers.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

struct version_record_t {
    std::string version;
};

void migrate(database_t& database, std::string_view target, std::span<const migration_t> migrations) {
    const auto ordered = ordered_migrations(migrations);
    if (!valid_version(target)) {
        throw std::invalid_argument(std::format("migrate: target '{}' is not a 25-character lowercase base36 UUIDv7 identity", target));
    }
    const auto position = [&](std::string_view version) -> std::optional<std::size_t> {
        if (version == baseline_version) return 0;
        for (std::size_t index = 0; index < ordered.size(); ++index) {
            if (ordered[index]->to == version) return index + 1;
        }
        return std::nullopt;
    };
    const auto target_position = position(target);
    if (!target_position) {
        throw std::invalid_argument(std::format("migrate: target '{}' is unreachable from baseline '{}'", target, baseline_version));
    }

    database.transaction([&](database_t& database) {
        const auto names = database.tables();
        const bool has_metadata = std::ranges::any_of(names, metadata_name);
        if (!has_metadata && !names.empty()) {
            throw std::runtime_error("migrate: nonempty unversioned database requires explicit adoption/import");
        }

        const auto metadata = database.create_table<version_record_t>(metadata_table_name);
        std::string version(baseline_version);
        if (has_metadata) {
            const auto records = metadata.rows();
            if (records.size() != 1) {
                throw std::runtime_error(std::format("migrate: metadata requires exactly one version record; found {}", records.size()));
            }
            version = records.front().version;
            if (!valid_version(version)) {
                throw std::runtime_error(std::format("migrate: malformed persisted version '{}'", version));
            }
        }
        const auto current_position = position(version);
        if (!current_position) {
            throw std::runtime_error(std::format("migrate: persisted version '{}' is unknown to this application", version));
        }
        if (*target_position < *current_position) {
            throw std::runtime_error(std::format("migrate: downgrade from '{}' to '{}' is unsupported", version, target));
        }
        if (has_metadata && *current_position == *target_position) return;

        if (!has_metadata) {
            metadata.insert(version_record_t{version});
        } else {
            // Establish a transactional write before any user callback. A stale
            // snapshot or competing writer must conflict here, not be retried.
            if (metadata.replace<&version_record_t::version>(version, version_record_t{version}) != 1) {
                throw std::runtime_error("migrate: version record changed before upgrade");
            }
        }
        for (std::size_t index = *current_position; index < *target_position; ++index) {
            ordered[index]->apply(database);
            const auto records = metadata.rows();
            if (records.size() != 1 || records.front().version != version) {
                throw std::runtime_error("migrate: callback modified reserved version metadata");
            }
        }
        if (metadata.replace<&version_record_t::version>(version, version_record_t{std::string(target)}) != 1) {
            throw std::runtime_error("migrate: final version record is inconsistent");
        }
    });
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations
