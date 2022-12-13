// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/make_ptr.hpp>

#include <vector>

// oh, well... just use a macro
#define maker_test(maker) \
std::vector<int> vec = { 1, 2, 3 }; \
auto copy = maker(vec); \
CHECK(copy->size() == 3); \
CHECK(vec.size() == 3); \
CHECK(vec.data() != copy->data()); \
copy->at(1) = 5; \
CHECK(*copy == std::vector<int>({1, 5, 3})); \
auto vdata = vec.data(); \
auto heist = maker(std::move(vec)); \
CHECK(heist->size() == 3); \
CHECK(heist->data() == vdata); \


TEST_CASE("[make-ptr:make_shared]")
{
    maker_test(itlib::make_shared);
}

TEST_CASE("[make-ptr:make_unique]")
{
    maker_test(itlib::make_unique);
}

struct vec { int x; int y; };

TEST_CASE("[make-ptr:make_aliased]")
{
    // non-null
    {
        auto ptr = itlib::make_shared(vec{1, 2});
        auto alias = itlib::make_aliased(ptr, &ptr->y);
        CHECK(alias);
        CHECK(*alias == 2);
        CHECK(alias.use_count() == 2);
    }

    // null
    {
        std::shared_ptr<vec> ptr;
        auto alias = itlib::make_aliased(ptr, &ptr->y);
        CHECK_FALSE(alias);
        CHECK(alias.use_count() == 0);
    }
}
