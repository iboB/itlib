// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/utility.hpp>
#include <doctest/doctest.h>
#include <string>
#include <itlib/span.hpp>
#include <vector>

TEST_CASE("force_move") {
    auto i = itlib::force_move(7);
    CHECK(i == 7);
    const auto str = itlib::force_move(std::string("str"));
    CHECK(str == "str");
    // won't compile:
    // auto s2 = vstl::force_move(str);
}

struct member {
    const std::string& get_owner_name() const;
    int x;
};

struct owner {
    std::string name;
    member m;
};

const std::string& member::get_owner_name() const {
    return itlib::owner_from_member(*this, &owner::m)->name;
}

TEST_CASE("owner_from_member") {
    owner x;
    CHECK(x.m.get_owner_name().empty());
    x.name = "xxx";
    CHECK(x.m.get_owner_name() == "xxx");
}

class incomplete;

TEST_CASE("make_nullptr") {
    auto in = itlib::make_nullptr<incomplete>();
    CHECK_FALSE(in);
    static_assert(std::is_same<incomplete*, decltype(in)>::value, "must be same type");
}

TEST_CASE("bit_cast") {
    float f = 2.5f;
    auto i = itlib::bit_cast<uint32_t>(f);
    CHECK(i == 0x40200000);
}

TEST_CASE("transfer_view") {
    std::vector<int> a = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> b = {10,11,12,13,14,15,16,17,18};

    auto va36 = itlib::span<int>(a).subspan(3, 3);
    auto cva28 = itlib::span<const int>(a).subspan(2, 6);

    auto vb = itlib::transfer_view(va36, a, b);
    CHECK(vb.size() == 3);
    CHECK(vb.data() == b.data() + 3);

    auto cvb = itlib::transfer_view(cva28, a, b);
    CHECK(cvb.size() == 6);
    CHECK(cvb.data() == b.data() + 2);

    auto vb2 = itlib::transfer_view_as<itlib::span<int>>(cva28, a, b);

    vb[0] = 100;
    CHECK(cvb[1] == 100);

    vb2[2] = 200;
    CHECK(cvb[2] == 200);
}
