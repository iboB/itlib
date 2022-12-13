// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/atomic_shared_ptr_storage.hpp>

#include <thread>
#include <atomic>

TEST_CASE("[itlib::atomic_shared_ptr_storage] basic") {
    itlib::atomic_shared_ptr_storage<int> pi;
    CHECK_FALSE(pi.load());
    pi.store({});
    CHECK_FALSE(pi.load());

    pi.store(std::make_shared<int>(15));
    {
        auto p = pi.load();
        CHECK(p);
        CHECK(p.use_count() == 2);
        CHECK(*p == 15);
    }

    pi.store({});
    CHECK_FALSE(pi.load());
}

TEST_CASE("[itlib::atomic_shared_ptr_storage] load/store") {
    // no sensible checks here
    // just confirm that there are no crashes and no sanitizer complaints

    std::atomic<bool> start{false};
    std::atomic<int> sum{0};

    itlib::atomic_shared_ptr_storage<int> storage(std::make_shared<int>(10000));

    std::thread a([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            sum += *storage.load();
            storage.store(std::make_shared<int>(i));
        }
    });

    std::thread b([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            sum += *storage.load();
            storage.store(std::make_shared<int>(i * 10));
        }
    });

    start = true;
    a.join();
    b.join();

    CHECK(sum > 10000);
}
