#include "doctest.hpp"

#include <type_traits>
#include <itlib/ufunction.hpp>
#include <itlib/make_ptr.hpp>

TEST_SUITE_BEGIN("ufunction");

TEST_CASE("Basic")
{
    using namespace itlib;

    auto uptr = make_unique(53);
    ufunction<void()> func([u = std::move(uptr)](){
        CHECK(*u == 53);
    });
    func();

    auto uptr2 = make_unique(102);
    func = [u = std::move(uptr2)]() {
        CHECK(*u == 102);
    };
    func();

    auto f2 = std::move(func);
    CHECK(!func);
    f2();
}

