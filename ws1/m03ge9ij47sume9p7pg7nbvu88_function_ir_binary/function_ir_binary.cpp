# include <m03ge9ij47sume9p7pg7nbvu88_function_ir_binary/function_ir_binary.h>

# include <chrono>
# include <cstdint>
# include <format>
# include <limits>
# include <stdexcept>
# include <utility>

namespace m03ge9ij47sume9p7pg7nbvu88_function_ir_binary {

enum class opcode_t : uint8_t {
    CREATE_FUNCTION,
    CONNECT_ARGUMENTS
};

static uint8_t serialize_function_index(uint16_t index) {
    if (index == std::numeric_limits<uint16_t>::max()) {
        return std::numeric_limits<uint8_t>::max();
    }
    if (std::numeric_limits<uint8_t>::max() <= index) {
        throw std::invalid_argument(std::format("function index {} is not representable in the binary format", index));
    }
    return static_cast<uint8_t>(index);
}

static uint16_t deserialize_function_index(uint8_t index) {
    if (index == std::numeric_limits<uint8_t>::max()) {
        return std::numeric_limits<uint16_t>::max();
    }
    return index;
}

static void serialize_int16(std::vector<uint8_t>& bytes, int value) {
    const auto serialized = static_cast<uint16_t>(static_cast<int16_t>(value));
    bytes.emplace_back(static_cast<uint8_t>(serialized >> 8));
    bytes.emplace_back(static_cast<uint8_t>(serialized & 0xFF));
}

static int deserialize_int16(uint8_t high, uint8_t low) {
    const auto serialized = static_cast<uint16_t>(static_cast<uint16_t>(high) << 8)
        | static_cast<uint16_t>(low);
    return static_cast<int16_t>(serialized);
}

static std::chrono::system_clock::time_point deserialize_creation_time(uint64_t serialized_creation_time) {
    return std::chrono::system_clock::time_point(std::chrono::seconds(serialized_creation_time));
}

static uint64_t serialize_creation_time(const std::chrono::system_clock::time_point& creation_time) {
    return std::chrono::duration_cast<std::chrono::seconds>(creation_time.time_since_epoch()).count();
}

function_ir_binary_t::function_ir_binary_t(std::vector<uint8_t> bytes):
    m_bytes(std::move(bytes))
{
}

function_ir_binary_t::function_ir_binary_t(const m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t& ir) {
    serialize_function_id(ir.function_id);

    for (const auto& child : ir.children) {
        m_bytes.emplace_back(static_cast<uint8_t>(opcode_t::CREATE_FUNCTION));

        serialize_function_id(child.function_id);

        if (!std::in_range<int16_t>(child.left)
            || !std::in_range<int16_t>(child.right)
            || !std::in_range<int16_t>(child.top)
            || !std::in_range<int16_t>(child.bottom)) {
            throw std::invalid_argument("child coordinates are not representable in the binary format");
        }

        serialize_int16(m_bytes, child.left);
        serialize_int16(m_bytes, child.right);
        serialize_int16(m_bytes, child.top);
        serialize_int16(m_bytes, child.bottom);
    }

    for (const auto& connection : ir.connections) {
        m_bytes.emplace_back(static_cast<uint8_t>(opcode_t::CONNECT_ARGUMENTS));

        m_bytes.emplace_back(serialize_function_index(connection.from_function_index));
        m_bytes.emplace_back(connection.from_argument_index);
        m_bytes.emplace_back(serialize_function_index(connection.to_function_index));
        m_bytes.emplace_back(connection.to_argument_index);
    }
}

const std::vector<uint8_t>& function_ir_binary_t::bytes() const {
    return m_bytes;
}

m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t function_ir_binary_t::function_ir() const {
    m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t result {};

    size_t offset = 0;
    result.function_id = deserialize_function_id(offset);
    while (offset < m_bytes.size()) {
        opcode_t op = static_cast<opcode_t>(m_bytes[offset++]);
        switch (op) {
            case opcode_t::CREATE_FUNCTION: {
                if (m_bytes.size() - offset < 8) {
                    throw std::runtime_error("failed to deserialize function_ir: unexpected end of data while reading CREATE_FUNCTION");
                }

                m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t::child_t child;

                child.function_id = deserialize_function_id(offset);
                if (m_bytes.size() - offset < 8) {
                    throw std::runtime_error("failed to deserialize function_ir: unexpected end of data while reading CREATE_FUNCTION coordinates");
                }
                child.left = deserialize_int16(m_bytes[offset], m_bytes[offset + 1]);
                offset += 2;
                child.right = deserialize_int16(m_bytes[offset], m_bytes[offset + 1]);
                offset += 2;
                child.top = deserialize_int16(m_bytes[offset], m_bytes[offset + 1]);
                offset += 2;
                child.bottom = deserialize_int16(m_bytes[offset], m_bytes[offset + 1]);
                offset += 2;

                result.children.emplace_back(std::move(child));
            } break;
            case opcode_t::CONNECT_ARGUMENTS: {
                if (m_bytes.size() - offset < 4) {
                    throw std::runtime_error("failed to deserialize function_ir: unexpected end of data while reading CONNECT_ARGUMENTS");
                }

                m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t::connection_info_t connection;

                connection.from_function_index = deserialize_function_index(m_bytes[offset]);
                ++offset;
                connection.from_argument_index = m_bytes[offset];
                ++offset;
                connection.to_function_index = deserialize_function_index(m_bytes[offset]);
                ++offset;
                connection.to_argument_index = m_bytes[offset];
                ++offset;

                result.connections.emplace_back(std::move(connection));
            } break;
            default: {
                throw std::runtime_error(std::format("failed to deserialize function_ir: unknown opcode {}", static_cast<uint8_t>(op)));
            } break ;
        }
    }

    return result;
}

void function_ir_binary_t::serialize_function_id(const m03ge9ij45dcznrmna12qow5r5_function_id::function_id_t& function_id) {
    if (function_id.ns.find('\0') != std::string::npos || function_id.name.find('\0') != std::string::npos) {
        throw std::invalid_argument("function id namespace and name must not contain null characters");
    }

    for (uint8_t c : function_id.ns) {
        m_bytes.emplace_back(c);
    }
    m_bytes.emplace_back(0);

    for (uint8_t c : function_id.name) {
        m_bytes.emplace_back(c);
    }
    m_bytes.emplace_back(0);

    uint64_t creation_time_serialized = serialize_creation_time(function_id.creation_time);
    for (size_t i = 0; i < 8; ++i) {
        m_bytes.emplace_back((uint8_t) (creation_time_serialized >> (56 - i * 8)));
    }
}

m03ge9ij45dcznrmna12qow5r5_function_id::function_id_t function_ir_binary_t::deserialize_function_id(size_t& offset) const {
    m03ge9ij45dcznrmna12qow5r5_function_id::function_id_t result;

    while (offset < m_bytes.size() && m_bytes[offset] != 0) {
        result.ns.push_back((char) m_bytes[offset]);
        ++offset;
    }

    if (offset == m_bytes.size() || m_bytes[offset] != 0) {
        throw std::runtime_error("failed to deserialize function_id: unexpected end of data while reading namespace");
    }
    ++offset;

    while (offset < m_bytes.size() && m_bytes[offset] != 0) {
        result.name.push_back((char) m_bytes[offset]);
        ++offset;
    }

    if (offset == m_bytes.size() || m_bytes[offset] != 0) {
        throw std::runtime_error("failed to deserialize function_id: unexpected end of data while reading name");
    }
    ++offset;

    if (m_bytes.size() - offset < 8) {
        throw std::runtime_error("failed to deserialize function_id: unexpected end of data while reading creation time");
    }
    uint64_t creation_time_serialized = 0;
    for (size_t i = 0; i < 8; ++i) {
        creation_time_serialized |= ((uint64_t) m_bytes[offset + i]) << (56 - i * 8);
    }
    result.creation_time = deserialize_creation_time(creation_time_serialized);
    offset += 8;

    return result;
}

} // namespace m03ge9ij47sume9p7pg7nbvu88_function_ir_binary
