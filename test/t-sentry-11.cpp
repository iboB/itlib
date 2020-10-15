#include "doctest.hpp"

#include <type_traits>
#include <itlib/sentry.hpp>

#include <memory>
#include <string>

TEST_SUITE_BEGIN("sentry");

TEST_CASE("Basic")
{
    using namespace itlib;

    int i = 0;
    {
        auto s = make_sentry([&i]() { i = 3; });
        CHECK(i == 0);
    }
    CHECK(i == 3);
}
