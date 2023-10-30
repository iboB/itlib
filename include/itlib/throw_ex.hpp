// itlib-throw_ex v0.11 beta
//
// Utility to throw compose and throw exceptions on a single line
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2023 Borislav Stanimirov
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files(the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and / or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
//                  VERSION HISTORY
//
//  0.11 (2023-10-30) Disable MSVC warning 4722 (destructor never returns)
//  0.10 (2023-10-24) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <sstream>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4722) // destructor never returns, potential memory leak
#endif

namespace itlib {

template <typename Exception>
class throw_ex {
    std::ostringstream out;
public:
    throw_ex() = default;
    [[noreturn]] ~throw_ex() noexcept(false) {
#if __cplusplus >= 201700
        if (std::uncaught_exceptions())
#else
        if (std::uncaught_exception())
#endif
            std::terminate(); // should never happen but helps us avoid warnings
        throw Exception(out.str());
    }

    template <typename T>
    throw_ex& operator<<(const T& t) {
        out << t;
        return *this;
    }

    // overload improve compilation time with literals
    throw_ex& operator<<(const char* str) {
        out << str;
        return *this;
    }
};

} // namespace itlib

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
