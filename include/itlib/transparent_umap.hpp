// itlib-transparent_umap v1.00
//
// A a really transparent unordered map which includes the C++26 features
// they forgot to add in C++20.
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2024 Borislav Stanimirov
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
//  1.00 (2024-xx-xx) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::transparent_umap which is is essentially a
// std::unordered_map which includes the C++26 features they forgot to add in
// C++20. Namely tansparent-aware overloads of:
//
// * try_emplace()
// * operator[]
// * at()
//
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <unordered_map>
#include <type_traits>
#include <stdexcept>

namespace itlib {

namespace tumimpl {
template <typename T, typename = void>
constexpr bool is_transparent = false;
template <class T>
constexpr bool is_transparent<T, std::void_t<typename T::is_transparent>> = true;
} // namespace tumimpl

// yes... a macro
// C++ still lacks the ability to
// #define I_ITLIB_EIT(T)

template <
    typename Key,
    typename T,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, T>>
> class transparent_umap : public std::unordered_map<Key, T, Hash, KeyEqual, Alloc> {
public:
    using base = std::unordered_map<Key, T, Hash, KeyEqual, Alloc>;
    using base::unordered_map;

    using base::iterator;
    using base::const_iterator;

    // enable if transparent
    template <typename K, typename R>
    using enable_if_tr = std::enable_if_t<tumimpl::is_transparent<K>, R>;

    // enable if transparent and convertible
    template <typename K, typename R>
    using enable_if_tr_c = std::enable_if_t<
        tumimpl::is_transparent<K>
        && std::is_convertible_v<Key, K>
        // but not iterators
        && !std::is_convertible_v<K, iterator>
        && !std::is_convertible_v<K, const_iterator>
        , R
    >;

    using base::try_emplace;

    template <typename K, typename... Args>
    enable_if_tr_c<K, std::pair<iterator, bool>> try_emplace(K&& key, Args&&... args) {
        auto f = this->find(key);
        if (f != this->end()) return { f, false };
        return this->emplace(std::forward<K>(key), T(std::forward<Args>(args)...));
    }

    template <typename K, typename... Args>
    enable_if_tr<K, std::pair<iterator, bool>> try_emplace(const_iterator hint, K&& key, Args&&... args) {
        auto f = this->find(key);
        if (f != this->end()) return { f, false };
        return this->emplace_hint(hint, std::forward<K>(key), T(std::forward<Args>(args)...));
    }

    using base::operator[];

    template <typename K>
    enable_if_tr_c<K, T&> operator[](K&& key) {
        return this->try_emplace(std::forward<K>(key)).first->second;
    }

    using base::at;

    template <typename K>
    enable_if_tr<K, T&> at(const K& key) {
        auto f = this->find(key);
        if (f == this->end()) throw std::out_of_range("transparent_umap::at");
        return f->second;
    }

    template <typename K>
    enable_if_tr<K, const T&>  at(const K& key) const {
        auto f = this->find(key);
        if (f == this->end()) throw std::out_of_range("transparent_umap::at");
        return f->second;
    }
};


} // namespace itlib