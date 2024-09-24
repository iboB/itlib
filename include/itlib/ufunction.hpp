// itlib-ufunction v1.02
//
// Unique Function
// Non-copyable and noexcept move-constructible replacement for std::function
// Similar to C++23's move_only_function
//
// SPDX-License-Identifier: MIT
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
//  1.02 (2024-09-24) Allow binding to copies of source functions as per C++23
//  1.01 (2022-09-23) Allow ufunction from a free function
//  1.00 (2020-10-15) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::ufunction. It can serve as a replacement
// of std::function, but it differs in the following two ways:
//
// 1. itlib::ufunction is not copyable. Thus you can capture non-copyable
// values from the outside world, like std::unique_ptr, or wrap other
// non-copyable function objects.
// 2. itlib::ufunction is noexcept move-constructible, thus vectors of
// ufunction won't copy when expanded and structures with ufunction members
// will not be implicitly no-noexcept move-constructuble
//
// You can use itlib::ufunction in most places where you would use
// std::function as long as you don't copy it
//
// ufunction is essentially equivalent to std::move_only_function from C++23
//
// Example:
//
// std::unique_ptr<foo> fp;
// itlib::ufunction<void()> = [captured = std::move(fp)]() { ... }
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <functional>
#include <type_traits>

namespace itlib
{

template <typename F>
class ufunction : private std::function<F>
{
    using function = std::function<F>;
public:
    ufunction() noexcept = default;

    ufunction(std::nullptr_t) noexcept : function(nullptr) {}
    ufunction& operator=(std::nullptr_t) noexcept { function::operator=(nullptr); return *this; }

    ufunction(const ufunction&) = delete;
    ufunction operator=(const ufunction&) = delete;

    ufunction(ufunction&&) noexcept = default;
    ufunction& operator=(ufunction&&) noexcept = default;

    template <typename FO>
    ufunction(FO f) noexcept : function(copy_wrapper<FO>{std::move(f)}) {}

    template <typename FO>
    ufunction& operator=(FO f) noexcept
    {
        function::operator=(copy_wrapper<FO>{std::move(f)});
        return *this;
    }

    // function pointer overloads (otherwise clang and gcc complain for const_cast of function pointers)
    // noexcept since we're relying on small function opti
    ufunction(F* fptr) noexcept : function(fptr) {}
    ufunction& operator=(F* fptr) noexcept
    {
        function::operator=(fptr);
        return *this;
    }

    using function::operator bool;
    using function::operator();
private:
    template <typename FO>
    struct copy_wrapper
    {
        static_assert(!std::is_const<FO>::value, "Cannot bind to a const function");
        FO func_object;
        copy_wrapper(FO&& f) : func_object(std::move(f)) {}

        // we need these copy ops in order for our parent std::function to compile, but they will never be called
        copy_wrapper(const copy_wrapper& other) : func_object(const_cast<FO&&>(other.func_object)) { throw 0; } // should never get to here
        copy_wrapper& operator=(const copy_wrapper&) { throw 0; } // should never get to here

        // not `= default` so we can force noexcept
        copy_wrapper(copy_wrapper&& other) noexcept : func_object(std::move(other.func_object)) {}
        copy_wrapper& operator=(copy_wrapper&& other) noexcept
        {
            func_object = std::move(other.func_object);
            return *this;
        }

        template <typename... Args>
        auto operator()(Args&&... args) -> decltype(func_object(std::forward<Args>(args)...))
        {
            return func_object(std::forward<Args>(args)...);
        }
    };
};

}
