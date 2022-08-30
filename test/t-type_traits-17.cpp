#include <doctest/doctest.h>

#include <itlib/type_traits.hpp>

#include <vector>
#include <string>

TEST_CASE("is_instantiation_of")
{
    CHECK_FALSE(itlib::is_instantiation_of_v<std::vector, int>);

    using ivec = std::vector<int>;
    using vecvec = std::vector<ivec>;
    CHECK(itlib::is_instantiation_of_v<std::vector, ivec>);
    CHECK(itlib::is_instantiation_of_v<std::vector, vecvec>);
    CHECK_FALSE(itlib::is_instantiation_of_v<std::vector, std::string>);

    CHECK(itlib::is_instantiation_of_v<std::basic_string, std::string>);
    CHECK_FALSE(itlib::is_instantiation_of_v<std::basic_string, ivec>);
}
