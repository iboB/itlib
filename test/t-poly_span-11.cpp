#include "doctest.hpp"

#include <itlib/poly_span.hpp>
#include <itlib/make_ptr.hpp>

#include <memory>
#include <vector>

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