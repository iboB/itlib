#include "doctest.hpp"

#include <itlib/dynamic_bitset.hpp>

using db32 = itlib::dynamic_bitset<std::vector<uint32_t>>;

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

    const auto cx = x;
    auto cit = cx.begin();
    for (int i=0; i<130; ++i, ++cit);
    CHECK(*cit);
    CHECK(cx[130]);
}
