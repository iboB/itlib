
itlib: iboB's Template Libraries
================================

A collection of small single-header C++ libraries similar to or extending the C++ standard library. See below for a list.

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

itlib was forked from [chobo-shl](https://github.com/Chobolabs/chobo-shl) which is no longer supported. New libraries and updates to the existing ones are added here.

## Build Status

Building with GitHub actions on Windows with MSVC, Ubuntu with GCC, and macOS with clang. Debug and Release. With address sanitizer and thread sanitizer where applicable.

[![Build](https://github.com/iboB/itlib/workflows/Build/badge.svg)](https://github.com/iboB/itlib/actions?query=workflow%3ABuild)

## Libraries

Every `.hpp` file in `include/itlib` is a standalone library and has no dependencies other than the standard lib.

Documentation is provided in comments at the top of each file.

In the list below each library shows its minimum supported C++ standard and has icons for other standards if additional features are available for them.

 Library | Description
---------|-------------
 [**any.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/any.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | An alternative implementation of C++17's `std::any` without the limitation of required copyability for the values inside and with the possibility to set a custom allocator.
 [**atomic.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/atomic.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Utility extensions for `<atomic>`.
 [**atomic_shared_ptr_storage.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/atomic_shared_ptr_storage.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A wrapper for `std::shared_ptr<T>` which allows atomic load, store and exchange. An alternative to C++20's `std::atomic<std::shared_ptr<T>>`.
 [**data_mutex.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/data_mutex.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | A template pair of an object and a mutex used to synchronize access to it. It makes it hard to cause bugs by forgetting to lock a mutex associated with an object. 
 [**dynamic_bitset.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/dynamic_bitset.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A class similar to `std::bitset`, but the number of bits is not a part of the type. It's also somewhat similar to `std::vector<bool>`, but (so far) it has more limited modification capabilities.
 [**expected.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/expected.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A union type of a value and an error. Similar to the [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected) from C++23.
 [**flat_map.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/flat_map.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A class with the interface of `std::map` but implemented with an underlying `std::vector`-type container, thus providing better cache locality of the elements. Similar to [`boost::flat_map`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/flat_map.html) with the notable difference that the underlying container can be changed via a template argument.
 [**flat_set.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/flat_set.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A class with the interface of `std::set` but implemented with an underlying `std::vector`-type container, thus providing better cache locality of the elements. Similar to [`boost::flat_set`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/flat_set.html) with the notable difference that the underlying container can be changed via a template argument.
 [**generator.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/generator.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-20-purple.svg)](https://en.cppreference.com/w/cpp/20.html) | A helper for making simple generator coroutines with `co_yield`.
 [**mem_streambuf.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/mem_streambuf.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Two helper classes: `mem_ostreambuf` and `mem_istreambuf` which allow you to work with `std::stream`-s with buffers of contiguous memory.
 [**opt_ref_buffer.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/opt_ref_buffer.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-20-purple.svg)](https://en.cppreference.com/w/cpp/20.html) | A buffer that can either point to (reference) or own a contiguous block of memory
 [**pmr_allocator.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/pmr_allocator.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | A C++17 wrapper of `std::pmr::polymorphic_allocator` which provides functionalities introduced in C++20 for it.
 [**pod_vector.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/pod_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A container similar to `std::vector`, which contains PODs. This fact is used to improve performance by skipping constructor and destructor calls and using `memcpy` and `memmove` to copy data, and `malloc` and `free`, and, most importantly `realloc`, and `_expand` if available, to manage memory.
 [**poly_span.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/poly_span.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A class similar to C++20's `std::span` which offers a polymorphic view over a buffer of objects.
 [**qalgorithm.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/qalgorithm.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Wrappers of `<algorithm>` functions which work on entire containers for less typing in the most common use-cases.
 [**rand_dist.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/rand_dist.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | Alternative random distributions compatible with std::random.
 [**ref_ptr.hpp**](https://github.com/iboB/itlib/blob/master/include/itlib/ref_ptr.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A ref-coutning (shared) pointer with, similar to `std::shared_ptr`, but with no `weak_ptr` support. This allows reliable use of `unique()`. The main purpose is implementing copy-on-write semantics.
 [**rstream.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/rstream.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Read stream. Simple `std::istream` wrappers which don't allow seeks, allowing you to be certain that reads are sequential, and thus allow a redirect, so you can represent several streams as one.
 [**sentry.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/sentry.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | A sentry class which executes a function object on destruction. Works with C++11, but it's slightly easier to use with C++17.
 [**shared_from.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/shared_from.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A helper class to replace `std::enable_shared_from_this` providing a more powerful interface. Similar to `enable_shared_from` from [Boost.SmartPtr](https://www.boost.org/doc/libs/1_75_0/libs/smart_ptr/doc/html/smart_ptr.html#enable_shared_from)
 [**small_vector.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/small_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A mix between `std::vector` and `itlib::static_vector`. It's a dynamic array, optimized for use when the number of elements is small. Like `static_vector` is has a static buffer with a given capacity, but can fall back to dynamically allocated memory, should the size exceed it. Similar to [`boost::small_vector`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/small_vector.html)
 [**span.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/span.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | A C++11 implementation of C++20's `std::span`
 [**static_vector.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/static_vector.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A mix between `std::vector` and `std::array`: A dynamically sized container with fixed capacity (supplied as a template parameter). This allows you to have dynamically sized vectors on the stack or as cache-local value members, as long as you know a big enough capacity beforehand. Similar to [`boost::static_vector`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/static_vector.html).
 [**stride_span.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/stride_span.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A C++11 implementation C++20's of std::span with a dynamic extent *and an associated stride*.
 [**strutil.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/strutil.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html) | A collection of small utilities for `std::string_view`
 [**throw_ex.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/throw_ex.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Utility to compose and throw exceptions on a single line
 [**time_t.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/time_t.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | A thin wrapper of `std::time_t` which provides thread safe `std::tm` getters and type-safe (`std::chrono::duration`-based) arithmetic
 [**type_traits.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/type_traits.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) [![Standard](https://img.shields.io/badge/C%2B%2B-17-red.svg)](https://en.cppreference.com/w/cpp/17.html)  | Additional type traits to extend the standard library's `<type_traits>`
 [**ufunction.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/ufunction.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) [![Standard](https://img.shields.io/badge/C%2B%2B-14-yellow.svg)](https://en.cppreference.com/w/cpp/14.html) | Unique function. A replacement of `std::function` which is non-copyable (can capture non-copyable values, and wrap non-copyable objects), and noexcept move-constructible (won't implicitly make owners no-noexcept move-constructible)
  [**utility.hpp**](https://github.com/iboB/itlib/tree/master/include/itlib/utility.hpp) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.cppreference.com/w/cpp/11.html) | Several generally unrelated utility functions and helpers

## Usage

Clone the repo or choose one or more libraries that you like and copy them somewhere in your include paths.

Every library is self-contained so you can copy, move, and modify whichever you like and not wory about interdependencies.

## Contributing

Pull requests and issues are welcome.

Please make separate commits per library, tagging them with the library name in the title with brackets. Example:

* *[small_vector] Added insert methods*
* *[flat_map] Crash when using with xxxx container*

You can use CMake to generate a project and run the tests locally.

## Copyright

Copyright &copy; 2016-2019 [Chobolabs Inc.](http://www.chobolabs.com/)

Copyright &copy; 2020-2026 Borislav Stanimirov

These libraries are distributed under the MIT Software License. See LICENSE.txt for further details or copy [here](http://opensource.org/licenses/MIT).
