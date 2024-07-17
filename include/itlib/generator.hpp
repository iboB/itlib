// itlib-generator v1.00
//
// Simple coroutine generator class for C++20 and later
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2024 Borislav Stanimirov
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
//  1.00 (2024-07-xx) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class generator which allows you to write simple coroutine-
// based generators.
//
// Example:
//
// itlib::generator<int> range(int begin, int end) {
//     for (int i = begin; i < end; ++i) {
//        co_yield i;
//     }
// }
// ...
// for (int i : range(0, 10)) {
//    std::cout << i << std::endl;
// }
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <coroutine>
#include <type_traits>
#include <optional>

namespace itlib {

// why std::optional still doesn't have T& specialization is beyond me
// it's tempting to include expected here so we could have optional of ref and
// ditch our T& specialization, but we promised to make standalone libs

template <typename T>
class generator_value : public std::optional<T> {};

template <typename T>
class generator_value <T&> {
    T* m_val = nullptr;
public:
    void emplace(T& v) noexcept { m_val = &v; }
    void reset() noexcept { m_val = nullptr; }
    T& operator*() noexcept { return *m_val; }
    bool has_value() const noexcept { return m_val != nullptr; }
    explicit operator bool() const noexcept { return has_value(); }
};

template <typename T>
class generator {
public:
    // return const ref in case we're generating values, otherwise keep the ref type
    using value_ret_t = std::conditional_t<std::is_reference_v<T>, T, const T&>;

    struct promise_type {
        generator_value<T> m_val;

        promise_type() noexcept = default;

        ~promise_type() = default;
        generator get_return_object() noexcept {
            return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) noexcept { // assume T is noexcept move constructible
            if constexpr (std::is_reference_v<T>) {
                m_val.emplace(value);
            }
            else {
                m_val.emplace(std::move(value));
            }
            return {};
        }
        void return_void() noexcept {}
        void unhandled_exception() { throw; }

        value_ret_t val() & noexcept {
            return *m_val;
        }
        T&& val() && noexcept {
            return std::move(*m_val);
        }
        void clear_value() noexcept {
            m_val.reset();
        }
    };

    using handle_t = std::coroutine_handle<promise_type>;

    ~generator() {
        if (m_handle) m_handle.destroy();
    }

    // next (optional-based) interface

    // NOTE: this won't return true until next() has returned an empty optional at least once
    bool done() const noexcept {
        return m_handle.done();
    }

    generator_value<T> next() {
        if (done()) return {};
        m_handle.promise().clear_value();
        m_handle.resume();
        return std::move(m_handle.promise().m_val);
    }

    // iterator-like/range-for interface

    // emphasize that this is not a real iterator
    class pseudo_iterator {
        handle_t m_handle;
    public:
        using value_type = std::decay_t<T>;
        using reference = value_ret_t;

        pseudo_iterator() noexcept = default;
        explicit pseudo_iterator(handle_t handle) noexcept : m_handle(handle) {}

        reference operator*() const noexcept {
            return m_handle.promise().val();
        }

        pseudo_iterator& operator++() {
            m_handle.promise().clear_value();
            m_handle.resume();
            return *this;
        }

        struct end_t {};

        // we're not really an iterator, but we can pretend to be one
        friend bool operator==(const pseudo_iterator& i, end_t) noexcept { return i.m_handle.done(); }
        friend bool operator==(end_t, const pseudo_iterator& i) noexcept { return i.m_handle.done(); }
        friend bool operator!=(const pseudo_iterator& i, end_t) noexcept { return !i.m_handle.done(); }
        friend bool operator!=(end_t, const pseudo_iterator& i) noexcept { return !i.m_handle.done(); }
    };

    pseudo_iterator begin() {
        m_handle.resume();
        return pseudo_iterator{m_handle};
    }

    pseudo_iterator::end_t end() {
        return {};
    }

private:
    handle_t m_handle;
    explicit generator(handle_t handle) noexcept : m_handle(handle) {}
};

} // namespace itlib
