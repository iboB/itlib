// itlib-atomic-shared-ptr-storage v1.00
//
// A saner alternative to std::atomic<std::shared_ptr<T>>
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
//  1.00 (2020-13-12) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
//
//                  TESTS
//
// You can find unit tests for atomic-shared-ptr-storage in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <memory>

#if __cplusplus >= 202000L
// do what the stdlib implementers chose as best in case it's available
namespace itlib::impl {
template <typename T>
using asps_holder = std::atomic<std::shared_ptr<T>>;
}
#else
#include <atomic>
namespace itlib {
namespace impl {

struct alignas(64) asps_spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire)) /* spin */;
    }
    void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }

    struct lock_guard {
        lock_guard(asps_spinlock& sl) : m_sl(sl) { m_sl.lock(); }
        ~lock_guard() { m_sl.unlock(); }
    private:
        asps_spinlock& m_sl;
    };
};

template <typename T>
class asps_holder {
    using sptr = std::shared_ptr<T>;
    sptr m_ptr;
    mutable asps_spinlock m_spinlock;
public:
    asps_holder() = default;
    asps_holder(std::shared_ptr<T> ptr) : m_ptr(std::move(ptr)) {}

    sptr load() const noexcept {
        asps_spinlock::lock_guard _l(m_spinlock);
        return m_ptr;
    }

    void store(sptr ptr) noexcept {
        asps_spinlock::lock_guard _l(m_spinlock);
        m_ptr.swap(ptr);
    }
};

}
}
#endif

namespace itlib {
template <typename T>
class atomic_shared_ptr_storage {
    impl::asps_holder<T> m_holder;
public:
    using shared_pointer_type = std::shared_ptr<T>;

    atomic_shared_ptr_storage() = default;
    atomic_shared_ptr_storage(shared_pointer_type ptr) : m_holder(std::move(ptr)) {}

    atomic_shared_ptr_storage(const atomic_shared_ptr_storage&) = delete;
    atomic_shared_ptr_storage& operator=(const atomic_shared_ptr_storage&) = delete;

    shared_pointer_type load() const noexcept { return m_holder.load(); }
    void store(shared_pointer_type ptr) noexcept { m_holder.store(std::move(ptr)); }
};
}
