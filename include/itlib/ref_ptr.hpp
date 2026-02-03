// itlib-ref_ptr v1.01
//
// A ref-counting smart pointer with a stable use_count
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2026 Borislav Stanimirov
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
//  1.01 (2026-02-03) * nullptr_t constructor and assignment
//                    * _as_shared_ptr_unsafe return ref to avoid copy
//  1.00 (2026-01-31) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
// ref_ptr is a smart, ref-couting pointer im most ways equivalent to
// std::shared_ptr, but without weak_ptr support.
//
// It is currently implemented through shared_ptr, but can be changed to an
// optimized custom implementation in the future.
// Since there is no weak_ptr, use_count() == 1 is reliable and can be used to
// determine if the state is unique.
//
// An important use case for this is copy-on-write implementations.
// ref_ptr<const T> would be a detached read-only view of a state.//
// Since unique() is reliable, a safe "promotion" via a const_cast is possible
// (We're not doing this until we have a good motivational use case)
//
// Note that ref_ptr is just a facility and not a complete CoW implementation.
// While ref_ptr<const T> will likely "spill" out of an implementation as the
// read-only view of the state, ref_ptr<mutable T> will typically be used only
// internally within the implementation where copy guards and other logic will
// be implemented as needed
//
// NO VOID SUPPORT (YET)
// Note that ref_ptr<void> is not supported at the moment.
// It's not a huge additional effort, but there is no immediate use case for it
// either. The main use case is CoW and complete type erasure in this context
// is simply not very useful.
//
// API:
// * everything from std::shared_ptr including...
// * bool unique() const noexcept; - reliable uniqueness check (which used to
//   be in std::shared_ptr but was deprecated and removed)
// * _as_shared_ptr_unsafe() - to get a shared_ptr as a last resort or to
//   interop with existing APIs
// * external factory functions:
//  * make_ref_ptr<T>(Args&&... args) - corresponds to std::make_shared<T>
//  * make_ref_ptr_from(T&& obj) - creates a ref_ptr by copying/moving a value
//
// Future Ideas:
// * void support
// * static/const/dynamic casts
// * strong_ref_ptr - a non-nullable ref_ptr variant
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <memory>
#include <cstddef>
#include <type_traits>

namespace itlib {

template <typename T>
class ref_ptr : private std::shared_ptr<T> {
    template <typename U>
    friend class ref_ptr;
    using super = std::shared_ptr<T>;

    // only used when creating a new ref_ptr
    explicit ref_ptr(std::shared_ptr<T> ptr)
        : super(std::move(ptr))
    {}
public:
    ref_ptr() = default;
    ref_ptr(const ref_ptr&) = default;
    ref_ptr& operator=(const ref_ptr&) = default;
    ref_ptr(ref_ptr&&) noexcept = default;
    ref_ptr& operator=(ref_ptr&&) noexcept = default;

    ref_ptr(std::nullptr_t)
        : super(nullptr)
    {}
    ref_ptr& operator=(std::nullptr_t) {
        super::operator=(nullptr);
        return *this;
    }

    template <typename U>
    ref_ptr(const ref_ptr<U>& ptr) noexcept
        : super(ptr)
    {}
    template <typename U>
    ref_ptr& operator=(const ref_ptr<U>& ptr) noexcept {
        super::operator=(ptr);
        return *this;
    }
    template <typename U>
    ref_ptr(const ref_ptr<U>&& ptr) noexcept
        : super(std::move(ptr))
    {}
    template <typename U>
    ref_ptr& operator=(const ref_ptr<U>&& ptr) noexcept {
        super::operator=(std::move(ptr));
        return *this;
    }

    using super::get;
    using super::operator->;
    using super::operator*;
    using super::operator bool;
    using super::use_count;
    using super::reset;

    // we could just do an operator <=>, but let's support pre-c++20
    template <typename U>
    bool operator==(const ref_ptr<U>& other) const noexcept { return get() == other.get(); }
    template <typename U>
    bool operator!=(const ref_ptr<U>& other) const noexcept { return get() != other.get(); }
    template <typename U>
    bool operator<(const ref_ptr<U>& other) const noexcept { return get() < other.get(); }
    template <typename U>
    bool operator<=(const ref_ptr<U>& other) const noexcept { return get() <= other.get(); }
    template <typename U>
    bool operator>(const ref_ptr<U>& other) const noexcept { return get() > other.get(); }
    template <typename U>
    bool operator>=(const ref_ptr<U>& other) const noexcept { return get() >= other.get(); }

    bool unique() const noexcept {
        return use_count() == 1;
    }

    template <typename... Args>
    static ref_ptr make(Args&&... args) {
        return ref_ptr(std::make_shared<T>(std::forward<Args>(args)...));
    }

    // only use as a last resort
    // when ref_ptr needs to be provided to an existing API relying on shared_ptr
    // be sure that the leaked shared_ptr will not be used to create weak_ptr-s
    const std::shared_ptr<T>& _as_shared_ptr_unsafe() const& noexcept {
        return *this;
    }
    std::shared_ptr<T> _as_shared_ptr_unsafe() && noexcept {
        return std::move(*this);
    }
    static ref_ptr<T> _from_shared_ptr_unsafe(std::shared_ptr<T> ptr) noexcept {
        return ref_ptr<T>(std::move(ptr));
    }
};

template <typename T, typename... Args>
ref_ptr<T> make_ref_ptr(Args&&... args) {
    return ref_ptr<T>::make(std::forward<Args>(args)...);
}

template <typename T>
auto make_ref_ptr_from(T&& obj) -> ref_ptr<typename std::remove_reference<T>::type>{
    return ref_ptr<typename std::remove_reference<T>::type>::make(std::forward<T>(obj));
}

template <typename T>
ref_ptr<T> _ref_ptr_from_shared_ptr_unsafe(std::shared_ptr<T> ptr) noexcept {
    return ref_ptr<T>::_from_shared_ptr_unsafe(std::move(ptr));
}

} // namespace itlib
