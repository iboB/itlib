// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/mutex.hpp>

#include <atomic>

TEST_CASE("strong_try_rec_mutex")
{
    using mut = itlib::strong_try_rec_mutex;
    static constexpr int n = 10;

    auto run = [](mut& m) {
        for (int i=0;i<n;++i) {
            CHECK(m.try_lock());
        }
        for (int i=0;i<n;++i) {
            m.unlock();
        }
    };

    SUBCASE("basic")
    {
        mut a;
        CHECK(a.try_lock());
        CHECK(a.try_lock());
        a.unlock();
        a.unlock();
    }
    SUBCASE("no contend")
    {
        std::atomic_bool go = {false};
        mut ma, mb;
        std::thread ta([&]() {
            while (!go);
            run(ma);
        });
        go = true;
        run(mb);
        ta.join();
    }
    SUBCASE("contend")
    {
        std::atomic_bool go = {false};
        mut ma;
        std::thread ta([&]() {
            while (!go);
            while (!ma.try_lock());
            run(ma);
            ma.unlock();
        });
        go = true;
        while (!ma.try_lock());
        run(ma);
        ma.unlock();
        ta.join();
    }
    SUBCASE("try fail")
    {
        mut m;
        CHECK(m.try_lock());
        std::thread ta([&]() {
            CHECK_FALSE(m.try_lock());
        });
        CHECK(m.try_lock());
        m.unlock();
        std::thread tb([&]() {
            CHECK_FALSE(m.try_lock());
        });
        ta.join();
        CHECK(m.try_lock());
        tb.join();
        m.unlock();
        m.unlock();
    }
}
