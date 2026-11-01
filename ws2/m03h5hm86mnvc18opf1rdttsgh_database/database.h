#ifndef M03H5HM86MNVC18OPF1RDTTSGH_DATABASE_DATABASE_H
# define M03H5HM86MNVC18OPF1RDTTSGH_DATABASE_DATABASE_H

# include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>

# include <cmath>
# include <concepts>
# include <cstddef>
# include <cstdint>
# include <filesystem>
# include <format>
# include <functional>
# include <limits>
# include <memory>
# include <meta>
# include <optional>
# include <span>
# include <stdexcept>
# include <string>
# include <string_view>
# include <tuple>
# include <type_traits>
# include <utility>
# include <variant>
# include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

/**
 * @brief Maps an owning field type directly to one canonical column representation.
 *
 * Specializations must be visible before record operations are instantiated.
 * storage_t is exactly int64_t, double, string, or byte_stream_t. encode(const T&)
 * returns storage_t or const storage_t&; borrowed storage must remain valid until
 * consumed by the operation. decode accepts an owned storage_t rvalue and returns T
 * by value, retaining its data independently of that argument and the database.
 * Codec exceptions propagate unchanged. Decoding runs inside the read snapshot;
 * codecs must not re-enter the same database. Encoding compatibility and migration
 * belong to the application; no codec identifier or version is persisted.
 * Nested objects and containers require explicit codecs. Views, enums, and long
 * double have no built-in codec.
 *
 * optional<U> always uses U's codec and represents an absent or present value. Direct codecs
 * for optional and nested optionals are unsupported; use a domain wrapper instead.
 * Storage kinds are not converted. Canonical double must not be NaN when encoding
 * (invalid_argument). Built-in numeric range failures throw out_of_range.
 */
template <typename T>
struct column_codec;

/**
 * @brief Stores standard integral types as checked signed 64-bit integers.
 *
 * Insertion and decoding check their destination ranges; bool accepts only 0/1.
 */
template <std::integral T>
struct column_codec<T> {
    using storage_t = std::int64_t;

    static storage_t encode(const T& number);
    static T decode(storage_t number);
};

/**
 * @brief Stores float and double using canonical double precision.
 *
 * Rounding and underflow are allowed, finite overflow is rejected, and infinities
 * are preserved.
 */
template <typename T> requires (std::same_as<T, float> || std::same_as<T, double>)
struct column_codec<T> {
    using storage_t = double;

    static storage_t encode(const T& number);
    static T decode(storage_t number);
};

/** @brief Stores owning strings and byte streams, distinguishing empty data from absence. */
template <typename T> requires (std::same_as<T, std::string> || std::same_as<T, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>)
struct column_codec<T> {
    using storage_t = T;

    static const storage_t& encode(const T& field);
    static T decode(storage_t stored);
};

/**
 * @brief Manages typed tables and atomic transactions in one database connection.
 *
 * Serialize access to the database and all its handles, including destruction.
 * Invalid runtime arguments throw invalid_argument; schema, database-state and
 * backend failures throw contextual runtime_error. Allocation exceptions propagate
 * unchanged. See column_codec for codec and numeric failures.
 * Failures attempt transaction cleanup, preserving the original exception.
 * Destruction never throws. Persistent storage is an opaque, module-owned
 * directory. Close all connections before copying or moving the entire directory.
 * Live filesystem copying, external changes, and externally authored storage
 * layouts are unsupported.
 *
 * @code
 * struct person_t { std::int64_t id; std::string name; std::optional<int> age; };
 * database_t db;
 * auto people = db.create_table<person_t>("people");
 * people.insert(person_t{1, "Ada", 36});
 * auto names = db.tables();
 * auto matches = people.rows<&person_t::id>(1);
 * people.replace<&person_t::id>(1, person_t{1, "Ada", 37});
 * people.erase<&person_t::id>(1);
 * db.drop_table<person_t>("people");
 * @endcode
 */
class database_t {
public:
    /** @brief Creates an independent in-memory database. */
    database_t();
    /**
     * @brief Opens or creates a database storage directory and reads its schema.
     *
     * Rejects empty paths and embedded NULs. Relative paths become normalized
     * absolute paths; the parent directory must exist. All paths are literal.
     * Creates missing directories or initializes empty ones. Nonempty directories
     * must contain recognized, supported-version module storage with no extra
     * entries or linked private files. Legacy database files, unsupported versions,
     * and incomplete initialization are rejected without conversion or repair.
     * Failed initialization may leave incomplete storage that must be removed
     * before retrying. Does not perform an integrity scan; damaged data may be
     * detected by later operations.
     */
    explicit database_t(const std::filesystem::path& directory);
    ~database_t() noexcept;

    database_t(const database_t&) = delete;
    database_t& operator=(const database_t&) = delete;
    database_t(database_t&&) = delete;
    database_t& operator=(database_t&&) = delete;

    /**
     * @brief Runs action once with this database and atomically commits its changes.
     *
     * Invokes action(*this) synchronously after setup; never retains or retries it.
     * All handles borrowing this database participate, including existing handles.
     * Reads share one external snapshot established before the callback and see
     * the transaction's own changes. Operations do not independently commit.
     * Other connections may commit while this transaction reads; conflicting
     * writes fail with runtime_error, without automatic retries.
     *
     * Normal return commits only if the transaction remains valid. An escaping
     * exception rolls back and propagates unchanged. A caught operation error
     * permits continuation only when its effects were undone and the transaction
     * remains valid; otherwise this call fails even if action returns normally.
     * Commit failures propagate. Cleanup failures never mask the original error
     * and may leave the connection unusable. Nested transactions on this database
     * throw logic_error before invoking their callback.
     */
    void transaction(std::function_ref<void(database_t&)> action);

    template <typename T>
    class table_t;

    /**
     * @brief Atomically creates a missing table or validates and reuses a compatible one.
     *
     * Logical names must be nonempty and contain no NUL. Lookup is ASCII
     * case-insensitive. Names are independent of the private storage representation.
     * Incompatible schemas are rejected without changing existing data.
     *
     * T must be a nonempty nonunion aggregate with named accessible fields, no
     * bases or bitfields, and no reference or raw-pointer fields. Each field needs
     * a valid codec; duplicate and ASCII case-colliding member names are rejected.
     * Unsupported fields fail compilation with their names.
     * T must support vector construction; default construction and assignment
     * are unnecessary, and move-only owning fields are supported.
     *
     * Compatibility requires matching field names, canonical representations and
     * optionality; declaration order may differ. Member renaming and changes to
     * codec encodings require application-managed migration. Outside transaction(),
     * acquisition commits before returning; inside it, acquisition participates.
     */
    template <typename T>
    table_t<T> create_table(std::string_view name);

    /**
     * @brief Atomically deletes a compatible table, doing nothing when it is missing.
     *
     * Uses create_table<T>()'s name and compatibility rules without creating tables.
     * Lookup, validation and deletion join transaction() or commit before return.
     * An incompatible table is rejected without deleting it. Existing handles keep
     * their logical names; record operations fail until compatible recreation.
     */
    template <typename T>
    void drop_table(std::string_view name);

    /**
     * @brief Returns all existing application table names in ordinary string order.
     *
     * Owning names preserve creation spelling, survive database destruction and
     * exclude private storage and backend objects. Case-insensitive acquisition
     * never renames a table. Empty databases return an empty vector.
     * Reads one consistent snapshot, using transaction()'s snapshot and observing
     * its own table creations and deletions when inside a transaction.
     */
    std::vector<std::string> tables() const;

private:
    class state_t;
    using cell_t = std::variant<std::monostate, std::int64_t, double, std::string, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>;
    // Member name, canonical variant index, nullable.
    using column_t = std::tuple<std::string_view, std::size_t, bool>;
    template <typename T, std::size_t I>
    using member_t = [:std::meta::type_of(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())[I]):];
    template <typename T, auto Member>
    static consteval std::meta::info selected_field();
    template <typename T, auto Member>
    using selected_t = [:std::meta::type_of(selected_field<T, Member>()):];

    template <typename T>
    static consteval bool is_optional();
    template <typename T>
    static consteval bool valid_codec();
    template <typename T>
    static consteval bool valid_field();
    template <typename T>
    static consteval std::meta::info codec_type();
    template <typename S>
    static consteval std::size_t storage_kind();
    template <typename T>
    static consteval bool validate_record();
    template <typename T>
    static std::vector<column_t> columns();
    template <typename F>
    static cell_t encode_field(const F& field);
    template <typename T>
    static std::vector<cell_t> encode_record(const T& record);
    template <typename F>
    static std::remove_cv_t<F> decode_field(cell_t&& stored);
    template <typename T, std::size_t... I>
    static T decode_record(std::span<cell_t> stored, std::index_sequence<I...>);
    void ensure_table(std::string_view table, std::span<const column_t> columns);
    void insert_impl(std::string_view table, std::span<const column_t> columns, std::span<const cell_t> stored);
    void rows_impl(std::string_view table, std::span<const column_t> columns, std::function_ref<void(std::span<cell_t>)> consume, std::string_view field = {}, const cell_t& match = {}) const;
    std::size_t mutate_impl(std::string_view table, std::span<const column_t> columns, std::string_view field, const cell_t& match, std::span<const cell_t> replacement);
    void drop_impl(std::string_view table, std::span<const column_t> columns);

    std::unique_ptr<state_t> m_state;
};

/**
 * @brief Manages records in a typed table borrowed from a database.
 *
 * The database must outlive this handle, all copies, and all operations. Handles
 * own their table name and can be copied or moved. A const handle still permits
 * record changes through its database. Returned records are detached C++ values;
 * persist edited values explicitly with replace(). Use transaction() for atomic
 * read–modify–write sequences.
 * Each operation revalidates the schema; later incompatible changes are rejected.
 * After moving a handle, use its destination; the source may only be reassigned
 * or destroyed.
 */
template <typename T>
class database_t::table_t {
public:
    table_t(const table_t&) = default;
    table_t& operator=(const table_t&) = default;
    table_t(table_t&&) = default;
    table_t& operator=(table_t&&) = default;

    /** @brief Inserts current field values, joining transaction() or committing before return. */
    void insert(const T& record) const;
    /**
     * @brief Retrieves owning records from one validated read snapshot.
     *
     * Returns vector<T> by value, preserving duplicates with no ordering guarantee.
     * Records survive handle and database destruction. Failures return no partial
     * result.
     */
    std::vector<T> rows() const;

    /**
     * @brief Retrieves every record whose selected field has the encoded match value.
     *
     * Member must select a supported accessible direct nonstatic data member of T;
     * invalid selectors fail compilation. match has that field's type and uses its
     * codec. Equality compares canonical representations: exact strings and bytes
     * (including NULs), numeric equality, and absence for absent optionals. Custom
     * encoded equality may differ from C++ equality. Preserves duplicates and uses
     * the same owning, unordered snapshot contract as rows().
     */
    template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
    std::vector<T> rows(const selected_t<T, Member>& match) const;

    /**
     * @brief Replaces every matching record completely and returns the matched count.
     *
     * Uses rows<Member>()'s selection semantics, evaluated against the original
     * match. Replacement may change the selected field. Identical replacements
     * count as matches; zero matches succeed. Joins transaction() or commits before
     * return. Failures roll back the operation under transaction()'s recovery rules.
     */
    template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
    std::size_t replace(const selected_t<T, Member>& match, const T& replacement) const;

    /**
     * @brief Deletes every matching record and returns the deleted count.
     *
     * Uses rows<Member>()'s selection semantics. Zero matches succeed; joins
     * transaction() or commits before return with the same recovery as replace().
     */
    template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
    std::size_t erase(const selected_t<T, Member>& match) const;

private:
    friend class database_t;
    table_t(database_t& database, std::string_view name);

    database_t* m_database;
    std::string m_name;
    std::vector<column_t> m_columns;
};

/** @brief A typed table handle borrowing its owning database. */
template <typename T>
using table_t = database_t::table_t<T>;

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

namespace std {

template <typename T>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::column_codec<T>>;
template <>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::database_t>;
template <typename T>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::database_t::table_t<T>>;

} // namespace std

namespace m03h5hm86mnvc18opf1rdttsgh_database {

template <std::integral T>
auto column_codec<T>::encode(const T& number) -> storage_t {
    if constexpr (std::numeric_limits<storage_t>::digits < std::numeric_limits<T>::digits) {
        if (!std::in_range<storage_t>(number)) {
            throw std::out_of_range(std::format("database encode integer: {} exceeds signed 64-bit storage", number));
        }
    }
    return static_cast<storage_t>(number);
}

template <std::integral T>
T column_codec<T>::decode(storage_t number) {
    if constexpr (std::same_as<T, bool>) {
        if (number != 0 && number != 1) {
            throw std::out_of_range(std::format("database decode bool: expected 0 or 1; found {}", number));
        }
    } else if constexpr (std::numeric_limits<T>::digits < std::numeric_limits<storage_t>::digits || !std::is_signed_v<T>) {
        if (number < 0 && !std::is_signed_v<T>) {
            throw std::out_of_range(std::format("database decode integer: {} is negative for an unsigned field", number));
        }
        if constexpr (std::is_signed_v<T>) {
            if (number < static_cast<storage_t>(std::numeric_limits<T>::min()) || static_cast<storage_t>(std::numeric_limits<T>::max()) < number) {
                throw std::out_of_range(std::format("database decode integer: {} exceeds the destination range", number));
            }
        } else if (static_cast<std::uint64_t>(std::numeric_limits<T>::max()) < static_cast<std::uint64_t>(number)) {
            throw std::out_of_range(std::format("database decode integer: {} exceeds the destination range", number));
        }
    }
    return static_cast<T>(number);
}

template <typename T> requires (std::same_as<T, float> || std::same_as<T, double>)
auto column_codec<T>::encode(const T& number) -> storage_t {
    if (std::isnan(number)) {
        throw std::invalid_argument("database encode real: NaN is unsupported");
    }
    return static_cast<storage_t>(number);
}

template <typename T> requires (std::same_as<T, float> || std::same_as<T, double>)
T column_codec<T>::decode(storage_t number) {
    if (std::isfinite(number) && static_cast<double>(std::numeric_limits<T>::max()) < std::abs(number)) {
        throw std::out_of_range(std::format("database decode real: {} exceeds the destination finite range", number));
    }
    return static_cast<T>(number);
}

template <typename T> requires (std::same_as<T, std::string> || std::same_as<T, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>)
auto column_codec<T>::encode(const T& field) -> const storage_t& {
    return field;
}

template <typename T> requires (std::same_as<T, std::string> || std::same_as<T, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>)
T column_codec<T>::decode(storage_t stored) {
    return stored;
}

template <typename T>
auto database_t::create_table(std::string_view name) -> table_t<T> {
    return table_t<T>(*this, name);
}

template <typename T>
void database_t::drop_table(std::string_view name) {
    drop_impl(name, columns<T>());
}

template <typename T, auto Member>
consteval std::meta::info database_t::selected_field() {
    if constexpr (std::is_member_object_pointer_v<decltype(Member)>) {
        if constexpr (Member != nullptr) {
            template for (constexpr auto field : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
                using field_t = [:std::meta::type_of(field):];
                if constexpr (!std::meta::is_bit_field(field) && valid_field<field_t>()) {
                    if constexpr (std::same_as<decltype(Member), decltype(&[:field:])>) {
                        if constexpr (Member == &[:field:]) return field;
                    }
                }
            }
        }
    }
    return {};
}

template <typename T>
consteval bool database_t::is_optional() {
    return requires { []<typename U>(std::type_identity<std::optional<U>>) {}(std::type_identity<T>{}); };
}

template <typename T>
consteval bool database_t::valid_codec() {
    return requires(const T& field, typename column_codec<T>::storage_t stored) {
        requires (std::same_as<typename column_codec<T>::storage_t, std::int64_t> ||
                  std::same_as<typename column_codec<T>::storage_t, double> ||
                  std::same_as<typename column_codec<T>::storage_t, std::string> ||
                  std::same_as<typename column_codec<T>::storage_t, m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t>);
        requires (std::same_as<decltype(column_codec<T>::encode(field)), typename column_codec<T>::storage_t> ||
                  std::same_as<decltype(column_codec<T>::encode(field)), const typename column_codec<T>::storage_t&>);
        { column_codec<T>::decode(std::move(stored)) } -> std::same_as<T>;
    };
}

template <typename T>
consteval bool database_t::valid_field() {
    using field_t = std::remove_cv_t<T>;
    if constexpr (std::is_reference_v<T> || std::is_pointer_v<field_t>) {
        return false;
    } else if constexpr (is_optional<field_t>()) {
        using inner_t = std::remove_cv_t<typename field_t::value_type>;
        return !requires { sizeof(column_codec<field_t>); } && !is_optional<inner_t>() && valid_field<inner_t>();
    } else {
        return valid_codec<field_t>();
    }
}

template <typename T>
consteval std::meta::info database_t::codec_type() {
    using field_t = std::remove_cv_t<T>;
    if constexpr (is_optional<field_t>()) {
        using inner_t = std::remove_cv_t<typename field_t::value_type>;
        return ^^inner_t;
    } else {
        return ^^field_t;
    }
}

template <typename S>
consteval std::size_t database_t::storage_kind() {
    if constexpr (std::same_as<S, std::int64_t>) return 1;
    else if constexpr (std::same_as<S, double>) return 2;
    else if constexpr (std::same_as<S, std::string>) return 3;
    else return 4;
}

template <typename T>
consteval bool database_t::validate_record() {
    static_assert(std::is_aggregate_v<T> && !std::is_union_v<T>, "database record must be a nonunion aggregate");
    if constexpr (std::is_aggregate_v<T> && !std::is_union_v<T>) {
        static_assert(std::meta::bases_of(^^T, std::meta::access_context::current()).empty(), "database record must not have base classes");
        static constexpr auto fields = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
        static_assert(!fields.empty(), "database record must contain at least one field");
        static_assert(std::is_constructible_v<T, T&&>, "database record must support vector construction from decoded records");
        template for (constexpr auto field : fields) {
            static_assert(std::meta::has_identifier(field), "database record fields must be named");
            if constexpr (std::meta::has_identifier(field)) {
                using field_t = [:std::meta::type_of(field):];
                static_assert(!std::meta::is_bit_field(field) && valid_field<field_t>(), std::string("Unsupported field '") + std::string(std::meta::identifier_of(field)) + "': requires an owning column codec and a supported field form");
                constexpr bool unique = [=] {
                    const auto name = std::meta::identifier_of(field);
                    for (const auto previous : fields) {
                        if (previous == field) break;
                        const auto other = std::meta::identifier_of(previous);
                        if (name.size() != other.size()) continue;
                        bool equal = true;
                        for (std::size_t index = 0; index < name.size(); ++index) {
                            const auto fold = [](char character) { return 'A' <= character && character <= 'Z' ? character + ('a' - 'A') : character; };
                            if (fold(name[index]) != fold(other[index])) equal = false;
                        }
                        if (equal) return false;
                    }
                    return true;
                }();
                static_assert(unique, std::string("Duplicate or ASCII case-colliding field '") + std::string(std::meta::identifier_of(field)) + "'");
            }
        }
    }
    return true;
}

template <typename T>
auto database_t::columns() -> std::vector<column_t> {
    static_assert(validate_record<T>());
    std::vector<column_t> schema;
    template for (constexpr auto field : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
        using field_t = [:std::meta::type_of(field):];
        if constexpr (valid_field<field_t>()) {
            using codec_field_t = [:codec_type<field_t>():];
            using codec_t = column_codec<codec_field_t>;
            schema.emplace_back(std::meta::identifier_of(field), storage_kind<typename codec_t::storage_t>(), is_optional<std::remove_cv_t<field_t>>());
        }
    }
    return schema;
}

template <typename F>
auto database_t::encode_field(const F& field) -> cell_t {
    using field_t = std::remove_cv_t<F>;
    if constexpr (is_optional<field_t>()) {
        if (!field) return std::monostate{};
        return encode_field(*field);
    } else {
        using codec_t = column_codec<field_t>;
        using storage_t = typename codec_t::storage_t;
        decltype(auto) encoded = codec_t::encode(field);
        if constexpr (std::same_as<storage_t, double>) {
            if (std::isnan(encoded)) throw std::invalid_argument("database encode: canonical double must not be NaN");
        }
        return cell_t(std::in_place_type<storage_t>, std::forward<decltype(encoded)>(encoded));
    }
}

template <typename T>
auto database_t::encode_record(const T& record) -> std::vector<cell_t> {
    std::vector<cell_t> stored;
    template for (constexpr auto field : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
        stored.push_back(encode_field(record.[:field:]));
    }
    return stored;
}

template <typename F>
std::remove_cv_t<F> database_t::decode_field(cell_t&& stored) {
    using field_t = std::remove_cv_t<F>;
    if constexpr (is_optional<field_t>()) {
        if (std::holds_alternative<std::monostate>(stored)) return std::nullopt;
        using inner_t = std::remove_cv_t<typename field_t::value_type>;
        return field_t(std::in_place, decode_field<inner_t>(std::move(stored)));
    } else {
        using codec_t = column_codec<field_t>;
        return codec_t::decode(std::get<typename codec_t::storage_t>(std::move(stored)));
    }
}

template <typename T, std::size_t... I>
T database_t::decode_record(std::span<cell_t> stored, std::index_sequence<I...>) {
    return T{decode_field<member_t<T, I>>(std::move(stored[I]))...};
}

template <typename T>
void database_t::table_t<T>::insert(const T& record) const {
    const auto stored = database_t::encode_record(record);
    m_database->insert_impl(m_name, m_columns, stored);
}

template <typename T>
std::vector<T> database_t::table_t<T>::rows() const {
    std::vector<T> result;
    m_database->rows_impl(m_name, m_columns, [&result](std::span<cell_t> stored) {
        result.emplace_back(database_t::decode_record<T>(stored, std::make_index_sequence<std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()).size()>{}));
    });
    return result;
}

template <typename T>
template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
std::vector<T> database_t::table_t<T>::rows(const selected_t<T, Member>& match) const {
    const auto encoded = database_t::encode_field(match);
    std::vector<T> result;
    m_database->rows_impl(m_name, m_columns, [&result](std::span<cell_t> stored) {
        result.emplace_back(database_t::decode_record<T>(stored, std::make_index_sequence<std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()).size()>{}));
    }, std::meta::identifier_of(selected_field<T, Member>()), encoded);
    return result;
}

template <typename T>
template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
std::size_t database_t::table_t<T>::replace(const selected_t<T, Member>& match, const T& replacement) const {
    const auto encoded = database_t::encode_field(match);
    const auto stored = database_t::encode_record(replacement);
    return m_database->mutate_impl(m_name, m_columns, std::meta::identifier_of(selected_field<T, Member>()), encoded, stored);
}

template <typename T>
template <auto Member> requires (database_t::selected_field<T, Member>() != std::meta::info{})
std::size_t database_t::table_t<T>::erase(const selected_t<T, Member>& match) const {
    const auto encoded = database_t::encode_field(match);
    return m_database->mutate_impl(m_name, m_columns, std::meta::identifier_of(selected_field<T, Member>()), encoded, {});
}

template <typename T>
database_t::table_t<T>::table_t(database_t& database, std::string_view name):
    m_database(&database),
    m_name(name),
    m_columns(database_t::columns<T>())
{
    m_database->ensure_table(m_name, m_columns);
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

namespace std {

template <typename T>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::column_codec<T>> : formatter<string_view> {
    auto format(const m03h5hm86mnvc18opf1rdttsgh_database::column_codec<T>&, auto& ctx) const {
        auto out = ctx.out();
        out = formatter<string_view>::format("column_codec", ctx);
        return out;
    }
};

template <>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::database_t> : formatter<string_view> {
    auto format(const m03h5hm86mnvc18opf1rdttsgh_database::database_t&, auto& ctx) const {
        auto out = ctx.out();
        out = formatter<string_view>::format("database", ctx);
        return out;
    }
};

template <typename T>
struct formatter<m03h5hm86mnvc18opf1rdttsgh_database::database_t::table_t<T>> : formatter<string_view> {
    auto format(const m03h5hm86mnvc18opf1rdttsgh_database::database_t::table_t<T>&, auto& ctx) const {
        auto out = ctx.out();
        out = formatter<string_view>::format("table", ctx);
        return out;
    }
};

} // namespace std

#endif // M03H5HM86MNVC18OPF1RDTTSGH_DATABASE_DATABASE_H
