#include "doctest.hpp"

#include <itlib/expected.hpp>

#include <type_traits>
#include <string>

#if defined(_MSC_VER)
#pragma warning(disable : 4244)
#endif

using namespace itlib;

enum class ecode
{
    error_a,
    error_b,
    error_c,
};

TEST_CASE("i/e")
{
    using iee = expected<int, ecode>;
    static_assert(std::is_same<iee::value_type, int>::value, "is_same");
    static_assert(std::is_same<iee::error_type, ecode>::value, "is_same");

    iee a;
    REQUIRE(!!a);
    CHECK(a.has_value());
    CHECK(!a.has_error());
    CHECK(a.value() == 0);
    CHECK(*a == 0);
    CHECK(a.value_or(74) == 0);

    iee b = 123;
    REQUIRE(!!b);
    CHECK(b.has_value());
    CHECK(!b.has_error());
    CHECK(b.value() == 123);
    CHECK(*b == 123);
    CHECK(b.value_or(74) == 123);

    iee e = unexpected(ecode::error_b);
    REQUIRE(!e);
    CHECK(!e.has_value());
    CHECK(e.has_error());
    CHECK(e.error() == ecode::error_b);
    CHECK(e.value_or(74) == 74);
}

TEST_CASE("i/d")
{
    using ide = expected<int, double>;
    static_assert(std::is_same<ide::value_type, int>::value, "is_same");
    static_assert(std::is_same<ide::error_type, double>::value, "is_same");

    ide a = 2.3;
    CHECK(a.value_or(5) == 2);

    ide b = unexpected(7);
    REQUIRE(b.has_error());
}

TEST_CASE("string")
{
    using ise = expected<int, std::string>;
    static_assert(std::is_same<ise::value_type, int>::value, "is_same");
    static_assert(std::is_same<ise::error_type, std::string>::value, "is_same");

    ise a = 432;
    CHECK(!!a);

    ise b = unexpected("foo");
    REQUIRE(!b);
    CHECK(b.error() == "foo");

    ise u = unexpected_t<void>{};
    REQUIRE(!u);
    CHECK(u.error().empty());

    using sie = expected<std::string, int>;
    static_assert(std::is_same<sie::value_type, std::string>::value, "is_same");
    static_assert(std::is_same<sie::error_type, int>::value, "is_same");

    sie c;
    REQUIRE(c.has_value());
    CHECK(c->empty());

    sie d = unexpected(23);
    REQUIRE(d.has_error());

    sie u2 = itlib::unexpected();
    REQUIRE(!u2);
    CHECK(u2.error() == 0);
}


// tracks lifetime
template <typename C>
struct track_base
{
    track_base() { ++def_constructions; }
    track_base(const track_base&) { ++copy_constructions; }
    track_base(track_base&&) { ++move_constructions; }
    track_base& operator=(const track_base&) { ++copy_assignments; return *this; }
    track_base& operator=(track_base&&) { ++move_assignments; return *this; }
    ~track_base() { ++destructions; }
    static void reset_counters()
    {
        def_constructions = 0;
        move_constructions = 0;
        move_assignments = 0;
        destructions = 0;

        // explicitly not cleared
        // should always be zero
        // copy_constructions = 0;
        // copy_assignments = 0;
    }

    static int def_constructions;
    static int copy_constructions;
    static int move_constructions;
    static int copy_assignments;
    static int move_assignments;
    static int destructions;
};
template <typename C> int track_base<C>::def_constructions;
template <typename C> int track_base<C>::copy_constructions;
template <typename C> int track_base<C>::move_constructions;
template <typename C> int track_base<C>::copy_assignments;
template <typename C> int track_base<C>::move_assignments;
template <typename C> int track_base<C>::destructions;

struct value : track_base<value> {};
struct error : track_base<error> {};

using vee = expected<value, error>;
static_assert(std::is_same<vee::value_type, value>::value, "is_same");
static_assert(std::is_same<vee::error_type, error>::value, "is_same");

vee func(bool b)
{
    if(b) return {};
    else return itlib::unexpected();
}

TEST_CASE("lifetime")
{
    {
        const auto x = func(false);
        CHECK(!x);
    }
    CHECK(error::def_constructions == 1);
    CHECK(error::move_constructions == 0);
    CHECK(error::destructions == 1);
    CHECK(value::def_constructions == 0);
    CHECK(value::move_constructions == 0);
    CHECK(value::destructions == 0);

    {
        const auto x = func(true);
        CHECK(!!x);
    }

    CHECK(error::def_constructions == 1);
    CHECK(error::move_constructions == 0);
    CHECK(error::destructions == 1);
    CHECK(value::def_constructions == 1);
    CHECK(value::move_constructions == 0);
    CHECK(value::destructions == 1);

    value::reset_counters();
    error::reset_counters();

    {
        auto x = func(false);
        x = func(false);
        REQUIRE(x.has_error());
        CHECK(error::def_constructions == 2);
        CHECK(error::move_constructions == 0);
        CHECK(error::move_assignments == 1);
        CHECK(error::destructions == 1);

        auto e = func(false).error();
        CHECK(error::def_constructions == 3);
        CHECK(error::move_constructions == 1);
        CHECK(error::move_assignments == 1);
        CHECK(error::destructions == 2);
    }

    CHECK(error::destructions == 4);

    CHECK(value::def_constructions == 0);
    CHECK(value::move_constructions == 0);
    CHECK(value::move_assignments == 0);
    CHECK(value::destructions == 0);

    error::reset_counters();

    {
        auto x = func(true);
        x = func(true);
        REQUIRE(x.has_value());
        CHECK(value::def_constructions == 2);
        CHECK(value::move_constructions == 0);
        CHECK(value::move_assignments == 1);
        CHECK(value::destructions == 1);

        auto e = func(true).value();
        CHECK(value::def_constructions == 3);
        CHECK(value::move_constructions == 1);
        CHECK(value::move_assignments == 1);
        CHECK(value::destructions == 2);
    }

    CHECK(value::destructions == 4);

    CHECK(error::def_constructions == 0);
    CHECK(error::move_constructions == 0);
    CHECK(error::move_assignments == 0);
    CHECK(error::destructions == 0);

    value::reset_counters();

    // value to error
    {
        auto x = func(false);
        x = func(true);
        CHECK(x.has_value());
    }

    CHECK(error::def_constructions == 1);
    CHECK(error::move_constructions == 0);
    CHECK(error::move_assignments == 0);
    CHECK(error::destructions == 1);
    CHECK(value::def_constructions == 1);
    CHECK(value::move_constructions == 1);
    CHECK(value::move_assignments == 0);
    CHECK(value::destructions == 2);

    value::reset_counters();
    error::reset_counters();

    // error to value
    {
        auto x = func(true);
        x = func(false);
        CHECK(x.has_error());
    }

    CHECK(error::def_constructions == 1);
    CHECK(error::move_constructions == 1);
    CHECK(error::move_assignments == 0);
    CHECK(error::destructions == 2);
    CHECK(value::def_constructions == 1);
    CHECK(value::move_constructions == 0);
    CHECK(value::move_assignments == 0);
    CHECK(value::destructions == 1);

    value::reset_counters();
    error::reset_counters();

    CHECK(value::copy_constructions == 0);
    CHECK(value::copy_assignments == 0);
    CHECK(error::copy_constructions == 0);
    CHECK(error::copy_assignments == 0);
}
