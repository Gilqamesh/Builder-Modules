# include <m03ge9ij49xkr5obofujoj7ltw_function_runtime/function.h>

# include <cassert>
# include <cmath>
# include <cstring>
# include <limits>
# include <new>
# include <utility>

namespace m03ge9ij49xkr5obofujoj7ltw_function_runtime {

namespace {

int checked_coordinate(double value, const char* operation) {
    if (
        !std::isfinite(value) ||
        value < static_cast<double>(std::numeric_limits<int>::lowest()) ||
        static_cast<double>(std::numeric_limits<int>::max()) < value
    ) {
        throw std::out_of_range(std::format("{}: result is outside the int range", operation));
    }

    return static_cast<int>(value);
}

} // namespace

function_t::function_t(m03ge9ij43jyxy821pda20jhwh_typesystem::typesystem_t& typesystem, m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t function_ir, void (*call)(function_t&, uint8_t)):
    m_typesystem(&typesystem),
    m_function_ir(function_ir),
    m_call(call),
    m_parent(nullptr),
    m_is_expanded(false),
    m_left(0),
    m_right(0),
    m_top(0),
    m_bottom(0),
    m_coordinate_system_width(0.0f),
    m_coordinate_system_height(0.0f),
    m_is_dimensions_finalized(false)
{
    if (m_call == nullptr) {
        throw std::invalid_argument("function_t: function call must not be null");
    }
}

function_t::~function_t() {
    for (size_t i = 0; i < m_arguments.size(); ++i) {
        disconnect((uint8_t) i);
    }
}

function_t* function_t::parent() {
    return m_parent;
}

void function_t::parent(function_t* parent) {
    m_parent = parent;
}

m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t& function_t::function_ir() {
    return m_function_ir;
}

void (*function_t::function_call() const)(function_t&, uint8_t) {
    return m_call;
}

void function_t::function_call(void (*call)(function_t&, uint8_t)) {
    if (call == nullptr) {
        throw std::invalid_argument("function_t::function_call: function call must not be null");
    }

    m_call = call;
}

void function_t::argument_name(uint8_t argument_index, std::string name) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }

    m_arguments[argument_index].m_name = std::move(name);
}

const std::string& function_t::argument_name(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }

    return m_arguments[argument_index].m_name;
}

void function_t::connect(function_t* other, uint8_t other_argument_index, uint8_t self_argument_index) {
    if (other == nullptr) {
        throw std::invalid_argument("function_t::connect: other must not be null");
    }
    if (m_arguments.size() <= self_argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", self_argument_index));
    }
    if (other->m_arguments.size() <= other_argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", other_argument_index));
    }

    argument_t& self_argument = m_arguments[self_argument_index];
    self_argument.m_connection = other;
    self_argument.m_connection_argument_index = other_argument_index;

    send(self_argument_index);
}

bool function_t::is_connected(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }
    return m_arguments[argument_index].m_connection != nullptr;
}

function_t* function_t::connection(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }
    return m_arguments[argument_index].m_connection;
}

void function_t::disconnect(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }

    argument_t& self_argument = m_arguments[argument_index];
    self_argument.m_connection = nullptr;
    self_argument.m_connection_argument_index = -1;
}

void function_t::call(uint8_t caller_argument_index) {
    m_call(*this, caller_argument_index);
}

function_t::reader_t function_t::read(uint8_t index) {
    return reader_t {
        .self = this,
        .index = index
    };
}

void function_t::write(uint8_t argument_index, const void* data, int data_type_id) {
    if (data_type_id == -1) {
        return ;
    }

    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }
    if (data == nullptr) {
        throw std::invalid_argument("function_t::write: data must not be null");
    }
    if (!m_typesystem->is_trivially_copyable(data_type_id)) {
        throw std::invalid_argument("function_t::write: argument values must be trivially copyable");
    }
    argument_t& argument = m_arguments[argument_index];

    const auto data_size = m_typesystem->sizeof_type(data_type_id);
    argument.assign(data, data_size, m_typesystem->alignof_type(data_type_id));
    argument.m_data_type_id = data_type_id;

    send(argument_index);
}

void function_t::send(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }

    argument_t& argument = m_arguments[argument_index];
    if (argument.m_data_type_id != -1 && argument.m_connection) {
        const auto data_size = m_typesystem->sizeof_type(argument.m_data_type_id);
        if (argument.m_data_size != data_size) {
            throw std::runtime_error(std::format("argument {} has an invalid data size", argument_index));
        }
        if (argument.m_connection->m_arguments.size() <= argument.m_connection_argument_index) {
            throw std::runtime_error(std::format("connected argument index out of range: {}", argument.m_connection_argument_index));
        }
        argument_t& other_argument = argument.m_connection->m_arguments[argument.m_connection_argument_index];
        other_argument.assign(
            argument.m_data,
            data_size,
            m_typesystem->alignof_type(argument.m_data_type_id)
        );
        other_argument.m_data_type_id = argument.m_data_type_id;
        argument.m_connection->call(argument.m_connection_argument_index);
    }
}

void function_t::clear(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }

    argument_t& argument = m_arguments[argument_index];
    argument.m_data_type_id = -1;
    argument.reset();

    if (argument.m_connection) {
        if (argument.m_connection->m_arguments.size() <= argument.m_connection_argument_index) {
            throw std::runtime_error(std::format("connected argument index out of range: {}", argument.m_connection_argument_index));
        }
        argument_t& other_argument = argument.m_connection->m_arguments[argument.m_connection_argument_index];
        other_argument.m_data_type_id = -1;
        other_argument.reset();
    }
}

void function_t::copy(uint8_t from_argument_index, uint8_t to_argument_index) {
    if (m_arguments.size() <= from_argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", from_argument_index));
    }
    if (m_arguments.size() <= to_argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", to_argument_index));
    }

    argument_t& from_argument = m_arguments[from_argument_index];
    if (from_argument.m_data_type_id == -1) {
        return ;
    }

    write(to_argument_index, from_argument.m_data, from_argument.m_data_type_id);
}

int function_t::left() {
    return m_left;
}

int function_t::right() {
    return m_right;
}

int function_t::top() {
    return m_top;
}

int function_t::bottom() {
    return m_bottom;
}

std::vector<function_t*>& function_t::children() {
    return m_children;
}

std::vector<function_t::argument_t>& function_t::arguments() {
    return m_arguments;
}

void function_t::morph(m03ge9ij43jyxy821pda20jhwh_typesystem::typesystem_t& typesystem, m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t function_ir, void (*call)(function_t&, uint8_t)) {
    if (call == nullptr) {
        throw std::invalid_argument("function_t::morph: function call must not be null");
    }
    if (m_is_expanded) {
        if (m_function_ir.function_id != function_ir.function_id) {
            shrink();
        }
    }

    m_typesystem = &typesystem;
    m_function_ir = std::move(function_ir);
    m_call = call;

    expand();
}

void function_t::expand() {
    if (m_is_expanded) {
        return ;
    }
    if (!m_children.empty()) {
        throw std::logic_error("function_t::expand: unexpanded function has children");
    }

    for (const auto& child : m_function_ir.children) {
        function_t* node = m_typesystem->coerce(child.function_id);
        if (node == nullptr) {
            throw std::runtime_error("function_t::expand: child coercion returned null");
        }
        node->left(child.left);
        node->right(child.right);
        node->top(child.top);
        node->bottom(child.bottom);
        node->finalize_dimensions();
        m_children.emplace_back(node);
    }

    for (const auto& connection : m_function_ir.connections) {
        function_t* from_function = nullptr;
        if (connection.from_function_index == static_cast<uint16_t>(-1)) {
            from_function = this;
        } else if (connection.from_function_index < m_children.size()) {
            from_function = m_children[connection.from_function_index];
        } else {
            throw std::runtime_error(std::format("from function index out of range: {}", connection.from_function_index));
        }

        function_t* to_function = nullptr;
        if (connection.to_function_index == static_cast<uint16_t>(-1)) {
            to_function = this;
        } else if (connection.to_function_index < m_children.size()) {
            to_function = m_children[connection.to_function_index];
        } else {
            throw std::runtime_error(std::format("to function index out of range: {}", connection.to_function_index));
        }

        from_function->connect(to_function, connection.to_argument_index, connection.from_argument_index);
    }

    m_is_expanded = true;
}

void function_t::shrink() {
    if (!m_is_expanded) {
        return ;
    }

    throw std::runtime_error("shrink() is not implemented yet");

    m_is_expanded = false;
}

void function_t::left(int left) {
    m_left = left;
    m_is_dimensions_finalized = false;
}

void function_t::right(int right) {
    m_right = right;
    m_is_dimensions_finalized = false;
}

void function_t::top(int top) {
    m_top = top;
    m_is_dimensions_finalized = false;
}

void function_t::bottom(int bottom) {
    m_bottom = bottom;
    m_is_dimensions_finalized = false;
}

void function_t::finalize_dimensions() {
    const std::int64_t width = static_cast<std::int64_t>(m_right) - m_left;
    const std::int64_t height = static_cast<std::int64_t>(m_bottom) - m_top;

    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("function_t::finalize_dimensions: right and bottom must be greater than left and top");
    }

    if (width < height) {
        m_coordinate_system_width = static_cast<float>(
            static_cast<double>(width) / height * std::numeric_limits<int16_t>::max()
        );
        m_coordinate_system_height = std::numeric_limits<int16_t>::max();
    } else {
        m_coordinate_system_width = std::numeric_limits<int16_t>::max();
        m_coordinate_system_height = static_cast<float>(
            static_cast<double>(height) / width * std::numeric_limits<int16_t>::max()
        );
    }

    m_is_dimensions_finalized = true;
}

float function_t::coordinate_system_width() {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::coordinate_system_width: dimensions are not finalized");
    }
    return m_coordinate_system_width;
}

float function_t::coordinate_system_height() {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::coordinate_system_height: dimensions are not finalized");
    }
    return m_coordinate_system_height;
}

int function_t::to_child_x(int x) {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::to_child_x: dimensions are not finalized");
    }
    const double result = (
        static_cast<double>(static_cast<std::int64_t>(x) - m_left) /
        (static_cast<std::int64_t>(m_right) - m_left) * m_coordinate_system_width -
        m_coordinate_system_width / 2.0
    );
    return checked_coordinate(result, "function_t::to_child_x");
}

int function_t::to_child_y(int y) {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::to_child_y: dimensions are not finalized");
    }
    const double result = (
        static_cast<double>(static_cast<std::int64_t>(y) - m_top) /
        (static_cast<std::int64_t>(m_bottom) - m_top) * m_coordinate_system_height -
        m_coordinate_system_height / 2.0
    );
    return checked_coordinate(result, "function_t::to_child_y");
}

int function_t::from_child_x(int x) {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::from_child_x: dimensions are not finalized");
    }
    const double result = (
        (x + m_coordinate_system_width / 2.0) / m_coordinate_system_width *
        (static_cast<std::int64_t>(m_right) - m_left) + m_left
    );
    return checked_coordinate(result, "function_t::from_child_x");
}

int function_t::from_child_y(int y) {
    if (!m_is_dimensions_finalized) {
        throw std::logic_error("function_t::from_child_y: dimensions are not finalized");
    }
    const double result = (
        (y + m_coordinate_system_height / 2.0) / m_coordinate_system_height *
        (static_cast<std::int64_t>(m_bottom) - m_top) + m_top
    );
    return checked_coordinate(result, "function_t::from_child_y");
}

function_t::argument_t::argument_t():
    m_connection(nullptr),
    m_connection_argument_index(static_cast<uint8_t>(-1)),
    m_data_type_id(-1),
    m_data(nullptr),
    m_data_size(0),
    m_data_alignment(alignof(std::max_align_t))
{
}

function_t::argument_t::~argument_t() {
    reset();
}

function_t::argument_t::argument_t(argument_t&& other) noexcept:
    m_connection(other.m_connection),
    m_connection_argument_index(other.m_connection_argument_index),
    m_name(std::move(other.m_name)),
    m_data_type_id(other.m_data_type_id),
    m_data(std::exchange(other.m_data, nullptr)),
    m_data_size(std::exchange(other.m_data_size, 0)),
    m_data_alignment(std::exchange(other.m_data_alignment, alignof(std::max_align_t)))
{
    other.m_connection = nullptr;
    other.m_connection_argument_index = static_cast<uint8_t>(-1);
    other.m_data_type_id = -1;
}

function_t::argument_t& function_t::argument_t::operator=(argument_t&& other) noexcept {
    if (this != &other) {
        reset();
        m_connection = other.m_connection;
        m_connection_argument_index = other.m_connection_argument_index;
        m_name = std::move(other.m_name);
        m_data_type_id = other.m_data_type_id;
        m_data = std::exchange(other.m_data, nullptr);
        m_data_size = std::exchange(other.m_data_size, 0);
        m_data_alignment = std::exchange(other.m_data_alignment, alignof(std::max_align_t));
        other.m_connection = nullptr;
        other.m_connection_argument_index = static_cast<uint8_t>(-1);
        other.m_data_type_id = -1;
    }

    return *this;
}

int function_t::argument_t::data_type_id() const noexcept {
    return m_data_type_id;
}

size_t function_t::argument_t::data_size() const noexcept {
    return m_data_size;
}

void function_t::argument_t::assign(const void* data, size_t size, size_t alignment) {
    void* replacement = ::operator new(size, std::align_val_t(alignment));
    std::memcpy(replacement, data, size);

    reset();
    m_data = replacement;
    m_data_size = size;
    m_data_alignment = alignment;
}

void function_t::argument_t::reset() noexcept {
    if (m_data != nullptr) {
        ::operator delete(m_data, std::align_val_t(m_data_alignment));
        m_data = nullptr;
    }
    m_data_size = 0;
}

} // namespace m03ge9ij49xkr5obofujoj7ltw_function_runtime
