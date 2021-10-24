#include "doctest.hpp"

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
}
