// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/utility.hpp>

#include <string>

#include <doctest/doctest.h>

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
