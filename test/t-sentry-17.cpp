// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <type_traits>
#include <itlib/sentry.hpp>

#include <memory>
#include <string>

TEST_SUITE_BEGIN("sentry");

TEST_CASE("Basic")
{
    using namespace itlib;

    int i = 0;
    {
        sentry s([&i]() { i = 3; });
        CHECK(i == 0);
    }
    CHECK(i == 3);

    {
        auto s = make_sentry([&i]() { i = 5; });
        CHECK(i == 3);
    }
    CHECK(i == 5);

    std::unique_ptr<std::string> ptr(new std::string("asd"));
    {
        sentry s([cptr = std::move(ptr)]() {
            CHECK(*cptr == "asd");
        });
    }
    CHECK(!ptr);
}
