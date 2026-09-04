#ifndef M03GE9IJ49XKR5OBOFUJOJ7LTW_FUNCTION_RUNTIME_FUNCTION_H
# define M03GE9IJ49XKR5OBOFUJOJ7LTW_FUNCTION_RUNTIME_FUNCTION_H

# include <m03ge9ij43jyxy821pda20jhwh_typesystem/typesystem.h>
# include <m03ge9ij46lc986vpdamnc2fka_function_ir/function_ir.h>

# include <cstddef>
# include <cstdint>
# include <format>
# include <stdexcept>
# include <string>
# include <type_traits>
# include <vector>

namespace m03ge9ij49xkr5obofujoj7ltw_function_runtime {

class function_t {
public:
    struct argument_t {
        argument_t();
        ~argument_t();

        argument_t(const argument_t& other) = delete;
        argument_t& operator=(const argument_t& other) = delete;

        argument_t(argument_t&& other) noexcept;
        argument_t& operator=(argument_t&& other) noexcept;

        int data_type_id() const noexcept;
        size_t data_size() const noexcept;

    private:
        friend class function_t;

        void assign(const void* data, size_t size, size_t alignment);
        void reset() noexcept;

        function_t* m_connection;
        uint8_t m_connection_argument_index;
        std::string m_name;
        int m_data_type_id;
        void* m_data;
        size_t m_data_size;
        size_t m_data_alignment;
    };

    struct reader_t {
        function_t* self;
        uint8_t index;

        template <typename T>
        operator T() {
            return self->read<T>(index);
        }
    };

public:
    function_t(m03ge9ij43jyxy821pda20jhwh_typesystem::typesystem_t& typesystem, m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t function_ir, void (*function_call)(function_t&, uint8_t));

    virtual ~function_t();

    function_t(const function_t& other) = delete;
    function_t& operator=(const function_t& other) = delete;
    function_t(function_t&& other) = delete;
    function_t& operator=(function_t&& other) = delete;

    function_t* parent();
    void parent(function_t* parent);

    m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t& function_ir();
    void (*function_call() const)(function_t&, uint8_t);
    void function_call(void (*call)(function_t&, uint8_t));

    void argument_name(uint8_t argument_index, std::string name);
    const std::string& argument_name(uint8_t argument_index);

    /**
     * Connects on `self_argument_index` to `other` on `other_argument_index`.
     * If there was data on `self_argument_index`, it copies it to `other` and calls it.
    */
    void connect(function_t* other, uint8_t other_argument_index, uint8_t self_argument_index);

    /**
     * Returns true if `argument_index` is connected to another node.
    */
    bool is_connected(uint8_t argument_index);

    /**
     * Returns the connected node on `argument_index`, or `nullptr` if not connected.
    */
    function_t* connection(uint8_t argument_index);

    /**
     * Disconnects the connected node on `argument_index` if any.
    */
    void disconnect(uint8_t argument_index);

    /**
     * Expands into the defined (by `name`) combination of nodes.
     * `caller_argument_index` is the argument index on which this node was called.
    */
    void call(uint8_t caller_argument_index);

    /**
     * Returns coerced data of type `T` from the `index` argument.
     * T is deduced from the call site.
    */
    reader_t read(uint8_t index);

    /**
     * Writes data of type `T` to the `argument_index`.
     * If the argument is connected, it then copies the data to the connection and calls it.
     * Stored values must be trivially copyable.
    */
    template <typename T>
    requires std::is_trivially_copyable_v<std::remove_cv_t<T>>
    void write(uint8_t argument_index, T data);

    /**
     * Writes a live object of the registered type to `argument_index`.
     * Stored values must be trivially copyable.
     * A `data_type_id` of -1 is ignored without inspecting the other arguments.
     * Otherwise, fails if `data` is null or the registered type is not trivially copyable.
     */
    void write(uint8_t argument_index, const void* data, int data_type_id);

    /**
     * If connected, copies data on `argument_index` to the connected argument and calls it.
    */
    void send(uint8_t argument_index);

    /**
     * Clears data on `argument_index`.
     * If connected, it also clears the connected argument.
    */
    void clear(uint8_t argument_index);

    /**
     * Calls `write` operation on `to_argument_index` with the data from `from_argument_index`.
     * Implies that if `to_argument_index` is connected, it will also copies the data to the connection and call it.
    */
    void copy(uint8_t from_argument_index, uint8_t to_argument_index);

    std::vector<function_t*>& children();

    std::vector<argument_t>& arguments();

    void morph(m03ge9ij43jyxy821pda20jhwh_typesystem::typesystem_t& typesystem, m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t function_ir, void (*call)(function_t&, uint8_t));

    void expand();
    void shrink();

    int left();
    int right();
    int top();
    int bottom();

    void left(int left);
    void right(int right);
    void top(int top);
    void bottom(int bottom);

    void finalize_dimensions();

    float coordinate_system_width();
    float coordinate_system_height();

    /**
     * Converts `x` from this node's coordinate system to the child's coordinate system.
    */
    int to_child_x(int x);

    /**
     * Converts `y` from this node's coordinate system to the child's coordinate system.
    */
    int to_child_y(int y);

    /**
     * Converts `x` from the child's coordinate system to this node's coordinate system.
    */
    int from_child_x(int x);

    /**
     * Converts `y` from the child's coordinate system to this node's coordinate system.
    */
    int from_child_y(int y);

private:
    template <typename T>
    T read(uint8_t argument_index);

private:
    m03ge9ij43jyxy821pda20jhwh_typesystem::typesystem_t* m_typesystem;
    m03ge9ij46lc986vpdamnc2fka_function_ir::function_ir_t m_function_ir;
    void (*m_call)(function_t&, uint8_t);
    std::vector<function_t*> m_children;
    function_t* m_parent;

    bool m_is_expanded;

    int m_left;
    int m_right;
    int m_top;
    int m_bottom;

    float m_coordinate_system_width;
    float m_coordinate_system_height;

    bool m_is_dimensions_finalized;

    std::vector<argument_t> m_arguments;
};

template <typename T>
T function_t::read(uint8_t argument_index) {
    if (m_arguments.size() <= argument_index) {
        throw std::runtime_error(std::format("argument_index out of range: {}", argument_index));
    }
    argument_t& argument = m_arguments[argument_index];
    if (argument.m_data_type_id == -1) {
        throw std::runtime_error(std::format("argument {} has no data", argument_index));
    }
    if (argument.m_data_size != m_typesystem->sizeof_type(argument.m_data_type_id)) {
        throw std::runtime_error(std::format("argument {} has an invalid data size", argument_index));
    }
    return m_typesystem->coerce<T>(argument.m_data, argument.m_data_type_id);
}

template <typename T>
requires std::is_trivially_copyable_v<std::remove_cv_t<T>>
void function_t::write(uint8_t argument_index, T data) {
    using value_t = std::remove_cv_t<T>;
    m_typesystem->register_type<value_t>();

    write(argument_index, &data, m_typesystem->type_id<value_t>());
}

} // namespace m03ge9ij49xkr5obofujoj7ltw_function_runtime

#endif // M03GE9IJ49XKR5OBOFUJOJ7LTW_FUNCTION_RUNTIME_FUNCTION_H
