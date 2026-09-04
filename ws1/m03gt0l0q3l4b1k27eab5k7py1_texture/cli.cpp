# include "api.h"

# include <format>
# include <iostream>

int main() {
    namespace texture_api = m03gt0l0q3l4b1k27eab5k7py1_texture;

    std::cout << std::format(
        "texture formats: {}, {}, {}, {}\n",
        texture_api::format_t::rgba8_unorm,
        texture_api::format_t::rgba8_srgb,
        texture_api::format_t::rgba16_float,
        texture_api::format_t::rgba32_float
    );
    return 0;
}
