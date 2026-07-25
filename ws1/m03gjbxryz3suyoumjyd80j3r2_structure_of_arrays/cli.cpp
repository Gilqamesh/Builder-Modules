#include "api.h"

#include <iostream>
#include <format>
#include <array>

#include <m03ginwy24ng8o487c4beoms6l_vector/api.h>

int main() {
    m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::structure_of_arrays_t<
        m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 2>,
        m03ginwy24ng8o487c4beoms6l_vector::vector_t<float, 3>
    > soa;

    soa.push_back({1.0f, 2.0f}, {3.0f, 4.0f, 5.0f});
    soa.push_back({4.0f, 5.0f}, {6.0f, 7.0f, 8.0f});
    soa.push_back({7.0f, 8.0f}, {9.0f, 10.0f, 11.0f});

    m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays::erased_structure_of_arrays_t esoa(soa);

    std::cout << std::format("{}", soa) << std::endl;
    std::cout << std::format("{}", esoa) << std::endl;

    return 0;
}
