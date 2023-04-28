// itlib-any v1.02
//
// An alternative implementation of C++17's std::any
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
//  1.02 (2023-xx-xx) * Support for typied and any_cast
//                    * Fixed tdata<const T>
//  1.01 (2023-02-28) Fixed allocator awareness
//  1.00 (2023-02-14) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the type itlib::any, which is a reimplementation of C++17's
// std::any with the following differences:
// * A custom allocator can be provided to itlib::any. The allocator is
//   compatible with C++20's std::pmr::polymorphic_allocator and
//   itlib::pmr_allocator. A default allocator is provided.
// * The type in itlib::any does not need to be copyable. If a copy is
//   attempted for a non-copyable type, std::bad_cast is thrown
//
//                  TESTS
//
// You can find unit tests for qalgorithm in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#include <new>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace itlib {

template <typename Alloc>
class any;

namespace anyimpl {
struct default_allocator {
    void* allocate_bytes(std::size_t n, std::size_t a) {
        void* ret =
#if defined(_MSC_VER)
            _aligned_malloc(n, a);
#else
            aligned_alloc(a, n);
#endif
        if (!ret) throw std::bad_alloc{};
        return ret;
    }
    void deallocate_bytes(void* p, std::size_t /*n*/, std::size_t /*a*/) noexcept {
#if defined(_MSC_VER)
        _aligned_free(p);
#else
        free(p);
#endif
    }
};

struct obj_block {
    uint32_t size; // size of block
    uint32_t alignment; // alignment of block

    virtual ~obj_block() = default;

    virtual void* data() noexcept = 0;

    virtual void clone_to(obj_block* buf) const = 0;

    virtual const std::type_info& type() const noexcept = 0;
};

template <typename T>
struct obj_block_for final : public obj_block {
    T m_data;

    template <typename... Args>
    obj_block_for(Args&&... args)
        : m_data(std::forward<Args>(args)...)
    {}

    ~obj_block_for() = default;

    virtual void* data() noexcept override {
        return &m_data;
    }

    template <typename U = T, typename std::enable_if<!std::is_copy_constructible<U>::value, int>::type = 0>
    [[noreturn]] void do_clone_to(obj_block*) const {
        throw std::bad_cast();
    }

    template <typename U = T, typename std::enable_if<std::is_copy_constructible<U>::value, int>::type = 0>
    void do_clone_to(obj_block* buf) const {
        new (buf) obj_block_for(m_data);
    }

    virtual void clone_to(obj_block* buf) const override {
        do_clone_to<>(buf);
    }

    virtual const std::type_info& type() const noexcept override {
        return typeid(T);
    }
};

// copied from itlib-type_traits
template <typename>
struct is_any : public std::false_type {};
template <typename Alloc>
struct is_any<any<Alloc>> : public std::true_type {};

}

template <typename Alloc = anyimpl::default_allocator>
class any : private /*EBO*/ Alloc {
    anyimpl::obj_block* m_block = nullptr;
public:
    using allocator_type = Alloc;

    any() noexcept = default;
    explicit any(const Alloc& a) noexcept : Alloc(a) {}
    any(std::allocator_arg_t, const Alloc& a) : Alloc(a) {}

    any(any&& o) noexcept : Alloc(o), m_block(o.m_block) {
        o.m_block = nullptr;
    }
    any& operator=(any&& o) noexcept {
        if (&o == this) return *this; // prevent self usurp
        reset();
        m_block = o.m_block;
        o.m_block = nullptr;
        return *this;
    }

    template <typename OAlloc>
    any(const any<OAlloc>& o, const Alloc& a = {}) : Alloc(a) {
        copy_from(o);
    }
    any(const any& o) : any(o, Alloc{}) {}
    any& operator=(const any& o) {
        copy_from(o);
        return *this;
    }

    // only enable these if T is not another any
    template <typename T, typename std::enable_if<!anyimpl::is_any<typename std::decay<T>::type>::value, int>::type = 0>
    any(T&& t) {
        emplace<T>(std::forward<T>(t));
    }
    template <typename T, typename std::enable_if<!anyimpl::is_any<typename std::decay<T>::type>::value, int>::type = 0>
    any(std::allocator_arg_t, const Alloc& a, T&& t) : Alloc(a) {
        emplace<T>(std::forward<T>(t));
    }
    template <typename T, typename std::enable_if<!anyimpl::is_any<typename std::decay<T>::type>::value, int>::type = 0>
    any& operator=(T&& t) {
        emplace<T>(std::forward<T>(t));
        return *this;
    }

    ~any() { reset(); }

    bool has_value() const noexcept { return !!m_block; }
    explicit operator bool() const noexcept { return has_value(); }

    void* data() noexcept {
        if (m_block) return m_block->data();
        return nullptr;
    }
    const void* data() const noexcept {
        if (m_block) return m_block->data();
        return nullptr;
    }

    template <typename T>
    T* tdata() noexcept { return static_cast<T*>(data()); }
    template <typename T>
    const T* tdata() const noexcept { return static_cast<const T*>(data()); }

    Alloc get_allocator() const noexcept { return *this; }

    void reset() noexcept {
        if (!m_block) return;
        auto size = m_block->size;
        auto alignment = m_block->alignment;
        m_block->~obj_block();
        free_block(size, alignment);
    }

    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        reset();
        using obj_block_for_t = anyimpl::obj_block_for<T>;
        constexpr uint32_t size = sizeof(obj_block_for_t);
        constexpr uint32_t alignment = alignof(obj_block_for_t);
        m_block = static_cast<anyimpl::obj_block*>(Alloc::allocate_bytes(size, alignment));
        try {
            auto r = new (m_block) obj_block_for_t(std::forward<Args>(args)...);
            r->size = size;
            r->alignment = alignment;
            return r->m_data;
        }
        catch (...) {
            free_block(size, alignment);
            throw;
        }
    }

    template <typename OAlloc>
    void copy_from(const any<OAlloc>& o) {
        reset();
        if (!o.has_value()) return;
        m_block = static_cast<anyimpl::obj_block*>(Alloc::allocate_bytes(o.m_block->size, o.m_block->alignment));
        try {
            o.m_block->clone_to(m_block);
            m_block->size = o.m_block->size;
            m_block->alignment = o.m_block->alignment;
        }
        catch (...) {
            free_block(o.m_block->size, o.m_block->alignment);
            throw;
        }
    }

    const std::type_info& type() const noexcept {
        if (m_block) return m_block->type();
        return typeid(void);
    }

private:
    void free_block(size_t size, size_t alignment) noexcept {
        Alloc::deallocate_bytes(m_block, size, alignment);
        m_block = nullptr;
    }
};

template <typename T>
const T* any_cast(nullptr_t) { return nullptr; }

template <typename T, typename Alloc>
const T* any_cast(const any<Alloc>* a) {
    if (!a) return nullptr;
    if (typeid(T) != a->type()) return nullptr;
    return a->template tdata<T>();
}

template <typename T, typename Alloc>
T* any_cast(any<Alloc>* a) {
    if (!a) return nullptr;
    if (typeid(T) != a->type()) return nullptr;
    return a->template tdata<T>();
}

namespace anyimpl {
template <typename T, typename A>
T do_cast(A& a) {
    using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    auto ret = any_cast<U>(&a);
    if (!ret) throw std::bad_cast();
    return *ret;
}
}

template <typename T, typename Alloc>
T any_cast(const any<Alloc>& a) { return anyimpl::do_cast<T>(a); }

template <typename T, typename Alloc>
T any_cast(any<Alloc>& a) { return anyimpl::do_cast<T>(a); }

template <typename T, typename Alloc>
T any_cast(any<Alloc>&& a) { return std::move(anyimpl::do_cast<T&>(a)); }

}
