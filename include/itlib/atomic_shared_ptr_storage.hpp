// itlib-atomic-shared-ptr-storage v1.01
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
//  1.01 (2023-04-29) Disable MSVC warning 4243 (padding due do alignment)
//  1.00 (2022-13-12) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines a class atomic_shared_ptr_storage<T> which wraps
// std::shared_ptr<T> and provides atomic load, store, and exchange on it.
//
// It's more or less the same as std::atomic<std::shared_ptr<T>>, but it does
// not provide a pointer interface.
//
// Personal opinion:
// I consider the pointer interface of std::atomic<std::shared_ptr<T>>
// dangerous and bad. This class provides the interface which I consider
// valuable and explicitly describes the purpose - atomic ops.
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <memory>

#if __cplusplus >= 202000L && defined(__cpp_lib_atomic_shared_ptr)
// do what the stdlib implementers chose as best in case it's available
namespace itlib::impl {
template <typename T>
using asps_holder = std::atomic<std::shared_ptr<T>>;
}
#else
#include <atomic>
namespace itlib {
namespace impl {

struct asps_spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire)) /* spin */;
    }
    void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }

    struct lock_guard {
        lock_guard(asps_spinlock& sl) noexcept : m_sl(sl) { m_sl.lock(); }
        ~lock_guard() { m_sl.unlock(); }
    private:
        asps_spinlock& m_sl;
    };
};

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4324)
#endif
template <typename T>
class alignas(64) asps_holder {
    using sptr = std::shared_ptr<T>;
    sptr m_ptr;
    mutable asps_spinlock m_spinlock;
public:
    asps_holder() noexcept = default;
    asps_holder(std::shared_ptr<T> ptr) noexcept : m_ptr(std::move(ptr)) {}

    sptr load() const noexcept {
        asps_spinlock::lock_guard _l(m_spinlock);
        return m_ptr;
    }

    void store(sptr ptr) noexcept {
        asps_spinlock::lock_guard _l(m_spinlock);
        m_ptr.swap(ptr);
    }

    sptr exchange(sptr ptr) noexcept {
        {
            asps_spinlock::lock_guard _l(m_spinlock);
            m_ptr.swap(ptr);
        }
#if __cplusplus >= 201700
        return ptr; // C++17 guarantees NRVO
#else
        return std::move(ptr);
#endif
    }

    // have _strong to match atomic<shared_ptr>
    bool compare_exchange_strong(sptr& expect, sptr ptr) noexcept {
        asps_spinlock::lock_guard _l(m_spinlock);
        if (m_ptr == expect) {
            m_ptr.swap(ptr);
            return true;
        }
        else {
            expect = m_ptr;
            return false;
        }
    }
};
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}
}
#endif

namespace itlib {
template <typename T>
class atomic_shared_ptr_storage {
    impl::asps_holder<T> m_holder;
public:
    using shared_pointer_type = std::shared_ptr<T>;

    atomic_shared_ptr_storage() noexcept = default;
    atomic_shared_ptr_storage(shared_pointer_type ptr) noexcept : m_holder(std::move(ptr)) {}

    atomic_shared_ptr_storage(const atomic_shared_ptr_storage&) = delete;
    atomic_shared_ptr_storage& operator=(const atomic_shared_ptr_storage&) = delete;

    shared_pointer_type load() const noexcept { return m_holder.load(); }
    void store(shared_pointer_type ptr) noexcept { m_holder.store(std::move(ptr)); }

    shared_pointer_type exchange(shared_pointer_type ptr) noexcept { return m_holder.exchange(std::move(ptr)); }

    bool compare_exchange(shared_pointer_type& expect, shared_pointer_type ptr) noexcept {
        return m_holder.compare_exchange_strong(expect, std::move(ptr));
    }
};
}
