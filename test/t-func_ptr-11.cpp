// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/func_ptr.hpp>
#include <doctest/doctest.h>
#include <doctest/util/lifetime_counter.hpp>
#include <string>

TEST_CASE("empty") {
    SUBCASE("default") {
        itlib::func_ptr<void()> f;
        CHECK(!f);
    }
    SUBCASE("nullptr") {
        itlib::func_ptr<void()> f(nullptr);
        CHECK(!f);
    }
}

TEST_CASE("lambda") {
    int x = 0;
    auto lambda = [&](int v) { x += v; };
    auto lambda2 = [&](int v) { x = v * 2; };
    itlib::func_ptr<void(int)> lptr(&lambda);
    CHECK(!!lptr);
    lptr(3);
    CHECK(x == 3);
    lptr(4);
    CHECK(x == 7);

    lptr.reset();
    CHECK(!lptr);

    lptr.reset(&lambda2);
    CHECK(!!lptr);
    lptr(5);
    CHECK(x == 10);

    auto lptr2 = lptr;
    lptr2(6);
    CHECK(x == 12);

    lptr.reset();
    CHECK_FALSE(lptr);

    lptr2(7);
    CHECK(x == 14);
}

TEST_CASE("function pointer") {
    auto func = +[](int v) { return v * 3; };
    itlib::func_ptr<int(int)> fptr(func);
    CHECK(!!fptr);
    int r = fptr(4);
    CHECK(r == 12);
    fptr.reset();
    CHECK(!fptr);
    fptr.reset(func);
    CHECK(!!fptr);
    r = fptr(5);
    CHECK(r == 15);

    auto fptr2 = fptr;
    CHECK(fptr2(3) == 9);
}

struct val : public doctest::util::lifetime_counter<val> {
    explicit val(int v = 0) : value(v) {}
    int value = 0;
};

struct str : public doctest::util::lifetime_counter<str> {
    explicit str(std::string v = "") : value(std::move(v)) {}
    std::string value;
};

TEST_CASE("arg lifetime") {
    using namespace doctest::util;
    lifetime_counter_sentry vsentry(val::root_lifetime_stats()), ssentry(str::root_lifetime_stats());

    // refs
    {
        auto func_vr = [](val& v) { v.value += 5; return v.value; };
        auto func_vcr = [](const val& v) { return v.value; };

        val::lifetime_stats vs;
        val v(42);

        itlib::func_ptr<int(val&)> fptr(&func_vr);
        CHECK(fptr(v) == 47);
        CHECK(v.value == 47);

        fptr.reset(&func_vcr);
        CHECK(fptr(v) == 47);

        CHECK(vs.total == 1);
    }

    // copies and moves
    {
        auto func_v = [](val v) { return v.value; };
        auto func_vs = [](val v, str s) { return v.value + std::stoi(s.value); };

        val::lifetime_stats vs;
        str::lifetime_stats ss;
        val v(42);

        itlib::func_ptr<int(val)> fptr(&func_v);

        CHECK(fptr(v) == 42);
        CHECK(vs.copies == 1);

        str s("58");

        itlib::func_ptr<int(val, str)> fptr2(&func_vs);
        CHECK(fptr2(v, s) == 100);
        CHECK(vs.copies == 2);
        CHECK(ss.copies == 1);

        CHECK(fptr2(std::move(v), std::move(s)) == 100);
        CHECK(vs.copies == 2);
        CHECK(ss.copies == 1);

        CHECK(fptr2(val(10), str("20")) == 30);
        CHECK(vs.copies == 2);
        CHECK(ss.copies == 1);
    }
}
