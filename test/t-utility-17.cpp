// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/utility.hpp>
#include <doctest/doctest.h>
#include <string>
#include <string_view>

TEST_CASE("transfer string_view") {
    std::string a = "hello world!!";
    std::string b = "abcdefghijk";

    auto hello = std::string_view(a).substr(0, 5);
    auto world = std::string_view(a).substr(6, 5);
    auto o_w = std::string_view(a).substr(4, 3);
    auto o0 = std::string_view(a).substr(4, 0);
    std::string_view empty;

    // sanity
    CHECK(hello == "hello");
    CHECK(world == "world");
    CHECK(o_w == "o w");

    {
        auto ts = itlib::transfer_view(hello, a, b);
        CHECK(ts == std::string_view("abcde"));
    }
    {
        auto ts = itlib::transfer_view(world, a, b);
        CHECK(ts == std::string_view("ghijk"));
    }
    {
        auto ts = itlib::transfer_view(o_w, a, b);
        CHECK(ts == std::string_view("efg"));
    }
    {
        auto ts = itlib::transfer_view(o0, a, b);
        CHECK(ts.empty());
        CHECK(ts.data() == b.data() + 4);
    }
    {
        auto ts = itlib::transfer_view(empty, a, b);
        CHECK(ts.empty());
        CHECK(ts.data() == nullptr);
    }
}
