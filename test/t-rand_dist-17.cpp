// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/rand_dist.hpp>
#include <doctest/doctest.h>
#include <deque>
#include <stdexcept>

template <typename R, R Min = 0, R Max = std::numeric_limits<R>::max()>
struct i_test_rng {
    using result_type = R;
    static constexpr result_type min() { return Min; }
    static constexpr result_type max() { return Max; }

    R current;

    explicit i_test_rng(R start = Min)
        : current(start)
    {}

    void reset(R start = Min) {
        current = start;
    }

    result_type operator()() {
        if (current > Max) {
            current = Min;
        }
        return current++;
    }
};

template <typename R, R Min = 0, R Max = std::numeric_limits<R>::max()>
struct q_test_rng {
    using result_type = R;

    static constexpr result_type min() { return Min; }
    static constexpr result_type max() { return Max; }

    std::deque<result_type> values;

    q_test_rng() = default;
    explicit q_test_rng(std::initializer_list<result_type> vals) {
        push(vals);
    }

    void push(result_type v) {
        if (v < Min || v > Max) {
            throw std::runtime_error("test_rng: bad value");
        }
        values.push_back(v);
    }

    void push(std::initializer_list<result_type> vals) {
        for (auto v : vals) {
            push(v);
        }
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
        i_test_rng<uint8_t> rng;
        itlib::uniform_uint_max_distribution<uint8_t> dist(10);
        for (uint8_t i = 0; i <= 4; ++i) {
            auto v = dist(rng);
            CHECK(v == i);
        }
    }
    SUBCASE("rejection") {
        // rng range 0-5, max 3
        q_test_rng<uint8_t, 0, 5> rng{
            0, // ok -> 0
            1, // ok -> 1
            2, // ok -> 2
            4, // reject
            5, // reject
            0, // ok -> 0
        };
        itlib::uniform_uint_max_distribution<uint8_t> dist(3);
        for (uint8_t i = 0; i < 3; ++i) {
            auto v = dist(rng);
            CHECK(v == i);
        }
        auto v = dist.roll(3, rng);
        CHECK(v == 0);
        CHECK(rng.values.empty());
    }
}

TEST_CASE("uniform_int_distribution") {
    SUBCASE("trivial") {
        i_test_rng<uint32_t> rng;
        itlib::uniform_int_distribution<int32_t> dist(-5, 4);
        for (int32_t expected = -5; expected <= 4; ++expected) {
            auto v = dist(rng);
            CHECK(v == expected);
        }
    }
}
