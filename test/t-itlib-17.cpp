#include "doctest.hpp"

TEST_CASE("__cplusplus")
{
    CHECK(__cplusplus >= 201700);
}
