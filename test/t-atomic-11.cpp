// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/atomic.hpp>

#include <thread>

TEST_CASE("atomic_relaxed_counter ops") {
    itlib::atomic_relaxed_counter<int> a{4};
    CHECK(a == 4);
    CHECK(4 == a);
    CHECK(5 > a);
    CHECK(a < 5);
    CHECK(a++ == 4);
    CHECK(a == 5);
    CHECK(a-- == 5);
    CHECK(a == 4);
    CHECK(a + 10 == 14);
    CHECK(a - 3 == 1);

    auto b = a;
    CHECK(a == b);
    CHECK(a <= b);

    CHECK(++b == 5);
    CHECK(b > a);
    CHECK(--b == 4);
    CHECK(b.load() == 4);

    CHECK((b += 5) == 9);
    CHECK(b == 9);
    CHECK((b -= 8) == 1);
    CHECK(1 == b);

    a = b;
    CHECK(a == 1);

    a.store(19);
    CHECK(a.load() == 19);

    b = 12;
    CHECK(b == 12);
}

TEST_CASE("atomic_relaxed_counter atomic") {
    itlib::atomic_relaxed_counter<int> cnta{0};
    itlib::atomic_relaxed_counter<uint8_t> cntb{0}; // will likely overflow

    itlib::atomic_relaxed_counter<bool> start{false};

    std::thread a([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            ++cnta;
            cntb += 10;
        }
    });

    std::thread b([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            --cnta;
            cntb -= 5;
        }
    });

    start = true;
    a.join();
    b.join();

    CHECK(cnta == 0);
    CHECK(cntb == 250);
}
