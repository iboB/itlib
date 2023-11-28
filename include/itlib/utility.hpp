// itlib-utility v1.02
//
// Utility functions to extend <utility>
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2022-2023 Borislav Stanimirov
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
//  1.02 (2023-11-27) Added bit_cast
//  1.01 (2023-02-08) Added make_nullptr
//  1.00 (2022-11-28) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines several utility functions with generally unrelated features
//
// * force_move:
//      A function which guarantees that a move (and not a copy) happens.
//      If the op would result in copy construction or assignment, the call
//      produces a compilation error.
// * owner_from_member:
//      A "hacky" ofsetof-like func which allows you to get a pointer to the
//      owner from a pointer to a member.
// * make_nullptr:
//      A function to make a null pointer from a (potentially incomplete) type
//      Equivalent to static_cast<T*>(nullptr), but with a clearer intent
// * bit_cast:
//      A function which allows you to reinterpret_cast between types of the
//      same size without UB. The same as C++20's std::bit_cast
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <type_traits>
#include <cstdint>
#include <cstring>

namespace itlib
{

template <typename T>
constexpr typename std::remove_reference<T>::type&& force_move(T&& t) noexcept {
    using NoRefT = typename std::remove_reference<T>::type;
    static_assert(!std::is_const<NoRefT>::value, "cannot force_move a const object");
    return static_cast<NoRefT&&>(t);
}

template <typename Owner, typename Member>
const Owner* owner_from_member(const Member& member, Member(Owner::* ptr)) noexcept {
    auto m_p = reinterpret_cast<const uint8_t*>(&member);
    Owner* null_owner = nullptr;
    auto m_offset = reinterpret_cast<uintptr_t>(&(null_owner->*ptr));
    return reinterpret_cast<const Owner*>(m_p - m_offset);
}

template <typename T>
T* make_nullptr() { return static_cast<T*>(nullptr); }

template <typename Dst, typename Src>
constexpr Dst bit_cast(const Src& src) noexcept {
    static_assert(sizeof(Dst) == sizeof(Src), "bit_cast requires types of equal size");
    Dst dst;
    std::memcpy(&dst, &src, sizeof(Src));
    return dst;
}

}
