// itlib-expected v1.00
//
// A union-type of a value and an error
//
// MIT License:
// Copyright(c) 2021 Borislav Stanimirov
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
//  1.00 (2021-09-25) Initial-release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::expected,
//
//
//                  TESTS
//
// You can find unit tests for expected in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <cassert>
#include <utility>
#include <new>

namespace itlib
{

template <typename E>
class unexpected_t
{
public:
    explicit unexpected_t(E&& e) : m_error(std::forward<E>(e)) {}

private:
    template<typename T, typename E2>
    friend class expected;

    E m_error;
};

template <typename E>
unexpected_t<E> unexpected(E&& e)
{
    return unexpected_t<E>(std::forward<E>(e));
}

struct unexpect {};
inline unexpect unexpected() noexcept { return {}; }

template <typename T, typename E>
class expected
{
public:
    using value_type = T;
    using error_type = E;

    expected() : m_value(), m_has_value(true) {}
    expected(T&& t) : m_value(std::forward<T>(t)), m_has_value(true) {}

    template <typename E2>
    expected(unexpected_t<E2>&& u) : m_error(std::move(u.m_error)), m_has_value(false) {}

    expected(unexpect) : m_error(), m_has_value(false) {}

    // do not copy
    expected(const expected&) = delete;
    expected& operator=(const expected&) = delete;

    // do move
    expected(expected&& other) noexcept
        : m_has_value(other.has_value())
    {
        if (m_has_value)
        {
            new (&m_value) T(std::move(other.m_value));
        }
        else
        {
            new (&m_error) E(std::move(other.m_error));
        }
    }

    expected& operator=(expected&& other) noexcept
    {
        if (m_has_value && other.has_value())
        {
            m_value = std::move(other.m_value);
        }
        else if(m_has_value && !other.has_value())
        {
            m_has_value = false;
            m_value.~T();
            ::new (&m_error) E(std::move(other.m_error));
        }
        else if(!m_has_value && other.has_value())
        {
            m_has_value = true;
            m_error.~E();
            ::new (&m_value) T(std::move(other.m_value));
        }
        else
        {
            m_error = std::move(other.m_error);
        }
        return *this;
    }

    ~expected()
    {
        if (m_has_value)
        {
            m_value.~T();
        }
        else
        {
            m_error.~E();
        }
    }

    // bool interface
    bool has_value() const { return m_has_value; }
    bool has_error() const { return !m_has_value; }
    explicit operator bool() const { return m_has_value; }

    // value getters
    T& value() &
    {
        assert(has_value());
        return m_value;
    }

    const T& value() const &
    {
        assert(has_value());
        return m_value;
    }

    T&& value() &&
    {
        assert(has_value());
        return std::move(m_value);
    }

    T& operator*() & { return value(); }
    const T& operator*() const & { return value(); }
    T&& operator*() && { return std::move(value()); }

    T& value_or(T& v) & { return has_value() ? value() : v; }
    const T& value_or(const T& v) const & { return has_value() ? value() : v; }
    T&& value_or(T&& v) && { return has_value() ? value() : std::move(v); }

    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }

    // error getters

    E& error() &
    {
        assert(has_error());
        return m_error;
    }

    const E& error() const &
    {
        assert(has_error());
        return m_error;
    }

    E&& error() &&
    {
        assert(has_error());
        return std::move(m_error);
    }

private:
    union
    {
        value_type m_value;
        error_type m_error;
    };
    bool m_has_value;
};

}
