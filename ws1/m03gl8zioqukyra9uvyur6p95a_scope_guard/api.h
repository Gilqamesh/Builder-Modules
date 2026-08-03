#ifndef M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_MODULE_H
# define M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_MODULE_H

# include <functional>

namespace m03gl8zioqukyra9uvyur6p95a_scope_guard {

class scope_guard_t {
public:
    explicit scope_guard_t(std::function<void()> cleanup);

    ~scope_guard_t();

    void release() noexcept;

private:
    std::function<void()> m_cleanup;
};

} // namespace m03gl8zioqukyra9uvyur6p95a_scope_guard

#endif // M03GL8ZIOQUKYRA9UVYUR6P95A_SCOPE_GUARD_MODULE_H
