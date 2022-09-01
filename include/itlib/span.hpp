// itlib-span v1.00
//
// A C++11 implementation C++20's of std::span with a dynamic extent
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
//  1.00 (2022-05-16) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
// This class is designed as a drop-in replacement of std::span from C++20
// witha a dynamic extent. For a reference of std::span see here:
// https://en.cppreference.com/w/cpp/container/span
//
//              Differences from std::span
//
// * itlib does not (yet) provide a static-extent span
// * no Iter-Iter range construction (no good way to safely implement without
//   C++20)
// * additional methods remove_prefix/suffix like in std::string_view
// * additional method byte_size
//
//                  Configuration
//
// itlib::span has a single configurable setting:
//
//                  Config bounds checks:
//
// By default bounds checks are made in debug mode (via an assert) when
// accessing elements (with `[]`). Iterators are not checked (yet...)
//
// To disable them you can define ITLIB_SPAN_NO_DEBUG_BOUNDS_CHECK before
// including this header.
//
//
//                  TESTS
//
// You can find unit tests for span in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

#if defined(ITLIB_SPAN_NO_DEBUG_BOUNDS_CHECK)
#   define I_ITLIB_SPAN_BOUNDS_CHECK(i)
#else
#   include <cassert>
#   define I_ITLIB_SPAN_BOUNDS_CHECK(i) assert((i) < this->size())
#endif

namespace itlib
{

template <typename T>
class span
{
public:
    using element_type = T;
    using value_type = typename std::remove_cv<T>::type;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // byte type with the same constness as T
    // can't have std::byte here with no c++17 guaranteed, so use the next best thing
    using byte_t = typename std::conditional<std::is_const<T>::value, const uint8_t, uint8_t>::type;

    span() noexcept = default;

    // from range
    template <typename U, typename = typename std::enable_if<
        std::is_same<T, U>::value ||
        std::is_same<T, const U>::value, int>::type>
    span(U* begin, U* end)
        : m_begin(begin)
        , m_end(end)
    {}

    template <typename U, typename = typename std::enable_if<
        std::is_same<T, U>::value ||
        std::is_same<T, const U>::value, int>::type>
    span(U* begin, size_t size)
        : span(begin, begin + size)
    {}

    // there is no good way to implement Iter-Iter ranges without C++20

    // span from container
    // note the non-const container pointer. this is to avoid a dangling span from a temporary
    template <typename Container, typename = typename std::enable_if<
        std::is_same<T*, decltype(std::declval<Container>().data())>::value ||
        std::is_same<T*, decltype(std::declval<const Container>().data())>::value, int>::type>
    span(Container& c)
        : span(c.data(), c.size())
    {}

    // span of const from span of non-const
    template <typename U, typename = typename std::enable_if<
        std::is_same<T, U>::value ||
        std::is_same<T, const U>::value, int>::type>
    span(const span<U>& s)
        : span(s.data(), s.size())
    {}

    template <size_t N>
    span(T(&ar)[N])
        : span(ar, N)
    {}

    span(const span&) noexcept = default;
    span& operator=(const span&) noexcept = default;

    // assign non-const span to const
    template <typename U>
    typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, U>::value,
        span&>::type operator=(const span<U>& other) noexcept
    {
        m_begin = other.begin();
        m_end = other.end();
        return *this;
    }

    span(span&&) noexcept = default;
    span& operator=(span&&) noexcept = default;

    explicit operator bool() const
    {
        return !!m_begin;
    }

    const T& at(size_t i) const
    {
        I_ITLIB_SPAN_BOUNDS_CHECK(i);
        return *(m_begin + i);
    }

    T& at(size_t i)
    {
        I_ITLIB_SPAN_BOUNDS_CHECK(i);
        return *(m_begin + i);
    }

    const T& operator[](size_t i) const
    {
        return at(i);
    }

    T& operator[](size_t i)
    {
        return at(i);
    }

    const T& front() const
    {
        return at(0);
    }

    T& front()
    {
        return at(0);
    }

    const T& back() const
    {
        return at(size() - 1);
    }

    T& back()
    {
        return at(size() - 1);
    }

    const T* data() const noexcept
    {
        return m_begin;
    }

    T* data() noexcept
    {
        return m_begin;
    }

    // iterators
    iterator begin() noexcept
    {
        return m_begin;
    }

    const_iterator begin() const noexcept
    {
        return m_begin;
    }

    const_iterator cbegin() const noexcept
    {
        return m_begin;
    }

    iterator end() noexcept
    {
        return m_end;
    }

    const_iterator end() const noexcept
    {
        return m_end;
    }

    const_iterator cend() const noexcept
    {
        return m_end;
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    // capacity
    bool empty() const noexcept
    {
        return m_begin == m_end;
    }

    size_t size() const noexcept
    {
        return m_end - m_begin;
    }

    // byte access
    size_t byte_size() const noexcept
    {
        return size() * sizeof(T);
    }

    span<const uint8_t> as_bytes() const noexcept
    {
        return span<const uint8_t>(reinterpret_cast<const uint8_t*>(m_begin), byte_size());
    }

    span<byte_t> as_writable_bytes() noexcept
    {
        return span<byte_t>(reinterpret_cast<byte_t*>(m_begin), byte_size());
    }

    // slicing
    span subspan(size_t off, size_t count = size_t(-1)) const noexcept
    {
        if (m_begin + off > m_end) return span(m_end, 0);
        auto newSize = size() - off;
        if (count > newSize) count = newSize;
        return span(m_begin + off, count);
    }

    span first(size_t n) const noexcept
    {
        return subspan(0, n);
    }

    span last(size_t n) const noexcept
    {
        return subspan(size() - n, n);
    }

    void remove_prefix(size_t n) noexcept
    {
        m_begin += n;
    }

    void remove_suffix(size_t n) noexcept
    {
        m_end -= n;
    }

private:
    T* m_begin = nullptr;
    T* m_end = nullptr;
};

template <typename T>
span<T> make_span(T* begin, T* end)
{
    return span<T>(begin, end);
}

template <typename T>
span<T> make_span(T* begin, size_t size)
{
    return span<T>(begin, size);
}

template <typename Container, typename Ptr = decltype(std::declval<Container>().data())>
auto make_span(Container& c) -> span<typename std::remove_pointer<Ptr>::type>
{
    return span<typename std::remove_pointer<Ptr>::type>(c);
}

template <typename T, size_t N>
span<T> make_span(T(&ar)[N])
{
    return span<T>(ar);
}

#if __cplusplus >= 201700
// provide constructor deduction
template <typename T> span(T*, T*) -> span<T>;
template <typename T> span(T*, size_t) -> span<T>;
template <typename C> span(C& c)->span<std::remove_pointer_t<decltype(c.data())>>;
#endif

}
