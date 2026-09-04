#ifndef M03GT1DJVVY5ATIA5EVKBG6RQY_SOFTWARE_SHADER_SOFTWARE_SHADER_H
# define M03GT1DJVVY5ATIA5EVKBG6RQY_SOFTWARE_SHADER_SOFTWARE_SHADER_H

# include <m03gsy25j4v7nccgmsdov9ioft_shader/api.h>
# include <m03gt0l0q3l4b1k27eab5k7py1_texture/api.h>

# include <any>
# include <cstdint>
# include <format>
# include <functional>
# include <optional>
# include <stdexcept>
# include <type_traits>
# include <unordered_map>
# include <utility>

namespace m03gt1djvvy5atia5evkbg6rqy_software_shader {

namespace shader = m03gsy25j4v7nccgmsdov9ioft_shader;
namespace texture = m03gt0l0q3l4b1k27eab5k7py1_texture;

/**
 * @brief Holds the values and CPU resources visible to shader invocations.
 *
 * Uniform, texture, and sampler indices occupy separate namespaces. Uniform values are owned by the bindings. Textures and samplers are borrowed and must outlive every `run()` that can use their binding.
 */
class bindings_t {
public:
    template <shader::shader_value T>
    void uniform(std::uint32_t binding, T uniform);

    template <shader::shader_value T>
    std::remove_cvref_t<T> uniform(std::uint32_t binding) const;

    void texture(std::uint32_t binding, const texture::texture_t& texture);
    void texture(std::uint32_t binding, texture::texture_t&& texture) = delete;
    void texture(std::uint32_t binding, const texture::texture_t&& texture) = delete;
    const texture::texture_t& texture(std::uint32_t binding) const;

    void sampler(std::uint32_t binding, const texture::sampler_t& sampler);
    void sampler(std::uint32_t binding, texture::sampler_t&& sampler) = delete;
    void sampler(std::uint32_t binding, const texture::sampler_t&& sampler) = delete;
    const texture::sampler_t& sampler(std::uint32_t binding) const;

private:
    std::unordered_map<std::uint32_t, std::any> m_uniforms;
    std::unordered_map<std::uint32_t, std::reference_wrapper<const texture::texture_t>> m_textures;
    std::unordered_map<std::uint32_t, std::reference_wrapper<const texture::sampler_t>> m_samplers;
};

/**
 * @brief Carries typed vertex inputs, built-ins, and results for one invocation at a time.
 */
class vertex_io_t {
public:
    vertex_io_t(std::int32_t vertex_index, std::int32_t instance_index);

    std::int32_t vertex_index() const;
    std::int32_t instance_index() const;

    template <shader::shader_value T>
    void input(std::uint32_t location, T input);

    template <shader::shader_value T>
    std::remove_cvref_t<T> input(std::uint32_t location) const;

    template <shader::shader_value T>
    void output(std::uint32_t location, T output);

    template <shader::shader_value T>
    std::optional<std::remove_cvref_t<T>> output(std::uint32_t location) const;

    void position(shader::vector_t<float, 4> position);
    shader::vector_t<float, 4> position() const;

    void clear_results();

private:
    std::int32_t m_vertex_index;
    std::int32_t m_instance_index;
    std::unordered_map<std::uint32_t, std::any> m_inputs;
    std::unordered_map<std::uint32_t, std::any> m_outputs;
    std::optional<shader::vector_t<float, 4>> m_position;
};

/**
 * @brief Carries typed fragment inputs, built-ins, and results for one invocation at a time.
 */
class fragment_io_t {
public:
    fragment_io_t(shader::vector_t<float, 4> fragment_coordinate, bool front_facing);

    shader::vector_t<float, 4> fragment_coordinate() const;
    bool front_facing() const;

    template <shader::shader_value T>
    void input(std::uint32_t location, T input);

    template <shader::shader_value T>
    std::remove_cvref_t<T> input(std::uint32_t location) const;

    template <shader::shader_value T>
    void output(std::uint32_t location, T output);

    template <shader::shader_value T>
    std::optional<std::remove_cvref_t<T>> output(std::uint32_t location) const;

    bool discarded() const;
    void discard();
    void clear_results();

private:
    shader::vector_t<float, 4> m_fragment_coordinate;
    bool m_front_facing;
    bool m_discarded;
    std::unordered_map<std::uint32_t, std::any> m_inputs;
    std::unordered_map<std::uint32_t, std::any> m_outputs;
};

/**
 * @brief Owns a linked vertex and fragment shader program executable on the CPU.
 */
class program_t {
public:
    /**
     * @throws std::invalid_argument if the AST stages are wrong, a fragment input has no compatible vertex output, or a binding has incompatible types across stages.
     */
    program_t(shader::shader_ast_t vertex, shader::shader_ast_t fragment);

    const shader::shader_interface_t& vertex_interface() const;
    const shader::shader_interface_t& fragment_interface() const;

    /**
     * @brief Runs a fresh vertex invocation and replaces all prior vertex results.
     *
     * @throws std::invalid_argument if a required input or binding is absent or has the wrong type.
     * @throws std::runtime_error if execution completes without writing position.
     */
    void run(const bindings_t& bindings, vertex_io_t& io) const;

    /**
     * @brief Runs a fresh fragment invocation and replaces all prior fragment results.
     *
     * Discard terminates execution, clears all outputs, and is reported by `fragment_io_t::discarded()`.
     *
     * @throws std::invalid_argument if a required input or binding is absent or has the wrong type.
     */
    void run(const bindings_t& bindings, fragment_io_t& io) const;

private:
    shader::shader_ast_t m_vertex;
    shader::shader_ast_t m_fragment;
};

} // namespace m03gt1djvvy5atia5evkbg6rqy_software_shader

namespace std {

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t>;

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::vertex_io_t>;

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::fragment_io_t>;

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t>;

} // namespace std

namespace m03gt1djvvy5atia5evkbg6rqy_software_shader {

template <shader::shader_value T>
void bindings_t::uniform(std::uint32_t binding, T uniform) {
    using type = std::remove_cvref_t<T>;
    m_uniforms.insert_or_assign(binding, std::any(type(std::move(uniform))));
}

template <shader::shader_value T>
std::remove_cvref_t<T> bindings_t::uniform(std::uint32_t binding) const {
    using type = std::remove_cvref_t<T>;
    const auto iterator = m_uniforms.find(binding);
    if (iterator == m_uniforms.end()) {
        throw std::invalid_argument("missing software shader uniform binding");
    }
    const auto* uniform = std::any_cast<type>(&iterator->second);
    if (!uniform) {
        throw std::invalid_argument("software shader uniform binding has the wrong type");
    }
    return *uniform;
}

template <shader::shader_value T>
void vertex_io_t::input(std::uint32_t location, T input) {
    using type = std::remove_cvref_t<T>;
    m_inputs.insert_or_assign(location, std::any(type(std::move(input))));
}

template <shader::shader_value T>
std::remove_cvref_t<T> vertex_io_t::input(std::uint32_t location) const {
    using type = std::remove_cvref_t<T>;
    const auto iterator = m_inputs.find(location);
    if (iterator == m_inputs.end()) {
        throw std::invalid_argument("missing software shader vertex input");
    }
    const auto* input = std::any_cast<type>(&iterator->second);
    if (!input) {
        throw std::invalid_argument("software shader vertex input has the wrong type");
    }
    return *input;
}

template <shader::shader_value T>
void vertex_io_t::output(std::uint32_t location, T output) {
    using type = std::remove_cvref_t<T>;
    m_outputs.insert_or_assign(location, std::any(type(std::move(output))));
}

template <shader::shader_value T>
std::optional<std::remove_cvref_t<T>> vertex_io_t::output(std::uint32_t location) const {
    using type = std::remove_cvref_t<T>;
    const auto iterator = m_outputs.find(location);
    if (iterator == m_outputs.end()) {
        return std::nullopt;
    }
    const auto* output = std::any_cast<type>(&iterator->second);
    if (!output) {
        throw std::invalid_argument("software shader vertex output has the wrong type");
    }
    return *output;
}

template <shader::shader_value T>
void fragment_io_t::input(std::uint32_t location, T input) {
    using type = std::remove_cvref_t<T>;
    m_inputs.insert_or_assign(location, std::any(type(std::move(input))));
}

template <shader::shader_value T>
std::remove_cvref_t<T> fragment_io_t::input(std::uint32_t location) const {
    using type = std::remove_cvref_t<T>;
    const auto iterator = m_inputs.find(location);
    if (iterator == m_inputs.end()) {
        throw std::invalid_argument("missing software shader fragment input");
    }
    const auto* input = std::any_cast<type>(&iterator->second);
    if (!input) {
        throw std::invalid_argument("software shader fragment input has the wrong type");
    }
    return *input;
}

template <shader::shader_value T>
void fragment_io_t::output(std::uint32_t location, T output) {
    using type = std::remove_cvref_t<T>;
    m_outputs.insert_or_assign(location, std::any(type(std::move(output))));
}

template <shader::shader_value T>
std::optional<std::remove_cvref_t<T>> fragment_io_t::output(std::uint32_t location) const {
    using type = std::remove_cvref_t<T>;
    const auto iterator = m_outputs.find(location);
    if (iterator == m_outputs.end()) {
        return std::nullopt;
    }
    const auto* output = std::any_cast<type>(&iterator->second);
    if (!output) {
        throw std::invalid_argument("software shader fragment output has the wrong type");
    }
    return *output;
}

} // namespace m03gt1djvvy5atia5evkbg6rqy_software_shader

namespace std {

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iterator = ctx.begin();
        if (iterator != ctx.end() && *iterator != '}') {
            throw std::format_error("invalid bindings_t format specifier");
        }
        return iterator;
    }

    auto format(const m03gt1djvvy5atia5evkbg6rqy_software_shader::bindings_t&, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "software shader bindings");
        return out;
    }
};

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::vertex_io_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iterator = ctx.begin();
        if (iterator != ctx.end() && *iterator != '}') {
            throw std::format_error("invalid vertex_io_t format specifier");
        }
        return iterator;
    }

    auto format(const m03gt1djvvy5atia5evkbg6rqy_software_shader::vertex_io_t& io, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "vertex_index: {}", io.vertex_index());
        out = std::format_to(out, ", instance_index: {}", io.instance_index());
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::fragment_io_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iterator = ctx.begin();
        if (iterator != ctx.end() && *iterator != '}') {
            throw std::format_error("invalid fragment_io_t format specifier");
        }
        return iterator;
    }

    auto format(const m03gt1djvvy5atia5evkbg6rqy_software_shader::fragment_io_t& io, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "fragment_coordinate: {}", io.fragment_coordinate());
        out = std::format_to(out, ", front_facing: {}", io.front_facing());
        out = std::format_to(out, ", discarded: {}", io.discarded());
        out = std::format_to(out, " }}");
        return out;
    }
};

template <>
struct formatter<m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iterator = ctx.begin();
        if (iterator != ctx.end() && *iterator != '}') {
            throw std::format_error("invalid program_t format specifier");
        }
        return iterator;
    }

    auto format(const m03gt1djvvy5atia5evkbg6rqy_software_shader::program_t& program, auto& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "{{ ");
        out = std::format_to(out, "vertex_inputs: {}", program.vertex_interface().inputs().size());
        out = std::format_to(out, ", vertex_outputs: {}", program.vertex_interface().outputs().size());
        out = std::format_to(out, ", fragment_inputs: {}", program.fragment_interface().inputs().size());
        out = std::format_to(out, ", fragment_outputs: {}", program.fragment_interface().outputs().size());
        out = std::format_to(out, " }}");
        return out;
    }
};

} // namespace std

#endif // M03GT1DJVVY5ATIA5EVKBG6RQY_SOFTWARE_SHADER_SOFTWARE_SHADER_H
