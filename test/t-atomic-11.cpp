#include <doctest/doctest.h>

#include <itlib/atomic.hpp>

#include <thread>

TEST_CASE("atomic_relaxed_counter") {
    itlib::atomic_relaxed_counter<int> cnta = 0;
    itlib::atomic_relaxed_counter<uint8_t> cntb = 0; // will likely overflow

    itlib::atomic_relaxed_counter<bool> start = false;

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