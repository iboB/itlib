// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <itlib/shared_func.hpp>
#include <doctest/doctest.h>

using itlib::shared_func;

TEST_CASE("empty") {
    SUBCASE("empty") {
        shared_func<void(int, char)> func;
        CHECK_FALSE(func);
    }
    SUBCASE("nullptr") {
        shared_func<void(int, char)> func = nullptr;
        CHECK_FALSE(func);
    }
}

TEST_CASE("lambda") {
    int acc = 0;
    {
        shared_func<void(int)> func{[&acc](int n) { acc += n; }};
        CHECK(func);
        func(5);
        CHECK(acc == 5);
        func(10);
        CHECK(acc == 15);

        auto f2 = func;
        CHECK(f2);
        f2(3);
        CHECK(acc == 18);

        CHECK(func.use_count() == 2);
    }
}

int sum(int a, int b) { return a + b; }

TEST_CASE("free func") {
    shared_func<int(int, int)> func{sum};
    CHECK(func(1, 2) == 3);
    func.reset([](int a, int b) { return a * b; });
    CHECK(func(3, 4) == 12);
    func.reset(sum);
    CHECK(func(3, 4) == 7);
}

struct fnocopy {
    fnocopy() = default;
    fnocopy(const fnocopy&) = delete;
    fnocopy& operator=(const fnocopy&) = delete;
    fnocopy(fnocopy&& other) noexcept {
        other.owner = false;
    }
    fnocopy& operator=(fnocopy&& other) noexcept {
        other.owner = false;
        return *this;
    }
    int operator()(int n) { return acc += n; }

    int acc = 0;
    bool owner = true;
};

TEST_CASE("no copy") {
    fnocopy f;
    shared_func<int(int)> func{std::move(f)};
    CHECK(f.owner == false);

    CHECK(func(5) == 5);
    CHECK(func(10) == 15);
    CHECK(func.use_count() == 1);

    auto f2 = func;
    CHECK(f2.use_count() == 2);
    CHECK(f2(15) == 30);
}

TEST_CASE("weak_func") {
    fnocopy f;
    shared_func<int(int)> func{std::move(f)};
    itlib::weak_func<int(int)> wfunc{func};
    CHECK(func.use_count() == 1);
    CHECK_FALSE(wfunc.expired());
    {
        auto locked = wfunc.lock();
        CHECK(locked.use_count() == 2);
        CHECK(locked(5) == 5);
        CHECK(locked(10) == 15);
    }
    func.reset();
    CHECK(func.use_count() == 0);
    CHECK(wfunc.expired());
}
