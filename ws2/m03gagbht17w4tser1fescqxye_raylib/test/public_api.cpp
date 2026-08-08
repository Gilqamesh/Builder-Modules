#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03gagbht17w4tser1fescqxye_raylib/raylib.h>
#include <m03gagbht17w4tser1fescqxye_raylib/raymath.h>
#include <m03gagbht17w4tser1fescqxye_raylib/rlgl.h>

namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

int main() {
    return test::run([] {
        test::expect(true, "Raylib public headers compile");
    });
}
