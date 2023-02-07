// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/data_mutex.hpp>

#include <itlib/qalgorithm.hpp>

#include <vector>
#include <mutex>
#include <cstdlib>
#include <random>

#include <doctest/doctest.h>

TEST_CASE("data_mutex std::mutex") {
    // this test is mostly about tsan
    // it's not testing any functionality (and there barely is any to test)

    std::vector<int> init;
    for (int i = 0; i < 100; ++i) {
        init.push_back(rand() % 4 == 0);
    }

    itlib::data_mutex<std::vector<int>, std::mutex> bools(init);

    std::thread writer([&]() {
        std::minstd_rand rnd;
        for (;; std::this_thread::yield()) {
            auto l = bools.unique_lock();
            auto f = itlib::pfind(*l, 1);
            if (!f) return;
            *f = rnd() % 2 == 0;
        }
    });

    std::thread reader([&] {
        const auto& b = bools;
        while (true) {
            if (itlib::qnone_of(*b.unique_lock())) return;
            std::this_thread::yield();
        }
    });

    writer.join();
    reader.join();
    auto l = bools.try_unique_lock();
    REQUIRE(l);
    CHECK(itlib::qnone_of(*l));
}
