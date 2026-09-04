# include <m03gilsfsv3k34ej14ytz8a29k_tower_defense_game/geometry.h>
# include <m03gilsfsv3k34ej14ytz8a29k_tower_defense_game/index_buffer.h>
# include <m03gilsfsv3k34ej14ytz8a29k_tower_defense_game/mesh.h>
# include <m03gilsfsv3k34ej14ytz8a29k_tower_defense_game/vertex_attribute.h>
# include <m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays/api.h>
# include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>

# include <cstddef>
# include <functional>
# include <limits>
# include <memory>
# include <stdexcept>
# include <utility>
# include <vector>

namespace api = m03gilsfsv3k34ej14ytz8a29k_tower_defense_game;
namespace soa_api = m03gjbxryz3suyoumjyd80j3r2_structure_of_arrays;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

std::shared_ptr<api::mesh_t> make_mesh() {
    soa_api::structure_of_arrays_t<float> vertices;
    vertices.push_back(0.0F);
    vertices.push_back(1.0F);
    vertices.push_back(2.0F);

    return std::make_shared<api::mesh_t>(
        std::move(vertices),
        std::vector<api::vertex_attribute_t> {
            api::vertex_attribute_t(api::vertex_attribute_type_t::R32, 1)
        }
    );
}

std::shared_ptr<api::index_buffer_t> make_indices(
    api::index_buffer_t::indices_t values
) {
    auto result = std::make_shared<api::index_buffer_t>();
    result->indices() = std::move(values);
    return result;
}

} // namespace

int main() {
    return test::run([] {
        const auto mesh = make_mesh();
        test::expect(std::equal_to<>(), mesh->number_of_vertices(), std::size_t(3));

        const auto index_buffer = make_indices({0, 1, 2});
        api::geometry_t geometry(index_buffer);
        geometry.mesh() = mesh;
        test::expect_no_throw([&] { geometry.finalize(); });
        test::expect(std::equal_to<>(), geometry.indices().size(), std::size_t(3));

        test::expect_throws<std::invalid_argument>([] {
            [[maybe_unused]] const api::geometry_t invalid(nullptr);
        });
        test::expect_throws<std::out_of_range>([&] {
            [[maybe_unused]] const api::geometry_t invalid(
                index_buffer,
                api::index_range_t { .offset = 2, .count = 2 }
            );
        });

        api::geometry_t missing_mesh(make_indices({0, 1, 2}));
        test::expect_throws<std::runtime_error>([&] { missing_mesh.finalize(); });

        api::geometry_t invalid_index(make_indices({0, 1, 3}));
        invalid_index.mesh() = mesh;
        test::expect_throws<std::runtime_error>([&] { invalid_index.finalize(); });

        api::geometry_t invalid_line(make_indices({0, 1, 2}));
        invalid_line.mesh() = mesh;
        invalid_line.primitive_topology() = api::vertex_primitive_topology_t::line;
        test::expect_throws<std::runtime_error>([&] { invalid_line.finalize(); });

        api::geometry_t mutable_range(index_buffer);
        index_buffer->indices().resize(1);
        test::expect_throws<std::out_of_range>([&] {
            [[maybe_unused]] const auto selected = mutable_range.indices();
        });

        soa_api::structure_of_arrays_t<float> overflow_vertices;
        overflow_vertices.push_back(0.0F);
        test::expect_throws<std::length_error>([&] {
            [[maybe_unused]] const api::mesh_t invalid(
                std::move(overflow_vertices),
                std::vector<api::vertex_attribute_t> {
                    api::vertex_attribute_t(
                        api::vertex_attribute_type_t::R64,
                        std::numeric_limits<std::size_t>::max()
                    )
                }
            );
        });
    });
}
