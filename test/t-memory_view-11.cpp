// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

// we know it's deprecated
// no need to cause a bunch of warnings here
#define ITLIB_DEPRECATED(...)
#include <itlib/memory_view.hpp>

#include <vector>

TEST_CASE("[memory_view] test")
{
    using namespace itlib;

    memory_view<int> e;
    CHECK(!e);
    CHECK(e.size() == 0);
    CHECK(e.begin() == e.end());
    CHECK(e.cbegin() == e.cend());
    CHECK(e.empty());
    CHECK(e.data() == nullptr);
    CHECK(e.get() == nullptr);

    int i[] = { 0,2,3,4 };

    e.reset(i, 4);
    CHECK(e);
    CHECK(e.size() == 4);
    CHECK(e.begin() + 4 == e.end());
    CHECK(e.cbegin() + 4 == e.cend());
    CHECK(!e.empty());
    CHECK(e.data() == i);
    CHECK(e.get() == i);

    e.front() = 1;
    CHECK(i[0] == 1);

    auto iview = make_memory_view(i, 4);
    static_assert(std::is_same<memory_view<int>, decltype(iview)>::value, "iview must be memory_view<int>");
    CHECK(iview.size() == 4);
    CHECK(iview.data() == e.data());
    CHECK(iview.at(0) == *e.begin());

    auto iview2 = iview;
    CHECK(iview2.data() == iview.get());

    std::vector<int> ivec = { 6, 7, 8, 9, 10 };

    auto vecview = make_memory_view(ivec);
    static_assert(std::is_same<memory_view<int>, decltype(vecview)>::value, "vecview must be memory_view<int>");
    CHECK(vecview.size() == 5);
    CHECK(vecview.data() == ivec.data());

    auto slice = make_memory_view(ivec, 1);
    static_assert(std::is_same<memory_view<int>, decltype(slice)>::value, "slice must be memory_view<int>");
    CHECK(slice.size() == 4);

    auto sb = slice.begin();
    for (auto iv : iview)
    {
        CHECK(iv + 6 == *sb);
        ++sb;
    }

    sb = vecview.begin();
    for (auto is : slice)
    {
        CHECK(is - 1 == *sb);
        ++sb;
    }
}

TEST_CASE("[const_memory_view] test")
{
    using namespace itlib;

    const_memory_view<int> e;
    CHECK(!e);
    CHECK(e.size() == 0);
    CHECK(e.begin() == e.end());
    CHECK(e.cbegin() == e.cend());
    CHECK(e.empty());
    CHECK(e.data() == nullptr);
    CHECK(e.get() == nullptr);

    const int i[] = { 1,2,3,4 };

    e.reset(i, 4);
    CHECK(e);
    CHECK(e.size() == 4);
    CHECK(e.begin() + 4 == e.end());
    CHECK(e.cbegin() + 4 == e.cend());
    CHECK(!e.empty());
    CHECK(e.data() == i);
    CHECK(e.get() == i);

    auto iview = make_memory_view(i, 4);
    static_assert(std::is_same<const_memory_view<int>, decltype(iview)>::value, "iview must be memory_view<int>");
    CHECK(iview.size() == 4);
    CHECK(iview.data() == e.data());
    CHECK(iview.at(0) == *e.begin());

    auto iview2 = iview;
    CHECK(iview2.data() == iview.get());

    const std::vector<int> ivec = { 6, 7, 8, 9, 10 };

    auto vecview = make_memory_view(ivec);
    CHECK(vecview.size() == 5);
    static_assert(std::is_same<const_memory_view<int>, decltype(vecview)>::value, "vecview must be memory_view<int>");
    CHECK(vecview.size() == 5);
    CHECK(vecview.data() == ivec.data());

    auto slice = make_memory_view(ivec, 1);
    static_assert(std::is_same<const_memory_view<int>, decltype(slice)>::value, "slice must be memory_view<int>");

    CHECK(slice.size() == 4);

    auto sb = slice.begin();
    for (auto iv : iview)
    {
        CHECK(iv + 6 == *sb);
        ++sb;
    }

    sb = vecview.begin();
    for (auto is : slice)
    {
        CHECK(is - 1 == *sb);
        ++sb;
    }
}

template <typename MV>
void test_slicing(const MV& mv)
{
    {
        auto s = mv.slice(10);
        CHECK(!!s);
        CHECK(s.empty());
        CHECK(s.begin() == mv.end());
    }
    {
        auto s = mv.slice(1);
        CHECK(s.size() == 4);
        CHECK(s.begin() == mv.begin() + 1);
    }
    {
        auto s = mv.slice(3, 1);
        CHECK(s.size() == 1);
        CHECK(s.begin() == mv.begin() + 3);
    }

    {
        auto cp = mv;
        cp.remove_prefix(2);
        CHECK(cp.size() == 3);
        CHECK(cp.begin() == mv.begin() + 2);
        cp.remove_suffix(2);
        CHECK(cp.size() == 1);
        CHECK(cp.end() == mv.begin() + 3);
    }
}

TEST_CASE("[memory_view] slicing")
{
    std::vector<int> ivec = { 6, 7, 8, 9, 10 };
    auto mv = itlib::make_memory_view(ivec);
    test_slicing(mv);
    itlib::const_memory_view<int> cmv = mv;
    test_slicing(cmv);
}
