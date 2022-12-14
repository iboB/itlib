// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/atomic_shared_ptr_storage.hpp>

#include <thread>
#include <atomic>

TEST_CASE("[itlib::atomic_shared_ptr_storage] basic") {
    static_assert(sizeof(itlib::atomic_shared_ptr_storage<int>) <= 64, "We want true sharing here");

    {
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

    {
        auto ptr1 = std::make_shared<int>(11);
        itlib::atomic_shared_ptr_storage<int> pi(ptr1);
        auto ptr2 = std::make_shared<int>(32);
        auto ret = pi.exchange(ptr2);
        CHECK(ret == ptr1);
        CHECK(pi.load() == ptr2);

        auto ptr3 = std::make_shared<int>(99);
        CHECK_FALSE(pi.compare_exchange(ptr1, ptr3));
        CHECK(ptr1 == ptr2);
        CHECK(pi.load() == ptr2);
        CHECK(pi.compare_exchange(ptr2, ptr3));
        CHECK(ptr1 == ptr2);
        CHECK(pi.load() == ptr3);

        CHECK(pi.compare_exchange(ptr3, {}));
        CHECK_FALSE(pi.load());

        ret = pi.exchange({});
        CHECK_FALSE(ret);

        std::shared_ptr<int> empty;
        CHECK(pi.compare_exchange(empty, ptr1));
        CHECK_FALSE(empty);
        CHECK(pi.load() == ptr1);

        ret = pi.exchange(ptr1);
        CHECK(ret == ptr1);
    }
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

TEST_CASE("[itlib::atomic_shared_ptr_storage] exchange") {
    // no sensible checks here as well
    // just confirm that there are no crashes and no sanitizer complaints

    std::atomic<bool> start{false};
    std::atomic<int> sum{0};

    auto init = std::make_shared<int>(-1);
    itlib::atomic_shared_ptr_storage<int> storage(init);

    auto a = std::make_shared<int>(1);
    auto b = std::make_shared<int>(2);

    std::thread ta([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            auto ret = storage.exchange(a);
            if (ret != a) ++sum;
        }
    });

    std::thread tb([&]() {
        while (!start);
        for (int i = 0; i < 50; ++i) {
            auto ac = a;
            auto success = storage.compare_exchange(ac, b);
            if (!success) {
                CHECK((ac == b || ac == init));
            }
            else {
                ++sum;
                CHECK(ac == a);
            }
        }
    });

    start = true;
    ta.join();
    tb.join();

    // if thread b completed all of its iterations before a managed to do one,
    // we will end up with 100% fail rate in b and a stored in storage
    sum += storage.compare_exchange(a, b);

    CHECK(sum >= 2);
}
