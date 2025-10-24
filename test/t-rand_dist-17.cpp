// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/rand_dist.hpp>
#include <doctest/doctest.h>
#include <deque>
#include <stdexcept>

template <typename R, R Min = 0, R Max = std::numeric_limits<R>::max()>
struct test_rng {
    using result_type = R;

    static constexpr result_type min() { return Min; }
    static constexpr result_type max() { return Max; }

    std::deque<result_type> values;

    test_rng() = default;
    explicit test_rng(std::initializer_list<result_type> vals) {
        push(vals);
    }

    test_rng& push(result_type v) {
        if (v < Min || v > Max) {
            throw std::runtime_error("test_rng: bad value");
        }
        values.push_back(v);
        return *this;
    }

    test_rng& push(std::initializer_list<result_type> vals) {
        for (auto v : vals) {
            push(v);
        }
        return *this;
    }

    result_type operator()() {
        if (values.empty()) {
            throw std::runtime_error("test_rng: no more values");
        }
        auto v = values.front();
        values.pop_front();
        return v;
    }
};

TEST_CASE("uniform_uint_max_distribution") {
    SUBCASE("trivial") {
        test_rng<uint8_t> rng{0, 1, 2, 3, 4};
        itlib::uniform_uint_max_distribution<uint8_t> dist(10);
        for (uint8_t i = 0; i <= 4; ++i) {
            auto v = dist(rng);
            CHECK(v == i);
        }
    }
    SUBCASE("rejection") {
        // rng range 0-5, max 3
        test_rng<uint8_t, 0, 5> rng{
            0, // ok -> 0
            1, // ok -> 1
            2, // ok -> 2
            3, // ok -> 3
            4, // reject
            5, // reject
            0, // ok -> 0
        };
        itlib::uniform_uint_max_distribution<uint8_t> dist(3);
        for (uint8_t i = 0; i <= 3; ++i) {
            auto v = dist(rng);
            CHECK(v == i);
        }
        auto v = dist.roll(3, rng);
        CHECK(v == 0);
        CHECK(rng.values.empty());
    }
}


