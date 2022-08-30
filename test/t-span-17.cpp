#include <doctest/doctest.h>

#include <itlib/span.hpp>

TEST_CASE("[span] deduction")
{
    using namespace itlib;

    int x[] = { 1, 2, 3 };
    span xs(x, x + 3);
    static_assert(std::is_same<span<int>, decltype(xs)>::value, "must deduce span<int>");
    CHECK(xs.size() == 3);
    CHECK(xs.byte_size() == 3 * 4);

    span xs2(x, 3);
    static_assert(std::is_same<span<int>, decltype(xs2)>::value, "must deduce span<int>");

    const int* cx = x;
    span cxs(cx, cx + 3);
    static_assert(std::is_same<span<const int>, decltype(cxs)>::value, "must deduce span<const int>");
    span cxs2(cx, 3);
    static_assert(std::is_same<span<const int>, decltype(cxs2)>::value, "must deduce span<const int>");
}
