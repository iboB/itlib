// itlib-data-mutex v1.00
//
// A lockable data type. Merging mutex with data
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2023 Borislav Stanimirov
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
//  1.00 (2023-02-07) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// The library defines the template type data_mutex<T, Mutex> which is a
// pair of an object and an associated mutex used to synchronize access to it.
//
// To access the object inside one must use one of the following methods:
// * unique_lock (mutex lock/unlock)
// * try_unique_lock (mutex try_lock/unlock)
// * shared_lock (mutex lock_shared/unlock_shared)
// * try_shared_lock (mutex try_lock_shared/unlock_shared)
//
// They all return thin pointer-like wrappers of T. Which have operators
// -> and *. In the case of the try_* functions, they also have a bool
// interface
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <utility>

namespace itlib {

template <typename T, typename Mutex>
class data_mutex {
    mutable Mutex m_mutex;
    T m_data;
public:
    template <typename... Args>
    data_mutex(Args&&... args) : m_data(std::forward<Args>(args)...) {}

    data_mutex(const data_mutex&) = delete;
    data_mutex& operator=(const data_mutex&) = delete;
    data_mutex(data_mutex&&) = delete;
    data_mutex& operator=(data_mutex&&) = delete;

    template <typename CT>
    class lock_base_t {
    protected:
        Mutex& m_mutex;
        CT* m_data;
        lock_base_t(Mutex& mutex, CT& data) : m_mutex(mutex), m_data(&data) {}
        lock_base_t(Mutex& mutex) : m_mutex(mutex) {}
        ~lock_base_t() = default;
        lock_base_t(const lock_base_t&) = delete;
        lock_base_t& operator=(const lock_base_t&) = delete;
        lock_base_t(lock_base_t&& o) noexcept : m_mutex(o.m_mutex), m_data(o.m_data) {
            o.m_data = nullptr;
        }
        lock_base_t& operator=(lock_base_t&& o) noexcept = delete;
    public:
        CT& operator*() { return *m_data; }
        CT* operator->() { return m_data; }
        const CT& operator*() const { return *m_data; }
        const CT* operator->() const { return m_data; }
    };

    template <typename CT>
    class unique_lock_base_t : public lock_base_t<CT> {
    protected:
        using lock_base_t<CT>::lock_base_t;
        unique_lock_base_t(unique_lock_base_t&&) = default;
        ~unique_lock_base_t() { if (this->m_data) this->m_mutex.unlock(); }
    };

    struct unique_lock_t : public unique_lock_base_t<T> {
        explicit unique_lock_t(data_mutex& dm) : unique_lock_base_t<T>(dm.m_mutex, dm.m_data) {
            this->m_mutex.lock();
        }
    };

    struct try_unique_lock_t : public unique_lock_base_t<T> {
        explicit try_unique_lock_t(data_mutex& dm) : unique_lock_base_t<T>(dm.m_mutex) {
            if (this->m_mutex.try_lock()) this->m_data = &dm.m_data;
            else this->m_data = nullptr;
        }
        explicit operator bool() const noexcept { return !!this->m_data; }
    };

    struct const_unique_lock_t : public unique_lock_base_t<const T> {
        explicit const_unique_lock_t(const data_mutex& dm) : unique_lock_base_t<const T>(dm.m_mutex, dm.m_data) {
            this->m_mutex.lock();
        }
    };

    struct try_const_unique_lock_t : public unique_lock_base_t<const T> {
        explicit try_const_unique_lock_t(const data_mutex& dm) : unique_lock_base_t<const T>(dm.m_mutex) {
            if (this->m_mutex.try_lock()) this->m_data = &dm.m_data;
            else this->m_data = nullptr;
        }
        explicit operator bool() const noexcept { return !!this->m_data; }
    };

    class shared_lock_base_t : public lock_base_t<const T> {
    protected:
        using lock_base_t<const T>::lock_base_t;
        shared_lock_base_t(shared_lock_base_t&&) = default;
        ~shared_lock_base_t() { if (this->m_data) this->m_mutex.unlock_shared(); }
    };

    struct shared_lock_t : public shared_lock_base_t {
        explicit shared_lock_t(const data_mutex& dm) : shared_lock_base_t(dm.m_mutex, dm.m_data) {
            this->m_mutex.lock_shared();
        }
    };

    struct try_shared_lock_t : public shared_lock_base_t {
        explicit try_shared_lock_t(const data_mutex& dm) : shared_lock_base_t(dm.m_mutex) {
            if (this->m_mutex.try_lock_shared()) this->m_data = &dm.m_data;
            else this->m_data = nullptr;
        }
        explicit operator bool() const noexcept { return !!this->m_data; }
    };

    unique_lock_t unique_lock() {
        unique_lock_t x(*this);
        return x;
    }

    const_unique_lock_t unique_lock() const {
        return const_unique_lock_t(*this);
    }

    try_unique_lock_t try_unique_lock() {
        return try_unique_lock_t(*this);
    }

    try_const_unique_lock_t try_unique_lock() const {
        return try_const_unique_lock_t(*this);
    }

    shared_lock_t shared_lock() const {
        return shared_lock_t(*this);
    }

    try_shared_lock_t try_shared_lock() const {
        return try_shared_lock_t(*this);
    }
};

}
