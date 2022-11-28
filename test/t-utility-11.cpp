#include <doctest/doctest.h>

#include <itlib/utility.hpp>

#include <string>

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
