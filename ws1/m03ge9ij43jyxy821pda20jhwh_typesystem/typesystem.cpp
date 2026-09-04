# include <m03ge9ij43jyxy821pda20jhwh_typesystem/typesystem.h>

# include <format>
# include <memory>
# include <new>

# include <x86intrin.h>

namespace m03ge9ij43jyxy821pda20jhwh_typesystem {

void typesystem_t::coerce(const void* from, int id_from, void* to, int id_to) {
    static_cast<void>(sizeof_type(id_from));
    static_cast<void>(sizeof_type(id_to));
    if (from == nullptr || to == nullptr) {
        throw std::invalid_argument("coercion source and destination must not be null");
    }

    if (id_from == id_to) {
        m_type_operations[id_to].copy_assign(from, to);
        return ;
    }

    int id_subresult = m_type_parents[id_from][id_to];
    if (id_subresult == -1) {
        throw std::runtime_error(std::format("no coercion procedure found between types ({}, {})", id_from, id_to));
    }

    const type_operations_t operations = m_type_operations[id_subresult];
    void* subresult = ::operator new(operations.size, std::align_val_t(operations.alignment));
    try {
        operations.default_construct(subresult);
    } catch (...) {
        ::operator delete(subresult, std::align_val_t(operations.alignment));
        throw ;
    }
    const auto subresult_deleter = [operations](void* value) {
        operations.destroy(value);
        ::operator delete(value, std::align_val_t(operations.alignment));
    };
    std::unique_ptr<void, decltype(subresult_deleter)> owned_subresult(
        subresult,
        subresult_deleter
    );
    coerce(from, id_from, subresult, id_subresult);

    auto coercion_procedure = m_coercions[id_subresult][id_to];
    if (!coercion_procedure) {
        throw std::runtime_error(std::format("no coercion procedure found between types ({}, {})", id_subresult, id_to));
    }

    size_t t_start = __rdtsc();
    coercion_procedure(subresult, to);
    size_t t_end = __rdtsc();

    double& average_cost_kcy = m_type_distance[id_subresult][id_to];
    size_t& n_calls = m_type_calls[id_subresult][id_to];
    average_cost_kcy = ((average_cost_kcy * n_calls) + (double)(t_end - t_start) / 1000.0) / (double)(n_calls + 1);
    ++n_calls;
}

int typesystem_t::type_id(const void* addr) {
    auto it = m_addr_to_typeid.find(addr);
    if (it == m_addr_to_typeid.end()) {
        throw std::runtime_error("type is not registered");
    }
    return it->second;
}

void typesystem_t::update_coercion_graph() {
    const size_t n = m_type_parents.size();
    for (size_t k = 0; k < n; ++k) {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (m_type_distance[i][k] + m_type_distance[k][j] < m_type_distance[i][j]) {
                    m_type_distance[i][j] = m_type_distance[i][k] + m_type_distance[k][j];
                    m_type_parents[i][j] = m_type_parents[k][j];
                }
            }
        }
    }
}

void typesystem_t::update_coercion_graph(int id_from, int id_to) {
    static_cast<void>(sizeof_type(id_from));
    static_cast<void>(sizeof_type(id_to));

    // Floyd-Warshall
    const size_t n = m_type_parents.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (m_type_distance[i][id_from] + m_type_distance[id_from][id_to] + m_type_distance[id_to][j] < m_type_distance[i][j]) {
                m_type_distance[i][j] = m_type_distance[i][id_from] + m_type_distance[id_from][id_to] + m_type_distance[id_to][j];
                m_type_parents[i][j] = m_type_parents[id_from][j];
            }
        }
    }
}

size_t typesystem_t::sizeof_type(int type_id) {
    if (type_id < 0 || static_cast<std::size_t>(type_id) >= m_type_operations.size()) {
        throw std::runtime_error("type id is out of bounds");
    }

    return m_type_operations[type_id].size;
}

size_t typesystem_t::alignof_type(int type_id) {
    static_cast<void>(sizeof_type(type_id));
    return m_type_operations[type_id].alignment;
}

bool typesystem_t::is_trivially_copyable(int type_id) {
    static_cast<void>(sizeof_type(type_id));
    return m_type_operations[type_id].trivially_copyable;
}

} // namespace m03ge9ij43jyxy821pda20jhwh_typesystem
