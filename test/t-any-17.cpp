// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <cstdint> // just something that will give us _LIBCPP_VERSION

// completely disable this test on older versions of libc++ which don't have pmr
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 16000


#include <itlib/any.hpp>
#include <itlib/pmr_allocator.hpp>

#include <doctest/doctest.h>

#include <string>

TEST_CASE("any pmr_allocator") {
    itlib::any<itlib::pmr_allocator<>> a = std::string("xxx");
    itlib::any b = a;
    CHECK(*b.tdata<std::string>() == "xxx");
}

#endif
