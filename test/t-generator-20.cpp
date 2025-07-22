// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/generator.hpp>

#include <doctest/doctest.h>
#include <doctest/util/lifetime_counter.hpp>

#include <stdexcept>
#include <iterator>
#include <vector>
#include <string>
#include <span>

itlib::generator<int> range(int begin, int end) {
    // this absolutely pointless vector here is to trigger clang's ridiculous handling of coroutines
    // if we simply rethrow in unhandled_exception(), for some reason it calls the destructors of locals twice
    // having a local whose destructor is not safe to call twice will cause a crash
    // if we don't crash here on clang, then generator works as expected
    std::vector<int> store;
    for (int i = begin; i < end; ++i) {
        store.emplace_back(i);
    }

    for (auto i : store) {
        if (i == 103) throw std::runtime_error("test exception");
        co_yield i;
    }
}

TEST_CASE("simple") {
    int i = 50;

    // range for
    for (int x : range(i, i+10)) {
        CHECK(x == i);
        ++i;
    }
    CHECK(i == 60);

    // next
    auto r = range(1, 5);
    CHECK(*r.next() == 1);
    CHECK_FALSE(r.done());
    CHECK(*r.next() == 2);
    CHECK(r.next().value() == 3);
    CHECK(r.next().value() == 4);
    CHECK_FALSE(r.next().has_value());
    CHECK_FALSE(r.next().has_value()); // check once again to make sure it's safe
    CHECK(r.done());

    // exceptions

    i = 0;
    CHECK_THROWS_WITH_AS(
        [&]() {
            for (int x : range(100, 105)) {
                i = x;
            }
        }(),
        "test exception",
        std::runtime_error
    );
    CHECK(i == 102);

    r = range(101, 105);
    CHECK_NOTHROW(r.next());
    CHECK_NOTHROW(r.next());
    CHECK_THROWS_WITH_AS(r.next(), "test exception", std::runtime_error);

    CHECK(!!r);
    r.reset();
    CHECK_FALSE(r);
}

template <typename T>
itlib::generator<T&> ref_gen(std::span<T> vals) {
    for (T& v : vals) {
        co_yield v;
    }
}

TEST_CASE("ref") {
    std::vector<int> ints = {1, 2, 3, 4, 5};
    auto g = ref_gen(std::span(ints));
    for (int& i : g) {
        i += 10;
    }
    CHECK(ints == std::vector<int>{11, 12, 13, 14, 15});

    auto cg = ref_gen(std::span<const int>(ints));
    const int& a = *cg.next();
    const int& b = *cg.next();
    for (const int& i : cg) {
        CHECK(i > 12);
        CHECK(i < 16);
    }
    CHECK(cg.done());
    CHECK(a == 11);
    CHECK(&a == ints.data());
    CHECK(b == 12);
    CHECK(&b == ints.data() + 1);
}

struct value : doctest::util::lifetime_counter<value>
{
    value() = default;
    explicit value(int i) : val(i) {}
    int val = 0;
};

itlib::generator<value> value_range(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        co_yield value(i);
    }
}

TEST_CASE("lifetime") {
    doctest::util::lifetime_counter_sentry lcsentry(value::root_lifetime_stats());

    int i = 0;
    {
        value::lifetime_stats ls;

        auto r = value_range(0, 5);
        for (value v : r) {
            CHECK(v.val == i);
            ++i;
        }
        CHECK(i == 5);

        CHECK(ls.living == 0);
        CHECK(ls.copies == 5);
        CHECK(ls.m_ctr == 5);
    }

    {
        value::lifetime_stats ls;

        auto r = value_range(0, 3);
        auto v1 = r.next();
        auto v2 = r.next();
        auto v3 = r.next();
        auto vend = r.next();
        CHECK(ls.living == 3);
        CHECK(ls.copies == 0);
        CHECK(ls.m_ctr == 6);
    }
}

struct non_copyable {
    non_copyable(std::string v) : value(std::move(v)) {}
    non_copyable(const non_copyable&) = delete;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable(non_copyable&&) noexcept = default;
    non_copyable& operator=(non_copyable&&) noexcept = default;
    std::string value;
};

itlib::generator<non_copyable> non_copyable_range(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        co_yield non_copyable(std::to_string(i));
    }
}

TEST_CASE("yield non copyable") {
    auto gen = non_copyable_range(10, 15);
    std::vector<std::string> values;
    int i = 10;
    for (auto mi = std::make_move_iterator(gen.begin()); mi.base() != gen.end(); ++mi) {
        auto elem = *mi;
        CHECK(elem.value == std::to_string(i));
        ++i;
    }
}

itlib::generator<std::string, int, itlib::noexcept_generator> yield_strings(int begin, int end) {
    for (int i = begin; i < end; ++i) {
        co_yield std::to_string(i);
    }
    co_return end - begin;
}

TEST_CASE("return iter") {
    auto gen = yield_strings(10, 15);
    std::vector<std::string> values;
    int i = 10;
    for (auto s : gen) {
        CHECK(s == std::to_string(i));
        ++i;
    }
    CHECK(i == 15);
    CHECK(gen.rval() == 5);
}

TEST_CASE("return next") {
    auto gen = yield_strings(10, 13);
    CHECK(*gen.next() == "10");
    CHECK(*gen.next() == "11");
    CHECK(*gen.next() == "12");
    CHECK_FALSE(gen.next().has_value());
    CHECK(gen.rval() == 3);
    CHECK(gen.done());
    CHECK(gen.rval() == 3);
}

TEST_CASE("empty") {
    {
        int i = 0;
        for (auto val : range(5, 5)) {
            CHECK(val == 0xBAADBEEF);
            ++i;
        }
        CHECK(i == 0);
    }

    {
        auto gen = range(10, 10);
        CHECK_FALSE(gen.next());
        CHECK(gen.done());
    }

    {
        auto gen = yield_strings(5, 5);
        CHECK_FALSE(gen.next());
        CHECK(gen.done());
        CHECK(gen.rval() == 0);
    }
}
