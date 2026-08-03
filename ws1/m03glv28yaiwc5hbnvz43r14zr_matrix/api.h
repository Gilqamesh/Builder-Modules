#ifndef M03GLV28YAIWC5HBNVZ43R14ZR_MATRIX_MODULE_H
# define M03GLV28YAIWC5HBNVZ43R14ZR_MATRIX_MODULE_H

# include <array>
# include <algorithm>
# include <cstddef>
# include <format>
# include <initializer_list>
# include <stdexcept>

# include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

namespace m03glv28yaiwc5hbnvz43r14zr_matrix {

/**
 * @brief N-row by M-column matrix of type T.
 *
 * Storage is row-major.
 */
template <typename T, std::size_t N, std::size_t M>
class matrix_t {
    static_assert(0 < N, "matrix_t does not support matrices with 0 rows.");
    static_assert(0 < M, "matrix_t does not support matrices with 0 columns.");

public:
    using value_type = T;
    static constexpr std::size_t row_count = N;
    static constexpr std::size_t column_count = M;

public:
    matrix_t();
    matrix_t(const T& value);
    matrix_t(const std::array<T, N * M>& data);
    matrix_t(std::initializer_list<T> list);

    matrix_t(const matrix_t&) = default;
    matrix_t(matrix_t&&) = default;

    matrix_t& operator=(const matrix_t&) = default;
    matrix_t& operator=(matrix_t&&) = default;

    typename std::array<T, N * M>::const_iterator begin() const;
    typename std::array<T, N * M>::const_iterator end() const;
    typename std::array<T, N * M>::iterator begin();
    typename std::array<T, N * M>::iterator end();

    T& operator()(std::size_t row, std::size_t column); // does no bound checking
    const T& operator()(std::size_t row, std::size_t column) const; // does no bound checking

    bool operator==(const matrix_t& other) const;

    matrix_t& operator+=(const matrix_t& other);
    matrix_t& operator-=(const matrix_t& other);
    matrix_t operator+(const matrix_t& other) const;
    matrix_t operator-(const matrix_t& other) const;
    matrix_t operator-() const;

    template <typename U>
    matrix_t& operator*=(U value);
    template <typename U>
    matrix_t& operator/=(U value);
    template <typename U>
    matrix_t operator*(U value) const;
    template <typename U>
    matrix_t operator/(U value) const;

    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> operator*(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, M>& vector) const;

    template <std::size_t P>
    matrix_t<T, N, P> operator*(const matrix_t<T, M, P>& other) const;

private:
    std::array<T, N * M> m_data;
};

} // namespace m03glv28yaiwc5hbnvz43r14zr_matrix

namespace std {

template <typename T, std::size_t N, std::size_t M>
struct formatter<m03glv28yaiwc5hbnvz43r14zr_matrix::matrix_t<T, N, M>>;

} // namespace std

namespace m03glv28yaiwc5hbnvz43r14zr_matrix {

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>::matrix_t()
{
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>::matrix_t(const T& value) {
    m_data.fill(value);
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>::matrix_t(const std::array<T, N * M>& data):
    m_data(data)
{
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>::matrix_t(std::initializer_list<T> list) {
    if (list.size() != N * M) {
        throw std::invalid_argument("matrix_t does not support initializer lists of size different than N * M.");
    }
    std::copy(list.begin(), list.end(), m_data.begin());
}

template <typename T, std::size_t N, std::size_t M>
typename std::array<T, N * M>::const_iterator matrix_t<T, N, M>::begin() const {
    return m_data.begin();
}

template <typename T, std::size_t N, std::size_t M>
typename std::array<T, N * M>::const_iterator matrix_t<T, N, M>::end() const {
    return m_data.end();
}

template <typename T, std::size_t N, std::size_t M>
typename std::array<T, N * M>::iterator matrix_t<T, N, M>::begin() {
    return m_data.begin();
}

template <typename T, std::size_t N, std::size_t M>
typename std::array<T, N * M>::iterator matrix_t<T, N, M>::end() {
    return m_data.end();
}

template <typename T, std::size_t N, std::size_t M>
T& matrix_t<T, N, M>::operator()(std::size_t row, std::size_t column) {
    return m_data[row * M + column];
}

template <typename T, std::size_t N, std::size_t M>
const T& matrix_t<T, N, M>::operator()(std::size_t row, std::size_t column) const {
    return m_data[row * M + column];
}

template <typename T, std::size_t N, std::size_t M>
bool matrix_t<T, N, M>::operator==(const matrix_t& other) const {
    return m_data == other.m_data;
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>& matrix_t<T, N, M>::operator+=(const matrix_t& other) {
    for (std::size_t i = 0; i < m_data.size(); ++i) {
        m_data[i] += other.m_data[i];
    }
    return *this;
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M>& matrix_t<T, N, M>::operator-=(const matrix_t& other) {
    for (std::size_t i = 0; i < m_data.size(); ++i) {
        m_data[i] -= other.m_data[i];
    }
    return *this;
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M> matrix_t<T, N, M>::operator+(const matrix_t& other) const {
    matrix_t result(*this);
    result += other;
    return result;
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M> matrix_t<T, N, M>::operator-(const matrix_t& other) const {
    matrix_t result(*this);
    result -= other;
    return result;
}

template <typename T, std::size_t N, std::size_t M>
matrix_t<T, N, M> matrix_t<T, N, M>::operator-() const {
    matrix_t result;
    for (std::size_t i = 0; i < m_data.size(); ++i) {
        result.m_data[i] = -m_data[i];
    }
    return result;
}

template <typename T, std::size_t N, std::size_t M>
template <typename U>
matrix_t<T, N, M>& matrix_t<T, N, M>::operator*=(U value) {
    for (auto& element : m_data) {
        element = static_cast<T>(element * value);
    }
    return *this;
}

template <typename T, std::size_t N, std::size_t M>
template <typename U>
matrix_t<T, N, M>& matrix_t<T, N, M>::operator/=(U value) {
    for (auto& element : m_data) {
        element = static_cast<T>(element / value);
    }
    return *this;
}

template <typename T, std::size_t N, std::size_t M>
template <typename U>
matrix_t<T, N, M> matrix_t<T, N, M>::operator*(U value) const {
    matrix_t result(*this);
    result *= value;
    return result;
}

template <typename T, std::size_t N, std::size_t M>
template <typename U>
matrix_t<T, N, M> matrix_t<T, N, M>::operator/(U value) const {
    matrix_t result(*this);
    result /= value;
    return result;
}

template <typename T, std::size_t N, std::size_t M>
m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> matrix_t<T, N, M>::operator*(const m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, M>& vector) const {
    m03ginwy24ng8o487c4beoms6l_vector::vector_t<T, N> result(static_cast<T>(0));
    for (std::size_t row = 0; row < N; ++row) {
        for (std::size_t column = 0; column < M; ++column) {
            result[row] = static_cast<T>(result[row] + (*this)(row, column) * vector[column]);
        }
    }
    return result;
}

template <typename T, std::size_t N, std::size_t M>
template <std::size_t P>
matrix_t<T, N, P> matrix_t<T, N, M>::operator*(const matrix_t<T, M, P>& other) const {
    matrix_t<T, N, P> result(static_cast<T>(0));
    for (std::size_t row = 0; row < N; ++row) {
        for (std::size_t column = 0; column < P; ++column) {
            for (std::size_t i = 0; i < M; ++i) {
                result(row, column) = static_cast<T>(result(row, column) + (*this)(row, i) * other(i, column));
            }
        }
    }
    return result;
}

} // namespace m03glv28yaiwc5hbnvz43r14zr_matrix

namespace std {

template <typename T, std::size_t N, std::size_t M>
struct formatter<m03glv28yaiwc5hbnvz43r14zr_matrix::matrix_t<T, N, M>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            throw std::format_error("invalid matrix_t format specifier");
        }
        return it;
    }

    auto format(const m03glv28yaiwc5hbnvz43r14zr_matrix::matrix_t<T, N, M>& matrix, auto& ctx) const {
        auto out = ctx.out();

        out = std::format_to(out, "{{ ");
        for (std::size_t row = 0; row < N; ++row) {
            if (0 < row) {
                out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "{{ ");
            for (std::size_t column = 0; column < M; ++column) {
                if (0 < column) {
                    out = std::format_to(out, ", ");
                }
                out = std::format_to(out, "{}", matrix(row, column));
            }
            out = std::format_to(out, " }}");
        }
        out = std::format_to(out, " }}");

        return out;
    }
};

} // namespace std

#endif // M03GLV28YAIWC5HBNVZ43R14ZR_MATRIX_MODULE_H
