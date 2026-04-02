// itlib-shared_func v1.00
//
// A std::functon alternative which keeps the function object data in a
// shared_ptr, combining the functionality of both.
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
//  1.00 (2026-03-31) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::shared_func. A shared_func is an alternative to
// std::function which keeps the function object data in a shared_ptr.
//
// This means that the same function object can be shared between multiple
// shared_func instances, allowing copy semantics without copying the function
// object data. This can be useful when there are huge captures or non-copyable
// captures.
//
// Still, the function is shared so the captures are shared too. This may
// introduce race conditions if the function is called from multiple threads,
// so use the mental model for shared_ptr when you're using shared_func.
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <memory>
#include <type_traits>

namespace itlib {

template <typename F>
class shared_func;

template <typename F>
class weak_func;

template <typename R, typename... Args>
class shared_func<R(Args...)> {
public:
    shared_func() noexcept = default;
    shared_func(std::nullptr_t) noexcept : m_wrapper(nullptr) {}
    shared_func(const shared_func&) = default;
    shared_func& operator=(const shared_func&) = default;
    shared_func(shared_func&&) noexcept = default;
    shared_func& operator=(shared_func&&) noexcept = default;
    template <typename FO>
    shared_func(FO f) : m_wrapper(make_wrapper(std::move(f))) {}

    // this also serves to handle shared_func = nullptr_t
    template <typename FO>
    shared_func& operator=(FO f) {
        m_wrapper = make_wrapper(std::move(f));
        return *this;
    }

    explicit operator bool() const noexcept {
        return !!m_wrapper;
    }

    R operator()(Args&&... args) const {
        // intentionally not checking and throwing bad_function_call here
        // * I personally dislike this behavior
        // * now we don't have to include <functional> for this exception
        return m_wrapper->func(m_wrapper.get(), std::forward<Args>(args)...);
    }

    // shared_ptr interface
    size_t use_count() const noexcept {
        return m_wrapper.use_count();
    }

private:
    friend class weak_func<R(Args...)>;

    struct wrapper_base {
        // Note that we're not using virtual here, even though we have dynamic
        // dispatch. The problem with virtual is that it will create a vtable
        // pointer. It this is used across shared libraries (.so/.dll/.dylib),
        // it guarantees that the typeid and vtable pointer will be different
        // across the modules.
        // This is not a problem in practice since the compiler will generate
        // the same layout for the vtables, but if used with sanitizers or
        // other type of runtime instrumentation, it can cause false positives
        // for type mismatches.
        // So... poor man's vtable:
        R(*func)(void* obj, Args&&...);
    protected:
        // we can't afford a virtual dtor, so make sure that it's not available
        ~wrapper_base() = default;
    };
    std::shared_ptr<wrapper_base> m_wrapper;

    template <typename FO>
    struct wrapper : public wrapper_base {
        FO func_object;
        wrapper(FO f) : func_object(std::move(f)) {
            this->func = +[](void* obj, Args&&... args) -> R {
                return static_cast<wrapper*>(obj)->func_object(std::forward<Args>(args)...);
            };
        }
    };

    static std::shared_ptr<wrapper_base> make_wrapper(std::nullptr_t) {
        return {};
    }

    template <typename FO>
    static std::shared_ptr<wrapper_base> make_wrapper(FO f) {
        return std::make_shared<wrapper<FO>>(std::move(f));
    }

    template <typename FP>
    static std::shared_ptr<wrapper_base> make_wrapper(FP* f) {
        if (!f) {
            return {};
        }
        return std::make_shared<wrapper<FP*>>(f);
    }
};

template <typename T>
class weak_func {
public:
    weak_func() noexcept = default;
    weak_func(const shared_func<T>& func) : m_wrapper(func.m_wrapper) {}

    shared_func<T> lock() const {
        shared_func<T> result;
        result.m_wrapper = m_wrapper.lock();
        return result;
    }

    bool expired() const noexcept {
        return m_wrapper.expired();
    }

private:
    std::weak_ptr<typename shared_func<T>::wrapper_base> m_wrapper;
};

} // namespace itlib
