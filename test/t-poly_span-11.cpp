#include <doctest/doctest.h>

#include <itlib/poly_span.hpp>
#include <itlib/make_ptr.hpp>

#include <memory>
#include <vector>
#include <algorithm>

TEST_CASE("[poly_span] empty")
{
    using namespace itlib;

    {
        poly_span<int> e;
        CHECK(!e);
        CHECK(e.size() == 0);
        CHECK(e.begin() == e.end());
        CHECK(e.cbegin() == e.cend());
        CHECK(e.rbegin() == e.rend());
        CHECK(e.empty());
        CHECK(e.stride() != 0);
        CHECK(e.data() == nullptr);
    }
}

struct selectable
{
    int a;
    int b;
    bool use_a;
};

TEST_CASE("[poly_span] field")
{
    using namespace itlib;

    std::vector<selectable> v = {{1, 2, true}, {3, 4, false}, {5, 6, false}, {7, 8, true}};

    poly_span<int&> ss(v.data(), v.size(), [](selectable& s) -> int& {
        if (s.use_a) return s.a;
        return s.b;
    });
    REQUIRE(ss.size() == 4);

    CHECK(ss);
    CHECK(!ss.empty());
    CHECK(ss.size() == 4);
    CHECK(ss.begin() + 4 == ss.end());

    CHECK(*ss.rbegin() == 7);

    CHECK(ss.front() == 1);
    CHECK(ss.back() == 7);

    CHECK(ss[0] == 1);
    CHECK(ss[1] == 4);
    CHECK(ss[2] == 6);
    CHECK(ss[3] == 7);

    for (auto& i : ss) i = 8;

    CHECK(v[0].a == 8);
    CHECK(v[1].b == 8);
    CHECK(v[2].b == 8);
    CHECK(v[3].a == 8);
}

TEST_CASE("[stride_span] algorithm") {
    std::vector<selectable> v = {{1, 20, true}, {40, 3, false}, {80, 5, false}, {7, 80, true}};

    itlib::poly_span<int&> odds(v.data(), v.size(), [](selectable& s) -> int& {
        if (s.use_a) return s.a;
        return s.b;
    });

    CHECK(odds.front() == 1);
    CHECK(odds[1] == 3);
    CHECK(odds.back() == 7);
    CHECK(std::all_of(odds.begin(), odds.end(), [](int i) -> bool { return i % 2 == 1; }));
    auto f = std::find(odds.begin(), odds.end(), 20);
    CHECK(f == odds.end());
    f = std::find(odds.begin(), odds.end(), 3);
    CHECK(f == odds.begin() + 1);

    std::vector<int> codds = {1, 3, 5, 7};
    CHECK(std::equal(odds.begin(), odds.end(), codds.begin()));

    std::vector<int> clone;
    clone.assign(odds.begin(), odds.end());
    CHECK(codds == clone);
}

struct shape
{
    int form;
    shape(int s) : form(s) {}
    virtual ~shape() = default;
    virtual int area() const = 0;
};

struct drawable
{
    int visage;
    drawable(int d) : visage(d) {}
    virtual ~drawable() = default;
    virtual int draw() const = 0;
};

struct sprite : drawable, shape
{
    int sdata;
    sprite(int d, int s, int m) : drawable(d), shape(s), sdata(m) {}
    int area() const override { return sdata + form; }
    int draw() const override { return sdata + visage; }
};

TEST_CASE("[poly_span] hierarchy")
{
    using namespace itlib;

    std::vector<std::unique_ptr<sprite>> sprites;
    sprites.push_back(itlib::make_unique(sprite(100, 10, 1)));
    sprites.push_back(itlib::make_unique(sprite(200, 20, 2)));
    sprites.push_back(itlib::make_unique(sprite(300, 30, 3)));

    poly_span<drawable*> ds(sprites.data(), sprites.size(), [](std::unique_ptr<sprite>& ptr) -> drawable* {
        return ptr.get();
    });
    int visage_sum = 0, draw_sum = 0;
    for (auto d : ds)
    {
        visage_sum += d->visage;
        draw_sum += d->draw();
    }
    CHECK(visage_sum == 600);
    CHECK(draw_sum == 606);

    poly_span<shape*> ss(sprites.data(), sprites.size(), [](std::unique_ptr<sprite>& ptr) -> shape* {
        return ptr.get();
    });

    int form_sum = 0, area_sum = 0;
    for (auto s : ss)
    {
        form_sum += s->form;
        area_sum += s->area();
    }
    CHECK(form_sum == 60);
    CHECK(area_sum == 66);
}

TEST_CASE("[poly_span] slicing")
{
    using namespace itlib;

    std::vector<selectable> v = {{6, 1, true}, {1, 7, false}, {1, 8, false}, {9, 1, true}, {10, 1, true}};

    poly_span<int&> span(v.data(), v.size(), [](selectable& s) -> int& {
        if (s.use_a) return s.a;
        return s.b;
    });

    {
        auto s = span.subspan(10);
        CHECK(!!s);
        CHECK(s.empty());
        CHECK(s.cbegin() == span.end());
    }
    {
        auto s = span.subspan(1);
        CHECK(s.size() == 4);
        CHECK(s.cbegin() == span.begin() + 1);
    }
    {
        auto s = span.subspan(3, 1);
        CHECK(s.size() == 1);
        CHECK(s.cbegin() == span.begin() + 3);
    }
    {
        auto s = span.first(3);
        CHECK(s.size() == 3);
        CHECK(s.cbegin() == span.begin());
    }
    {
        auto s = span.last(2);
        CHECK(s.size() == 2);
        CHECK(s.cbegin() == span.begin() + 3);
    }
    {
        auto cp = span;
        cp.remove_prefix(2);
        CHECK(cp.size() == 3);
        CHECK(cp.cbegin() == span.begin() + 2);
        cp.remove_suffix(2);
        CHECK(cp.size() == 1);
        CHECK(cp.cend() == span.begin() + 3);
    }
}