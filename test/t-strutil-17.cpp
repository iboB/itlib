#include <doctest/doctest.h>

#include <itlib/strutil.hpp>

TEST_SUITE_BEGIN("strutil");

using namespace itlib;

TEST_CASE("make_string_view")
{
    const char* x = "0123456789";
    auto s = make_string_view(x, x + 3);
    CHECK(s == "012");

    s = make_string_view(x + 2, x + 8);
    CHECK(s == "234567");

    s = make_string_view(s.begin() + 1, s.end() - 2);
    CHECK(s == "345");

    auto e = make_string_view(s.begin(), s.begin());
    CHECK(e.empty());

    e = make_string_view(s.end(), s.end());
    CHECK(e.empty());
}

TEST_CASE("starts/ends_with")
{
    std::string_view x = "start_123_end";
    CHECK(starts_with(x, ""));
    CHECK(starts_with(x, "s"));
    CHECK(starts_with(x, "start"));
    CHECK(starts_with(x, x));
    CHECK(!starts_with(x, "tart"));
    CHECK(!starts_with(x, "t"));
    CHECK(!starts_with(x, "start_123_enz"));
    CHECK(!starts_with(x, "start_123_endd"));

    CHECK(ends_with(x, ""));
    CHECK(ends_with(x, "d"));
    CHECK(ends_with(x, "end"));
    CHECK(ends_with(x, x));
    CHECK(!ends_with(x, "en"));
    CHECK(!ends_with(x, "start_123_endz"));
    CHECK(!ends_with(x, "start_123_enddd"));
    CHECK(!ends_with(x, "atart_123_end"));
}

TEST_CASE("remove_prefix/suffix_all")
{
    std::string_view x = "stst_endend";
    CHECK(remove_prefix_all(x, "st") == "_endend");
    CHECK(remove_suffix_all(x, "end") == "stst_");
    CHECK(remove_prefix_all(x, "zz") == x);
    CHECK(remove_suffix_all(x, "zz") == x);
    CHECK(remove_prefix_all(x, "") == x);
    CHECK(remove_suffix_all(x, "") == x);

    CHECK(remove_prefix_all("aaa", "a").empty());
    CHECK(remove_suffix_all("aaa", "a").empty());

    CHECK(remove_prefix_all("aaa", "aa") == "a");
    CHECK(remove_suffix_all("aaa", "aa") == "a");
}
