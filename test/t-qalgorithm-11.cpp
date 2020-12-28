#include "doctest.hpp"

#include <itlib/qalgorithm.hpp>

#include <vector>

TEST_CASE("find")
{
    std::vector<int> vec = {1,2,3,4};
    auto i = itlib::qfind(vec, 1);
    CHECK(std::is_same<std::vector<int>::iterator, decltype(i)>::value);
    CHECK(i == vec.begin());
    CHECK(*i == 1);
    *i = 100;
    CHECK(vec[0] == 100);

    i = itlib::qfind(vec, 5);
    CHECK(i == vec.end());

    auto p = itlib::pfind(vec, 2);
    CHECK(std::is_same<int*, decltype(p)>::value);
    CHECK(p != nullptr);
    CHECK(*p == 2);

    p = itlib::pfind(vec, 5);
    CHECK(!p);

    i = itlib::qfind_if(vec, [](int i) { return i < 3; });
    CHECK(i != vec.end());
    CHECK(*i == 2);

    i = itlib::qfind_if(vec, [](int i) { return i > 300; });
    CHECK(i == vec.end());

    p = itlib::pfind_if(vec, [](int i) { return i < 3; });
    CHECK(p != nullptr);
    CHECK(*p == 2);

    p = itlib::pfind_if(vec, [](int i) { return i > 300; });
    CHECK(!p);

    const auto& cvec = vec;

    auto ci = itlib::qfind(cvec, 100);
    CHECK(std::is_same<std::vector<int>::const_iterator, decltype(ci)>::value);
    CHECK(ci == cvec.begin());
    CHECK(*ci == 100);

    ci = itlib::qfind(cvec, 5);
    CHECK(ci == vec.end());

    auto cp = itlib::pfind(cvec, 2);
    CHECK(std::is_same<const int*, decltype(cp)>::value);
    CHECK(cp != nullptr);
    CHECK(*cp == 2);

    cp = itlib::pfind(cvec, 5);
    CHECK(!cp);

    ci = itlib::qfind_if(cvec, [](int i) { return i < 3; });
    CHECK(ci != cvec.end());
    CHECK(*ci == 2);

    ci = itlib::qfind_if(cvec, [](int i) { return i > 300; });
    CHECK(ci == cvec.end());

    cp = itlib::pfind_if(cvec, [](int i) { return i < 3; });
    CHECK(cp != nullptr);
    CHECK(*cp == 2);

    cp = itlib::pfind_if(cvec, [](int i) { return i > 300; });
    CHECK(!cp);
}
