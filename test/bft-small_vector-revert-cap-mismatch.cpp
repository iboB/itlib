#include <itlib/small_vector.hpp>

// build error: exceed the static capacity by more than one
itlib::small_vector<int, 10, 12> vec;
