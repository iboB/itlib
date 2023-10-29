// itlib-tep_vector v0.10 beta
//
// A type-earased POD vector
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
#include <cstdint>

namespace itlib {

class tep_vector {
public:
private:
    uint8_t* m_begin = nullptr; // should be std::byte, but we want to support <C++17
    size_t m_size = 0; // number of elements
    size_t m_elem_size = 0; // size in bytes, also stride between elements
    size_t m_elem_align = 0; // alignment of element (size of element is always a multiple of this)
};

}
