// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/any.hpp>

#include <doctest/doctest.h>

#include <string>
#include <vector>

TEST_CASE("basic") {
    itlib::any<> a;
    CHECK_FALSE(a);
    CHECK_FALSE(a.has_value());
    CHECK_FALSE(a.data());
    CHECK_FALSE(a.tdata<std::string>());

    auto b = a;
    CHECK_FALSE(b);

    itlib::any<> c(std::string("foo bar"));
    CHECK(c);
    CHECK(c.has_value());
    auto d = c;
    CHECK(d);
    CHECK(*d.tdata<std::string>() == "foo bar");

    d.reset();
    CHECK_FALSE(d);
    CHECK(*c.tdata<std::string>() == "foo bar");

    a = c;
    CHECK(a);
    CHECK(*a.tdata<std::string>() == "foo bar");

    a = std::unique_ptr<int>(new int(43));
    CHECK(a);

    auto& uptr = *a.tdata<std::unique_ptr<int>>();
    CHECK(*uptr == 43);

    CHECK_THROWS_AS(b = a, std::bad_cast);
    CHECK_THROWS_AS(auto e = a, std::bad_cast);

    b = std::move(a);
    CHECK_FALSE(a);
    CHECK(b);
    CHECK(&uptr == b.data());
    CHECK(*uptr == 43);

    auto e = std::move(b);
    CHECK_FALSE(b);
    CHECK(e);
    CHECK(&uptr == e.data());
    CHECK(*uptr == 43);

    auto& str = a.emplace<std::string>("baz");
    CHECK(a);
    CHECK(&str == a.data());
}

TEST_CASE("allocator aware") {
    static_assert(!std::uses_allocator<itlib::any<>, std::allocator<char>>::value, "not compatible with std::allocator (yet)");

    std::vector<itlib::any<>> vec;
    vec.emplace_back();
    CHECK(vec.size() == 1);
    auto& back = vec.back();
    CHECK(back.has_value() == false);
}

TEST_CASE("any cast") {
    {
        CHECK_FALSE(itlib::any_cast<std::string>(nullptr));

        itlib::any<>* a = nullptr;
        CHECK_FALSE(itlib::any_cast<std::string>(a));
    }

    {
        itlib::any<> a(5);
        CHECK_FALSE(itlib::any_cast<std::string>(&a));
        auto pi = itlib::any_cast<int>(&a);
        CHECK(pi);
        CHECK(*pi == 5);

        *pi = 6;
        CHECK_THROWS_AS(itlib::any_cast<std::string>(a), std::bad_cast);

        CHECK(itlib::any_cast<int>(a) == 6);

        itlib::any_cast<int&>(a) = 42;

        CHECK(itlib::any_cast<const int>(a) == 42);

        const auto& ca = a;

        CHECK(itlib::any_cast<int>(ca) == 42);
        CHECK(&itlib::any_cast<const int&>(ca) == pi);
    }

    {
        itlib::any<> a = std::string("abc");
        CHECK_FALSE(itlib::any_cast<char>(&a));
        auto pstr = itlib::any_cast<std::string>(&a);
        CHECK(pstr);
        CHECK(*pstr == "abc");

        itlib::any_cast<std::string&>(a) = "xyz";
        CHECK(*pstr == "xyz");

        auto take_str = itlib::any_cast<std::string>(std::move(a));
        CHECK(take_str == "xyz");
        CHECK(pstr->empty());

        itlib::any_cast<std::string&>(a) = "mnp";
        take_str = itlib::any_cast<std::string&&>(std::move(a));
        CHECK(take_str == "mnp");
        CHECK(pstr->empty());
    }
}