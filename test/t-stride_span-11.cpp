#include "doctest.hpp"

#include <itlib/stride_span.hpp>

#include <vector>
#include <cstring>

TEST_CASE("[stride_span] construction")
{
    using namespace itlib;

    {
        stride_span<int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.begin() == e.end());
        // CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        // CHECK(e.data() == nullptr);
    }
}
