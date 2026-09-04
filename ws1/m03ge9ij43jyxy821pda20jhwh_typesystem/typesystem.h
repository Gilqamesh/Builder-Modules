#ifndef M03GE9IJ43JYXY821PDA20JHWH_TYPESYSTEM_TYPESYSTEM_H
# define M03GE9IJ43JYXY821PDA20JHWH_TYPESYSTEM_TYPESYSTEM_H

# include <cmath>
# include <concepts>
# include <cstddef>
# include <memory>
# include <stdexcept>
# include <type_traits>
# include <unordered_map>
# include <utility>
# include <vector>

namespace m03ge9ij43jyxy821pda20jhwh_typesystem {

class typesystem_t {
public:
    struct reader_t {
        typesystem_t* self;
        const void* from;
        int type_id_from;

        template <typename T>
        operator T() const {
            return self->coerce<T>(from, type_id_from);
        }
    };

    struct coercion_t {
        void (*caller)(const void*, void*, const void*) = nullptr;
        const void* context = nullptr;
        std::shared_ptr<const void> context_owner;

        constexpr operator bool() const {
            return caller != nullptr;
        }

        void operator()(const void* from, void* to) const {
            caller(from, to, context);
        }
    };

public:
    /**
     * Registers the unqualified form of a default-initializable, copy-assignable value type.
     */
    template <typename T>
    void register_type();

    template <typename From, typename To>
    void register_coercion(To (*coercion_procedure)(From));

    template <typename From>
    reader_t coerce(const From& from);

    template <typename To>
    To coerce(const void* from, int id_from);

    /**
     * Assigns a coerced value between live objects matching `id_from` and `id_to`.
     * The source is borrowed for the call and the destination must already be constructed.
     * Fails for null objects, invalid type identifiers, or a missing coercion path.
     */
    void coerce(const void* from, int id_from, void* to, int id_to);

    template <typename T>
    const void* type_addr();

    template <typename T>
    int type_id();

    int type_id(const void* addr);

    void update_coercion_graph();
    void update_coercion_graph(int id_from, int id_to);

    template <typename T>
    size_t sizeof_type();

    size_t sizeof_type(int type_id);
    size_t alignof_type(int type_id);
    bool is_trivially_copyable(int type_id);

private:
    template <typename T>
    static const void* type_addr_impl();

    template <typename From, typename To>
    static void coercion_bridge(const void* from_raw, void* to_raw, const void* context);

    struct type_operations_t {
        size_t size;
        size_t alignment;
        bool trivially_copyable;
        void (*default_construct)(void*);
        void (*destroy)(void*);
        void (*copy_assign)(const void*, void*);
    };

private:
    std::unordered_map<const void*, int> m_addr_to_typeid;

    std::vector<std::vector<int>> m_type_parents;
    std::vector<std::vector<double>> m_type_distance;
    std::vector<std::vector<size_t>> m_type_calls;
    std::vector<std::vector<coercion_t>> m_coercions;
    std::vector<type_operations_t> m_type_operations;
};

template <typename T>
void typesystem_t::register_type() {
    using value_t = std::remove_cv_t<T>;
    static_assert(std::default_initializable<value_t>);
    static_assert(std::is_copy_assignable_v<value_t>);

    int old_size = (int) m_addr_to_typeid.size();
    const void* addr = type_addr<value_t>();
    if (!m_addr_to_typeid.emplace(addr, old_size).second) {
        return ;
    }

    for (int i = 0; i < old_size; ++i) {
        m_type_parents[i].emplace_back(-1);
        m_type_distance[i].emplace_back(INFINITY);
        m_type_calls[i].emplace_back(0);
        m_coercions[i].emplace_back(coercion_t{});
    }
    m_type_parents.emplace_back(std::vector<int>(old_size + 1, -1));
    m_type_distance.emplace_back(std::vector<double>(old_size + 1, INFINITY));
    m_type_calls.emplace_back(std::vector<size_t>(old_size + 1, 0));
    m_coercions.emplace_back(std::vector<coercion_t>(old_size + 1, coercion_t{}));

    m_type_parents[old_size][old_size] = old_size;
    m_type_distance[old_size][old_size] = 0.0;

    m_type_operations.emplace_back(type_operations_t {
        .size = sizeof(value_t),
        .alignment = alignof(value_t),
        .trivially_copyable = std::is_trivially_copyable_v<value_t>,
        .default_construct = [](void* storage) {
            std::construct_at(static_cast<value_t*>(storage));
        },
        .destroy = [](void* storage) {
            std::destroy_at(static_cast<value_t*>(storage));
        },
        .copy_assign = [](const void* from, void* to) {
            *static_cast<value_t*>(to) = *static_cast<const value_t*>(from);
        }
    });
}

template <typename From, typename To>
void typesystem_t::register_coercion(To (*coercion_procedure)(From)) {
    using procedure_t = To (*)(From);
    static_assert(std::is_copy_assignable_v<std::remove_cv_t<To>>);

    if (coercion_procedure == nullptr) {
        throw std::invalid_argument("coercion procedure must not be null");
    }

    int id_from = type_id<From>();
    int id_to = type_id<To>();
    if (m_coercions[id_from][id_to]) {
        throw std::runtime_error("coercion is already registered between types");
    }
    m_type_parents[id_from][id_to] = id_from;
    m_type_distance[id_from][id_to] = 0.0;
    m_type_calls[id_from][id_to] = 0;
    auto context_owner = std::make_shared<procedure_t>(coercion_procedure);
    m_coercions[id_from][id_to] = coercion_t {
        .caller = &coercion_bridge<From, To>,
        .context = context_owner.get(),
        .context_owner = std::move(context_owner)
    };
    update_coercion_graph(id_from, id_to);
}

template <typename From>
typesystem_t::reader_t typesystem_t::coerce(const From& from) {
    return reader_t {
        .self = this,
        .from = &from,
        .type_id_from = type_id<From>()
    };
}

template <typename To>
To typesystem_t::coerce(const void* from, int id_from) {
    static_assert(std::default_initializable<To>);

    int id_to = type_id<To>();
    To result{};
    coerce(from, id_from, &result, id_to);
    return result;
}

template <typename T>
const void* typesystem_t::type_addr() {
    using value_t = std::remove_cv_t<T>;
    return type_addr_impl<value_t>();
}

template <typename T>
const void* typesystem_t::type_addr_impl() {
    static int addr;
    return &addr;
}

template <typename T>
int typesystem_t::type_id() {
    return type_id(type_addr<std::remove_cv_t<T>>());
}

template <typename T>
size_t typesystem_t::sizeof_type() {
    return sizeof(std::remove_cv_t<T>);
}

template <typename From, typename To>
void typesystem_t::coercion_bridge(const void* from_raw, void* to_raw, const void* context) {
    using procedure_t = To (*)(From);
    const auto procedure = *static_cast<const procedure_t*>(context);
    *static_cast<To*>(to_raw) = procedure(*static_cast<const From*>(from_raw));
}

} // namespace m03ge9ij43jyxy821pda20jhwh_typesystem

#endif // M03GE9IJ43JYXY821PDA20JHWH_TYPESYSTEM_TYPESYSTEM_H
