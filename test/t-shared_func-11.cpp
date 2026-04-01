// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/shared_func.hpp>
#include <doctest/doctest.h>

struct fnocopy
{
    fnocopy() = default;
    fnocopy(const fnocopy&) = delete;
    fnocopy& operator=(const fnocopy&) = delete;
    fnocopy(fnocopy&& other) noexcept
    {
        other.owner = false;
    }
    fnocopy& operator=(fnocopy&& other) noexcept
    {
        other.owner = false;
        return *this;
    }
    int operator()(int n) { return acc += n; }

    int acc = 0;
    bool owner = true;
};

using namespace itlib;

TEST_CASE("empty") {
    SUBCASE("empty") {
        itlib::shared_func<void(int, char)> func;
        CHECK_FALSE(func);
    }
    SUBCASE("nullptr") {
        itlib::shared_func<void(int, char)> func = nullptr;
        CHECK_FALSE(func);
    }
}

TEST_CASE("lambda") {
    int acc = 0;
    {
        itlib::shared_func<void(int)> func = [&acc](int n) { acc += n; };
        CHECK(func);
        func(5);
        CHECK(acc == 5);
        func(10);
        CHECK(acc == 15);

        auto f2 = func;
        CHECK(f2);
        f2(3);
        CHECK(acc == 18);

        CHECK(func.use_count() == 2);
    }
}
