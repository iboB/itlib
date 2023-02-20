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

struct value : doctest::util::lifetime_counter<value>
{
    value() = default;
    explicit value(int i) : val(i) {}
    int val = 0;
};
struct error : doctest::util::lifetime_counter<error>
{
    int err = 1;
};

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

    lifetime_stats empty;
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        const auto x = func(false);
        CHECK(!x);
        CHECK(vs.checkpoint() == empty);
        CHECK(es.d_ctr == 1);
        CHECK(es.living == 1);
        CHECK(es.total == 1);
    }


    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        const auto x = func(true);
        CHECK(es.checkpoint() == empty);
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
        CHECK(x.error().err == 1);
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

        CHECK(vs.checkpoint() == empty);
    }

    {
        value::lifetime_stats vs;
        error::lifetime_stats es;

        auto x = func(true);
        x = func(true);
        REQUIRE(x.has_value());
        REQUIRE(x->val == 0);
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

        CHECK(es.checkpoint() == empty);
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

struct obj {
    expected<value&, error> iv() { return v; }
    expected<value&, error> ie() { return unexpected(error()); }
    expected<const value&, error> iv() const { return v; }
    expected<const value&, error> ie() const { return itlib::unexpected(); }

    value v{5};
};

TEST_CASE("ref basic")
{
    {
        obj o;
        auto v = o.iv();
        CHECK(!!v);
        CHECK(v.has_value());
        CHECK_FALSE(v.has_error());
        CHECK(v->val == 5);
        value x{45};
        CHECK(v.value_or(x).val == 5);

        v->val = 28;
        CHECK(o.v.val == 28);
        CHECK(v.value_or(x).val == 28);

        v = o.ie();
        CHECK_FALSE(v);
        CHECK_FALSE(v.has_value());
        REQUIRE(v.has_error());
        CHECK(v.error().err == 1);
        CHECK(v.value_or(x).val == 45);
    }

    {
        const obj o;
        auto v = o.iv();
        CHECK(!!v);
        CHECK(v->val == 5);
        CHECK(v.value_or(value(542)).val == 5);

        v = o.ie();
        CHECK(!v);
        CHECK(v.value_or(value(45)).val == 45);
    }
}

TEST_CASE("ref lifetime")
{
    using namespace doctest::util;

    lifetime_counter_sentry vsentry(value::root_lifetime_stats()), esentry(error::root_lifetime_stats());

    lifetime_stats empty;
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        obj o;

        const auto x = o.ie();
        CHECK(!x);
        CHECK(vs.total == 1);
        CHECK(vs.living == 1);
        CHECK(es.d_ctr == 1);
        CHECK(es.living == 1);
        CHECK(es.total == 3);
    }


    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        const obj o;

        const auto x = o.iv();
        CHECK(es.checkpoint() == empty);
        CHECK(vs.living == 1);
        CHECK(vs.total == 1);
    }

    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        const obj o;

        auto x = o.ie();
        x = o.ie();
        REQUIRE(x.has_error());
        CHECK(x.error().err == 1);
        CHECK(es.d_ctr == 2);
        CHECK(es.m_ctr == 0);
        CHECK(es.m_asgn == 1);
        CHECK(es.living == 1);

        auto e = o.ie().error();
        CHECK(es.d_ctr == 3);
        CHECK(es.m_ctr == 1);
        CHECK(es.m_asgn == 1);
        CHECK(es.living == 2);
        CHECK(es.total == 4);

        CHECK(vs.living == 1);
        CHECK(vs.total == 1);
    }

    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        obj o;

        auto x = o.iv();
        x = o.iv();
        REQUIRE(x.has_value());
        CHECK(x->val == 5);
        CHECK(vs.living == 1);
        CHECK(vs.total == 1);

        CHECK(es.checkpoint() == empty);
    }


    // error to value
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        const obj o;

        auto x = o.ie();
        x = o.iv();

        CHECK(x.has_value());
        CHECK(es.d_ctr == 1);
        CHECK(es.total == 1);
        CHECK(es.living == 0);
        CHECK(vs.total == 1);
        CHECK(vs.living == 1);
    }

    // value to error
    {
        value::lifetime_stats vs;
        error::lifetime_stats es;
        const obj o;

        auto x = o.iv();
        x = o.ie();
        CHECK(x.has_error());
        CHECK(vs.d_ctr == 1);
        CHECK(vs.total == 1);
        CHECK(vs.living == 1);
        CHECK(es.d_ctr == 1);
        CHECK(es.m_ctr == 1);
        CHECK(es.total == 2);
        CHECK(es.living == 1);
    }
}

itlib::expected<void, error> vfunc(bool b)
{
    if (b) return {};
    else return itlib::unexpected();
}

TEST_CASE("void basic")
{
    itlib::expected<void, int> x = itlib::unexpected(5);
    CHECK_FALSE(x);
    CHECK(x.has_error());
    CHECK(x.error() == 5);

    itlib::expected<void, int> y;
    CHECK(y);
    CHECK(y.has_value());

    auto v = vfunc(true);
    CHECK(v.has_value());
    CHECK(!v.has_error());
    CHECK(!!v);

    v = vfunc(false);
    CHECK(!v);
    REQUIRE(v.has_error());
    CHECK(v.error().err == 1);
}

TEST_CASE("void lifetime")
{
    using namespace doctest::util;

    lifetime_counter_sentry esentry(error::root_lifetime_stats());

    lifetime_stats empty;
    {
        error::lifetime_stats es;

        const auto x = vfunc(false);
        CHECK(!x);
        CHECK(es.d_ctr == 1);
        CHECK(es.living == 1);
        CHECK(es.total == 1);
    }


    {
        error::lifetime_stats es;

        const auto x = func(true);
        CHECK(es.checkpoint() == empty);
    }

    {
        error::lifetime_stats es;

        auto x = func(false);
        x = func(false);
        REQUIRE(x.has_error());
        CHECK(x.error().err == 1);
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
    }

    {
        error::lifetime_stats es;

        auto x = func(true);
        x = func(true);
        CHECK(es.checkpoint() == empty);
    }


    // error to value
    {
        error::lifetime_stats es;
        auto x = func(false);
        x = func(true);
        CHECK(x.has_value());
        CHECK(es.d_ctr == 1);
        CHECK(es.total == 1);
        CHECK(es.living == 0);
    }

    // value to error
    {
        error::lifetime_stats es;
        auto x = func(true);
        x = func(false);
        CHECK(x.has_error());
        CHECK(es.d_ctr == 1);
        CHECK(es.m_ctr == 1);
        CHECK(es.total == 2);
        CHECK(es.living == 1);
    }
}

TEST_CASE("eoptional")
{
    itlib::eoptional<std::string> so;
    CHECK(so);
    CHECK(so.has_value());
    CHECK(!so.has_error());

    *so = "xx";
    CHECK(so->length() == 2);

    auto cp = so;
    CHECK(cp);
    CHECK(cp.value_or("yy") == "xx");

    *so = "very long string which is guaranteed to exceed small buf";
    auto ptr = so->data();
    auto mv = std::move(so);
    CHECK(mv->data() == (const void*)ptr);

    so.clear();
    CHECK_FALSE(so);
    CHECK(so.has_error());
    CHECK(so.value_or("asd") == "asd");

    itlib::eoptional<int> io = itlib::unexpected();
    CHECK_FALSE(io);
    CHECK(io.has_error());
    CHECK(io.value_or(44) == 44);

    io.emplace(5);
    CHECK(io);
    CHECK(*io == 5);
    CHECK(io.value_or(44) == 5);
}

TEST_CASE("eoptional ref")
{
    std::string str;
    itlib::eoptional<std::string&> so(str);
    CHECK(so);
    CHECK(so.has_value());
    CHECK(!so.has_error());

    *so = "xx";
    CHECK(so->length() == 2);
    CHECK(str == "xx");

    auto cp = so;
    CHECK(cp);
    cp.value() = "yyy";
    CHECK(so->length() == 3);
    CHECK(str == "yyy");

    so.clear();
    CHECK_FALSE(so);
    CHECK(so.has_error());

    itlib::eoptional<const int&> io = itlib::unexpected();
    CHECK_FALSE(io);
    CHECK(io.has_error());
    CHECK(io.value_or(44) == 44);

    int i = 34;
    io.emplace(i);
    CHECK(io);
    CHECK(*io == 34);
    CHECK(&io.value() == &i);
}

TEST_CASE("eoptional void")
{
    itlib::eoptional<void> vo;
    CHECK(vo);
    CHECK(vo.has_value());
    CHECK(!vo.has_error());

    vo.clear();
    CHECK_FALSE(vo);
    CHECK(vo.has_error());

    itlib::eoptional<void> vo2 = itlib::unexpected();
    CHECK_FALSE(vo2);
    CHECK(vo2.has_error());

    auto cp = vo2;
    CHECK_FALSE(cp);

    vo2.emplace();
    CHECK(vo2);

    CHECK_FALSE(cp);
}
