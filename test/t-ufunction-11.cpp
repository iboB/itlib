// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <type_traits>
#include <itlib/ufunction.hpp>

TEST_SUITE_BEGIN("ufunction");

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
    int operator()(int n) { return n+5;}

    bool owner = true;
};

TEST_CASE("Basic")
{
    using namespace itlib;

    fnocopy x;
    ufunction<int(int)> func(std::move(x));
    CHECK(x.owner == false);
    CHECK(func(4) == 9);

    auto f2 = std::move(func);
    CHECK(!func);
    CHECK(f2(1) == 6);
}

int sum(int a, int b) { return a + b; }

TEST_CASE("Free func")
{
    itlib::ufunction<int(int, int)> func = sum;
    CHECK(func(1, 2) == 3);
    func = [](int a, int b) { return a * b; };
    CHECK(func(3, 4) == 12);
    func = sum;
    CHECK(func(3, 4) == 7);
}
