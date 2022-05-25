#include "doctest.hpp"

#include <itlib/pod_vector.hpp>

int32_t mallocs, frees, reallocs;

constexpr size_t wasteful_copy_size = 32;

struct counting_allocator
{
    itlib::impl::pod_allocator a;

    using size_type = size_t;
    void* malloc(size_type size)
    {
        ++mallocs;
        return a.malloc(size);
    }

    void* realloc(void* old, size_type new_size)
    {
        ++mallocs;
        if (old) ++frees;
        ++reallocs;
        return a.realloc(old, new_size);
    }

    void free(void* mem)
    {
        if (mem) ++frees;
        a.free(mem);
    }
    constexpr size_type max_size() const
    {
        return ~size_type(0);
    }
    constexpr bool zero_fill_new() const
    {
        return true;
    }
    constexpr size_type realloc_wasteful_copy_size() const
    {
        return wasteful_copy_size;
    }

    constexpr size_type alloc_align() const
    {
        return alignof(max_align_t);
    }
};

template <typename T>
using cpodvec = itlib::pod_vector<T, counting_allocator>;

TEST_CASE("basic")
{
    {
        cpodvec<int32_t> ivec;
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 0);
        CHECK(!ivec.begin());
        CHECK(ivec.begin() == ivec.end());
        CHECK(!ivec.cbegin());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());

        auto d = ivec.data();
        ivec.reserve(2);
        CHECK(ivec.capacity() == 2);
        CHECK(d != ivec.data());
        CHECK(ivec.size() == 0);
        CHECK(mallocs == 1);

        ivec.resize(3, 8);
        CHECK(ivec.capacity() == 3);
        CHECK(ivec.size() == 3);
        CHECK(ivec.front() == 8);
        CHECK(ivec.back() == 8);
        CHECK(mallocs == 2);

        ivec.clear();
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 3);
        CHECK(ivec.begin() == ivec.end());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());

        ivec.shrink_to_fit();
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 0);
        CHECK(!ivec.begin());
        CHECK(ivec.begin() == ivec.end());
        CHECK(ivec.empty());

        ivec.push_back(5);
        CHECK(ivec.size() == 1);
        CHECK(ivec[0] == 5);
        auto it = ivec.begin();
        CHECK(it == ivec.data());
        CHECK(it == ivec.cbegin());
        CHECK(*it == 5);
        ++it;
        CHECK(it == ivec.end());
        CHECK(it == ivec.cend());

        auto& ee = ivec.emplace_back(3);
        CHECK(ee == 3);
        CHECK(&ee == &ivec.back());
        CHECK(ivec.size() == 2);
        auto rit = ivec.rbegin();
        CHECK(*rit == 3);
        ++rit;
        *rit = 12;
        ++rit;
        CHECK(rit == ivec.rend());
        CHECK(rit == ivec.crend());
        CHECK(ivec.front() == 12);
        CHECK(ivec.back() == 3);

        auto iret = ivec.insert(ivec.begin(), 53);
        CHECK(ivec.size() == 3);
        CHECK(ivec.capacity() == 3);
        CHECK(iret == ivec.begin());

        iret = ivec.insert(ivec.begin() + 2, 90);
        CHECK(iret == ivec.begin() + 2);
        iret = ivec.insert(ivec.begin() + 4, 17);
        CHECK(iret == ivec.begin() + 4);
        iret = ivec.insert(ivec.end(), 6);
        CHECK(iret == ivec.end() - 1);
        iret = ivec.insert(ivec.begin(), { 1, 2 });
        CHECK(iret == ivec.begin());

        int32_t ints[] = { 1, 2, 53, 12, 90, 3, 17, 6 };
        CHECK(ivec.capacity() >= 8);
        CHECK(ivec.size() == 8);
        CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

        ivec.pop_back();
        CHECK(ivec.size() == 7);
        CHECK(memcmp(ivec.data(), ints, sizeof(ints) - sizeof(int32_t)) == 0);

        CHECK(ivec.capacity() > 7);
        ivec.shrink_to_fit();
        CHECK(ivec.capacity() == 7);

        ivec.resize(8);
        CHECK(ivec.size() == 8);
        ints[7] = 0;
        CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

        const cpodvec<int32_t> ivec2 = { 1, 2, 3, 4 };
        CHECK(ivec2.size() == 4);
        CHECK(*ivec2.begin() == 1);
        CHECK(ivec2[1] == 2);
        CHECK(ivec2.at(2) == 3);
        CHECK(*ivec2.rbegin() == 4);

        auto eret = ivec.erase(ivec.begin());
        CHECK(ivec.size() == 7);
        CHECK(ivec.front() == 2);
        CHECK(memcmp(ivec.data(), ints + 1, ivec.size() * sizeof(int32_t)) == 0);
        CHECK(eret == ivec.begin());

        eret = ivec.erase(ivec.begin() + 2, ivec.begin() + 4);
        CHECK(ivec.size() == 5);
        CHECK(ivec[3] == 17);
        CHECK(eret == ivec.begin() + 2);

        // empty erase
        eret = ivec.erase(ivec.begin() + 1, ivec.begin() + 1);
        CHECK(ivec.size() == 5);
        CHECK(eret == ivec.begin() + 1);

        cpodvec<wchar_t> svec;
        svec.assign({ L's', L'f' });
        CHECK(svec.size() == 2);
        std::wstring s1 = L"the quick brown fox jumped over the lazy dog 1234567890";
        auto& esret = svec.emplace_back(s1[5]);
        CHECK(svec.back() == s1[5]);
        CHECK((&esret == &svec.back()));

        auto svec1 = svec;
        CHECK(svec1 == svec);

        auto cstr = svec.back();
        auto svec2 = std::move(svec);
        CHECK(svec2.size() == 3);
        CHECK(svec2.back() == s1[5]);

        CHECK(svec.empty());
        CHECK(svec2.back() == cstr);

        svec = std::move(svec2);
        CHECK(svec2.empty());
        CHECK(svec.back() == cstr);

        svec2 = svec;
        CHECK(svec2.back() == s1[5]);
        CHECK(svec.back() == s1[5]);
        CHECK(svec == svec2);

        auto isret = svec.insert(svec.begin(), cstr);
        CHECK(svec.size() == 4);
        CHECK(svec.back() == cstr);
        CHECK(svec.front() == svec.back());
        CHECK((isret == svec.begin()));

        isret = svec.emplace(svec.begin() + 2, std::move(s1[5]));
        CHECK(svec.size() == 5);
        CHECK(svec.front() == svec[2]);
        CHECK(svec[2] == s1[5]);
        CHECK((isret == svec.begin() + 2));

        svec.clear();
        CHECK(svec.empty());
        svec2.clear();
        CHECK(svec2.empty());
        CHECK(svec == svec2);

        svec.resize(svec.capacity());
        CHECK(svec.size() == svec.capacity());

        for (auto& s : svec) {
            CHECK(s == 0);
        }

        s1 = L"asdf";
        cpodvec<wchar_t> cvec(s1.begin(), s1.end());
        CHECK(cvec.size() == 4);
        CHECK(cvec.front() == L'a');
        CHECK(cvec.back() == L'f');

        cvec.clear();
        CHECK(cvec.size() == 0);
        CHECK(cvec.empty());

        s1 = L"baz";
        cvec.assign(s1.begin(), s1.end());
        CHECK(cvec.size() == 3);
        CHECK(cvec.front() == L'b');
        CHECK(cvec.back() == L'z');

        cvec.resize(1);
        CHECK(cvec.size() == 1);
        CHECK(cvec.front() == L'b');
        CHECK(cvec.back() == L'b');

        // 0 is implicitly castable to nullptr_t which can be an iterator in our case
        cpodvec<int32_t> nullptr_test(2, 0);
        CHECK(nullptr_test.size() == 2);
        CHECK(nullptr_test.front() == 0);
        CHECK(nullptr_test.back() == 0);

        nullptr_test.assign(3, 0);
        CHECK(nullptr_test.size() == 3);
        CHECK(nullptr_test.front() == 0);
        CHECK(nullptr_test.back() == 0);

        nullptr_test.insert(nullptr_test.begin(), 1, 0);
        CHECK(nullptr_test.size() == 4);
        CHECK(nullptr_test.front() == 0);
    }

    CHECK(mallocs == frees);
    mallocs = frees = reallocs = 0;
}

TEST_CASE("compare")
{
    {
        cpodvec<int32_t> a;
        CHECK(a == a);
        cpodvec<int32_t> b;
        CHECK(a == b);

        b.push_back(1);
        CHECK(a != b);
        CHECK(b != a);

        a.push_back(1);
        CHECK(a == a);
        CHECK(a == b);

        for (int32_t i = 0; i < 23; ++i) {
            a.push_back(i);
            b.push_back(i);
        }
        CHECK(a == b);

        b.back() = 1232;
        CHECK(a != b);
    }
    CHECK(mallocs == frees);
    mallocs = frees = reallocs = 0;
}

TEST_CASE("swap")
{
    using namespace itlib;
    {
        cpodvec<int32_t>
            a = { 1, 2, 3 },
            b = { 5, 6 };
        CHECK(a.size() == 3);
        CHECK(b.size() == 2);

        CHECK(mallocs == 2);
        a.swap(b);
        CHECK(mallocs == 2);

        CHECK(a.size() == 2);
        CHECK(a.front() == 5);
        CHECK(b.size() == 3);
        CHECK(b.front() == 1);

        std::swap(a, b);

        CHECK(mallocs == 2);
        CHECK(b.size() == 2);
        CHECK(b.front() == 5);
        CHECK(a.size() == 3);
        CHECK(a.front() == 1);
    }

    CHECK(mallocs == frees);
    mallocs = frees = reallocs = 0;
}

TEST_CASE("empty")
{
    {
        cpodvec<int32_t> foo = {1, 2, 3, 4};
        foo = cpodvec<int32_t>();
        CHECK(foo.empty());
        CHECK(foo.capacity() == 0);
        CHECK(foo.data() == nullptr);

        foo = {1, 2, 3, 4};
        cpodvec<int32_t> empty;

        foo = empty;
        CHECK(foo.empty());
        CHECK(foo.capacity() == 4);
        CHECK(foo.data() != nullptr);

        foo.shrink_to_fit();
        CHECK(foo.capacity() == 0);
        CHECK(foo.data() == nullptr);
    }

    CHECK(mallocs == frees);
    mallocs = frees = reallocs = 0;
}

struct three { uint8_t x, y, z; };
struct two { uint16_t a; };

TEST_CASE("reallocs reserve")
{
    {
        cpodvec<uint8_t> bv(wasteful_copy_size - 5);
        CHECK(bv.capacity() == wasteful_copy_size - 5);
        bv.clear();
        CHECK(bv.capacity() == wasteful_copy_size - 5);

        reallocs = 0;

        bv.reserve(wasteful_copy_size);

        CHECK(reallocs == 1);

        bv.reserve(bv.capacity() + wasteful_copy_size + 1);

        CHECK(reallocs == 1);
    }

    mallocs = frees = reallocs = 0;
}

TEST_CASE("reallocs insert")
{
    {
        cpodvec<uint8_t> bv(5);
        cpodvec<uint8_t> bv2(wasteful_copy_size, 10);

        reallocs = 0;
        bv.insert(bv.begin(), bv2.begin(), bv2.end());
        CHECK(reallocs == 1);
    }

    {
        cpodvec<uint8_t> bv(50);
        cpodvec<uint8_t> bv2(wasteful_copy_size + 1, 10);

        reallocs = 0;
        bv.insert(bv.begin(), bv2.begin(), bv2.end());
        CHECK(reallocs == 0);
    }

    CHECK(mallocs == frees);
    mallocs = frees = reallocs = 0;
}

TEST_CASE("recast")
{
    {
        cpodvec<int32_t> ivec = { 1, 2, 3, 4, 5, 6, 7, 8 };
        CHECK(ivec.size() == 8);
        CHECK(ivec.size() == ivec.capacity());

        cpodvec<uint8_t> itocvec;
        itocvec.recast_copy_from(ivec);
        CHECK(itocvec.size() == ivec.byte_size());
        for (auto i = 0u; i < ivec.size(); ++i) {
            CHECK((reinterpret_cast<int32_t*>(itocvec.data()))[0] == ivec[0]);
        }

        CHECK(itocvec.capacity() >= ivec.byte_size());

        cpodvec<int32_t> ctoivec;
        ctoivec.recast_copy_from(itocvec);
        CHECK(ctoivec == ivec);

        cpodvec<uint8_t> itocvec2;
        itocvec2.recast_take_from(std::move(ctoivec));
        CHECK(itocvec2 == itocvec);
        CHECK(itocvec2.capacity() >= sizeof(ivec[0]) * ivec.size());
        CHECK(ctoivec.empty());
        CHECK(ctoivec.capacity() == 0);

        cpodvec<double> itodvec;
        itodvec.recast_copy_from(ivec);
        CHECK(itodvec.size() == (ivec.size() / 2));
        for (auto i = 0u; i < ivec.size(); ++i) {
            CHECK((reinterpret_cast<int32_t*>(itodvec.data()))[i] == ivec[i]);
        }
        CHECK(itodvec.capacity() >= ivec.size() / 2);

        cpodvec<int32_t> dtoivec;
        dtoivec.recast_copy_from(itodvec);
        CHECK(dtoivec == ivec);

        ivec.pop_back();
        cpodvec<double> itodvec2;
        itodvec2.recast_copy_from(ivec);
        CHECK(itodvec2.size() == (ivec.size() / 2));
        CHECK(itodvec2.capacity() >= (ivec.size() / 2));
        for (auto i = 0u; i < ivec.size() -1; ++i) {
            CHECK((reinterpret_cast<int32_t*>(itodvec2.data()))[i] == ivec[i]);
        }

        dtoivec.recast_copy_from(itodvec2);
        CHECK(dtoivec.size() == ivec.size() - 1);
        for (auto i = 0u; i < ivec.size() - 1; ++i) {
            CHECK(dtoivec[i] == ivec[i]);
        }
    }

    CHECK(mallocs == frees);

    {
        cpodvec<three> t3(5, {1, 2, 3});
        CHECK(t3.size() == 5);
        CHECK(t3.capacity() == 5);
        CHECK(t3[3].y == 2);

        cpodvec<two> t2;
        t2.recast_copy_from(t3);
        CHECK(t2.size() == 7);
        CHECK(t2.capacity() == 7);

        t3.recast_copy_from(t2);
        CHECK(t3.size() == 4);
        CHECK(t3.capacity() == 5);
        t3.shrink_to_fit();

        CHECK(t3.capacity() == 4);
    }

    CHECK(mallocs == frees);

    mallocs = frees = reallocs = 0;
}

struct alignas(64) avx_512 { double d[8]; };

TEST_CASE("align")
{
    cpodvec<avx_512> vec;
    vec.resize(2);
}

