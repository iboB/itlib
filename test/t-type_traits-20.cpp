// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/type_traits.hpp>
#include <doctest/doctest.h>
#include <vector>
#include <string>

template <itlib::instantiation_of<std::vector> T>
int test_instantiation_of(T&) {
    return 42;
}

template <itlib::instantiation_of<std::basic_string> T>
int test_instantiation_of(T&) {
    return 24;
}

TEST_CASE("instantiation_of concept") {
    std::string s;
    CHECK(test_instantiation_of(s) == 24);
    std::vector<int> v;
    CHECK(test_instantiation_of(v) == 42);
}

template <itlib::noop_convertible_to<int> T>
int test_noop_convertible(T) {
    return 7;
}

template <itlib::noop_convertible_to<void*> T>
int test_noop_convertible(T) {
    return 14;
}

TEST_CASE("noop_convertible_to concept") {
    CHECK(test_noop_convertible(5) == 7);
    CHECK(test_noop_convertible(size_t(32)) == 14);
}
