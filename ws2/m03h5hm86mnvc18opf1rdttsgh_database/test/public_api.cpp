#include <m03h5hm86mnvc18opf1rdttsgh_database/database.h>

#include <m03gagbhsnusi43zogoacgj2ez_filesystem/filesystem.h>
#include <m03gagbht2l61mj6qitacwbmea_byte_stream/byte_stream.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace m03h5hm86mnvc18opf1rdttsgh_database {

using bytes_t = m03gagbht2l61mj6qitacwbmea_byte_stream::byte_stream_t;

struct person_t {
    std::int64_t id;
    std::string name;
    std::optional<int> age;
};

class token_t {
public:
    explicit token_t(std::string text): m_text(std::move(text)) {
        if (m_text == "construction error") throw std::domain_error("token construction failed");
    }
    token_t(token_t&&) = default;
    token_t(const token_t&) = delete;
    token_t& operator=(token_t&&) = delete;
    const std::string& text() const { return m_text; }
private:
    std::string m_text;
};

template <>
struct column_codec<token_t> {
    using storage_t = std::string;
    static const storage_t& encode(const token_t& token) { return token.text(); }
    static token_t decode(storage_t text) { return token_t(std::move(text)); }
};

struct owned_row_t {
    const std::int64_t id;
    token_t token;
    std::optional<token_t> alias;
    std::optional<bytes_t> photo;
};

struct nested_t { int number; };
template <>
struct column_codec<nested_t> {
    using storage_t = std::int64_t;
    static storage_t encode(const nested_t& nested) { return nested.number; }
    static nested_t decode(storage_t number) { return nested_t{column_codec<int>::decode(number)}; }
};

template <>
struct column_codec<std::vector<std::int64_t>> {
    using storage_t = std::string;
    static storage_t encode(const std::vector<std::int64_t>& numbers) {
        storage_t text;
        for (const auto number : numbers) text += std::format("{} ", number);
        return text;
    }
    static std::vector<std::int64_t> decode(storage_t text) {
        std::vector<std::int64_t> numbers;
        while (!text.empty()) {
            const auto end = text.find(' ');
            numbers.push_back(std::stoll(text.substr(0, end)));
            text.erase(0, end + 1);
        }
        return numbers;
    }
};

struct structured_row_t { nested_t nested; std::vector<std::int64_t> numbers; };
struct default_row_t { std::int64_t number = 42; std::string label = "C++ default"; };
struct text_row_t { std::string token; };
struct nullable_text_row_t { std::optional<std::string> token; };
struct token_row_t { token_t token; };
template <typename T>
struct scalar_row_t { T number; };
struct nan_t {};
template <>
struct column_codec<nan_t> {
    using storage_t = double;
    static storage_t encode(const nan_t&) { return std::numeric_limits<double>::quiet_NaN(); }
    static nan_t decode(storage_t) { return {}; }
};
struct nan_row_t { nan_t number; };
struct codec_error_t : std::exception {};
struct guarded_t { std::int64_t number; };
template <>
struct column_codec<guarded_t> {
    using storage_t = std::int64_t;
    static storage_t encode(const guarded_t& guarded) {
        if (guarded.number == -1) throw codec_error_t{};
        return guarded.number;
    }
    static guarded_t decode(storage_t number) {
        if (number == -2) throw codec_error_t{};
        if (number == -3) throw std::runtime_error("custom codec runtime exception");
        if (number == -4) throw std::bad_alloc();
        return guarded_t{number};
    }
};
struct guarded_row_t { guarded_t number; };
struct integer_row_t { std::int64_t number; };
struct small_row_t { std::int8_t number; };
struct unsigned_row_t { std::uint64_t number; };
struct boolean_row_t { bool number; };
struct real_row_t { double number; };
struct float_row_t { float number; };
struct optional_row_t { std::optional<std::int64_t> number; };
struct other_row_t { std::int64_t other; };
struct reordered_person_t { std::optional<int> age; std::string name; std::int64_t id; };
struct selector_probe_t {
    static int number;
    int function() const;
private:
    int hidden;
public:
    static constexpr auto hidden_member() { return &selector_probe_t::hidden; }
};
struct normalized_t { int number; bool operator==(const normalized_t&) const = default; };
template <>
struct column_codec<normalized_t> {
    using storage_t = std::int64_t;
    static storage_t encode(const normalized_t& normalized) { return std::abs(normalized.number); }
    static normalized_t decode(storage_t number) { return {column_codec<int>::decode(number)}; }
};
struct normalized_row_t { normalized_t number; };

template <typename H, auto Member, typename F>
concept has_matching = requires(const H& handle, const F& field, const person_t& person) {
    { handle.template rows<Member>(field) } -> std::same_as<std::vector<person_t>>;
    { handle.template replace<Member>(field, person) } -> std::same_as<std::size_t>;
    { handle.template erase<Member>(field) } -> std::same_as<std::size_t>;
};
template <typename D>
concept has_old_table = requires(D& database) { database.template table<person_t>("people"); };
template <typename H>
concept has_handle_drop = requires(const H& handle) { handle.drop(); };

template <typename D>
concept has_create_table = requires(D& database) { database.template create_table<person_t>("people"); };
template <typename D>
concept has_open_table = requires(D& database) { database.template open_table<person_t>("people"); };
template <typename D>
concept has_database_insert = requires(D& database, const person_t& person) { database.insert("people", person); };
template <typename D>
concept has_database_rows = requires(D& database) { database.template rows<person_t>("people"); };

template <typename E, typename F>
void expect_exception(F&& action, std::string_view message = {}) {
    bool caught = false;
    try {
        action();
    } catch (const E& error) {
        caught = true;
        if (!message.empty()) assert(std::string_view(error.what()).find(message) != std::string_view::npos);
    }
    assert(caught);
}

template <std::integral T>
void integral_codec() {
    database_t database;
    const auto integral_table = database.create_table<scalar_row_t<T>>("integral");
    for (const T number : { T{}, std::numeric_limits<T>::min(), std::numeric_limits<T>::max() }) {
        if constexpr (std::numeric_limits<std::int64_t>::digits < std::numeric_limits<T>::digits) {
            if (!std::in_range<std::int64_t>(number)) {
                expect_exception<std::out_of_range>([&] { (void)column_codec<T>::encode(number); });
                continue;
            }
        }
        assert(column_codec<T>::decode(column_codec<T>::encode(number)) == number);
        integral_table.insert(scalar_row_t<T>{number});
    }
    assert(!integral_table.rows().empty());
}

void basic_records() {
    database_t database;
    std::string name = "people \"; --";
    const auto people_table = database.create_table<person_t>(name);
    static_assert(std::same_as<std::remove_cv_t<decltype(people_table)>, table_t<person_t>>);
    static_assert(!std::is_default_constructible_v<table_t<person_t>>);
    name = "different name";
    assert(people_table.rows().empty());
    people_table.insert(person_t{1, std::string("A\0da", 4), 36});
    people_table.insert(person_t{1, std::string("A\0da", 4), 36});
    people_table.insert(person_t{2, "", std::nullopt});
    auto people = people_table.rows();
    assert(people.size() == 3);
    assert(std::ranges::count_if(people, [](const person_t& person) { return person.id == 1 && person.name == std::string("A\0da", 4) && person.age == 36; }) == 2);
    assert(std::ranges::count_if(people, [](const person_t& person) { return person.id == 2 && person.name.empty() && !person.age; }) == 1);
    auto reused = database.create_table<person_t>("people \"; --");
    assert(reused.rows().size() == 3);
    auto copied = reused;
    copied.insert(person_t{3, "copied handle", 4});
    auto moved = std::move(copied);
    assert(moved.rows().size() == 4);
    copied = reused;
    assert(copied.rows().size() == 4);
    static_assert(!std::is_constructible_v<table_t<person_t>, database_t&, std::string_view>);
    expect_exception<std::invalid_argument>([&] { (void)database.create_table<person_t>(""); });
    expect_exception<std::invalid_argument>([&] { (void)database.create_table<person_t>(std::string_view("a\0b", 3)); });
    assert(database.create_table<person_t>("missing").rows().empty());
    people_table.insert(person_t{4, "still valid", 5});
    assert(people_table.rows().size() == 5);
    database_t independent;
    assert(independent.create_table<person_t>("people \"; --").rows().empty());
    assert(std::format("{}", database) == "database");
    assert(std::format("{}", people_table) == "table");
    assert(std::format("{}", column_codec<int>{}) == "column_codec");
    const auto defaults = database.create_table<default_row_t>("defaults");
    defaults.insert(default_row_t{});
    assert(defaults.rows().front().number == 42);
    assert(defaults.rows().front().label == "C++ default");
    const auto nullable_text = database.create_table<nullable_text_row_t>("nullable text");
    nullable_text.insert(nullable_text_row_t{std::string{}});
    nullable_text.insert(nullable_text_row_t{std::nullopt});
    const auto text = nullable_text.rows();
    assert(text.size() == 2);
    assert(std::ranges::count_if(text, [](const nullable_text_row_t& row) { return row.token && row.token->empty(); }) == 1);
    assert(std::ranges::count_if(text, [](const nullable_text_row_t& row) { return !row.token; }) == 1);
}

void owning_records() {
    static_assert(!std::is_default_constructible_v<owned_row_t>);
    static_assert(!std::is_copy_constructible_v<owned_row_t>);
    static_assert(std::is_move_constructible_v<owned_row_t>);
    static_assert(!std::is_move_assignable_v<owned_row_t>);
    std::vector<owned_row_t> result;
    {
        database_t database;
        const auto owned = database.create_table<owned_row_t>("owned");
        owned_row_t record{1, token_t("original"), token_t("alias"), bytes_t(std::vector{std::byte{0}, std::byte{255}})};
        assert(&column_codec<token_t>::encode(record.token) == &record.token.text());
        owned.insert(record);
        owned.insert(owned_row_t{2, token_t("empty"), {}, bytes_t{}});
        owned.insert(owned_row_t{3, token_t("null"), {}, {}});
        // Force vector growth while only move construction is available.
        for (std::int64_t id = 4; id < 70; ++id) owned.insert(owned_row_t{id, token_t("growth"), {}, {}});
        result = owned.rows();
    }
    assert(result.size() == 69);
    const auto original = std::ranges::find_if(result, [](const owned_row_t& row) { return row.id == 1; });
    assert(original->token.text() == "original" && original->alias->text() == "alias");
    assert(original->photo && original->photo->size() == 2 && original->photo->bytes()[1] == std::byte{255});
    const auto empty = std::ranges::find_if(result, [](const owned_row_t& row) { return row.id == 2; });
    const auto null = std::ranges::find_if(result, [](const owned_row_t& row) { return row.id == 3; });
    assert(empty->photo && empty->photo->empty() && !empty->alias);
    assert(!null->photo && !null->alias);
    database_t database;
    const auto structured_table = database.create_table<structured_row_t>("structured");
    structured_table.insert(structured_row_t{{42}, {1, 2, 3}});
    const auto structured = structured_table.rows();
    assert(structured.front().nested.number == 42 && structured.front().numbers == std::vector<std::int64_t>({1, 2, 3}));
}

void matching_records() {
    using handle_t = table_t<person_t>;
    static_assert(has_matching<handle_t, &person_t::id, std::int64_t>);
    static_assert(!has_matching<handle_t, &other_row_t::other, std::int64_t>);
    static_assert(!has_matching<handle_t, static_cast<std::int64_t person_t::*>(nullptr), std::int64_t>);
    static_assert(!has_matching<handle_t, nullptr, std::int64_t>);
    static_assert(!has_matching<handle_t, 1, std::int64_t>);
    static_assert(!has_matching<handle_t, &selector_probe_t::function, std::int64_t>);
    static_assert(!has_matching<handle_t, &selector_probe_t::number, std::int64_t>);
    static_assert(!has_matching<handle_t, selector_probe_t::hidden_member(), std::int64_t>);
    static_assert(!has_matching<handle_t, &person_t::name, int>);
    static_assert(std::same_as<decltype(&handle_t::rows<&person_t::id>), std::vector<person_t> (handle_t::*)(const std::int64_t&) const>);
    static_assert(!has_old_table<database_t> && !has_handle_drop<handle_t>);
    database_t database;
    const auto people = database.create_table<person_t>("people");
    assert(people.rows<&person_t::id>(1).empty());
    assert(people.replace<&person_t::id>(1, person_t{2, "none", {}}) == 0);
    assert(people.erase<&person_t::id>(1) == 0);
    const std::string name("A\0da", 4);
    people.insert(person_t{1, name, 36});
    people.insert(person_t{1, name, 36});
    people.insert(person_t{2, "A", {}});
    assert(people.rows<&person_t::id>(1).size() == 2);
    assert(people.rows<&person_t::name>(name).size() == 2);
    assert(people.rows<&person_t::name>("A").size() == 1);
    assert(people.rows<&person_t::age>(std::nullopt).size() == 1);
    assert(people.replace<&person_t::id>(1, person_t{1, name, 36}) == 2);
    auto detached = people.rows<&person_t::id>(2);
    detached.front().name = "local edit";
    assert(people.rows<&person_t::id>(2).front().name == "A");
    assert(people.replace<&person_t::age>(std::nullopt, detached.front()) == 1);
    assert(people.rows<&person_t::name>("local edit").size() == 1);
    const auto reordered = database.create_table<reordered_person_t>("PEOPLE");
    assert(reordered.rows<&reordered_person_t::id>(1).size() == 2);
    assert(reordered.replace<&reordered_person_t::name>(name, reordered_person_t{37, "updated", 3}) == 2);
    assert(people.rows<&person_t::id>(1).empty());
    const auto changed = people.rows<&person_t::id>(3);
    assert(changed.size() == 2 && changed.front().name == "updated" && changed.front().age == 37);
    assert(people.erase<&person_t::id>(3) == 2);
    assert(people.erase<&person_t::name>("local edit") == 1);
    assert(people.rows().empty());

    const auto owned = database.create_table<owned_row_t>("owned matching");
    const bytes_t bytes(std::vector{std::byte{0}, std::byte{255}, std::byte{0}});
    owned.insert(owned_row_t{1, token_t(name), token_t("alias"), bytes});
    owned.insert(owned_row_t{2, token_t("other"), {}, bytes_t{}});
    owned.insert(owned_row_t{3, token_t("absent"), {}, {}});
    assert(owned.rows<&owned_row_t::token>(token_t(name)).size() == 1);
    assert(owned.rows<&owned_row_t::photo>(bytes).size() == 1);
    assert(owned.rows<&owned_row_t::photo>(bytes_t{}).size() == 1);
    assert(owned.rows<&owned_row_t::photo>(std::nullopt).size() == 1);
    assert(owned.replace<&owned_row_t::alias>(std::nullopt, owned_row_t{4, token_t("replacement"), {}, bytes}) == 2);
    assert(owned.rows<&owned_row_t::id>(4).size() == 2);
    assert(owned.erase<&owned_row_t::photo>(bytes) == 3);
    assert(owned.rows().empty());

    const auto normalized = database.create_table<normalized_row_t>("encoded equality");
    assert(normalized_t{7} != normalized_t{-7});
    normalized.insert(normalized_row_t{{7}});
    normalized.insert(normalized_row_t{{-7}});
    assert(normalized.rows<&normalized_row_t::number>(normalized_t{-7}).size() == 2);
    assert(normalized.replace<&normalized_row_t::number>(normalized_t{-7}, normalized_row_t{{-8}}) == 2);
    assert(normalized.erase<&normalized_row_t::number>(normalized_t{8}) == 2);

    const auto reals = database.create_table<real_row_t>("numeric equality");
    reals.insert(real_row_t{-0.0});
    reals.insert(real_row_t{0.0});
    reals.insert(real_row_t{std::numeric_limits<double>::infinity()});
    assert(reals.rows<&real_row_t::number>(0.0).size() == 2);
    assert(reals.erase<&real_row_t::number>(std::numeric_limits<double>::infinity()) == 1);
    assert(reals.replace<&real_row_t::number>(0.0, real_row_t{1.5}) == 2);
    assert(reals.rows<&real_row_t::number>(1.5).size() == 2);
}

void numeric_records() {
    integral_codec<bool>();
    integral_codec<char>();
    integral_codec<signed char>();
    integral_codec<unsigned char>();
    integral_codec<wchar_t>();
    integral_codec<char8_t>();
    integral_codec<char16_t>();
    integral_codec<char32_t>();
    integral_codec<short>();
    integral_codec<unsigned short>();
    integral_codec<int>();
    integral_codec<unsigned>();
    integral_codec<long>();
    integral_codec<unsigned long>();
    integral_codec<long long>();
    integral_codec<unsigned long long>();
    const auto maximum = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    assert(column_codec<std::uint64_t>::encode(maximum) == std::numeric_limits<std::int64_t>::max());
    expect_exception<std::out_of_range>([&] { (void)column_codec<std::uint64_t>::encode(maximum + 1); });
    expect_exception<std::out_of_range>([] { (void)column_codec<unsigned char>::decode(256); });
    expect_exception<std::out_of_range>([] { (void)column_codec<signed char>::decode(-129); });
    expect_exception<std::out_of_range>([] { (void)column_codec<bool>::decode(-1); });
    expect_exception<std::out_of_range>([] { (void)column_codec<bool>::decode(2); });
    expect_exception<std::out_of_range>([] { (void)column_codec<float>::decode(-std::numeric_limits<double>::max()); });
    database_t database;
    const auto numbers = database.create_table<integer_row_t>("numbers");
    numbers.insert(integer_row_t{std::numeric_limits<std::int64_t>::min()});
    numbers.insert(integer_row_t{std::numeric_limits<std::int64_t>::max()});
    expect_exception<std::out_of_range>([&] { database.create_table<unsigned_row_t>("numbers").insert(unsigned_row_t{std::numeric_limits<std::uint64_t>::max()}); });
    expect_exception<std::out_of_range>([&] { (void)database.create_table<small_row_t>("numbers").rows(); });
    expect_exception<std::out_of_range>([&] { (void)database.create_table<unsigned_row_t>("numbers").rows(); });
    expect_exception<std::out_of_range>([&] { (void)database.create_table<boolean_row_t>("numbers").rows(); });
    numbers.insert(integer_row_t{0});
    numbers.insert(integer_row_t{1});
    assert(numbers.rows().size() == 4);
    expect_exception<std::runtime_error>([&] { (void)database.create_table<real_row_t>("numbers").rows(); }, "expected double");
    const auto boolean_table = database.create_table<boolean_row_t>("bools");
    boolean_table.insert(boolean_row_t{true});
    boolean_table.insert(boolean_row_t{false});
    const auto bools = boolean_table.rows();
    assert(bools.size() == 2 && bools[0].number != bools[1].number);
    const auto reals = database.create_table<real_row_t>("reals");
    reals.insert(real_row_t{1.0 / 3.0});
    reals.insert(real_row_t{std::numeric_limits<double>::denorm_min()});
    reals.insert(real_row_t{std::numeric_limits<double>::infinity()});
    reals.insert(real_row_t{-std::numeric_limits<double>::infinity()});
    expect_exception<std::invalid_argument>([&] { reals.insert(real_row_t{std::numeric_limits<double>::quiet_NaN()}); });
    expect_exception<std::invalid_argument>([&] { database.create_table<float_row_t>("reals").insert(float_row_t{std::numeric_limits<float>::quiet_NaN()}); });
    expect_exception<std::invalid_argument>([&] { database.create_table<nan_row_t>("reals").insert(nan_row_t{}); });
    const auto rounded = database.create_table<float_row_t>("reals").rows();
    assert(std::ranges::any_of(rounded, [](const float_row_t& row) { return row.number == static_cast<float>(1.0 / 3.0); }));
    assert(std::ranges::any_of(rounded, [](const float_row_t& row) { return row.number == 0; }));
    assert(std::ranges::any_of(rounded, [](const float_row_t& row) { return std::isinf(row.number) && !std::signbit(row.number); }));
    assert(std::ranges::any_of(rounded, [](const float_row_t& row) { return std::isinf(row.number) && std::signbit(row.number); }));
    reals.insert(real_row_t{std::numeric_limits<double>::max()});
    expect_exception<std::out_of_range>([&] { (void)database.create_table<float_row_t>("reals").rows(); });
    assert(reals.rows().size() == 5);
}

void exceptional_records() {
    database_t database;
    const auto guarded = database.create_table<guarded_row_t>("guarded");
    expect_exception<codec_error_t>([&] { guarded.insert(guarded_row_t{{-1}}); });
    guarded.insert(guarded_row_t{{1}});
    expect_exception<std::runtime_error>([&] { database.create_table<other_row_t>("guarded").insert(other_row_t{2}); }, "unexpected column");
    expect_exception<std::runtime_error>([&] { (void)database.create_table<optional_row_t>("guarded").rows(); }, "nullable");
    guarded.insert(guarded_row_t{{2}});
    assert(guarded.rows().size() == 2);
    expect_exception<codec_error_t>([&] { (void)guarded.rows<&guarded_row_t::number>(guarded_t{-1}); });
    expect_exception<codec_error_t>([&] { (void)guarded.erase<&guarded_row_t::number>(guarded_t{-1}); });
    expect_exception<codec_error_t>([&] { (void)guarded.replace<&guarded_row_t::number>(guarded_t{-1}, guarded_row_t{{3}}); });
    expect_exception<codec_error_t>([&] { (void)guarded.replace<&guarded_row_t::number>(guarded_t{1}, guarded_row_t{{-1}}); });
    assert(guarded.rows().size() == 2 && guarded.rows<&guarded_row_t::number>(guarded_t{1}).size() == 1);
    const auto unsigneds = database.create_table<unsigned_row_t>("unsigned matching");
    const auto overflow = std::numeric_limits<std::uint64_t>::max();
    expect_exception<std::out_of_range>([&] { (void)unsigneds.rows<&unsigned_row_t::number>(overflow); });
    expect_exception<std::out_of_range>([&] { (void)unsigneds.replace<&unsigned_row_t::number>(0, unsigned_row_t{overflow}); });
    expect_exception<std::out_of_range>([&] { (void)unsigneds.erase<&unsigned_row_t::number>(overflow); });
    const auto nans = database.create_table<nan_row_t>("nan matching");
    expect_exception<std::invalid_argument>([&] { (void)nans.rows<&nan_row_t::number>(nan_t{}); });
    expect_exception<std::invalid_argument>([&] { (void)nans.replace<&nan_row_t::number>(nan_t{}, nan_row_t{}); });
    expect_exception<std::invalid_argument>([&] { (void)nans.erase<&nan_row_t::number>(nan_t{}); });
    for (const auto number : { -2, -3, -4 }) {
        const auto table = std::format("exception {}", number);
        const auto numbers = database.create_table<integer_row_t>(table);
        numbers.insert(integer_row_t{number});
        if (number == -2) expect_exception<codec_error_t>([&] { (void)database.create_table<guarded_row_t>(table).rows(); });
        if (number == -3) {
            try {
                (void)database.create_table<guarded_row_t>(table).rows();
                assert(false);
            } catch (const std::runtime_error& error) {
                assert(std::string_view(error.what()) == "custom codec runtime exception");
            }
        }
        if (number == -4) expect_exception<std::bad_alloc>([&] { (void)database.create_table<guarded_row_t>(table).rows(); });
        numbers.insert(integer_row_t{7});
        assert(numbers.rows().size() == 2);
    }
    const auto construction = database.create_table<text_row_t>("construction");
    construction.insert(text_row_t{"construction error"});
    expect_exception<std::domain_error>([&] { (void)database.create_table<token_row_t>("construction").rows(); }, "token construction failed");
    construction.insert(text_row_t{"after construction failure"});
    assert(construction.rows().size() == 2);
}

void metadata_table_names() {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-names-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    filesystem::create_directories(directory);
    const auto filename = directory / "names.db";
    constexpr std::array<std::string_view, 10> names = {
        "pragma_table_list",
        "pragma_table_xinfo",
        "pragma_index_list",
        "pragma_foreign_key_list",
        "quotes \"double\" and 'single'; --",
        "sqlite_people",
        "SQLITE_schema",
        "people",
        "table_people",
        "table_sqlite_people"
    };
    std::vector<std::string> expected(names.begin(), names.end());
    std::ranges::sort(expected);
    {
        database_t database;
        assert(database.tables().empty());
        database.drop_table<person_t>("never created");
        assert(database.tables().empty());
        expect_exception<std::invalid_argument>([&] { database.drop_table<person_t>(""); });
        expect_exception<std::invalid_argument>([&] { database.drop_table<person_t>(std::string_view("a\0b", 3)); });
        for (const auto name : names) {
            const auto numbers = database.create_table<integer_row_t>(name);
            numbers.insert(integer_row_t{7});
        }
        const database_t& read_database = database;
        assert(read_database.tables() == expected);
        for (const auto name : names) {
            const auto numbers = database.create_table<integer_row_t>(name);
            assert(numbers.rows().size() == 1 && numbers.rows().front().number == 7);
            database.drop_table<integer_row_t>(name);
            database.drop_table<integer_row_t>(name);
        }
        assert(database.tables().empty());
    }
    expected.push_back("ordinary");
    std::ranges::sort(expected);
    std::vector<std::string> owned_names;
    {
        database_t database(filename);
        const auto ordinary = database.create_table<integer_row_t>("ordinary");
        ordinary.insert(integer_row_t{1});
        for (std::size_t index = 0; index < names.size(); ++index) {
            const auto numbers = database.create_table<integer_row_t>(names[index]);
            assert(numbers.rows().empty());
            numbers.insert(integer_row_t{static_cast<std::int64_t>(index + 1)});
            const auto reused = database.create_table<integer_row_t>(names[index]);
            const auto rows = reused.rows();
            assert(rows.size() == 1 && rows.front().number == static_cast<std::int64_t>(index + 1));
        }
        for (std::size_t index = 0; index < names.size(); ++index) {
            const auto rows = database.create_table<integer_row_t>(names[index]).rows();
            assert(rows.size() == 1 && rows.front().number == static_cast<std::int64_t>(index + 1));
        }
        ordinary.insert(integer_row_t{2});
        assert(ordinary.rows().size() == 2);
        owned_names = database.tables();
        assert(owned_names == expected);
    }
    assert(owned_names == expected);
    {
        database_t database(filename);
        assert(database.tables() == expected);
        for (std::size_t index = 0; index < names.size(); ++index) {
            const auto numbers = database.create_table<integer_row_t>(names[index]);
            const auto persisted = static_cast<std::int64_t>(index + 1);
            const auto rows = numbers.rows();
            assert(rows.size() == 1 && rows.front().number == persisted);
            numbers.insert(integer_row_t{persisted + 10});
            std::string upper_name(names[index]);
            for (auto& character : upper_name) {
                if ('a' <= character && character <= 'z') character += 'A' - 'a';
            }
            const auto updated = database.create_table<integer_row_t>(upper_name).rows();
            assert(updated.size() == 2);
            assert(std::ranges::count_if(updated, [&](const integer_row_t& row) { return row.number == persisted; }) == 1);
            assert(std::ranges::count_if(updated, [&](const integer_row_t& row) { return row.number == persisted + 10; }) == 1);
            assert(database.tables() == expected);
        }
        const auto ordinary = database.create_table<integer_row_t>("ordinary");
        assert(ordinary.rows().size() == 2);
        ordinary.insert(integer_row_t{3});
        assert(ordinary.rows().size() == 3);
    }
    for (const auto name : names) {
        database_t database(filename);
        const auto numbers = database.create_table<integer_row_t>(name);
        database.drop_table<integer_row_t>(name);
        database.drop_table<integer_row_t>(name);
    }
    { database_t database(filename); assert(database.tables() == std::vector<std::string>{"ordinary"}); }
    filesystem::remove_all(directory);
}

void file_records() {
    namespace filesystem = m03gagbhsnusi43zogoacgj2ez_filesystem;
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-api-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    filesystem::create_directories(directory);
    const auto filename = directory / ":memory:";
    {
        database_t database(filename);
        const auto people = database.create_table<person_t>("people");
        people.insert(person_t{9, "persisted", {}});
    }
    assert(filesystem::is_directory(filename));
    {
        const auto relative = filename.lexically_relative(filesystem::current_path().to_native_path());
        database_t database(relative);
        assert(database.create_table<person_t>("people").rows().front().name == "persisted");
    }
    // Copy complete opaque storage only after every connection is closed.
    const auto copied_directory = std::filesystem::path(directory.string() + "-copy");
    std::filesystem::copy(filename, copied_directory, std::filesystem::copy_options::recursive);
    {
        database_t copied_database(copied_directory);
        assert(copied_database.create_table<person_t>("people").rows().front().name == "persisted");
    }
    const auto moved_directory = std::filesystem::path(directory.string() + "-moved");
    filesystem::rename_strict(copied_directory, moved_directory);
    {
        database_t moved_database(moved_directory);
        assert(moved_database.create_table<person_t>("people").rows().front().name == "persisted");
    }
    filesystem::remove_all(moved_directory);
    const auto empty_directory = directory / "empty";
    filesystem::create_directories(empty_directory);
    { database_t database(empty_directory); database.create_table<integer_row_t>("created").insert(integer_row_t{1}); }
    { database_t database(empty_directory); assert(database.create_table<integer_row_t>("created").rows().size() == 1); }
    expect_exception<std::invalid_argument>([] { database_t database(std::filesystem::path{}); });
    expect_exception<std::invalid_argument>([] { database_t database(std::filesystem::path(std::string("bad\0file", 8))); });
    expect_exception<std::runtime_error>([&] { database_t database(directory / "missing" / "file.db"); }, "open");
    filesystem::remove_all(directory);
}

void isolated_storage() {
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-isolation-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::create_directory(directory);
    constexpr std::array<std::string_view, 5> names = {
        "people", "people-wal", "people-shm", "people-journal", "file:literal?mode=memory"
    };
    const auto sentinel = directory / "unrelated.txt";
    { std::ofstream output(sentinel); output << "neighbor remains unchanged"; }
    {
        std::vector<std::unique_ptr<database_t>> databases;
        for (std::size_t index = 0; index < names.size(); ++index) {
            databases.push_back(std::make_unique<database_t>(directory / names[index]));
            databases.back()->create_table<integer_row_t>("sqlite_people").insert(integer_row_t{static_cast<std::int64_t>(index)});
        }
        for (std::size_t index = 0; index < names.size(); ++index) {
            const auto rows = databases[index]->create_table<integer_row_t>("SQLITE_PEOPLE").rows();
            assert(rows.size() == 1 && rows.front().number == static_cast<std::int64_t>(index));
        }
        // Backend activity must add nothing beside the dedicated storage directories.
        std::size_t count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            ++count;
            assert(entry.path() == sentinel || std::ranges::find(names, entry.path().filename().string()) != names.end());
        }
        assert(count == names.size() + 1);
    }
    for (std::size_t index = 0; index < names.size(); ++index) {
        database_t database(directory / names[index]);
        const auto rows = database.create_table<integer_row_t>("sqlite_people").rows();
        assert(rows.size() == 1 && rows.front().number == static_cast<std::int64_t>(index));
    }
    std::ifstream input(sentinel);
    std::string text;
    std::getline(input, text);
    assert(text == "neighbor remains unchanged");
    input.close();
    std::filesystem::remove_all(directory);
}

void unrelated_storage() {
    const auto directory = std::filesystem::temp_directory_path() / std::format("database-unrelated-{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::create_directory(directory);
    const auto filename = directory / "existing-file";
    { std::ofstream output(filename); output << "existing content"; }
    expect_exception<std::runtime_error>([&] { database_t database(filename); }, "storage directory");
    expect_exception<std::runtime_error>([&] { database_t database(directory); }, "open");
    const auto storage = directory / "recognized";
    for (const auto suffix : { "-wal", "-shm", "-journal" }) {
        std::ofstream(storage.string() + suffix) << "unrelated suffix neighbor";
    }
    { database_t database(storage); database.create_table<integer_row_t>("records").insert(integer_row_t{7}); }
    const auto extra = storage / "notes.txt";
    { std::ofstream output(extra); output << "unrelated content"; }
    expect_exception<std::runtime_error>([&] { database_t database(storage); }, "unrecognized storage entry");
    std::ifstream input(extra);
    std::string text;
    std::getline(input, text);
    assert(text == "unrelated content");
    input.close();
    std::filesystem::remove(extra);
    { database_t database(storage); assert(database.create_table<integer_row_t>("records").rows().front().number == 7); }
    for (const auto suffix : { "-wal", "-shm", "-journal" }) {
        std::ifstream neighbor(storage.string() + suffix);
        std::getline(neighbor, text);
        assert(text == "unrelated suffix neighbor");
    }
    std::ifstream original(filename);
    std::getline(original, text);
    assert(text == "existing content");
    original.close();
    std::filesystem::remove_all(directory);
}

} // namespace m03h5hm86mnvc18opf1rdttsgh_database

int main() {
    using namespace m03h5hm86mnvc18opf1rdttsgh_database;
    static_assert(!std::is_copy_constructible_v<database_t> && !std::is_move_constructible_v<database_t>);
    static_assert(std::is_nothrow_destructible_v<database_t>);
    static_assert(has_create_table<database_t> && !has_open_table<database_t>);
    static_assert(!has_database_insert<database_t> && !has_database_rows<database_t>);
    static_assert(std::is_copy_constructible_v<table_t<person_t>> && std::is_move_constructible_v<table_t<person_t>>);
    static_assert(std::is_nothrow_destructible_v<table_t<person_t>>);
    basic_records();
    owning_records();
    matching_records();
    numeric_records();
    exceptional_records();
    metadata_table_names();
    file_records();
    isolated_storage();
    unrelated_storage();
}
