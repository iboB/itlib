// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <type_traits>
#include <itlib/ufunction.hpp>

#include <memory>

TEST_SUITE_BEGIN("ufunction");

TEST_CASE("Basic")
{
    using namespace itlib;

    auto uptr = std::make_unique<int>(53);
    ufunction<void()> func([u = std::move(uptr)](){
        CHECK(*u == 53);
    });
    func();

    auto uptr2 = std::make_unique<int>(102);
    func = [u = std::move(uptr2)]() {
        CHECK(*u == 102);
    };
    func();

    auto f2 = std::move(func);
    CHECK(!func);
    f2();
}
