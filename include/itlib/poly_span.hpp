// itlib-poly_span v1.00
//
// A class similar to C++20's span which offers a polymorphic view to a block
// of data
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
//  1.01 (2023-02-27) Proper iterator support
//  1.00 (2022-05-19) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
// poly_span is similar to C++20's std::span, as it can be constructed from a
// contiguous block of data, but whereas std::span offers an identity view of
// the block, poly_span offers a polymorphic one. By providing an access
// function which is called upon dereferencing elements, one can access
// different aspects of the elements of the original array.
//
// As opposed to std::span, itlib::span and itlib::stride_span, poly_span's
// template argument is not a "value_type". It is instead what is returned
// by dereferencing elements.
//
//                  Example
//
//  struct person { std::string& first_name, std::string& last_name, int age };
//  std::vector<person> ps;
//  ...
//  poly_span<string&> names(ps.data(), ps.size(), [](person& p) -> std::string& {
//      if (p.age < 18) return p.first_name;
//      return p.last_name;
//  });
//
//  for (auto& name : names) cout << "Hello, " << name << ".\n";
//
// Thus we greet young people by first name, and the rest by last name.
// Note the template argument of poly_span there. It's string&. This allows us
// to change the names from the point of view of the span. You can use
// `const string&` if you want to disable that.
//
//
//                  Configuration
//
// itlib::poly_span has a single configurable setting:
//
//                  Config bounds checks:
//
// By default bounds checks are made in debug mode (via an assert) when
// accessing elements (with `[]`). Iterators are not checked (yet...)
//
// To disable them you can define ITLIB_POLY_SPAN_NO_DEBUG_BOUNDS_CHECK before
// including this header.
//
//
//                  TESTS
//
// You can find unit tests for poly_span in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

#if defined(ITLIB_POLY_SPAN_NO_DEBUG_BOUNDS_CHECK)
#   define I_ITLIB_POLY_SPAN_BOUNDS_CHECK(i)
#else
#   include <cassert>
#   define I_ITLIB_POLY_SPAN_BOUNDS_CHECK(i) assert((i) < this->size())
#endif

namespace itlib
{

template <typename RT>
class poly_span
{
    using poly_func_t = RT(*)(void*);
public:
    // can't have std::byte here with no c++17 guaranteed, so use the next best thing
    using byte_t = uint8_t;

    poly_span() noexcept = default;

    poly_span(const poly_span&) noexcept = default;
    poly_span& operator=(const poly_span&) noexcept = default;

    poly_span(poly_span&&) noexcept = default;
    poly_span& operator=(poly_span&&) noexcept = default;

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    template <typename U, typename F,
        typename = typename std::enable_if<std::is_convertible<F, RT(*)(U&)>::value, int>::type>
    poly_span(U* begin, size_t num, F func)
        : poly_span(
            const_cast<byte_t*>(reinterpret_cast<const byte_t*>(begin)),
            sizeof(U),
            num,
            reinterpret_cast<poly_func_t>((RT(*)(U&))func))
    {}
#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif

    explicit operator bool() const
    {
        return !!m_begin;
    }

    const RT at(size_t i) const
    {
        I_ITLIB_POLY_SPAN_BOUNDS_CHECK(i);
        return m_poly_func(m_begin + m_stride * i);
    }

    RT at(size_t i)
    {
        I_ITLIB_POLY_SPAN_BOUNDS_CHECK(i);
        return m_poly_func(m_begin + m_stride * i);
    }

    const RT operator[](size_t i) const
    {
        return at(i);
    }

    RT operator[](size_t i)
    {
        return at(i);
    }

    const RT front() const
    {
        return at(0);
    }

    RT front()
    {
        return at(0);
    }

    const RT back() const
    {
        return at(size() - 1);
    }

    RT back()
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
    template <typename CRT>
    class t_iterator
    {
        byte_t* p = 0;
        size_t stride = 1;
        poly_func_t poly_func = nullptr;

        friend class poly_span;
        t_iterator(byte_t* p, size_t stride, poly_func_t poly_func) noexcept : p(p), stride(stride), poly_func(poly_func) {}
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename std::remove_reference<CRT>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = typename std::remove_reference<CRT>::type*;
        using reference = CRT;

        t_iterator() noexcept = default;
        CRT operator*() const noexcept { return poly_func(p); }
        t_iterator& operator++() noexcept { p += stride; return *this; }
        t_iterator& operator--() noexcept { p -= stride; return *this; }
        t_iterator& operator+=(const ptrdiff_t diff) noexcept { p += diff * stride; return *this; }
        t_iterator& operator-=(const ptrdiff_t diff) noexcept { p -= diff * stride; return *this; }
        t_iterator operator+(const ptrdiff_t diff) const noexcept { return t_iterator(p + stride * diff, stride, poly_func); }
        t_iterator operator-(const ptrdiff_t diff) const noexcept { return t_iterator(p - stride * diff, stride, poly_func); }
        ptrdiff_t operator-(const t_iterator& other) const noexcept { return (p - other.p) / stride; }
        bool operator==(const t_iterator& other) const noexcept { return p == other.p; }
        bool operator!=(const t_iterator& other) const noexcept { return p != other.p; }
        bool operator<(const t_iterator& other) const noexcept { return p < other.p; }
        bool operator>(const t_iterator& other) const noexcept { return p > other.p; }
        bool operator<=(const t_iterator& other) const noexcept { return p <= other.p; }
        bool operator>=(const t_iterator& other) const noexcept { return p >= other.p; }
    };

    using iterator = t_iterator<RT>;
    using const_iterator = t_iterator<const RT>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() noexcept
    {
        return iterator(m_begin, m_stride, m_poly_func);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(m_begin, m_stride, m_poly_func);
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() noexcept
    {
        return iterator(m_begin + m_num_elements * m_stride, m_stride, m_poly_func);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(m_begin + m_num_elements * m_stride, m_stride, m_poly_func);
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
    poly_span subspan(size_t off, size_t count = size_t(-1)) const noexcept
    {
        if (off > m_num_elements) return poly_span(m_begin + m_num_elements * m_stride, m_stride, 0, m_poly_func);
        auto newSize = m_num_elements - off;
        if (count > newSize) count = newSize;
        return poly_span(m_begin + off * m_stride, m_stride, count, m_poly_func);
    }

    poly_span first(size_t n) const noexcept
    {
        return subspan(0, n);
    }

    poly_span last(size_t n) const noexcept
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
    poly_span(byte_t* begin, size_t stride, size_t num, poly_func_t func)
        : m_begin(begin)
        , m_stride(stride)
        , m_num_elements(num)
        , m_poly_func(func)
    {}

    byte_t* m_begin = nullptr;
    size_t m_stride = 1;
    size_t m_num_elements = 0;
    poly_func_t m_poly_func = nullptr;
};

}
