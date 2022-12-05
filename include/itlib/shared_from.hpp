// itlib-shared_from v1.00
//
// A utility class to replace std::enable_shared_from_this providing move
// powerful features
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
//  1.00 (2022-12-05) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::enable_shared_from. You can use it in places
// where you would otherwise use std::enable_shared_from_this. It is similar to
// boost::enable_shared_from from Boost.SmartPtr.
//
// It provides derived classes with several protected methods
//
// From void:
// * shared_from_this(): returns std::shared_ptr<void>
// * weak_from_this(): returns std::weak_ptr<void>
// These return the managing pointer but type erased to void
//
// From pointer:
// * shared_from(T*): returns std::shared_ptr<T>
// * weak_from(T*): returns std::weak_ptr<T>
// There return pointers to T, with its lifetime bound to the control block of
// owning shared pointer.
// To achieve C++ standard like behavior, use: shared_from(this)
//
// C++11 example:
//
//
//
//                  TESTS
//
// You can find unit tests for shared_from in its official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <memory>

namespace itlib
{

class enable_shared_from : public std::enable_shared_from_this<enable_shared_from>
{
    using esd = std::enable_shared_from_this<enable_shared_from>;
protected:
    std::shared_ptr<void> shared_from_this() { return esd::shared_from_this(); }
    std::shared_ptr<const void> shared_from_this() const { return esd::shared_from_this(); }

    std::weak_ptr<void> weak_from_this()
    {
#if __cplusplus >= 201700
        return esd::weak_from_this();
#else
        return shared_from_this();
#endif;
    }

    std::weak_ptr<const void> weak_from_this() const
    {
#if __cplusplus >= 201700
        return esd::weak_from_this();
#else
        return shared_from_this();
#endif;
    }

    template <typename T>
    std::shared_ptr<T> shared_from(T* ptr) const {
        return std::shared_ptr<T>(shared_from_this(), ptr);
    }

    template <typename T>
    std::weak_ptr<T> weak_from(T* ptr) const {
        // c++ doesn't have an aliasing weak_ptr
        return shared_from(ptr);
    }
};

}
