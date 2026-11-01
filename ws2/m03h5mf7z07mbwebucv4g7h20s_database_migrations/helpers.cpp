#include "helpers.h"

#include <array>
#include <cstddef>
#include <format>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace m03h5mf7z07mbwebucv4g7h20s_database_migrations {

bool valid_version(std::string_view version) {
    if (version.size() != 25) return false;

    // Decode only the fixed version representation. Keeping this format check
    // local leaves the migration library dependent on database_t and the STL.
    std::array<unsigned char, 16> bytes{};
    for (const char character : version) {
        unsigned carry;
        if ('0' <= character && character <= '9') {
            carry = static_cast<unsigned>(character - '0');
        } else if ('a' <= character && character <= 'z') {
            carry = static_cast<unsigned>(character - 'a' + 10);
        } else {
            return false;
        }
        for (std::size_t index = bytes.size(); 0 < index; --index) {
            const unsigned digit = bytes[index - 1] * 36u + carry;
            bytes[index - 1] = static_cast<unsigned char>(digit & 0xffu);
            carry = digit >> 8;
        }
        if (carry != 0) return false;
    }
    return (bytes[6] & 0xf0u) == 0x70u && (bytes[8] & 0xc0u) == 0x80u;
}

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
    std::unordered_map<std::string_view, const migration_t*> successors;
    std::unordered_set<std::string_view> destinations;
    for (const auto& migration : migrations) {
        if (!valid_version(migration.from) || !valid_version(migration.to)) {
            throw std::invalid_argument(std::format("migrate: transition {} requires 25-character lowercase base36 UUIDv7 identities", migration));
        }
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
