#include "doctest.hpp"

#include <itlib/dynamic_bitset.hpp>
using vec32 = std::vector<uint32_t>;
using db32 = itlib::dynamic_bitset<vec32>;

TEST_CASE("statics")
{
    CHECK(db32::word_size(3) == 1);
    CHECK(db32::word_size(32) == 1);
    CHECK(db32::word_size(63) == 2);
    CHECK(db32::word_size(64) == 2);
    CHECK(db32::word_index(3) == 0);
    CHECK(db32::word_index(32) == 1);
    CHECK(db32::word_index(63) == 1);
    CHECK(db32::word_index(64) == 2);
    CHECK(db32::bit_index(3) == 3);
    CHECK(db32::bit_index(32) == 0);
    CHECK(db32::bit_index(63) == 31);
    CHECK(db32::bit_index(64) == 0);
    CHECK(db32::word_mask(3) == 0x8);
    CHECK(db32::word_mask(32) == 1);
    CHECK(db32::word_mask(63) == 0x80000000u);
    CHECK(db32::word_mask(64) == 1);
}

TEST_CASE("test")
{
    db32 a;
    CHECK(a.size() == 0);
    CHECK(a.word_size() == 0);
    CHECK(a.empty());
    CHECK(!a.all());
    CHECK(!a.any());
    CHECK(a.none());

    db32 b(123, 0);
    CHECK(b.size() == 123);
    CHECK(b.word_size() == 4);
    CHECK(b.byte_size() == 16);
    CHECK(!b.empty());
    CHECK(b.none());
    auto data = b.data();

    CHECK(!b.test(77));
    b.set(77);
    CHECK(b.test(77));
    b.reset(77);
    CHECK(!b.test(77));
    b.flip(77);
    CHECK(b.test(77));
    b.set(77, false);
    CHECK(!b.test(77));
    b.set(77, true);
    CHECK(b.test(77));

    auto c = b;
    CHECK(c.size() == 123);
    CHECK(c.word_size() == 4);
    CHECK(!c.empty());
    CHECK(c.test(77));
    CHECK(!c.all());
    CHECK(c.any());
    CHECK(!c.none());

    auto d = std::move(b);
    CHECK(b.empty());
    CHECK(b.size() == 0);
    CHECK(d.size() == 123);
    CHECK(d.data() == data);
    CHECK(d.test(77));

    b = d;
    CHECK(b.size() == 123);
    CHECK(b.word_size() == 4);
    CHECK(!b.empty());
    CHECK(b.test(77));

    c = std::move(d);
    CHECK(d.empty());
    CHECK(d.size() == 0);
    CHECK(c.size() == 123);
    CHECK(c.data() == data);
    CHECK(c.test(77));
    CHECK(c.any());

    for (size_t i=0; i<c.size(); ++i)
    {
        c[i] = true;
    }
    CHECK(c.all());

    for (auto e : c)
    {
        e.flip();
    }
    CHECK(c.none());

    c.push_back(1);
    CHECK(c.any());
    c.push_back(1);
    c.push_back(1);
    c.push_back(1);
    c.push_back(1);
    CHECK(c.size() == 128);
    CHECK(c.word_size() == 4);
    CHECK(c.byte_size() == 16);
    CHECK(c.any());
    CHECK(!c.test(0));
    CHECK(!c.test(55));
    CHECK(c.test(124));
    CHECK(c.test(127));

    c.push_back(1);
    CHECK(c.size() == 129);
    CHECK(c.word_size() == 5);
    CHECK(c.byte_size() == 17);
    CHECK(c.any());
    CHECK(!c.test(0));
    CHECK(!c.test(55));
    CHECK(c.test(124));
    CHECK(c.test(127));
    CHECK(c.test(128));

    const auto cc = c;
    CHECK(!cc[0]);
    CHECK(!cc[55]);
    CHECK(cc[124]);
    CHECK(cc[127]);
    CHECK(cc[128]);

    int sum = 0;
    for (auto e : cc)
    {
        sum += e;
    }
    CHECK(sum == 6);

    db32 f(2 * db32::bits_per_word + db32::bits_per_word / 3 , unsigned(-1));
    CHECK(f.all());

    f.resize(2 * db32::bits_per_word);
    CHECK(f.all());

    f.resize(0);
    CHECK_FALSE(f.all());
}

TEST_CASE("set/get")
{
    db32 b(2);
    b.set(0);
    b.set(1);
    CHECK(b.test(0));
    CHECK(b.test(1));
    CHECK(b.buffer().size() == 1);
    CHECK(b.buffer().front() == 3);

    for (int i = 0; i < 29; ++i)
    {
        b.push_back(true);
        CHECK(b.size() == i + 3);
        CHECK(b.all());
        CHECK(b.buffer().size() == 1);
        CHECK(b.buffer().front() == (1u << b.size()) - 1);
    }

    b.push_back(true);
    CHECK(b.buffer().size() == 1);
    CHECK(b.buffer().front() == 0xffffffff);
    CHECK(b.all());

    b.push_back(true);
    CHECK(b.buffer().size() == 2);
    CHECK(b.buffer().front() == 0xffffffff);
    CHECK(b.buffer().back() == 1);
    CHECK(b.all());
}

template <typename Bitset>
bool advance_check(size_t n)
{
    Bitset a(n);
    auto iter = a.begin();
    for (size_t i=0; i<n; ++i)
    {
        ++iter;
    }
    return iter == a.end();
}

template <typename Bitset>
bool advance_check_r(size_t n)
{
    Bitset a(n);
    auto iter = a.end();
    for (size_t i=0; i<n; ++i)
    {
        --iter;
    }
    return iter == a.begin();
}

TEST_CASE("iterators")
{
    CHECK(advance_check<db32>(0));
    CHECK(advance_check<db32>(30));
    CHECK(advance_check<db32>(31));
    CHECK(advance_check<db32>(32));
    CHECK(advance_check<db32>(33));
    CHECK(advance_check<db32>(64));
    CHECK(advance_check<db32>(512));
    CHECK(advance_check<db32>(513));
    CHECK(advance_check<const db32>(0));
    CHECK(advance_check<const db32>(30));
    CHECK(advance_check<const db32>(31));
    CHECK(advance_check<const db32>(32));
    CHECK(advance_check<const db32>(33));
    CHECK(advance_check<const db32>(64));
    CHECK(advance_check<const db32>(512));
    CHECK(advance_check<const db32>(513));

    CHECK(advance_check_r<db32>(0));
    CHECK(advance_check_r<db32>(30));
    CHECK(advance_check_r<db32>(31));
    CHECK(advance_check_r<db32>(32));
    CHECK(advance_check_r<db32>(33));
    CHECK(advance_check_r<db32>(64));
    CHECK(advance_check_r<db32>(512));
    CHECK(advance_check_r<db32>(513));
    CHECK(advance_check_r<const db32>(0));
    CHECK(advance_check_r<const db32>(30));
    CHECK(advance_check_r<const db32>(31));
    CHECK(advance_check_r<const db32>(32));
    CHECK(advance_check_r<const db32>(33));
    CHECK(advance_check_r<const db32>(64));
    CHECK(advance_check_r<const db32>(512));
    CHECK(advance_check_r<const db32>(513));

    db32 x(150);
    auto it = x.begin();
    for (int i=0; i<130; ++i, ++it);
    *it = true;
    CHECK(*it);
    CHECK(x[130]);
    auto it2 = it + 20;
    CHECK(it2 == x.end());
    CHECK(x.end() - it == 20);

    const auto cx = x;
    auto cit = cx.begin();
    for (int i=0; i<130; ++i, ++cit);
    CHECK(*cit);
    CHECK(cx[130]);
    auto cit2 = cit + 20;
    CHECK(cit2 == cx.end());
    CHECK(cx.end() - cit == 20);
}

#define PART_A 0xbaadf00d, 0xfeee1234, 0x43210523, 0xfaadbeed
#define PART_B 0x78901234, 0x777cdcdc

TEST_CASE("asign")
{
    vec32 ia = {PART_A};
    db32 b;
    b.assign(ia);
    CHECK(b.size() == ia.size() * 32);
    CHECK(b.buffer() == ia);
}

TEST_CASE("resize")
{
    vec32 ia = {0xfaadbeed};
    db32 b;
    b.assign(ia);

    b.resize(b.size() - 1);
    CHECK(b.buffer() == ia);

    b.resize(b.size() - 15);
    CHECK(b.buffer().back() == 0x7aadbeed);

    b.push_back(false);
    CHECK(b.buffer().back() == 0xbeed);
}

TEST_CASE("append lucky")
{
    vec32 ia = {PART_A};
    vec32 ib = {PART_B};

    db32 b1;
    b1.assign(ia);

    db32 b1c;
    b1c.append(b1);
    CHECK(b1c.size() == b1.size());
    CHECK(b1c.buffer() == b1.buffer());

    db32 b2;
    b1.append(b2);
    CHECK(b1c.size() == b1.size());
    CHECK(b1c.buffer() == b1.buffer());

    b2.assign(ib);

    b1.append(b2);
    CHECK(b1.size() == (ia.size() + ib.size()) * 32);
    CHECK(b1.buffer() == vec32{PART_A, PART_B});

    b2.resize(b2.size() - 11);
    b1.append(b2);
    CHECK(b1.size() == (ia.size() + 2*ib.size()) * 32 - 11);
    CHECK(b1.buffer() == vec32{PART_A, PART_B, PART_B});
}

TEST_CASE("append actually")
{
    db32 b1;
    b1.push_back(true);
    db32 b2;
    b2.push_back(true);

    b1.append(b2);
    CHECK(b1.size() == 2);
    CHECK(b1.buffer().size() == 1);
    CHECK(b1.buffer().front() == 3);

    b1.append(b2);
    CHECK(b1.size() == 3);
    CHECK(b1.buffer().size() == 1);
    CHECK(b1.buffer().front() == 7);

    vec32 ia = {PART_A};
    b1.assign(ia);
    b1.resize(b1.size() - 16);

    vec32 ib = {PART_B};
    b2.assign(ib);
    b1.append(b2);
    CHECK(b1.size() == (ia.size() + ib.size()) * 32 - 16);

    {
        auto& buf = b1.buffer();
        CHECK(buf[0] == ia[0]);
        CHECK(buf[1] == ia[1]);
        CHECK(buf[2] == ia[2]);
        // 0xfaadbeed
        // 0x78901234, 0x777cdcdc
        CHECK(buf[3] == 0x1234beed);
        CHECK(buf[4] == 0xdcdc7890);
        CHECK(buf[5] == 0x0000777c);
    }

    b1.assign(ia);
    b1.resize(b1.size() - 8);
    b1.append(b2);

    {
        auto& buf = b1.buffer();
        CHECK(buf[0] == ia[0]);
        CHECK(buf[1] == ia[1]);
        CHECK(buf[2] == ia[2]);
        // 0xfaadbeed
        // 0x78901234, 0x777cdcdc
        CHECK(buf[3] == 0x34adbeed);
        CHECK(buf[4] == 0xdc789012);
        CHECK(buf[5] == 0x00777cdc);
    }
}