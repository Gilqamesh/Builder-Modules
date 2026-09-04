#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gilsfsv3k34ej14ytz8a29k_tower_defense_game/api.h>

#include <functional>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(std::identity(), true);
    });
}
