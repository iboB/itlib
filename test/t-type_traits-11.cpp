#include <doctest/doctest.h>

#include <itlib/type_traits.hpp>

#include <vector>
#include <string>

TEST_CASE("is_instantiation_of")
{
    CHECK_FALSE(itlib::is_instantiation_of<std::vector, int>::value);

    using ivec = std::vector<int>;
    using vecvec = std::vector<ivec>;
    CHECK(itlib::is_instantiation_of<std::vector, ivec>::value);
    CHECK(itlib::is_instantiation_of<std::vector, vecvec>::value);
    CHECK_FALSE(itlib::is_instantiation_of<std::vector, std::string>::value);

    CHECK(itlib::is_instantiation_of<std::basic_string, std::string>::value);
    CHECK_FALSE(itlib::is_instantiation_of<std::basic_string, ivec>::value);
}
