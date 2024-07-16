// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/shared_from.hpp>

class session : public itlib::enable_shared_from {
public:
    int id = 10;

    using itlib::enable_shared_from::weak_from;
    using itlib::enable_shared_from::shared_from;

    std::shared_ptr<session> clone() {
        return shared_from(this);
    }
};

template <typename T, typename U>
bool same_ptr(const std::shared_ptr<T>& a, const std::shared_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

template <typename T, typename U>
bool same_ptr(const std::shared_ptr<T>& a, const std::weak_ptr<U>& b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

TEST_CASE("shared_from") {
    auto ptr = std::make_shared<session>();

    CHECK(ptr == ptr->clone());

    {
        auto idptr = ptr->shared_from(&ptr->id);
        CHECK(same_ptr(ptr, idptr));
        CHECK(*idptr == 10);
        CHECK(idptr.use_count() == 2);
    }

    {
        auto idptr = ptr->weak_from(&ptr->id);
        CHECK(idptr.use_count() == 1);
        CHECK(*idptr.lock() == 10);
        ptr.reset();
        CHECK(idptr.expired());
    }
}
