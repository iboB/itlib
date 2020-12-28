// itlib-qalgorithm v1.00
//
// Wrappers of <algorithm> algorithms for entire containers
//
// MIT License:
// Copyright(c) 2020 Borislav Stanimirov
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
//  1.00 (2020-12-28) First pulic release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the following algorithms:
//
// * qfind - wraps std::find
// * qfind_if - wraps std::find_if
// * pfind - wraps std::find, returns a raw pointer to the element or nullptr if the element wasn't found
// * pfind_if - wraps std::find_if, returns a raw pointer to the element or nullptr if the element wasn't found
//
//
//                  TESTS
//
// You can find unit tests for qalgorithm in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <utility>
#include <algorithm>
#include <type_traits>

namespace itlib
{

namespace impl
{
// get the appropriate return type: iterator for non-const containers and const_iterator for const containers
template <typename Container>
struct iterator_t
{
    using type = typename std::conditional<std::is_const<Container>::value, typename Container::const_iterator, typename Container::iterator>::type;
};
template <typename Container>
struct pointer_t
{
    using type = typename std::conditional<std::is_const<Container>::value, typename Container::value_type const*, typename Container::value_type*>::type;
};
}

template <typename Container, typename Value>
typename impl::iterator_t<Container>::type qfind(Container& c, const Value& val)
{
    return std::find(c.begin(), c.end(), val);
}

template <typename Container, typename Value>
typename impl::pointer_t<Container>::type pfind(Container& c, const Value& val)
{
    auto f = std::find(c.begin(), c.end(), val);
    if (f == c.end()) return nullptr;
    return &(*f);
}

template <typename Container, typename Pred>
typename impl::iterator_t<Container>::type qfind_if(Container& c, Pred&& pred)
{
    return std::find_if(c.begin(), c.end(), std::forward<Pred>(pred));
}

template <typename Container, typename Pred>
typename impl::pointer_t<Container>::type pfind_if(Container& c, Pred&& pred)
{
    auto f = std::find_if(c.begin(), c.end(), std::forward<Pred>(pred));
    if (f == c.end()) return nullptr;
    return &(*f);
}

}
