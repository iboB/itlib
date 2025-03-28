// itlib-small-vector v2.06
//
// std::vector-like class with a static buffer for initial capacity
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2016-2018 Chobolabs Inc.
// Copyright(c) 2020-2025 Borislav Stanimirov
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
//  2.06 (2025-03-28) Minor: Add missing header <cstdint>
//  2.05 (2024-03-06) Minor: Return bool from shrink_to_fit
//  2.04 (2022-04-29) Minor: Disable MSVC warning for constant conditional
//  2.03 (2022-10-31) Minor: Removed unused local var
//  2.02 (2022-09-24) Minor: Fixed leftover arguments in error handling macros
//  2.01 (2022-08-26) Minor: renames, doc
//  2.00 (2022-08-26) Redesign
//                    * Smaller size
//                    * Inherit from allocator to make use of EBO
//  1.04 (2022-04-14) Noxcept move construct and assign
//  1.03 (2021-10-05) Use allocator member instead of inheriting from allocator
//                    Allow compare with small_vector of different static_size
//                    Don't rely on operator!= from T. Use operator== instead
//  1.02 (2021-09-15) Bugfix! Fixed bad deallocation when reverting to
//                    static size on resize()
//  1.01 (2021-08-05) Bugfix! Fixed return value of erase
//  1.00 (2020-10-14) Rebranded release from chobo-small-vector
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::small_vector, which is a drop-in replacement of
// std::vector, but with an initial capacity as a template argument.
// It gives you the benefits of using std::vector, at the cost of having a statically
// allocated buffer for the initial capacity, which gives you cache-local data
// when the vector is small (smaller than the initial capacity).
//
// When the size exceeds the capacity, the vector allocates memory via the provided
// allocator, falling back to classic std::vector behavior.
//
// The second size_t template argument, RevertToStaticBelow, is used when a
// small_vector which has already switched to dynamically allocated size reduces
// its size to a number smaller than that. In this case the vector's buffer
// switches back to the staticallly allocated one
//
// A default value for the initial static capacity is provided so a replacement
// in an existing code is possible with minimal changes to it.
//
// Example:
//
// itlib::small_vector<int, 4, 5> myvec; // a small_vector of size 0, initial capacity 4, and revert size 4 (below 5)
// myvec.resize(2); // vector is {0,0} in static buffer
// myvec[1] = 11; // vector is {0,11} in static buffer
// myvec.push_back(7); // vector is {0,11,7}  in static buffer
// myvec.insert(myvec.begin() + 1, 3); // vector is {0,3,11,7} in static buffer
// myvec.push_back(5); // vector is {0,3,11,7,5} in dynamically allocated memory buffer
// myvec.erase(myvec.begin());  // vector is {3,11,7,5} back in static buffer
// myvec.resize(5); // vector is {3,11,7,5,0} back in dynamically allocated memory
//
//
// Reference:
//
// itlib::small_vector is fully compatible with std::vector with
// the following exceptions:
// * when reducing the size with erase or resize the new size may fall below
//   RevertToStaticBelow (if it is not 0). In such a case the vector will
//   revert to using its static buffer, invalidating all iterators (contrary
//   to the standard)
// * a method is added `revert_to_static()` which reverts to the static buffer
//   if possible and does nothing if the size doesn't allow it
//
// Additionally, the following methods are added:
//
// * is_static() returns whether the vector is currently using its static buffer
//
// Other notes:
//
// * shrink_to_fit() returns a bool indicating whether the vector was shrunk (and the
//   iterators invalidated)
// * the default value for RevertToStaticBelow is zero. This means that once a dynamic
//   buffer is allocated the data will never be put into the static one, even if the
//   size allows it. Even if clear() is called. The only way to do so is to call
//   shrink_to_fit() or revert_to_static()
// * shrink_to_fit will free and reallocate if size != capacity and the data
//   doesn't fit into the static buffer. It also will revert to the static buffer
//   whenever possible regardless of the RevertToStaticBelow value
//
//
//                  Configuration
//
// The library has two configuration options. They can be set as #define-s
// before including the header file, but it is recommended to change the code
// of the library itself with the values you want, especially if you include
// the library in many compilation units (as opposed to, say, a precompiled
// header or a central header).
//
//                  Config out of range error handling
//
// An out of range error is a runtime error which is triggered when a method is
// called with an iterator that doesn't belong to the vector's current range.
// For example: vec.erase(vec.end() + 1);
//
// This is set by defining ITLIB_SMALL_VECTOR_ERROR_HANDLING to one of the
// following values:
// * ITLIB_SMALL_VECTOR_ERROR_HANDLING_NONE - no error handling. Crashes WILL
//      ensue if the error is triggered.
// * ITLIB_SMALL_VECTOR_ERROR_HANDLING_THROW - std::out_of_range is thrown.
// * ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT - asserions are triggered.
// * ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT_AND_THROW - combines assert and
//      throw to catch errors more easily in debug mode
//
// To set this setting by editing the file change the line:
// ```
// #   define ITLIB_SMALL_VECTOR_ERROR_HANDLING ITLIB_SMALL_VECTOR_ERROR_HANDLING_THROW
// ```
// to the default setting of your choice
//
//                  Config bounds checks:
//
// By default bounds checks are made in debug mode (via an asser) when accessing
// elements (with `at` or `[]`). Iterators are not checked (yet...)
//
// To disable them, you can define ITLIB_SMALL_VECTOR_NO_DEBUG_BOUNDS_CHECK
// before including the header.
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <memory>

#define ITLIB_SMALL_VECTOR_ERROR_HANDLING_NONE  0
#define ITLIB_SMALL_VECTOR_ERROR_HANDLING_THROW 1
#define ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT 2
#define ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT_AND_THROW 3

#if !defined(ITLIB_SMALL_VECTOR_ERROR_HANDLING)
#   define ITLIB_SMALL_VECTOR_ERROR_HANDLING ITLIB_SMALL_VECTOR_ERROR_HANDLING_THROW
#endif


#if ITLIB_SMALL_VECTOR_ERROR_HANDLING == ITLIB_SMALL_VECTOR_ERROR_HANDLING_NONE
#   define I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(cond)
#elif ITLIB_SMALL_VECTOR_ERROR_HANDLING == ITLIB_SMALL_VECTOR_ERROR_HANDLING_THROW
#   include <stdexcept>
#   define I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(cond) if (cond) throw std::out_of_range("itlib::small_vector out of range")
#elif ITLIB_SMALL_VECTOR_ERROR_HANDLING == ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT
#   include <cassert>
#   define I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(cond) assert(!(cond) && "itlib::small_vector out of range")
#elif ITLIB_SMALL_VECTOR_ERROR_HANDLING == ITLIB_SMALL_VECTOR_ERROR_HANDLING_ASSERT_AND_THROW
#   include <stdexcept>
#   include <cassert>
#   define I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(cond) \
    do { if (cond) { assert(false && "itlib::small_vector out of range"); throw std::out_of_range("itlib::small_vector out of range"); } } while(false)
#else
#error "Unknown ITLIB_SMALL_VECTOR_ERRROR_HANDLING"
#endif


#if defined(ITLIB_SMALL_VECTOR_NO_DEBUG_BOUNDS_CHECK)
#   define I_ITLIB_SMALL_VECTOR_BOUNDS_CHECK(i)
#else
#   include <cassert>
#   define I_ITLIB_SMALL_VECTOR_BOUNDS_CHECK(i) assert((i) < this->size())
#endif

namespace itlib
{

template<typename T, size_t StaticCapacity = 16, size_t RevertToStaticBelow = 0, class Alloc = std::allocator<T>>
struct small_vector : private Alloc
{
    static_assert(RevertToStaticBelow <= StaticCapacity + 1, "itlib::small_vector: the RevertToStaticBelow shouldn't exceed the static capacity by more than one");

    using atraits = std::allocator_traits<Alloc>;
public:
    using allocator_type = Alloc;
    using value_type = typename atraits::value_type;
    using size_type = typename atraits::size_type;
    using difference_type = typename atraits::difference_type;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename atraits::pointer;
    using const_pointer = typename atraits::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_t static_capacity = StaticCapacity;
    static constexpr intptr_t revert_to_static_below = RevertToStaticBelow;

    small_vector()
        : small_vector(Alloc())
    {}

    small_vector(const Alloc& alloc)
        : Alloc(alloc)
        , m_capacity(StaticCapacity)
    {
        m_begin = m_end = static_begin_ptr();
    }

    explicit small_vector(size_t count, const Alloc& alloc = Alloc())
        : small_vector(alloc)
    {
        resize(count);
    }

    explicit small_vector(size_t count, const T& value, const Alloc& alloc = Alloc())
        : small_vector(alloc)
    {
        assign_fill(count, value);
    }

    template <class InputIterator, typename = decltype(*std::declval<InputIterator>())>
    small_vector(InputIterator first, InputIterator last, const Alloc& alloc = Alloc())
        : small_vector(alloc)
    {
        assign_copy(first, last);
    }

    small_vector(std::initializer_list<T> l, const Alloc& alloc = Alloc())
        : small_vector(alloc)
    {
        assign_move(l.begin(), l.end());
    }

    small_vector(const small_vector& v)
        : small_vector(v, atraits::select_on_container_copy_construction(v.get_allocator()))
    {}

    small_vector(const small_vector& v, const Alloc& alloc)
        : small_vector(alloc)
    {
        assign_copy(v.begin(), v.end());
    }

    small_vector(small_vector&& v) noexcept
        : Alloc(std::move(v.get_alloc()))
        , m_capacity(v.m_capacity)
    {
        take_impl(v);
    }

    ~small_vector()
    {
        destroy_all();

        if (!is_static())
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }
    }

    small_vector& operator=(const small_vector& v)
    {
        if (this == &v)
        {
            // prevent self usurp
            return *this;
        }

        destroy_all();
        assign_copy(v.begin(), v.end());

        return *this;
    }

    small_vector& operator=(small_vector&& v) noexcept
    {
        if (this == &v)
        {
            // prevent self usurp
            return *this;
        }

        destroy_all();
        if (!is_static())
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }

        get_alloc() = std::move(v.get_alloc());
        m_capacity = v.m_capacity;

        take_impl(v);

        return *this;
    }

    void assign(size_type count, const T& value)
    {
        destroy_all();
        assign_fill(count, value);
    }

    template <class InputIterator, typename = decltype(*std::declval<InputIterator>())>
    void assign(InputIterator first, InputIterator last)
    {
        destroy_all();
        assign_copy(first, last);
    }

    void assign(std::initializer_list<T> ilist)
    {
        destroy_all();
        assign_move(ilist.begin(), ilist.end());
    }

    allocator_type get_allocator() const
    {
        return get_alloc();
    }

    const_reference at(size_type i) const
    {
        I_ITLIB_SMALL_VECTOR_BOUNDS_CHECK(i);
        return *(m_begin + i);
    }

    reference at(size_type i)
    {
        I_ITLIB_SMALL_VECTOR_BOUNDS_CHECK(i);
        return *(m_begin + i);
    }

    const_reference operator[](size_type i) const
    {
        return at(i);
    }

    reference operator[](size_type i)
    {
        return at(i);
    }

    const_reference front() const
    {
        return at(0);
    }

    reference front()
    {
        return at(0);
    }

    const_reference back() const
    {
        return *(m_end - 1);
    }

    reference back()
    {
        return *(m_end - 1);
    }

    const_pointer data() const noexcept
    {
        return m_begin;
    }

    pointer data() noexcept
    {
        return m_begin;
    }

    // iterators
    iterator begin() noexcept
    {
        return m_begin;
    }

    const_iterator begin() const noexcept
    {
        return m_begin;
    }

    const_iterator cbegin() const noexcept
    {
        return m_begin;
    }

    iterator end() noexcept
    {
        return m_end;
    }

    const_iterator end() const noexcept
    {
        return m_end;
    }

    const_iterator cend() const noexcept
    {
        return m_end;
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    // capacity
    bool empty() const noexcept
    {
        return m_begin == m_end;
    }

    size_t size() const noexcept
    {
        return m_end - m_begin;
    }

    size_t max_size() const noexcept
    {
        return atraits::max_size();
    }

    void reserve(size_type new_cap)
    {
        if (new_cap <= m_capacity) return;

        const auto cdr = choose_data(new_cap);

        assert(cdr.ptr != m_begin); // should've been handled by new_cap <= m_capacity
        assert(cdr.ptr != static_begin_ptr()); // we should never reserve into static memory

        auto s = size();

        // now we need to transfer the existing elements into the new buffer
        for (size_type i = 0; i < s; ++i)
        {
            atraits::construct(get_alloc(), cdr.ptr + i, std::move(*(m_begin + i)));
        }

        // free old elements
        for (size_type i = 0; i < s; ++i)
        {
            atraits::destroy(get_alloc(), m_begin + i);
        }

        if (!is_static())
        {
            // we've moved from dyn to dyn memory, so deallocate the old one
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }

        m_begin = cdr.ptr;
        m_end = m_begin + s;
        m_capacity = cdr.cap;
    }

    size_t capacity() const noexcept
    {
        return m_capacity;
    }

    bool shrink_to_fit()
    {
        const auto s = size();

        if (s == m_capacity) return false; // we're at max
        if (is_static()) return false; // can't shrink static buf

        auto old_begin = m_begin;
        auto old_end = m_end;
        auto old_cap = m_capacity;

        if (s < StaticCapacity)
        {
            // revert to static capacity
            m_begin = m_end = static_begin_ptr();
            m_capacity = StaticCapacity;
        }
        else
        {
            // alloc new smaller buffer
            m_begin = m_end = atraits::allocate(get_alloc(), s);
            m_capacity = s;
        }

        for (auto p = old_begin; p != old_end; ++p)
        {
            atraits::construct(get_alloc(), m_end, std::move(*p));
            ++m_end;
            atraits::destroy(get_alloc(), p);
        }

        atraits::deallocate(get_alloc(), old_begin, old_cap);
        return true;
    }

    // only revert if possible
    // otherwise don't shrink
    // return true if the the vector is now static
    bool revert_to_static()
    {
        const auto s = size();
        if (is_static()) return true; //we're already there
        if (s > StaticCapacity) return false; // nothing we can do

        shrink_to_fit();
        return true;
    }

    // modifiers
    void clear() noexcept
    {
        destroy_all();

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant
#endif
        if (RevertToStaticBelow > 0 && !is_static())
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
            m_begin = m_end = static_begin_ptr();
            m_capacity = StaticCapacity;
        }
        else
        {
            m_end = m_begin;
        }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    iterator insert(const_iterator position, const value_type& val)
    {
        auto pos = grow_at(position, 1);
        atraits::construct(get_alloc(), pos, val);
        return pos;
    }

    iterator insert(const_iterator position, value_type&& val)
    {
        auto pos = grow_at(position, 1);
        atraits::construct(get_alloc(), pos, std::move(val));
        return pos;
    }

    iterator insert(const_iterator position, size_type count, const value_type& val)
    {
        auto pos = grow_at(position, count);
        for (size_type i = 0; i < count; ++i)
        {
            atraits::construct(get_alloc(), pos + i, val);
        }
        return pos;
    }

    template <typename InputIterator, typename = decltype(*std::declval<InputIterator>())>
    iterator insert(const_iterator position, InputIterator first, InputIterator last)
    {
        auto pos = grow_at(position, last - first);
        auto np = pos;
        for (auto p = first; p != last; ++p, ++np)
        {
            atraits::construct(get_alloc(), np, *p);
        }
        return pos;
    }

    iterator insert(const_iterator position, std::initializer_list<T> ilist)
    {
        auto pos = grow_at(position, ilist.size());
        size_type i = 0;
        for (auto& elem : ilist)
        {
            atraits::construct(get_alloc(), pos + i, std::move(elem));
            ++i;
        }
        return pos;
    }

    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
        auto pos = grow_at(position, 1);
        atraits::construct(get_alloc(), pos, std::forward<Args>(args)...);
        return pos;
    }

    iterator erase(const_iterator position)
    {
        return shrink_at(position, 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(first > last);
        return shrink_at(first, last - first);
    }

    void push_back(const_reference val)
    {
        auto pos = grow_at(m_end, 1);
        atraits::construct(get_alloc(), pos, val);
    }

    void push_back(T&& val)
    {
        auto pos = grow_at(m_end, 1);
        atraits::construct(get_alloc(), pos, std::move(val));
    }

    template<typename... Args>
    reference emplace_back(Args&&... args)
    {
        auto pos = grow_at(m_end, 1);
        atraits::construct(get_alloc(), pos, std::forward<Args>(args)...);
        return *pos;
    }

    void pop_back()
    {
        shrink_at(m_end - 1, 1);
    }

    void resize(size_type n, const value_type& v)
    {
        reserve(n);

        auto new_end = m_begin + n;

        while (m_end > new_end)
        {
            atraits::destroy(get_alloc(), --m_end);
        }

        while (new_end > m_end)
        {
            atraits::construct(get_alloc(), m_end++, v);
        }
    }

    void resize(size_type n)
    {
        reserve(n);

        auto new_end = m_begin + n;

        while (m_end > new_end)
        {
            atraits::destroy(get_alloc(), --m_end);
        }

        while (new_end > m_end)
        {
            atraits::construct(get_alloc(), m_end++);
        }
    }

    bool is_static() const
    {
        return m_begin == static_begin_ptr();
    }

private:
    const T* static_begin_ptr() const
    {
        return reinterpret_cast<const_pointer>(m_static_data + 0);
    }

    T* static_begin_ptr()
    {
        return reinterpret_cast<pointer>(m_static_data + 0);
    }

    void destroy_all()
    {
        for (auto p = m_begin; p != m_end; ++p)
        {
            atraits::destroy(get_alloc(), p);
        }
    }

    void take_impl(small_vector& v)
    {
        if (v.is_static())
        {
            m_begin = m_end = static_begin_ptr();
            for (auto p = v.m_begin; p != v.m_end; ++p)
            {
                atraits::construct(get_alloc(), m_end, std::move(*p));
                ++m_end;
            }

            v.destroy_all();
        }
        else
        {
            m_begin = v.m_begin;
            m_end = v.m_end;
        }

        v.m_begin = v.m_end = v.static_begin_ptr();
        v.m_capacity = StaticCapacity;
    }

    // increase the size by splicing the elements in such a way that
    // a hole of uninitialized elements is left at position, with size num
    // returns the (potentially new) address of the hole
    T* grow_at(const T* cp, size_t num)
    {
        auto position = const_cast<T*>(cp);

        I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(position < m_begin || position > m_end);

        const auto s = size();
        const auto cdr = choose_data(s + num);

        if (cdr.ptr == m_begin)
        {
            // no special transfers needed

            m_end = m_begin + s + num;

            for (auto p = m_end - num - 1; p >= position; --p)
            {
                atraits::construct(get_alloc(), p + num, std::move(*p));
                atraits::destroy(get_alloc(), p);
            }

            return position;
        }
        else
        {
            // we need to transfer the elements into the new buffer

            position = cdr.ptr + (position - m_begin);

            auto p = m_begin;
            auto np = cdr.ptr;

            for (; np != position; ++p, ++np)
            {
                atraits::construct(get_alloc(), np, std::move(*p));
            }

            np += num; // hole
            for (; p != m_end; ++p, ++np)
            {
                atraits::construct(get_alloc(), np, std::move(*p));
            }

            // destroy old
            for (p = m_begin; p != m_end; ++p)
            {
                atraits::destroy(get_alloc(), p);
            }

            if (!is_static())
            {
                // we've moved from dyn memory, so deallocate the old one
                atraits::deallocate(get_alloc(), m_begin, m_capacity);
            }

            m_begin = cdr.ptr;
            m_end = m_begin + s + num;
            m_capacity = cdr.cap;

            return position;
        }
    }

    T* shrink_at(const T* cp, size_t num)
    {
        auto position = const_cast<T*>(cp);

        I_ITLIB_SMALL_VECTOR_OUT_OF_RANGE_IF(position < m_begin || position > m_end || position + num > m_end);

        const auto s = size();
        if (s - num == 0)
        {
            clear();
            return m_end;
        }

        const auto cdr = choose_data(s - num);

        if (cdr.ptr == m_begin)
        {
            // no special transfers needed

            for (auto p = position, np = position + num; np != m_end; ++p, ++np)
            {
                atraits::destroy(get_alloc(), p);
                atraits::construct(get_alloc(), p, std::move(*np));
            }

            for (auto p = m_end - num; p != m_end; ++p)
            {
                atraits::destroy(get_alloc(), p);
            }

            m_end -= num;
        }
        else
        {
            // we need to transfer the elements into the new buffer

            assert(cdr.ptr == static_begin_ptr()); // since we're shrinking that's the only way to have a new buffer

            auto p = m_begin, np = cdr.ptr;
            for (; p != position; ++p, ++np)
            {
                atraits::construct(get_alloc(), np, std::move(*p));
                atraits::destroy(get_alloc(), p);
            }

            for (; p != position + num; ++p)
            {
                atraits::destroy(get_alloc(), p);
            }

            for (; np != cdr.ptr + s - num; ++p, ++np)
            {
                atraits::construct(get_alloc(), np, std::move(*p));
                atraits::destroy(get_alloc(), p);
            }

            // we've moved from dyn memory, so deallocate the old one
            atraits::deallocate(get_alloc(), m_begin, m_capacity);

            position = cdr.ptr + (position - m_begin);
            m_begin = cdr.ptr;
            m_end = np;
            m_capacity = StaticCapacity;
        }

        return position;
    }

    void assign_fill(size_type count, const T& value)
    {
        const auto cdr = choose_data(count);

        m_end = cdr.ptr;
        for (size_t i=0; i<count; ++i)
        {
            atraits::construct(get_alloc(), m_end, value);
            ++m_end;
        }

        if (!is_static() && m_begin != cdr.ptr)
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }

        m_begin = cdr.ptr;
        m_capacity = cdr.cap;
    }

    template <class InputIterator>
    void assign_copy(InputIterator first, InputIterator last)
    {
        const auto cdr = choose_data(last - first);

        m_end = cdr.ptr;
        for (auto p = first; p != last; ++p)
        {
            atraits::construct(get_alloc(), m_end, *p);
            ++m_end;
        }

        if (!is_static() && m_begin != cdr.ptr)
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }

        m_begin = cdr.ptr;
        m_capacity = cdr.cap;
    }

    template <class InputIterator>
    void assign_move(InputIterator first, InputIterator last)
    {
        const auto cdr = choose_data(last - first);

        m_end = cdr.ptr;
        for (auto p = first; p != last; ++p)
        {
            atraits::construct(get_alloc(), m_end, std::move(*p));
            ++m_end;
        }

        if (!is_static() && m_begin != cdr.ptr)
        {
            atraits::deallocate(get_alloc(), m_begin, m_capacity);
        }

        m_begin = cdr.ptr;
        m_capacity = cdr.cap;
    }

    struct choose_data_result {
        T* ptr;
        size_t cap;
    };
    choose_data_result choose_data(size_t desired_capacity)
    {
        choose_data_result ret = {m_begin, m_capacity};

        if (!is_static())
        {
            // we're at the dyn buffer, so see if it needs resize or revert to static

            if (desired_capacity > m_capacity)
            {
                while (ret.cap < desired_capacity)
                {
                    // grow by roughly 1.5
                    ret.cap *= 3;
                    ++ret.cap;
                    ret.cap /= 2;
                }

                ret.ptr = atraits::allocate(get_alloc(), ret.cap);
            }
            else if (desired_capacity < RevertToStaticBelow)
            {
                // we're reverting to the static buffer
                ret.ptr = static_begin_ptr();
                ret.cap = StaticCapacity;
            }

            // else, do nothing
            // the capacity is enough and we don't revert to static
        }
        else if (desired_capacity > StaticCapacity)
        {
            // we must move to dyn memory
            // first move to dyn memory, use desired cap

            ret.cap = desired_capacity;
            ret.ptr = atraits::allocate(get_alloc(), ret.cap);
        }
        // else, do nothing
        // the capacity is and we're in the static buffer

        return ret;
    }

    allocator_type& get_alloc() { return *this; }
    const allocator_type& get_alloc() const { return *this; }

    pointer m_begin; // begin either points to m_static_data or to new memory
    pointer m_end;
    size_t m_capacity;
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_static_data[StaticCapacity];
};

template<typename T,
    size_t StaticCapacityA, size_t RevertToStaticBelowA, class AllocA,
    size_t StaticCapacityB, size_t RevertToStaticBelowB, class AllocB
>
bool operator==(const small_vector<T, StaticCapacityA, RevertToStaticBelowA, AllocA>& a,
    const small_vector<T, StaticCapacityB, RevertToStaticBelowB, AllocB>& b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (!(a[i] == b[i]))
            return false;
    }

    return true;
}

template<typename T,
    size_t StaticCapacityA, size_t RevertToStaticBelowA, class AllocA,
    size_t StaticCapacityB, size_t RevertToStaticBelowB, class AllocB
>
bool operator!=(const small_vector<T, StaticCapacityA, RevertToStaticBelowA, AllocA>& a,
    const small_vector<T, StaticCapacityB, RevertToStaticBelowB, AllocB>& b)

{
    return !operator==(a, b);
}

}
