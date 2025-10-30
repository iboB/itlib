// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/rand_dist.hpp>
#include <doctest/doctest.h>
#include <deque>

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

    result_type operator()() noexcept {
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
    ~q_test_rng() {
        CHECK(values.empty());
    }

    void push(result_type v) {
        if (v < Min || v > Max) {
            REQUIRE(false);
        }
        values.push_back(v);
    }

    void push(std::initializer_list<result_type> vals) {
        for (auto v : vals) {
            push(v);
        }
    }

    result_type operator()() noexcept {
        if (values.empty()) {
            REQUIRE(false);
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
        CHECK(dist.min() == 0);
        CHECK(dist.max() == 10);
        for (uint8_t i = 0; i <= 4; ++i) {
            auto v = dist(rng);
            CHECK(v == i);
        }
    }
    SUBCASE("rejection") {
        SUBCASE("full rng range") {
            itlib::uniform_uint_max_distribution<uint8_t> dist(9);

            q_test_rng<uint8_t> rng{
                104, // ok -> 4
                254, // reject
                232, // ok -> 2
                251, // reject
                18,  // ok -> 8
                250, // reject
                249, // ok -> 9
            };

            CHECK(dist(rng) == 4);
            CHECK(dist(rng) == 2);
            CHECK(dist(rng) == 8);
            CHECK(dist(rng) == 9);
        }
        SUBCASE("0-5 rng range") {
            itlib::uniform_uint_max_distribution<uint8_t> dist(3);

            q_test_rng<uint8_t, 0, 5> rng{
                0, // ok -> 0
                1, // ok -> 1
                2, // ok -> 2
                4, // reject
                5, // reject
                0, // ok -> 0
            };

            for (uint8_t i = 0; i < 3; ++i) {
                auto v = dist(rng);
                CHECK(v == i);
            }
            auto v = dist(rng);
            CHECK(v == 0);
            CHECK(rng.values.empty());
        }
    }
    SUBCASE("multi-roll") {
        SUBCASE("range 0 - 3, max 10") {
            // 10 in base 4 is 22
            itlib::uniform_uint_max_distribution<uint8_t> dist(10);

            q_test_rng<uint8_t, 0, 3> rng{
                2, // accept
                1, // accept => 9
            };
            auto v1 = dist(rng);
            CHECK(v1 == 9);

            rng.push({
                3, // reject
                0, // accept
                3, // accept => 3
            });
            v1 = dist(rng);
            CHECK(v1 == 3);

            rng.push({
                3, // reject
                2, // accept
                2, // accept => 10
            });
            v1 = dist(rng);
            CHECK(v1 == 10);

            rng.push({
                3, // reject
                2, // accept tight
                3, // reject and restart
                1, // accept
                3, // accept => 7
            });
            v1 = dist(rng);
            CHECK(v1 == 7);
        }
    }
}

TEST_CASE("uniform_int_distribution") {
    SUBCASE("trivial") {
        i_test_rng<uint32_t> rng;
        itlib::uniform_int_distribution<int32_t> dist(-5, 4);
        CHECK(dist.min() == -5);
        CHECK(dist.max() == 4);
        for (int32_t expected = -5; expected <= 4; ++expected) {
            auto v = dist(rng);
            CHECK(v == expected);
        }
    }
}

TEST_CASE("fast_uniform_real_distribution") {
    SUBCASE("bigger rng range") {
        itlib::fast_uniform_real_distribution<float> dist;
        q_test_rng<uint32_t> rng{
            0u,
            1u << 24,
            (1u << 24) - 1, // almost 1
            1u << 23,
        };
        CHECK(dist(rng) == 0.0f);
        CHECK(dist(rng) == 0.0f);

        auto f = dist(rng);
        CHECK(f > 0.999999f);
        CHECK(f < 1);
        CHECK(dist(rng) == 0.5);
    }
    SUBCASE("smaller rng range") {
        using dist = itlib::fast_uniform_real_distribution<double>;
        q_test_rng<uint16_t> rng{
            0u,
            1u,
            65535u, // almost 1
            32768u,
        };

        CHECK(dist::draw_01(rng) == 0.0);
        CHECK(dist::draw_01(rng) == double(1) / 65536.0);
        CHECK(dist::draw_01(rng) == 65535.0 / 65536.0);
        CHECK(dist::draw_01(rng) == 0.5);
    }
}

#include <random>
#include <initializer_list>

template <typename F, typename R>
void test_nondeterministic_reals(R& rng) {
    constexpr int lim = 10000;
    {
        itlib::fast_uniform_real_distribution<F> dist(1.0f, 2.0f);
        double sum = 0.0f;
        for (int i = 0; i <lim ; ++i) {
            auto v = dist(rng);
            CHECK(v >= 1.0f);
            CHECK(v < 2.0f);
            sum += v;
        }
        auto avg = sum / lim;
        CHECK(avg == doctest::Approx(1.5).epsilon(0.1));
    }
}

template <typename R>
void test_deterministic_ints_neg50_100(R& rng, std::initializer_list<int> expected) {
    itlib::uniform_int_distribution<int> dist(-50, 100);
    for (auto e : expected) {
        auto v = dist(rng);
        CHECK(v == e);
    }
}

TEST_CASE("std compat") {
    SUBCASE("minstd_rand") {
        std::minstd_rand rng(1234);
        test_nondeterministic_reals<float>(rng);
        test_nondeterministic_reals<double>(rng);
        test_deterministic_ints_neg50_100(rng, {-48, 89, -34, 57, 7, 91, 49, 71, 5, 24});
    }
    SUBCASE("mt19937") {
        std::mt19937 rng(5678);
        test_nondeterministic_reals<float>(rng);
        test_nondeterministic_reals<double>(rng);
        test_deterministic_ints_neg50_100(rng, {19, 88, 63, -25, 45, 86, 67, -50, 46, 61});
    }
    SUBCASE("mt19937_64") {
        std::mt19937_64 rng(91011);
        test_nondeterministic_reals<float>(rng);
        test_nondeterministic_reals<double>(rng);
        test_deterministic_ints_neg50_100(rng, {80, 57, 74, 95, 22, -24, 5, 44, 92, -30});
    }
}
