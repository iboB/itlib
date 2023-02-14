// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <cstdint> // just something that will give us _LIBCPP_VERSION

// completely disable this test on older versions of libc++ which don't have pmr
#if !defined(_LIBCPP_VERSION) || _LIBCPP_VERSION >= 16000

#include <itlib/pmr_allocator.hpp>

#include <doctest/doctest.h>
#include <doctest/util/lifetime_counter.hpp>

#include <type_traits>
#include <stdexcept>

struct lc : public doctest::util::lifetime_counter<lc> {
    lc() = default;
    lc(int ii) : x(ii) {
        if (ii == 0) throw std::runtime_error("uh-oh");
    }

    int x = 52;
};

TEST_CASE("pmr_allocator") {
    {
        itlib::pmr_allocator alloc;

        auto buf = alloc.allocate(10);
        CHECK(buf);
        static_assert(std::is_same_v<decltype(buf), std::byte*>);
        buf[9] = std::byte(50);
        alloc.deallocate(buf, 10);
    }

    {
        itlib::pmr_allocator<int> alloc;
        auto ibuf = alloc.allocate(42);
        CHECK(ibuf);
        static_assert(std::is_same_v<decltype(ibuf), int*>);
        ibuf[41] = 12;
        alloc.deallocate(ibuf, 42);

        auto bbuf = alloc.allocate_bytes(32, 16);
        CHECK(bbuf);
        CHECK(uintptr_t(bbuf) % 16 == 0);
        alloc.deallocate_bytes(bbuf, 32, 16);

        lc::lifetime_stats stats;
        auto lco = alloc.new_object<lc>();
        CHECK(lco);
        CHECK(lco->x == 52);
        alloc.delete_object(lco);

        CHECK(stats.living == 0);
        CHECK(stats.total == 1);

        lco = alloc.new_object<lc>(42);
        CHECK(lco);
        CHECK(lco->x == 42);
        alloc.delete_object(lco);

        CHECK(stats.living == 0);
        CHECK(stats.total == 2);

        CHECK_THROWS_WITH(lco = alloc.new_object<lc>(0), "uh-oh");
        CHECK(stats.living == 0);
        CHECK(stats.total == 3);
    }
}
#endif
