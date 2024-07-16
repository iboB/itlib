// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/span.hpp>

#include <vector>
#include <cstring>
#include <string>

TEST_CASE("[span] construction")
{
    using namespace itlib;

    {
        span<int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.size_bytes() == 0);
        CHECK(e.begin() == e.end());
        CHECK(e.cbegin() == e.cend());
        CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        CHECK(e.data() == nullptr);
    }

    {
        span<const int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.size_bytes() == 0);
        CHECK(e.begin() == e.end());
        CHECK(e.cbegin() == e.cend());
        CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        CHECK(e.data() == nullptr);
    }

    int i[] = { 0,2,3,4 };
    {
        span<int> ints(i);
        CHECK(ints);
        CHECK(ints.size() == 4);
        CHECK(ints.size_bytes() == 16);
        CHECK(ints.begin() + 4 == ints.end());
        CHECK(ints.cbegin() + 4 == ints.cend());
        CHECK(ints.rbegin() + 4 == ints.rend());
        CHECK(!ints.empty());
        CHECK(ints.data() == i);

        ints.front() = 1;
        CHECK(i[0] == 1);
    }

    {
        span<const int> ints(i);
        CHECK(ints);
        CHECK(ints.size() == 4);
        CHECK(ints.begin() + 4 == ints.end());
        CHECK(ints.rbegin() + 4 == ints.rend());
        CHECK(!ints.empty());
        CHECK(ints.data() == i);
    }

    std::vector<int> vec = { 1, 2, 3 };
    {
        span<int> ints(vec);
        CHECK(ints);
        CHECK(ints.size() == 3);
        CHECK(ints.begin() + 3 == ints.end());
        CHECK(ints.rbegin() + 3 == ints.rend());
        CHECK(!ints.empty());
        CHECK(ints.data() == vec.data());

        ints.back() = 17;
        CHECK(vec[2] == 17);
    }
}

TEST_CASE("[span] make_span")
{
    using namespace itlib;

    std::vector<int> vec = { 1, 2, 3 };

    {
        auto s = make_span(vec);
        CHECK(s.size() == 3);
        static_assert(std::is_same<span<int>, decltype(s)>::value, "make_span(vec<int>)");
    }

    {
        auto s = make_span(vec.data(), vec.size());
        CHECK(s.size() == 3);
        static_assert(std::is_same<span<int>, decltype(s)>::value, "make_span(int*, s)");
    }

    {
        auto s = make_span(vec.data(), vec.data() + vec.size());
        CHECK(s.size() == 3);
        static_assert(std::is_same<span<int>, decltype(s)>::value, "make_span(int*, int*)");
    }

    {
        // test that span of const can be safely constructed from temporary span of non-const
        span<const int> s = make_span(vec);
        CHECK(s.size() == 3);
        CHECK(s.data() == vec.data());
    }

    const std::vector<int> cvec = { 5, 6, 7, 8 };

    {
        auto s = make_span(cvec);
        CHECK(s.size() == 4);
        static_assert(std::is_same<span<const int>, decltype(s)>::value, "make_span(const vec<int>)");
    }

    {
        auto s = make_span(cvec.data(), cvec.size());
        CHECK(s.size() == 4);
        static_assert(std::is_same<span<const int>, decltype(s)>::value, "make_span(const int*, s)");
    }

    {
        auto s = make_span(cvec.data(), cvec.data() + cvec.size());
        CHECK(s.size() == 4);
        static_assert(std::is_same<span<const int>, decltype(s)>::value, "make_span(const int*, const int*)");
    }
}

size_t ifunc(itlib::span<const int> t) { return t.size(); }

TEST_CASE("[span] funcs")
{
    using namespace itlib;

    std::vector<int> vec = { 1, 2, 3 };
    CHECK(ifunc(vec) == 3);

    auto s = make_span(vec);
    CHECK(ifunc(s) == 3);
}

TEST_CASE("[span] copy-ctor and assign")
{
    using namespace itlib;

    std::vector<int> vec = { 1, 2, 3 };
    auto s = make_span(vec);

    auto s2 = s;
    CHECK(s2.size() == 3);
    CHECK(s2.data() == s.data());

    span<int> sc;
    sc = s2;
    CHECK(sc.size() == 3);
    CHECK(sc.data() == s.data());

    span<const int> cs;
    cs = s2;
    CHECK(cs.size() == 3);
    CHECK(cs.data() == s.data());

    span<const int> cs2 = vec;
    CHECK(cs2.size() == 3);
    CHECK(cs2.data() == s.data());
}

template <typename S>
void test_slicing(const S& span)
{
    {
        auto s = span.subspan(10);
        CHECK(!!s);
        CHECK(s.empty());
        CHECK(s.begin() == span.end());
    }
    {
        auto s = span.subspan(1);
        CHECK(s.size() == 4);
        CHECK(s.begin() == span.begin() + 1);
    }
    {
        auto s = span.subspan(3, 1);
        CHECK(s.size() == 1);
        CHECK(s.begin() == span.begin() + 3);
    }
    {
        auto s = span.first(3);
        CHECK(s.size() == 3);
        CHECK(s.begin() == span.begin());
    }
    {
        auto s = span.last(2);
        CHECK(s.size() == 2);
        CHECK(s.begin() == span.begin() + 3);
    }
    {
        auto cp = span;
        cp.remove_prefix(2);
        CHECK(cp.size() == 3);
        CHECK(cp.begin() == span.begin() + 2);
        cp.remove_suffix(2);
        CHECK(cp.size() == 1);
        CHECK(cp.end() == span.begin() + 3);
    }
}

TEST_CASE("[span] slicing")
{
    using namespace itlib;
    std::vector<int> ivec = { 6, 7, 8, 9, 10 };
    auto s = make_span(ivec);
    test_slicing(s);
    span<const int> cs = s;
    test_slicing(cs);
}

TEST_CASE("[span] bytes")
{
    using namespace itlib;
    {
        std::vector<uint32_t> ivec = { 0, 0xFFFFFFFF, 0x12345678 };
        auto vs = make_span(ivec);
        auto bs = vs.as_bytes();
        CHECK(bs.size() == 12);
        CHECK(bs[0] == 0);
        CHECK(bs[4] == 0xFF);
        uint32_t lasti;
        auto last4 = bs.last(4);
        REQUIRE(sizeof(lasti) == last4.size_bytes());
        memcpy(&lasti, last4.data(), last4.size_bytes());
        CHECK(lasti == 0x12345678);

        auto wbs = vs.as_writable_bytes();
        CHECK(wbs.size() == 12);
        uint32_t newi = 0xBAADF00D;
        memcpy(wbs.first(4).data(), &newi, sizeof(newi));
        CHECK(ivec.front() == 0xBAADF00D);
    }

    {
        const std::vector<uint32_t> ivec = {0, 0xFFFFFFFF, 0x12345678};
        auto vs = make_span(ivec);
        auto bs = vs.as_bytes();
        CHECK(bs.size() == 12);
        auto wbs = vs.as_writable_bytes();
        CHECK(wbs.size() == 12);
        static_assert(std::is_same<span<const uint8_t>, decltype(wbs)>::value, "writable bytes of const must be const");
    }
}

class sview
{
public:
    sview() = default;
    sview(const std::string&) {}
};

int disambiguate_test(sview) { return 0; }
int disambiguate_test(itlib::span<const int>) { return 1; }

TEST_CASE("[span] disambiguate")
{
    std::string str;
    CHECK(0 == disambiguate_test(str));
}
