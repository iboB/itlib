// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <cstdint> // just something that will give us _LIBCPP_VERSION

// completely disable this test on older versions of libc++ which don't have pmr
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 16000


#include <itlib/any.hpp>
#include <itlib/pmr_allocator.hpp>
#include <vector>

#include <doctest/doctest.h>

#include <string>

using pmr_any = itlib::any<itlib::pmr_allocator<>>;

TEST_CASE("any pmr_allocator") {
    pmr_any a = std::string("xxx");
    itlib::any b = a;
    CHECK(*b.tdata<std::string>() == "xxx");
}

TEST_CASE("allocator aware") {
    static_assert(std::uses_allocator_v<pmr_any, itlib::pmr_allocator<>>);
    static_assert(std::uses_allocator_v<pmr_any, itlib::pmr_allocator<int>>);
    static_assert(std::uses_allocator_v<pmr_any, std::pmr::polymorphic_allocator<int>>);

    std::pmr::monotonic_buffer_resource buf(2048);
    std::pmr::vector<pmr_any> vec(itlib::pmr_allocator<>{&buf});
    CHECK(vec.empty());
    CHECK(vec.get_allocator().resource() == &buf);
    auto& back = vec.emplace_back();
    CHECK(back.get_allocator() == vec.get_allocator());
    CHECK(vec.size() == 1);
    CHECK_FALSE(back.has_value());

    std::pmr::vector<pmr_any> vec_d;
    auto& back_d = vec_d.emplace_back();
    CHECK(back_d.get_allocator() == vec_d.get_allocator());

    CHECK(back.get_allocator() != back_d.get_allocator());
}

#endif
