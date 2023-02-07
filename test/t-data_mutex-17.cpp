// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/data_mutex.hpp>

#include <itlib/qalgorithm.hpp>

#include <vector>
#include <shared_mutex>
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

    itlib::data_mutex<std::vector<int>, std::shared_mutex> bools(init);

    std::thread writer([&]() {
        std::minstd_rand rnd;
        for (;; std::this_thread::yield()) {
            if (itlib::qnone_of(*bools.shared_lock())) return;

            auto l = bools.unique_lock();
            auto f = itlib::pfind(*l, 1);
            REQUIRE(f);
            *f = rnd() % 2 == 0;
        }
    });

    auto reader_func = [&] {
        const auto& b = bools;
        while (true) {
            if (itlib::qnone_of(*b.shared_lock())) return;
            std::this_thread::yield();
        }
    };

    std::thread reader_a(reader_func);
    std::thread reader_b(reader_func);

    writer.join();
    reader_a.join();
    reader_b.join();

    auto l = bools.try_shared_lock();
    REQUIRE(l);
    CHECK(itlib::qnone_of(*l));
}
