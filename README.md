itlib: iboB's Template Libraries
================================

A collection of small single-header C++ libraries similar to or extending the C++ standard library. See below for a list.

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

itlib was forked from [chobo-shl](https://github.com/Chobolabs/chobo-shl) which is no longer supported. New libraries and updates to the existing ones are added here.

## Build Status

Building with GitHub actions on Windows with MSVC, Ubuntu with GCC, and macOS with clang. Debug and Release. With address sanitizer.

[![Build](https://github.com/iboB/itlib/workflows/Build/badge.svg)](https://github.com/iboB/itlib/actions?query=workflow%3ABuild)

## Libraries

Every `.hpp` file in `include/itlib` is a standalone library and has no dependencies other than the standard lib.

Documentation is provided in comments at the top of each file.

In the list below each library shows its minimum supported C++ standard and has icons for other standards if additional features are available for them.

### [dynamic_bitset.hpp](https://github.com/iboB/itlib/blob/master/include/itlib/dynamic_bitset.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A class similar to `std::bitset`, but the number of bits is not a part of the type. It's also somewhat similar to `std::vector<bool>;`, but (so far) it has more limited modification capabilities.

### [flat_map.hpp](https://github.com/iboB/itlib/blob/master/include/itlib/flat_map.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A class with the interface of `std::map` but implemented with an underlying `std::vector`-type container, thus providing better cache locality of the elements. Similar to [`boost::flat_map`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/flat_map.html) with the notable difference that the underlying container can be changed via a template argument (thus making the class not strictly an `std::map` drop-in replacement)

### [make_ptr.hpp](https://github.com/iboB/itlib/blob/master/include/itlib/make_ptr.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

Small helper functions for creating `std::shared_ptr` and `std::unique_ptr` which make the code shorter and more readable.

### [mem_streambuf.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/mem_streambuf.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

Two helper classes: `mem_ostreambuf` and `mem_istreambuf` which allow you to work with `std::stream`-s with buffers of contiguous memory.

### [memory_view.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/memory_view.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A class which provides a `std::vector` like interface (sans the methods which might change the size or capacity) to a chunk of memory. Similar to C++20's `std::span`

### [pod_vector.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/pod_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A container similar to `std::vector`, which contains PODs. This fact is used to improve performance by skipping constructor and destructor calls and using `memcpy` and `memmove` to copy data, and `malloc` and `free`, and, most importantly `realloc`, to manage memory.

### [sentry.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/sentry.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A sentry class which executes a function object on destruction. Works with C++11, but it's slightly easier to use with C++17.

### [small_vector.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/small_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A mix between `std::vector` and `itlib::static_vector`. It's a dynamic array, optimized for use when the number of elements is small. Like `static_vector` is has a static buffer with a given capacity, but can fall back to dynamically allocated memory, should the size exceed it. Similar to [`boost::small_vector`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/small_vector.html)

### [static_vector.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/static_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

A mix between `std::vector` and `std::array`: A dynamically sized container with fixed capacity (supplied as a template parameter). This allows you to have dynamically sized vectors on the stack or as cache-local value members, as long as you know a big enough capacity beforehand. Similar to [`boost::static_vector`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/static_vector.html).

### [ufunction.hpp](https://github.com/iboB/itlib/tree/master/include/itlib/ufunction.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![Standard](https://img.shields.io/badge/C%2B%2B-14-yellow.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)

Unique function. A replacement of `std::function` which is non-copyable (can capture non-copyable values, and wrap non-copyable objects), and noexcept move-constructible (won't implicitly make owners no-noexcept move-constructible)

## Usage

Clone the repo or choose one or more libraries that you like and copy them somewhere in your include paths.

Every library is self-contained so you can copy, move, and modify whichever you like and not wory about interdependencies.

## Contributing

Pull requests and issues are welcome.

Please make one pull request and issue per library, tagging them with the library name in the title with brackets. Example:

* *[small_vector] Added insert methods*
* *[flat_map] Crash when using with xxxx container*

You can use CMake to generate a project and run the tests locally.

## Copyright

Copyright &copy; 2016-2019 [Chobolabs Inc.](http://www.chobolabs.com/)

Copyright &copy; 2020 Borislav Stanimirov

These libraries are distributed under the MIT Software License. See LICENSE.txt for further details or copy [here](http://opensource.org/licenses/MIT).
