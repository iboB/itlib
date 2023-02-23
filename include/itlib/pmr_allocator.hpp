// itlib-pmr_allocator v1.01
//
// A C++17 implementation of the C++20 additions to std::pmr::allocator
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
//  1.01 (2023-02-23) Proper constructors
//  1.00 (2023-02-09) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// The library defines the template type pmr_allocator<T> which is
// a C++17 implementation of the C++20 additions to std::pmr::allocator
//
// The type inherits from std::pmr_allocator and can be provided to functions
// which require the latter via slicing
//
//                  TESTS
//
// You can find unit tests for qalgorithm in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#include <memory_resource>
#include <cstddef>

namespace itlib {

template <typename T = std::byte>
class pmr_allocator : public std::pmr::polymorphic_allocator<T> {
public:
    using super = std::pmr::polymorphic_allocator<T>;
    using value_type = typename super::value_type;

    using super::polymorphic_allocator;

    // leave this to the standard implementation if available
#if __cplusplus < 202000L
    using super::resource;

    [[nodiscard]] void* allocate_bytes(std::size_t n, std::size_t a = alignof(std::max_align_t)) {
        return resource()->allocate(n, a);
    }

    void deallocate_bytes(void* p, std::size_t n, std::size_t a = alignof(std::max_align_t)) {
        resource()->deallocate(p, n, a);
    }

    template <class U>
    [[nodiscard]] U* allocate_object(std::size_t n = 1) {
        return static_cast<U*>(allocate_bytes(n * sizeof(U), alignof(U)));
    }

    template <class U>
    void deallocate_object(U* p, std::size_t n = 1) {
        deallocate_bytes(p, n * sizeof(U), alignof(U));
    }

    template <typename U, typename... Args>
    [[nodiscard]] U* new_object(Args&&... args) {
        U* p = allocate_object<U>();
        try {
            this->construct(p, std::forward<Args>(args)...);
            return p;
        }
        catch (...) {
            deallocate_object(p);
            throw;
        }
    }

    template <typename U>
    void delete_object(U* p) {
        p->~U();
        deallocate_object(p);
    }
#endif

private:
    using super::destroy;
};

}
