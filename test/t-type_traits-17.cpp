#include <doctest/doctest.h>

#include <itlib/type_traits.hpp>

#include <vector>
#include <string>

#define CCHECK(...) static_assert(__VA_ARGS__, "check failed")
#define CCHECK_FALSE(...) static_assert(!(__VA_ARGS__), "check failed")

TEST_CASE("is_instantiation_of")
{
    CCHECK_FALSE(itlib::is_instantiation_of_v<std::vector, int>);

    using ivec = std::vector<int>;
    using vecvec = std::vector<ivec>;
    CCHECK(itlib::is_instantiation_of_v<std::vector, ivec>);
    CCHECK(itlib::is_instantiation_of_v<std::vector, vecvec>);
    CCHECK_FALSE(itlib::is_instantiation_of_v<std::vector, std::string>);

    CCHECK(itlib::is_instantiation_of_v<std::basic_string, std::string>);
    CCHECK_FALSE(itlib::is_instantiation_of_v<std::basic_string, ivec>);
}

TEST_CASE("type_identity") {
    CCHECK(std::is_same_v<itlib::type_identity_t<int>, int>);

    extern void(&foo)(int, float);
    CCHECK(std::is_same_v<itlib::type_identity_t<decltype(foo)>, void(&)(int, float)>);
}