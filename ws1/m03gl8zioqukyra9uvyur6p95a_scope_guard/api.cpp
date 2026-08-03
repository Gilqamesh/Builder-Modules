#include "api.h"

namespace m03gl8zioqukyra9uvyur6p95a_scope_guard {

scope_guard_t::scope_guard_t(std::function<void()> cleanup):
    m_cleanup(std::move(cleanup))
{
}

scope_guard_t::~scope_guard_t() {
    if (m_cleanup) {
        m_cleanup();
    }
}

void scope_guard_t::release() noexcept {
    m_cleanup = nullptr;
}

} // namespace m03gl8zioqukyra9uvyur6p95a_scope_guard
