// itlib-func_ptr v1.00
//
// Type Erased Function Pointer
// Similar to C++26's std::function_ref, but nullable
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
// It defines the class itlib::func_ptr. A func_ptr is a non-owning type-erased
// function pointer. It can be used to store and call any callable object, but
// it doesn't own the object it points to, thus it doesn't manage its lifetime.
// This makes it a very lightweight alternative to std::function when you don't
// need ownership semantics.
//
// It's trivially copyable, movable, and destructible.
//
// func_ptr is more or less equivalent to std::function_ref from C++26, but
// it's nullable
//
// Example:
//
// // function is not a template and not defined in this unit
// void do_something(itlib::func_ptr<void()> callback);
//
// auto some_lambda = [...](){ ... };
// do_something(&some_lambda); // implicit conversion to func_ptr
//
// auto some_other_lambda = [...](){ ... };
// do_something(&some_other_lambda); // implicit conversion to func_ptr
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <cstddef> // nullptr_t
#include <utility> // std::forward

namespace itlib {

template <typename Func>
class func_ptr;

template <typename Ret, typename... Args>
class func_ptr<Ret(Args...)> {
    void* m_callable_payload;
    Ret(*m_invoke)(void*, Args...);
public:
    func_ptr() noexcept {
        reset();
    }
    func_ptr(std::nullptr_t) noexcept {
        reset(nullptr);
    }
    template <typename Func, typename = decltype(Ret(std::declval<Func>()(std::declval<Args>()...)))>
    func_ptr(Func* func) noexcept {
        reset(func);
    }

    template <typename Func>
    void reset(Func* func) noexcept {
        static_assert(sizeof(Func*) == sizeof(void*));
        m_callable_payload = reinterpret_cast<void*>(func);
        m_invoke = [](void* payload, Args... args) -> Ret {
            Func* f = reinterpret_cast<Func*>(payload);
            return (*f)(std::forward<Args>(args)...);
        };
    }

    void reset() noexcept {
        m_callable_payload = nullptr;
        m_invoke = nullptr;
    }

    void reset(std::nullptr_t) noexcept {
        reset();
    }

    explicit operator bool() const noexcept {
        return !!m_callable_payload;
    }

    template <typename... CallArgs>
    Ret operator()(CallArgs&&... args) const {
        return m_invoke(m_callable_payload, std::forward<CallArgs>(args)...);
    }
};


} // namespace itlib
