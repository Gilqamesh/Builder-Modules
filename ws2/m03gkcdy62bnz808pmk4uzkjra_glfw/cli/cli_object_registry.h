#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H

# include "cli_arguments.h"

# include <cstddef>
# include <format>
# include <functional>
# include <iostream>
# include <map>
# include <memory>
# include <optional>
# include <string>
# include <unordered_set>
# include <utility>
# include <vector>

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

template <typename T, typename Key>
class object_registry_t {
public:
    using object_type = T;
    using key_type = Key;
    using key_function_t = std::function<std::optional<Key>(const T&)>;

    struct entry_t {
        std::shared_ptr<T> object;
        bool connected = false;
        std::optional<Key> key;
        std::optional<std::size_t> enumeration_index;
    };

    object_registry_t(std::string label, key_function_t key_function):
        m_label(std::move(label)),
        m_key_function(std::move(key_function))
    {
    }

    void refresh(std::vector<std::shared_ptr<T>> objects, bool announce_changes) {
        std::map<id_t, bool> previous_connected;
        for (const auto& [id, entry] : m_entries) {
            previous_connected.emplace(id, entry.connected);
        }

        std::unordered_set<id_t> seen;
        for (std::size_t index = 0; index < objects.size(); ++index) {
            const std::shared_ptr<T>& object = objects[index];
            if (!object) {
                continue;
            }

            const std::optional<Key> key = m_key_function(*object);
            const std::optional<id_t> existing_id = find_id(object, key);
            const id_t id = existing_id ? *existing_id : m_next_id++;

            auto& entry = m_entries[id];
            entry.object = object;
            entry.connected = key.has_value();
            if (key) {
                entry.key = key;
            }
            entry.enumeration_index = index;
            seen.insert(id);
        }

        for (auto& [id, entry] : m_entries) {
            if (!seen.contains(id)) {
                entry.connected = false;
                entry.enumeration_index.reset();
            }
        }

        if (!announce_changes) {
            return;
        }

        for (const auto& [id, entry] : m_entries) {
            const auto previous = previous_connected.find(id);
            if (previous == previous_connected.end()) {
                std::cout << std::format(
                    "[{} {}] discovered: {}\n",
                    m_label,
                    id,
                    entry.connected ? "connected" : "retained snapshot"
                );
            } else if (previous->second != entry.connected) {
                std::cout << std::format(
                    "[{} {}] {}\n",
                    m_label,
                    id,
                    entry.connected ? "connected" : "disconnected"
                );
            }
        }
    }

    id_t observe(std::shared_ptr<T> object) {
        if (!object) {
            command_error(std::format("cannot observe a null {}", m_label));
        }

        const std::optional<Key> key = m_key_function(*object);
        if (const std::optional<id_t> existing_id = find_id(object, key)) {
            return *existing_id;
        }

        const id_t id = m_next_id++;
        m_entries.emplace(id, entry_t{
            .object = std::move(object),
            .connected = key.has_value(),
            .key = key,
            .enumeration_index = std::nullopt
        });
        return id;
    }

    entry_t& require(id_t id, bool require_connected) {
        const auto iterator = m_entries.find(id);
        if (iterator == m_entries.end() || !iterator->second.object) {
            command_error(std::format("no {} with ID {}", m_label, id));
        }
        if (require_connected && !iterator->second.connected) {
            command_error(std::format("{} {} is disconnected", m_label, id));
        }
        return iterator->second;
    }

    const entry_t& require(id_t id, bool require_connected) const {
        const auto iterator = m_entries.find(id);
        if (iterator == m_entries.end() || !iterator->second.object) {
            command_error(std::format("no {} with ID {}", m_label, id));
        }
        if (require_connected && !iterator->second.connected) {
            command_error(std::format("{} {} is disconnected", m_label, id));
        }
        return iterator->second;
    }

    std::optional<id_t> id_of(const std::shared_ptr<T>& object) const {
        if (!object) {
            return std::nullopt;
        }
        return find_id(object, m_key_function(*object));
    }

    const std::map<id_t, entry_t>& entries() const {
        return m_entries;
    }

    std::map<id_t, entry_t>& entries() {
        return m_entries;
    }

    std::vector<std::shared_ptr<T>> retained_objects() const {
        std::vector<std::shared_ptr<T>> result;
        std::unordered_set<const T*> inserted;

        for (const auto& [id, entry] : m_entries) {
            static_cast<void>(id);
            if (entry.object && inserted.insert(entry.object.get()).second) {
                result.push_back(entry.object);
            }
        }

        return result;
    }

private:
    std::optional<id_t> find_id(
        const std::shared_ptr<T>& object,
        const std::optional<Key>& key
    ) const {
        for (const auto& [id, entry] : m_entries) {
            if (entry.object.get() == object.get()) {
                return id;
            }
        }

        if (!key) {
            return std::nullopt;
        }

        for (const auto& [id, entry] : m_entries) {
            if (entry.key == key) {
                return id;
            }
        }

        return std::nullopt;
    }

private:
    std::string m_label;
    key_function_t m_key_function;
    std::map<id_t, entry_t> m_entries;
    id_t m_next_id = 1;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H
