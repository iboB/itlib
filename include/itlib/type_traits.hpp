// itlib-type_traits v1.03
//
// Additional helper type traits extending the standard <type_traits>
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2020-2023 Borislav Stanimirov
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
//  1.03 (2025-12-15) Add copy_cv, add concepts
//  1.02 (2023-11-27) Added is_noop_convertible
//  1.01 (2023-03-10) Added type_identity
//  1.00 (2020-12-28) First pulic release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the following classes:
//
// * is_instantiation_of<Template, Type> - checks whether a type is an
//   instantiation of a template
//      Example:
//      using MyVec = std::vector<int>;
//      static_assert(is_instantiation_of<std::vector, MyVec>::value,
//          "MyVec must be a std::vector");
// * type_identity<Type> - a reimplementation of C++20's std::type_identity
// * is_noop_convertible<A, B> - checks whether two types are binary the same -
//      i.e. whether casting from one to the other is a noop
//
// With C++17 all value traits have a _v template constant
// and all type traits have a _t type alias.
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <type_traits>

namespace itlib
{

template <template <typename...> class, typename...>
struct is_instantiation_of : public std::false_type {};

template <template <typename...> class Template, typename... TArgs>
struct is_instantiation_of<Template, Template<TArgs...>> : public std::true_type {};

template <typename T>
struct type_identity {
    using type = T;
};

template <typename From, typename To>
struct is_noop_convertible {
    using from_t = typename std::remove_cv<From>::type;
    using to_t = typename std::remove_cv<To>::type;

    static constexpr bool value = sizeof(from_t) == sizeof(to_t)
        && std::is_scalar<from_t>::value
        && std::is_scalar<to_t>::value
        && std::is_floating_point<from_t>::value == std::is_floating_point<to_t>::value
        // bool -> int8 is a noop, but int8 -> bool is not
        && (std::is_same<to_t, bool>::value ? std::is_same<from_t, bool>::value : true)
        ;
};

template <typename To, typename From>
struct copy_cv {
    using type = typename std::conditional<std::is_const<From>::value,
        typename std::conditional<std::is_volatile<From>::value, const volatile To, const To>::type,
        typename std::conditional<std::is_volatile<From>::value, volatile To, To>::type
    >::type;
};

#if __cplusplus >= 201700
template <template <typename...> class Template, typename Type>
inline constexpr bool is_instantiation_of_v = is_instantiation_of<Template, Type>::value;

template <typename T>
using type_identity_t = typename type_identity<T>::type;

template <typename From, typename To>
inline constexpr bool is_noop_convertible_v = is_noop_convertible<From, To>::value;

template <typename To, typename From>
using copy_cv_t = typename copy_cv<To, From>::type;
#endif

#if __cplusplus >= 202000
template <typename Type, template <typename...> class Template>
concept instantiation_of = is_instantiation_of_v<Template, Type>;

template <typename From, typename To>
concept noop_convertible_to = is_noop_convertible_v<From, To>;
#endif

}
