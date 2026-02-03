#include <itlib/ref_ptr.hpp>
#include <doctest/doctest.h>
#include <string>

using itlib::ref_ptr;
using itlib::make_ref_ptr;
using itlib::make_ref_ptr_from;

template <typename T>
void ensure_unique(ref_ptr<T>& p) {
    if (!p.unique()) {
        p = make_ref_ptr_from(*p);
    }
}

TEST_CASE("ref_ptr") {
    ref_ptr<int> p0;
    CHECK_FALSE(p0);
    CHECK(p0.use_count() == 0);
    CHECK_FALSE(p0.unique());

    auto p1 = make_ref_ptr<int>(42);
    CHECK(p1);
    CHECK(p1.use_count() == 1);
    CHECK(p1.unique());

    ref_ptr<const int> p2 = p1;
    CHECK(p2);
    CHECK(p2.use_count() == 2);
    CHECK_FALSE(p2.unique());
    CHECK(*p2 == 42);

    auto p3 = p1;

    CHECK(p3);
    CHECK(p3.use_count() == 3);
    CHECK_FALSE(p3.unique());

    CHECK(p3 == p1);
    CHECK(p2 == p1);

    CHECK(p1.use_count() == 3);

    ensure_unique(p1);
    CHECK(*p1 == 42);
    *p1 = 100;
    CHECK(p1.use_count() == 1);
    CHECK(p1.unique());

    auto pp1 = p1.get();

    ensure_unique(p1);
    CHECK(*p1 == 100);
    CHECK(p1.use_count() == 1);
    CHECK(p1.get() == pp1);

    CHECK(p1 != p2);

    CHECK(p2 == p3);
    CHECK(*p2 == 42);

    p2.reset();
    CHECK_FALSE(p2);
    CHECK(p3.unique());
    CHECK(*p3 == 42);

    p3 = nullptr;
    CHECK_FALSE(p3);
}

// other casts

struct animal {
    animal(std::string n) : name(std::move(n)) {}
    std::string name;
    virtual std::string speak() const = 0;
};

struct dog : public animal {
    using animal::animal;
    std::string speak() const override {
        return name + ": woof";
    }
};

TEST_CASE("poly") {
    auto d = itlib::make_ref_ptr_from(dog("Buddy"));
    CHECK(d->speak() == "Buddy: woof");
    auto d2 = d;
    CHECK(d2 == d);

    ref_ptr<animal> a = d;
    CHECK(a->speak() == "Buddy: woof");

    CHECK(a == d);
    CHECK(a >= d);
    CHECK_FALSE(a < d);
}

TEST_CASE("unsafe") {
    auto sp = std::make_shared<int>(55);
    auto rp = itlib::_ref_ptr_from_shared_ptr_unsafe(sp);
    CHECK(sp.get() == rp.get());
    CHECK_FALSE(rp.unique());
    auto sp2 = rp._as_shared_ptr_unsafe();
    CHECK(sp == sp2);
    CHECK(rp.use_count() == 3);
}