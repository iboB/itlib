// itlib-atomic v0.02 beta
//
// Extenstions for std::atomic
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
//  0.02 (2023-01-27) relaxed counter: copying, explicit ctor, explicit ops
//  0.01 (2022-12-06) Initial beta release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines several utility types which extend std::atomic
//
// * atomic_relaxed_counter
//      An atomic integral type for which the all operations are implemented
//      with std::memory_order_relaxed. It's useful for counters
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
#pragma once
#include <atomic>
#include <type_traits>

namespace itlib
{

template <typename I>
struct atomic_relaxed_counter {
    static_assert(std::is_integral<I>::value, "counter must be integral");

    std::atomic<I> a;

    static const std::memory_order rel = std::memory_order_relaxed;

    atomic_relaxed_counter() noexcept = default;
    explicit atomic_relaxed_counter(I init) noexcept : a(init) {};

    atomic_relaxed_counter(const atomic_relaxed_counter& b) : a(b.load()) {}
    atomic_relaxed_counter& operator=(const atomic_relaxed_counter& b) {
        a.store(b.load());
        return *this;
    }

    I load() const noexcept { return a.load(rel); }
    void store(I i) noexcept { return a.store(i, rel); }

    operator I() const noexcept { return load(); } // intentionally implicit

    I operator++() noexcept { return a.fetch_add(1, rel) + 1; }
    I operator++(int) noexcept { return a.fetch_add(1, rel); }
    I operator+=(I i) noexcept { return a.fetch_add(i, rel) + i; }
    I operator--() noexcept { return a.fetch_sub(1, rel) - 1; }
    I operator--(int) noexcept { return a.fetch_sub(1, rel); }
    I operator-=(I i) noexcept { return a.fetch_sub(i, rel) - i; }

    I operator=(I i) {
        store(i);
        return i;
    }
};

struct atomic_flag_bool {

   std::atomic_bool a{ true };

   atomic_flag_bool() noexcept = default;
   explicit atomic_flag_bool(bool init) noexcept {
      set(init);
   }

   void set(bool value) noexcept {
      a.store(value, std::memory_order_release);
   }

   atomic_flag_bool& operator=(bool i) {
      set(i);
      return *this;
   }

   operator bool() const noexcept {
      return get();
   }

   bool get() const noexcept {
      return a.load(std::memory_order_acquire);
   }
};


}
