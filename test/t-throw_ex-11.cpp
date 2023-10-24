// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/throw_ex.hpp>
#include <doctest/doctest.h>
#include <stdexcept>

using throw_ex = itlib::throw_ex<std::runtime_error>;

struct foo {
    foo(int a, int b) : a(a), b(b) {}
    int a, b;
};

std::ostream& operator<<(std::ostream& out, const foo& f) {
    out << f.a << ':' << f.b;
    return out;
}

TEST_CASE("throw_ex") {
    CHECK_THROWS_WITH_AS(
        throw_ex{} << "hello " << 42,
        "hello 42",
        std::runtime_error
    );

    CHECK_THROWS_WITH_AS(
        throw_ex{} << "hello " << foo(1, 2),
        "hello 1:2",
        std::runtime_error
    );
}
