// itlib-stride_span v1.00
//
// A C++11 implementation C++20's of std::span with a dynamic extent
// and an associated stride.
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
//  1.00 (2022-05-15) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
// This class is designed very similar to std::span from C++20 witha a dynamic
// extent. For a reference of std::span see here:
// https://en.cppreference.com/w/cpp/container/span
//
// The key difference is itlib::stride_span allows a stride between different
// elements. The stride is the number of bytes between two elements. Thus if
// the stride equals sizeof(T) stride_span is equivalent to std::span.
//
// A different stride allows users to provide a partial vector-like view to
// certain elements of an array or to members of a class or struct.
//
//                  Configuration
//
// itlib::stride_span has a single configurable setting:
//
//                  Config bounds checks:
//
// By default bounds checks are made in debug mode (via an assert) when
// accessing elements (with `[]`). Iterators are not checked (yet...)
//
// To disable them you can define ITLIB_STRIDE_SPAN_NO_DEBUG_BOUNDS_CHECK before
// including this header.
//
//
//                  TESTS
//
// You can find unit tests for stride_span in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(ITLIB_STRIDE_SPAN_NO_DEBUG_BOUNDS_CHECK)
#   define I_ITLIB_STRIDE_SPAN_BOUNDS_CHECK(i)
#else
#   include <cassert>
#   define I_ITLIB_STRIDE_SPAN_BOUNDS_CHECK(i) assert((i) < this->size())
#endif

namespace itlib
{

template <typename T>
class stride_span
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

    // byte type with the same constness as T
    // can't have std::byte here with no c++17 guaranteed, so use the next best thing
    using byte_t = typename std::conditional<std::is_const<T>::value, const uint8_t, uint8_t>::type;

    stride_span() noexcept = default;

    stride_span(byte_t* ptr, size_t stride, size_t num_elements)
        : m_begin(ptr)
        , m_stride(stride)
        , m_num_elements(num_elements)
    {}

    stride_span(const stride_span&) noexcept = default;
    stride_span& operator=(const stride_span&) noexcept = default;

    template <typename U, typename = typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, U>::value, int>::type>
    stride_span(const stride_span<U>& other)
        : m_begin(other.data()), m_stride(other.stride()), m_num_elements(other.size())
    {}

    template <typename U>
    typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, U>::value,
        stride_span&>::type operator=(const stride_span<U>& other) noexcept
    {
        m_begin = other.data();
        m_stride = other.stride();
        m_num_elements = other.size();
        return *this;
    }

    stride_span(stride_span&&) noexcept = default;
    stride_span& operator=(stride_span&&) noexcept = default;

    explicit operator bool() const
    {
        return !!m_begin;
    }

    const T& at(size_t i) const
    {
        I_ITLIB_STRIDE_SPAN_BOUNDS_CHECK(i);
        return *reinterpret_cast<T*>(m_begin + m_stride * i);
    }

    T& at(size_t i)
    {
        I_ITLIB_STRIDE_SPAN_BOUNDS_CHECK(i);
        return *reinterpret_cast<T*>(m_begin + m_stride * i);
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

    byte_t* data()
    {
        return m_begin;
    }

    const byte_t* data() const
    {
        return m_begin;
    }

    // iterators
    template <typename CT>
    class t_iterator
    {
        byte_t* p = 0;
        size_t stride = sizeof(T);

        friend class stride_span;
        t_iterator(byte_t* p, size_t stride) noexcept : p(p), stride(stride) {}
    public:
        t_iterator() noexcept = default;
        CT& operator*() const noexcept { return *reinterpret_cast<T*>(p); }
        t_iterator& operator++() noexcept { p += stride; return *this; }
        t_iterator& operator--() noexcept { p -= stride; return *this; }
        t_iterator operator+(const ptrdiff_t diff) const noexcept { return t_iterator(p + stride * diff, stride); }
        t_iterator operator-(const ptrdiff_t diff) const noexcept { return t_iterator(p - stride * diff, stride); }
        bool operator==(const t_iterator& other) const noexcept { return p == other.p; }
        bool operator!=(const t_iterator& other) const noexcept { return p != other.p; }
        bool operator<(const t_iterator& other) const noexcept { return p < other.p; }
        bool operator>(const t_iterator& other) const noexcept { return p > other.p; }
        bool operator<=(const t_iterator& other) const noexcept { return p <= other.p; }
        bool operator>=(const t_iterator& other) const noexcept { return p >= other.p; }
    };

    using iterator = t_iterator<T>;
    using const_iterator = t_iterator<const T>;

    template <typename Iter>
    class t_rev_iterator
    {
        Iter iter;
    public:
        explicit t_rev_iterator(Iter i) noexcept : iter(i) {}
        auto operator*() noexcept -> decltype(*std::declval<Iter>()) { return *(iter - 1); }
        t_rev_iterator& operator++() noexcept { --iter; return *this; }
        bool operator==(const t_rev_iterator& other) const noexcept { return iter == other.iter; }
        bool operator!=(const t_rev_iterator& other) const noexcept { return iter != other.iter; }
    };

    using reverse_iterator = t_rev_iterator<iterator>;
    using const_reverse_iterator = t_rev_iterator<const_iterator>;

    iterator begin() noexcept
    {
        return iterator(m_begin, m_stride);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(m_begin, m_stride);
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() noexcept
    {
        return iterator(m_begin + m_num_elements * m_stride, m_stride);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(m_begin + m_num_elements * m_stride, m_stride);
    }

    const_iterator cend() const noexcept
    {
        return end();
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
        return m_num_elements == 0;
    }

    size_t size() const noexcept
    {
        return m_num_elements;
    }

    size_t stride() const noexcept
    {
        return m_stride;
    }

    // slicing
    stride_span subspan(size_t off, size_t count = size_t(-1)) const noexcept
    {
        if (off > m_num_elements) return stride_span(m_begin + m_num_elements * m_stride, m_stride, 0);
        auto newSize = m_num_elements - off;
        if (count > newSize) count = newSize;
        return stride_span(m_begin + off * m_stride, m_stride, count);
    }

    stride_span first(size_t n) const noexcept
    {
        return subspan(0, n);
    }

    stride_span last(size_t n) const noexcept
    {
        return subspan(size() - n, n);
    }

    void remove_prefix(size_t n) noexcept
    {
        m_begin += n * m_stride;
        m_num_elements -= n;
    }

    void remove_suffix(size_t n) noexcept
    {
        m_num_elements -= n;
    }

private:
    byte_t* m_begin = nullptr;
    size_t m_stride = sizeof(T);
    size_t m_num_elements = 0;
};

template <typename T, typename B>
stride_span<T> make_stride_span_from_buf(B* buf, size_t offset, size_t stride, size_t num_elements)
{
    return stride_span<T>(reinterpret_cast<typename stride_span<T>::byte_t*>(buf) + offset,
        stride,
        num_elements);
}

template <typename T>
stride_span<T> make_stride_span_from_buf(T* buf, size_t stride, size_t num_elements)
{
    return stride_span<T>(reinterpret_cast<typename stride_span<T>::byte_t*>(buf),
        stride,
        num_elements);
}

template <typename T>
stride_span<T> make_stride_span_from_array(T* ar, size_t ar_length, size_t noffset = 0, size_t nstride = 1)
{
    return make_stride_span_from_buf(ar + noffset,
        sizeof(T) * nstride,
        (ar_length - noffset + nstride - 1) / nstride); // divide rounding up
}

template <typename Struct, typename Field>
stride_span<Field> make_stride_span_member_view(Struct* ar, size_t ar_length, Field (Struct::*member))
{
    auto begin = &(ar->*member);
    return make_stride_span_from_buf(begin, sizeof(Struct), ar_length);
}

template <typename Struct, typename Field>
stride_span<const Field> make_stride_span_member_view(const Struct* ar, size_t ar_length, Field (Struct::*member))
{
    auto begin = &(ar->*member);
    return make_stride_span_from_buf(begin, sizeof(Struct), ar_length);
}

template <typename Base, typename Derived>
stride_span<Base> make_stride_span_base_view(Derived* ar, size_t ar_length)
{
    static_assert(std::is_base_of<Base, Derived>::value, "Argument is not derived from base");
    Base* begin = ar;
    return make_stride_span_from_buf(begin, sizeof(Derived), ar_length);
}

}
