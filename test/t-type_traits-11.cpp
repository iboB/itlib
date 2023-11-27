#include <doctest/doctest.h>

#include <itlib/type_traits.hpp>

#include <vector>
#include <string>
#include <cstdint>

#define CCHECK(...) static_assert(__VA_ARGS__, "check failed")
#define CCHECK_FALSE(...) static_assert(!(__VA_ARGS__), "check failed")

TEST_CASE("is_instantiation_of")
{
    CCHECK_FALSE(itlib::is_instantiation_of<std::vector, int>::value);

    using ivec = std::vector<int>;
    using vecvec = std::vector<ivec>;
    CCHECK(itlib::is_instantiation_of<std::vector, ivec>::value);
    CCHECK(itlib::is_instantiation_of<std::vector, vecvec>::value);
    CCHECK_FALSE(itlib::is_instantiation_of<std::vector, std::string>::value);

    CCHECK(itlib::is_instantiation_of<std::basic_string, std::string>::value);
    CCHECK_FALSE(itlib::is_instantiation_of<std::basic_string, ivec>::value);
}

TEST_CASE("type_identity") {
    CCHECK(std::is_same<itlib::type_identity<int>::type, int>::value);

    extern void(&foo)(int, float);
    CCHECK(std::is_same<itlib::type_identity<decltype(foo)>::type, void(&)(int, float)>::value);
}

enum class E32 : int32_t {};
struct S32 { int x; };

TEST_CASE("is_noop_convertible") {
    CCHECK(itlib::is_noop_convertible<int, int>::value);
    CCHECK(itlib::is_noop_convertible<int, unsigned int>::value);
    CCHECK(itlib::is_noop_convertible<uint8_t, char>::value);
    CCHECK(itlib::is_noop_convertible<int8_t, char>::value);
    CCHECK(itlib::is_noop_convertible<uint8_t, char>::value);
    CCHECK(itlib::is_noop_convertible<E32, int>::value);
    CCHECK(itlib::is_noop_convertible<void*, size_t>::value);
    CCHECK(itlib::is_noop_convertible<void*, intptr_t>::value);
    CCHECK(itlib::is_noop_convertible<double, double>::value);

    CCHECK_FALSE(itlib::is_noop_convertible<int, float>::value);
    CCHECK_FALSE(itlib::is_noop_convertible<E32, float>::value);
    CCHECK_FALSE(itlib::is_noop_convertible<int, S32>::value);
    CCHECK_FALSE(itlib::is_noop_convertible<void*, float>::value);
    CCHECK_FALSE(itlib::is_noop_convertible<void*, double>::value);
}
