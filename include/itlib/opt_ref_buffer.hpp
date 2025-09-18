// itlib-opt_ref_buffer v1.00 alpha
//
// A buffer that can point to or own a contiguous block of memory
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2025 Borislav Stanimirov
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
//  1.00 (2025-09-18) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the type itlib::opt_ref_buffer which provides a span of a type to
// a contiguous block of memory. The memory itself can be owned by the buffer or
// it can be a reference to an external memory block.
//
// The type currently requires C++20 because of the use of std::span, std::any,
// and std::variant, and std::byte, but it can easily be adapted to C++11 if
// needed.
// * std::span by itlib::span from this library or just ptr + size
// * std::any by std::unique_ptr<void, void(*)(void*)>
// * std::variant by a union + enum
// * std::byte by uint8_t
//
// The type is move-only, though copying can be added if needed, at the cost
// of storing a an additional function pointer to redirect the span to a new
// memory block.
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <any>
#include <variant>
#include <span>
#include <type_traits>
#include <stdexcept>

namespace itlib {

using opt_ref_buffer_owned_storage = std::variant<
    std::monostate,
    std::vector<std::byte>,
    std::string,
    std::any
>;

template <typename T>
class opt_ref_buffer_t {
public:
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    using element_type = T;
    using span_type = std::span<element_type>;
    using value_type = std::remove_const_t<T>;
    using owned_type = opt_ref_buffer_owned_storage;

    opt_ref_buffer_t() noexcept = default;

    opt_ref_buffer_t(const opt_ref_buffer_t&) = delete;
    opt_ref_buffer_t& operator=(const opt_ref_buffer_t&) = delete;

    opt_ref_buffer_t(opt_ref_buffer_t&& o) noexcept {
        take_data(o.m_span, o.m_own);
        o.m_span = {};
    }
    opt_ref_buffer_t& operator=(opt_ref_buffer_t&& o) noexcept {
        if (this != &o)
        {
            take_data(o.m_span, o.m_own);
            o.m_span = {};
        }
        return *this;
    }

    // implicitly construct const buffer from a non-const one
    template <typename U>
    opt_ref_buffer_t(opt_ref_buffer_t<U>&& o) noexcept requires (std::is_same_v<T, const U>) {
        span_type span = o.m_span;
        take_data(span, o.m_own);
        o.m_span = {};
    }

    template <typename U>
    static constexpr bool is_congruent =
        sizeof(value_type) == sizeof(U) // either the same size
        || sizeof(U) % sizeof(value_type) == 0; // or value_type fits in U without spill

    template <typename Container>
    static span_type make_span(Container& container) noexcept (is_congruent<typename Container::value_type>) {
        const auto csize = std::span(container).size_bytes();

        if constexpr (!is_congruent<typename Container::value_type>) {
            if (csize % sizeof(value_type) != 0) {
                throw std::runtime_error("opt_ref_buffer_t: container size is not compatible with value_type");
            }
        }

        return {
            reinterpret_cast<element_type*>(container.data()),
            csize / sizeof(value_type)
        };
    }

    // non owning construction: explicit only
    template <typename Container>
    static opt_ref_buffer_t ref(Container& container) noexcept (is_congruent<typename Container::value_type>) {
        return opt_ref_buffer_t(0, make_span(container));
    }

    template <typename U, size_t N>
    static opt_ref_buffer_t ref(U(&ar)[N]) noexcept (is_congruent<U>) {
        std::span<U> uspan(ar, N);
        return opt_ref_buffer_t(0, make_span(uspan));
    }

    // taking ownership of existing buffer
    template <typename U>
    opt_ref_buffer_t(opt_ref_buffer_t<U>&& o)
        noexcept (is_congruent<U>)
        requires (!std::is_same_v<T, const U>)
    {
        auto span = make_span(o.span());
        take_data(span, o.m_own);
        o.m_span = {};
    }

    template <typename Container>
    explicit opt_ref_buffer_t(Container&& container) {
        if (container.empty()) return;

        m_own = std::any(std::forward<Container>(container));

        // make span after move in case of small object optimization
        auto& any_c = std::any_cast<Container&>(get<std::any>(m_own));
        m_span = make_span(any_c);
    }

    explicit opt_ref_buffer_t(std::vector<std::byte> vec) noexcept
        : m_span(make_span(vec))
    {
        if (vec.empty()) return;
        m_own = std::move(vec);
    }

    explicit opt_ref_buffer_t(std::string str) noexcept {
        if (str.empty()) return;
        m_own = std::move(str);
        // make span after move in case of small string optimization
        auto& s = get<std::string>(m_own);
        m_span = make_span(s);
    }

    explicit opt_ref_buffer_t(std::string_view str) noexcept
        : opt_ref_buffer_t(std::string(str))
    {}

    // helper for string literals
    template <size_t N>
    explicit opt_ref_buffer_t(const char(&ar)[N])
        : opt_ref_buffer_t(std::string(ar, N))
    {}

    template <typename Container>
    static opt_ref_buffer_t take(Container&& container)
        noexcept(noexcept(opt_ref_buffer_t(std::forward<Container>(container))))
    {
        return opt_ref_buffer_t(std::forward<Container>(container));
    }

    template <typename Container>
    static opt_ref_buffer_t copy(const Container& c) {
        return opt_ref_buffer_t(std::decay_t<Container>(c));
    }

    // query
    const span_type& span() const noexcept { return m_span; }

    element_type* data() const noexcept { return m_span.data(); }

    bool owns_data() const noexcept { return m_own.index() != 0; }

    owned_type& own()& { return m_own; }
    const owned_type& own() const& { return m_own; }

private:
    template <typename U>
    friend class opt_ref_buffer_t;

    void take_data(span_type& span, owned_type& own) noexcept {
        m_span = span;
        m_own = std::move(own);
        own = {}; // reset the moved-from buffer

        if (std::holds_alternative<std::string>(m_own)) {
            // in case of a small string optimization the data pointer may change on move
            // so we need to reinit our span
            m_span = make_span(get<std::string>(m_own));
        }
    }

    // non owning construction is private as it's dangerous
    // only use the provided static ref() methods
    explicit opt_ref_buffer_t(int, span_type span) noexcept
        : m_span(span)
    {}

    span_type m_span;
    owned_type m_own;
};

using opt_ref_buffer = opt_ref_buffer_t<std::byte>;
using const_opt_ref_buffer = opt_ref_buffer_t<const std::byte>;

} // namespace itlib