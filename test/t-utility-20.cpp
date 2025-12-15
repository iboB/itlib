// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/utility.hpp>
#include <doctest/doctest.h>
#include <vector>
#include <span>
#include <string>
#include <string_view>

TEST_CASE("transfer std::span") {
    std::vector<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::vector<int> b = { 10,11,12,13,14,15,16,17,18 };

    auto va36 = std::span<int>(a).subspan(3, 3);
    auto cva28 = std::span<const int>(a).subspan(2, 6);

    auto vb = itlib::transfer_view(va36, a, b);
    CHECK(vb.size() == 3);
    CHECK(vb.data() == b.data() + 3);

    vb[0] = 100;

    auto cvb = itlib::transfer_view(cva28, a, b);
    CHECK(cvb.size() == 6);
    CHECK(cvb.data() == b.data() + 2);

    CHECK(cvb[1] == 100);
}

TEST_CASE("transfer_view_as") {
    std::string a = "hello world!!";

    auto hello = std::string_view(a).substr(0, 5);
    auto hello_span = itlib::transfer_view_as<std::span<char>>(hello, a, a);

    hello_span[0] = 'H';
    CHECK(hello == "Hello");
}
