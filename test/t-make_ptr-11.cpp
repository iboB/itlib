#include <doctest/doctest.h>

#include <itlib/make_ptr.hpp>

#include <vector>

// oh, well... just use a macro
#define maker_test(maker) \
std::vector<int> vec = { 1, 2, 3 }; \
auto copy = maker(vec); \
CHECK(copy->size() == 3); \
CHECK(vec.size() == 3); \
CHECK(vec.data() != copy->data()); \
copy->at(1) = 5; \
CHECK(*copy == std::vector<int>({1, 5, 3})); \
auto vdata = vec.data(); \
auto heist = maker(std::move(vec)); \
CHECK(heist->size() == 3); \
CHECK(heist->data() == vdata); \


TEST_CASE("[make-ptr:make_shared] test")
{
    maker_test(itlib::make_shared);
}

TEST_CASE("[make-ptr:make_unique] test")
{
    maker_test(itlib::make_unique);
}
