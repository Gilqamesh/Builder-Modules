#include "helpers.h"

#include <cstddef>
#include <format>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

bool metadata_name(std::string_view name) {
    if (name.size() != metadata_table_name.size()) return false;
    for (std::size_t index = 0; index < name.size(); ++index) {
        char character = name[index];
        if ('A' <= character && character <= 'Z') character += 'a' - 'A';
        if (character != metadata_table_name[index]) return false;
    }
    return true;
}

std::vector<const migration_t*> ordered_migrations(std::span<const migration_t> migrations) {
    auto hash_unique_id = [](const unique_id_t& unique_id) {
        std::size_t hash = 0;
        const auto uuid = unique_id.uuid();
        for (const auto byte : uuid.bytes()) {
            hash = hash * 31 + std::to_integer<std::size_t>(byte);
        }
        return hash;
    };
    std::unordered_map<unique_id_t, const migration_t*, decltype(hash_unique_id)> successors;
    std::unordered_set<unique_id_t, decltype(hash_unique_id)> destinations;
    for (const auto& migration : migrations) {
        if (migration.apply == nullptr) {
            throw std::invalid_argument(std::format("migrate: transition {} has no callback", migration));
        }
        if (migration.from == migration.to || migration.to == baseline_version) {
            throw std::invalid_argument(std::format("migrate: transition {} cycles to itself or the baseline", migration));
        }
        if (!successors.emplace(migration.from, &migration).second) {
            throw std::invalid_argument(std::format("migrate: version '{}' has competing successors or duplicate transitions", migration.from));
        }
        if (!destinations.insert(migration.to).second) {
            throw std::invalid_argument(std::format("migrate: version '{}' has duplicate identities or competing predecessors", migration.to));
        }
    }

    std::vector<const migration_t*> ordered;
    ordered.reserve(migrations.size());
    auto next = successors.find(baseline_version);
    while (next != successors.end()) {
        ordered.push_back(next->second);
        next = successors.find(next->second->to);
    }
    // Unique destinations and no edge into baseline prevent revisiting a node
    // in this walk. Any remaining edge is disconnected, cyclic, or missing a link.
    if (ordered.size() != migrations.size()) {
        throw std::invalid_argument("migrate: history is disconnected, cyclic, or missing a transition from baseline");
    }
    return ordered;
}

} // namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations
