// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "t-atomic_shared_ptr_storage.inl"

#if defined(__cpp_lib_atomic_shared_ptr)
TEST_CASE("std::atomic<std::shared_ptr> is not lockfree") {
    // If somehow a lockfree implementation of a std::atomic<std::shared_ptr>
    // our lockful atomic_shared_ptr_storage will likely be performance hit
    // compared to it.
    // In such a case, this test will fail and we can inspect the lockfree
    // implementation.
    using iptr = std::atomic<std::shared_ptr<int>>;
    static_assert(!iptr::is_always_lock_free);
    iptr ptr;
    CHECK_FALSE(ptr.is_lock_free());
}
#endif
