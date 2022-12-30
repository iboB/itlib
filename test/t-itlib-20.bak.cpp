#include <doctest/doctest.h>

TEST_CASE("__cplusplus")
{
    CHECK(__cplusplus >= 202000);
}
