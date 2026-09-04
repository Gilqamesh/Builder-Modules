#ifndef M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_API_H
# define M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_API_H

# include <functional>

namespace m03gl8zioqukyra9uvyur6p95a_scope_guard {

class scope_guard_t {
public:
    explicit scope_guard_t(std::function<void()> cleanup);

    /**
     * Runs cleanup if the guard has not been released. Cleanup must not throw.
     */
    ~scope_guard_t() noexcept;

    scope_guard_t(const scope_guard_t& other) = delete;
    scope_guard_t& operator=(const scope_guard_t& other) = delete;
    scope_guard_t(scope_guard_t&& other) = delete;
    scope_guard_t& operator=(scope_guard_t&& other) = delete;

    void release() noexcept;

private:
    std::function<void()> m_cleanup;
};

} // namespace m03gl8zioqukyra9uvyur6p95a_scope_guard

#endif // M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_API_H
