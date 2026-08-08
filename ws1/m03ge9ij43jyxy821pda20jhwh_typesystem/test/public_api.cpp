#include <m03gn97n4iusbtl7uthb01wu9m_test_framework/test_framework.h>
#include <m03ge9ij43jyxy821pda20jhwh_typesystem/typesystem.h>

#include <cstddef>
#include <stdexcept>

namespace api = m03ge9ij43jyxy821pda20jhwh_typesystem;
namespace test = m03gn97n4iusbtl7uthb01wu9m_test_framework;

namespace {

struct source_t {
    int value;
};

struct middle_t {
    int value;
};

struct target_t {
    long value;
};

middle_t source_to_middle(source_t source) {
    return middle_t { source.value + 1 };
}

target_t middle_to_target(middle_t middle) {
    return target_t { static_cast<long>(middle.value) * 2L };
}

const int* pointer_to_const(int* value) {
    return value;
}

void add_context(void* from, void* to, void* context) {
    const int value = *static_cast<int*>(from);
    const int increment = *static_cast<int*>(context);
    *static_cast<int*>(to) = value + increment;
}

} // namespace

int main() {
    return test::run([] {
        api::typesystem_t typesystem;

        test::expect(typesystem.type_addr<source_t>() != nullptr);
        test::expect_equal(
            typesystem.type_addr<source_t>(),
            typesystem.type_addr<source_t>()
        );
        test::expect_not_equal(
            typesystem.type_addr<source_t>(),
            typesystem.type_addr<middle_t>()
        );
        test::expect_equal(typesystem.sizeof_type<source_t>(), sizeof(source_t));
        test::expect_equal(typesystem.sizeof_type<float>(), sizeof(float));

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto id = typesystem.type_id<source_t>();
        });
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto id = typesystem.type_id(
                typesystem.type_addr<source_t>()
            );
        });

        typesystem.register_type<source_t>();
        typesystem.register_type<middle_t>();
        typesystem.register_type<target_t>();
        typesystem.register_type<int*>();
        typesystem.register_type<const int*>();

        const int source_id = typesystem.type_id<source_t>();
        const int middle_id = typesystem.type_id<middle_t>();
        const int target_id = typesystem.type_id<target_t>();
        test::expect_equal(source_id, 0);
        test::expect_equal(middle_id, 1);
        test::expect_equal(target_id, 2);
        test::expect_equal(
            typesystem.type_id(typesystem.type_addr<target_t>()),
            target_id
        );
        test::expect_equal(typesystem.sizeof_type(source_id), sizeof(source_t));
        test::expect_equal(typesystem.sizeof_type(target_id), sizeof(target_t));

        typesystem.register_type<source_t>();
        test::expect_equal(typesystem.type_id<source_t>(), source_id);
        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto size = typesystem.sizeof_type(999);
        });

        typesystem.register_coercion<source_t, middle_t>(&source_to_middle);
        typesystem.register_coercion<middle_t, target_t>(&middle_to_target);
        typesystem.register_coercion<int*, const int*>(&pointer_to_const);
        test::expect_throws<std::runtime_error>([&] {
            typesystem.register_coercion<source_t, middle_t>(&source_to_middle);
        });

        typesystem.update_coercion_graph(source_id, middle_id);
        typesystem.update_coercion_graph();

        source_t source { 20 };
        source_t source_copy {};
        typesystem.coerce(&source, source_id, &source_copy, source_id);
        test::expect_equal(source_copy.value, 20);

        middle_t middle {};
        typesystem.coerce(&source, source_id, &middle, middle_id);
        test::expect_equal(middle.value, 21);

        target_t target {};
        typesystem.coerce(&source, source_id, &target, target_id);
        test::expect_equal(target.value, 42L);

        const auto copied_by_value = typesystem.coerce<source_t>(&source, source_id);
        test::expect_equal(copied_by_value.value, 20);
        const auto target_by_value = typesystem.coerce<target_t>(&source, source_id);
        test::expect_equal(target_by_value.value, 42L);

        auto reader = typesystem.coerce(source);
        test::expect_equal(reader.self, &typesystem);
        test::expect_equal(reader.from, static_cast<void*>(&source));
        test::expect_equal(reader.type_id_from, source_id);
        const middle_t reader_middle = reader;
        test::expect_equal(reader_middle.value, 21);
        const target_t reader_target = typesystem.coerce(source);
        test::expect_equal(reader_target.value, 42L);

        int integer = 7;
        int* pointer = &integer;
        const int* const_pointer = typesystem.coerce(pointer);
        test::expect_equal(const_pointer, &integer);
        test::expect_equal(*const_pointer, 7);

        test::expect_throws<std::runtime_error>([&] {
            [[maybe_unused]] const auto invalid = typesystem.coerce<source_t>(
                &target,
                target_id
            );
        });

        const api::typesystem_t::coercion_t empty_coercion;
        test::expect(!empty_coercion);

        int increment = 5;
        const api::typesystem_t::coercion_t manual_coercion {
            .caller = &add_context,
            .context = &increment
        };
        test::expect(static_cast<bool>(manual_coercion));
        int from = 10;
        int to = 0;
        manual_coercion(&from, &to);
        test::expect_equal(to, 15);
    });
}
