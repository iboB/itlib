#include <doctest/doctest.h>

#include <itlib/span.hpp>

#include <vector>
#include <string>

TEST_CASE("[span] deduction")
{
    using namespace itlib;

    int x[] = { 1, 2, 3 };
    span xs(x, x + 3);
    static_assert(std::is_same<span<int>, decltype(xs)>::value, "must deduce span<int>");
    CHECK(xs.size() == 3);
    CHECK(xs.size_bytes() == 12); // 3*4

    span xs2(x, 3);
    static_assert(std::is_same<span<int>, decltype(xs2)>::value, "must deduce span<int>");
    CHECK(xs2.size() == 3);
    CHECK(xs2.size_bytes() == 12);

    const int* cx = x;
    span cxs(cx, cx + 3);
    static_assert(std::is_same<span<const int>, decltype(cxs)>::value, "must deduce span<const int>");
    CHECK(cxs.size() == 3);
    CHECK(cxs.size_bytes() == 12);

    span cxs2(cx, 3);
    static_assert(std::is_same<span<const int>, decltype(cxs2)>::value, "must deduce span<const int>");
    CHECK(cxs2.size() == 3);
    CHECK(cxs2.size_bytes() == 12);

    std::vector vec = { 1, 2, 3 };
    span vs = vec;
    static_assert(std::is_same<span<int>, decltype(vs)>::value, "must deduce span<int>");
    CHECK(vs.size() == 3);
    CHECK(vs.size_bytes() == 12);

    const auto& cvec = vec;
    span cvs = cvec;
    static_assert(std::is_same<span<const int>, decltype(cvs)>::value, "must deduce span<const int>");
    CHECK(cvs.size() == 3);
    CHECK(cvs.size_bytes() == 12);

    std::string foo = "xyz";
    span ss = foo;
    static_assert(std::is_same<span<char>, decltype(ss)>::value, "must deduce span<const int>");
    CHECK(ss.size() == 3);
    CHECK(ss.size_bytes() == 3);
}
