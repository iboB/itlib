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

    ufunction<void()> func([u = std::make_unique<int>(53)](){
        CHECK(*u == 53);
    });
    func();

    func = [u = std::make_unique<int>(102)]() {
        CHECK(*u == 102);
    };
    func();

    auto f2 = std::move(func);
    CHECK(!func);
    f2();
}
