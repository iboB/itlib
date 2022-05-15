#include "doctest.hpp"

#include <itlib/stride_span.hpp>

#include <vector>
#include <cstring>

TEST_CASE("[stride_span] construction and iteration")
{
    using namespace itlib;

    {
        stride_span<int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.begin() == e.end());
        CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        CHECK(e.stride() == 4);
        CHECK(e.data() == nullptr);
    }

    {
        stride_span<const int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.begin() == e.end());
        CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        CHECK(e.stride() == 4);
        CHECK(e.data() == nullptr);
    }

    int i[] = {1,11,2,22,3,33,4,44};

    {
        stride_span<int> eints(reinterpret_cast<uint8_t*>(i), sizeof(int) * 2, 4);
        CHECK(eints);
        CHECK(!eints.empty());
        CHECK(eints.size() == 4);
        CHECK(eints.begin() + 4 == eints.end());
        CHECK(*eints.rbegin() == 4);

        CHECK(eints.front() == 1);
        CHECK(eints.back() == 4);

        CHECK(eints[0] == 1);

        int sum = 0;
        for (auto i : eints)
        {
            sum += i;
        }
        CHECK(sum == 10);

        sum = 0;
        for (auto i = eints.rbegin(); i != eints.rend(); ++i)
        {
            sum *= 10;
            sum += *i;
        }
        CHECK(sum == 4321);

        eints[1] = 5;
        CHECK(i[2] == 5);
    }

    {
        stride_span<const int> eints(reinterpret_cast<uint8_t*>(i + 1), sizeof(int) * 2, 4);
        CHECK(eints);
        CHECK(!eints.empty());
        CHECK(eints.size() == 4);
        CHECK(eints.begin() + 4 == eints.end());

        CHECK(*eints.rbegin() == 44);

        CHECK(eints.front() == 11);
        CHECK(eints.back() == 44);

        CHECK(eints[0] == 11);

        int sum = 0;
        for (auto i : eints)
        {
            sum += i;
        }
        CHECK(sum == 110);

        sum = 0;
        for (auto i = eints.rbegin(); i != eints.rend(); ++i)
        {
            sum *= 100;
            sum += *i;
        }
        CHECK(sum == 44332211);
    }
}
