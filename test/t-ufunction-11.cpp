#include "doctest.hpp"

#include <type_traits>
#include <itlib/ufunction.hpp>

TEST_SUITE_BEGIN("ufunction");

struct fnocopy
{
    fnocopy() = default;
    fnocopy(const fnocopy&) = delete;
    fnocopy& operator=(const fnocopy&) = delete;
    fnocopy(fnocopy&& other) noexcept
    {
        other.owner = false;
    }
    fnocopy& operator=(fnocopy&& other) noexcept
    {
        other.owner = false;
        return *this;
    }
    int operator()(int n) { return n+5;}

    bool owner = true;
};

TEST_CASE("Basic")
{
    using namespace itlib;

    fnocopy x;
    ufunction<int(int)> func(std::move(x));
    CHECK(x.owner == false);
    CHECK(func(4) == 9);

    auto f2 = std::move(func);
    CHECK(!func);
    CHECK(f2(1) == 6);
}

