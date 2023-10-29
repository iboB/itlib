// itlib-strutil v1.01
//
// String utilities
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2022 Borislav Stanimirov
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
//  1.01 (2022-03-04) Fixed potential UB in make_string_view.
//                    Added missing noexcept
//  1.00 (2022-03-03) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines several string utilities:
//
// * std::string_view make_string_view(T begin, T end)
//   Creates a string view from begin and end
//   Obsolete with C++20
//
// * bool starts_with(std::string_view str, std::string_view prefix)
//   Checks if the string view starts with the given prefix
//   Obsolete with C++20
//
// * bool ends_with(std::string_view str, std::string_view suffix)
//   Checks if the string view ends with the given suffix
//   Obsolete with C++20
//
// * std::string_view remove_prefix_all(std::string_view str, std::string_view prefix)
//   Removes prefix while string starts with prefix
//
// * std::string_view remove_suffix_all(std::string_view str, std::string_view suffix)
//   Removes suffix while string ends with suffix
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <string_view>

namespace itlib
{

template <typename CI> // contiguous iterator
inline constexpr std::string_view make_string_view(CI begin, CI end) noexcept
{
    if (begin == end) return {}; // avoid UB from dereferncing end if called with (end, end)
    return std::string_view(&(*begin), end - begin);
}

// overload which is likely to come up most often
inline constexpr std::string_view make_string_view(const char* begin, const char* end) noexcept
{
    return std::string_view(begin, end - begin);
}

inline bool starts_with(std::string_view str, std::string_view prefix) noexcept
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

inline bool ends_with(std::string_view str, std::string_view suffix) noexcept
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

inline std::string_view remove_prefix_all(std::string_view str, std::string_view prefix) noexcept
{
    if (prefix.empty()) return str;
    while (starts_with(str, prefix)) str.remove_prefix(prefix.size());
    return str;
}

inline std::string_view remove_suffix_all(std::string_view str, std::string_view suffix) noexcept
{
    if (suffix.empty()) return str;
    while (ends_with(str, suffix)) str.remove_suffix(suffix.size());
    return str;
}

}
