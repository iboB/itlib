Chobo Single-Header Libraries
=============================

A collection of small single-header C++11 libraries by [Chobolabs](http://www.chobolabs.com/).

[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## Build Status

OS and compiler | Build status
-----|-----
Linux / GCC 4.9 and 5.0 | [![Travis CI](https://travis-ci.org/Chobolabs/chobo-shl.svg?branch=master)](https://travis-ci.org/Chobolabs/chobo-shl)
Windows / MSVC 14 (2015) 32 and 64 bit | [![Appveyor](https://ci.appveyor.com/api/projects/status/vq4932w0wbo83jwg?svg=true)](https://ci.appveyor.com/project/iboB/chobo-shl)

## Libraries

Every `.hpp` file in `include/chobo` is a standalone library, and has no dependencies other than the standard lib.

library    | description
--------------------- | --------------------------------
[**optional.hpp**](https://github.com/Chobolabs/chobo-shl/blob/master/include/chobo/optional.hpp) [![Try it online](https://img.shields.io/badge/try%20it-online-yellowgreen.svg)](http://melpon.org/wandbox/permlink/qEYr5Tx3YvTf2k5F) | A class, which can hold an object by value but provides an invalid state as well. Similar to [`boost::optional`](http://www.boost.org/doc/libs/1_61_0/libs/optional/doc/html/index.html).
[**static_vector.hpp**](https://github.com/Chobolabs/chobo-shl/blob/master/include/chobo/static_vector.hpp) [![Try it online](https://img.shields.io/badge/try%20it-online-yellowgreen.svg)](http://melpon.org/wandbox/permlink/3b0jJpio1ggfZiwp) | A mix between `std::vector` and `std::array`: A dynamically sized container with fixed capacity (supplied as a template parameter). This allows you to have dynamically sized vectors on the stack or as cache-local value members, as long as you know a big enough capacity beforehand. Similar to [`boost::static_vector`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/static_vector.html).
[**flat_map.hpp**](https://github.com/Chobolabs/chobo-shl/blob/master/include/chobo/flat_map.hpp) [![Try it online](https://img.shields.io/badge/try%20it-online-yellowgreen.svg)](http://melpon.org/wandbox/permlink/paeJbUpTq2zBpGF9) | A class with the interface of `std::map` but implemented with an underlying `std::vector`-type container, thus providing better cache locality of the elements. Similar to [`boost::flat_map`](http://www.boost.org/doc/libs/1_61_0/doc/html/boost/container/flat_map.html) with the notable difference that the underlying container can be changed via a template argument (thus making the class not strictly an `std::map` drop-in replacement)
[**vector_ptr.hpp**](https://github.com/Chobolabs/chobo-shl/blob/master/include/chobo/vector_ptr.hpp) [![Try it online](https://img.shields.io/badge/try%20it-online-yellowgreen.svg)](http://melpon.org/wandbox/permlink/kUr2aITUjOFwGNSK) | A non-owning `std::vector` pointer. Its purpose is to be used in generic code, which requires a vector.
[**vector_view.hpp**](https://github.com/Chobolabs/chobo-shl/blob/master/include/chobo/vector_view.hpp) [![Try it online](https://img.shields.io/badge/try%20it-online-yellowgreen.svg)](http://melpon.org/wandbox/permlink/wX8pazpgYbVhE9Rz) | A dangerous class which gives a view of an `std::vector`, changing the `value_type`. A strictly "I-know-what-I'm-doing" library, it's supposed to be used to obtain a view of `std::vector<X>` as something very similar to `std::vector<Y>`. It is **HIGHLY** recommended that X and Y are POD types, where one's size is a multiple of the other.

More detailed documentation is available in each header file.

The libraries use the C++11 standard and there are no plans for support of earlier ones.

## Usage

Choose one or more libraries that you like and copy somewhere in your include paths.

Supported compilers:
* VC 2015
* GCC 4.9 or newer
* Clang 3.2 or newer

Other C++11 compliant compilers may also work.

## Copyright

Copyright &copy; 2016 [Chobolabs Inc.](http://www.chobolabs.com/)

This libraries are distributed under the MIT Software License. See LICENSE.txt for
further details or copy [here](http://opensource.org/licenses/MIT).
