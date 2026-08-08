#ifndef M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H
# define M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H

# include "cli_support.h"

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
    using key_function_t = std::function<std::optional<Key>(const T&)>;

    struct entry_t {
        entry_t();
        entry_t(std::shared_ptr<T> object, bool connected, std::optional<Key> key, std::optional<std::size_t> enumeration_index);

        std::shared_ptr<T> object;
        bool connected;
        std::optional<Key> key;
        std::optional<std::size_t> enumeration_index;
    };

public:
    object_registry_t(std::string label, key_function_t key_function);

    void refresh(std::vector<std::shared_ptr<T>> objects, bool announce_changes);
    id_t observe(std::shared_ptr<T> object);

    entry_t& require(id_t id, bool require_connected);
    const entry_t& require(id_t id, bool require_connected) const;

    std::optional<id_t> id_of(const std::shared_ptr<T>& object) const;

    const std::map<id_t, entry_t>& entries() const;
    std::map<id_t, entry_t>& entries();

    std::vector<std::shared_ptr<T>> retained_objects() const;

private:
    std::optional<id_t> find_id(const std::shared_ptr<T>& object, const std::optional<Key>& key) const;

private:
    std::string m_label;
    key_function_t m_key_function;
    std::map<id_t, entry_t> m_entries;
    id_t m_next_id;
};

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli {

template <typename T, typename Key>
object_registry_t<T, Key>::entry_t::entry_t():
    object(),
    connected(false),
    key(),
    enumeration_index()
{
}

template <typename T, typename Key>
object_registry_t<T, Key>::entry_t::entry_t(std::shared_ptr<T> object, bool connected, std::optional<Key> key, std::optional<std::size_t> enumeration_index):
    object(std::move(object)),
    connected(connected),
    key(std::move(key)),
    enumeration_index(std::move(enumeration_index))
{
}

template <typename T, typename Key>
object_registry_t<T, Key>::object_registry_t(std::string label, key_function_t key_function):
    m_label(std::move(label)),
    m_key_function(std::move(key_function)),
    m_entries(),
    m_next_id(1)
{
}

template <typename T, typename Key>
void object_registry_t<T, Key>::refresh(std::vector<std::shared_ptr<T>> objects, bool announce_changes) {
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
            std::cout << std::format("[{} {}] discovered: {}\n", m_label, id, entry.connected ? "connected" : "retained snapshot");
        } else if (previous->second != entry.connected) {
            std::cout << std::format("[{} {}] {}\n", m_label, id, entry.connected ? "connected" : "disconnected");
        }
    }
}

template <typename T, typename Key>
id_t object_registry_t<T, Key>::observe(std::shared_ptr<T> object) {
    if (!object) {
        command_error(std::format("cannot observe a null {}", m_label));
    }

    const std::optional<Key> key = m_key_function(*object);
    if (const std::optional<id_t> existing_id = find_id(object, key)) {
        return *existing_id;
    }

    const id_t id = m_next_id++;
    m_entries.emplace(id, entry_t(std::move(object), key.has_value(), std::move(key), std::nullopt));
    return id;
}

template <typename T, typename Key>
typename object_registry_t<T, Key>::entry_t& object_registry_t<T, Key>::require(id_t id, bool require_connected) {
    const auto iterator = m_entries.find(id);
    if (iterator == m_entries.end() || !iterator->second.object) {
        command_error(std::format("no {} with ID {}", m_label, id));
    }
    if (require_connected && !iterator->second.connected) {
        command_error(std::format("{} {} is disconnected", m_label, id));
    }
    return iterator->second;
}

template <typename T, typename Key>
const typename object_registry_t<T, Key>::entry_t& object_registry_t<T, Key>::require(id_t id, bool require_connected) const {
    const auto iterator = m_entries.find(id);
    if (iterator == m_entries.end() || !iterator->second.object) {
        command_error(std::format("no {} with ID {}", m_label, id));
    }
    if (require_connected && !iterator->second.connected) {
        command_error(std::format("{} {} is disconnected", m_label, id));
    }
    return iterator->second;
}

template <typename T, typename Key>
std::optional<id_t> object_registry_t<T, Key>::id_of(const std::shared_ptr<T>& object) const {
    if (!object) {
        return std::nullopt;
    }
    return find_id(object, m_key_function(*object));
}

template <typename T, typename Key>
const std::map<id_t, typename object_registry_t<T, Key>::entry_t>& object_registry_t<T, Key>::entries() const {
    return m_entries;
}

template <typename T, typename Key>
std::map<id_t, typename object_registry_t<T, Key>::entry_t>& object_registry_t<T, Key>::entries() {
    return m_entries;
}

template <typename T, typename Key>
std::vector<std::shared_ptr<T>> object_registry_t<T, Key>::retained_objects() const {
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

template <typename T, typename Key>
std::optional<id_t> object_registry_t<T, Key>::find_id(const std::shared_ptr<T>& object, const std::optional<Key>& key) const {
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

} // namespace m03gkcdy62bnz808pmk4uzkjra_glfw_cli

#endif // M03GKCDY62BNZ808PMK4UZKJRA_GLFW_CLI_OBJECT_REGISTRY_H
