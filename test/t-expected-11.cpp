#include <doctest/doctest.h>

#include <itlib/expected.hpp>
#include <doctest/util/lifetime_counter.hpp>

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

    const std::string fallback = "asdf";
    CHECK(u2.value_or(fallback) == fallback);
    CHECK(sie(itlib::unexpected()).value_or(fallback) == fallback);
    CHECK(sie(itlib::unexpected()).value_or("mnp") == "mnp");
    CHECK(sie("xyz").value_or(fallback) == "xyz");
    CHECK(sie("xyz").value_or("mnp") == "xyz");
}

struct value : doctest::util::lifetime_counter<value> {};
struct error : doctest::util::lifetime_counter<error> {};

using vee = expected<value, error>;
static_assert(std::is_same<vee::value_type, value>::value, "is_same");
static_assert(std::is_same<vee::error_type, error>::value, "is_same");

vee func(bool b)
{
    if (b) return {};
    else return itlib::unexpected();
}

TEST_CASE("lifetime")
{
    using namespace doctest::util;

    lifetime_counter_sentry vsentry(value::root_lifetime_stats()), esentry(error::root_lifetime_stats());

    impl::lifetime_stats empty;
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        const auto x = func(false);
        CHECK(!x);
        CHECK(vs == empty);
        CHECK(es.d_ctr == 1);
        CHECK(es.living == 1);
        CHECK(es.total == 1);
    }


    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        const auto x = func(true);
        CHECK(es == empty);
        CHECK(vs.d_ctr == 1);
        CHECK(vs.living == 1);
        CHECK(vs.total == 1);
    }

    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        auto x = func(false);
        x = func(false);
        REQUIRE(x.has_error());
        CHECK(es.d_ctr == 2);
        CHECK(es.m_ctr == 0);
        CHECK(es.m_asgn == 1);
        CHECK(es.living == 1);

        auto e = func(false).error();
        CHECK(es.d_ctr == 3);
        CHECK(es.m_ctr == 1);
        CHECK(es.m_asgn == 1);
        CHECK(es.living == 2);
        CHECK(es.total == 4);

        CHECK(vs == empty);
    }

    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        auto x = func(true);
        x = func(true);
        REQUIRE(x.has_value());
        CHECK(vs.d_ctr == 2);
        CHECK(vs.m_ctr == 0);
        CHECK(vs.m_asgn == 1);
        CHECK(vs.living == 1);

        auto v = func(true).value();
        CHECK(vs.d_ctr == 3);
        CHECK(vs.m_ctr == 1);
        CHECK(vs.m_asgn == 1);
        CHECK(vs.living == 2);
        CHECK(vs.total == 4);

        CHECK(es == empty);
    }


    // error to value
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        auto x = func(false);
        x = func(true);
        CHECK(x.has_value());
        CHECK(es.d_ctr == 1);
        CHECK(es.total == 1);
        CHECK(es.living == 0);
        CHECK(vs.d_ctr == 1);
        CHECK(vs.m_ctr == 1);
        CHECK(vs.total == 2);
        CHECK(vs.living == 1);
    }

    // value to error
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        auto x = func(true);
        x = func(false);
        CHECK(x.has_error());
        CHECK(vs.d_ctr == 1);
        CHECK(vs.total == 1);
        CHECK(vs.living == 0);
        CHECK(es.d_ctr == 1);
        CHECK(es.m_ctr == 1);
        CHECK(es.total == 2);
        CHECK(es.living == 1);
    }
}
