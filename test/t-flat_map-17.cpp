#include <doctest/doctest.h>

#include <itlib/flat_map.hpp>
#include <string>
#include <string_view>

// struct with no operator==
TEST_CASE("[flat_map] std::string_view transparency")
{
    itlib::flat_map<std::string, int> map = {{"querty", 13}, {"asdf", 42}};
    std::string_view asdf = "asdf";
    CHECK(map[asdf] == 42);
    CHECK(map.find(asdf) == map.begin());
    CHECK(map.find(std::string_view("querty")) != map.end());
    CHECK(map.find(std::string_view("foo")) == map.end());

    std::string_view zxcv = "zxcv";
    CHECK(map.find(zxcv) == map.end());
    map[zxcv] = 123;
    CHECK(map[zxcv] == 123);
    CHECK(map["zxcv"] == 123);

    CHECK(map[std::string_view("yuiop")] == 0);
    CHECK(map.find("yuiop") != map.end());
}
