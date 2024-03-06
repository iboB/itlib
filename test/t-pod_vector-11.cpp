// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <itlib/pod_vector.hpp>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(disable : 4127)
#endif

int32_t mallocs, frees, reallocs;

void clear_alloc_counters()
{
    mallocs = frees = reallocs = 0;
}

constexpr size_t wasteful_copy_size = 32;

template <typename Alloc>
void basic_test()
{
    using namespace itlib;

    {
        pod_vector<int32_t, Alloc> ivec;
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 0);
        CHECK(!ivec.begin());
        CHECK(ivec.begin() == ivec.end());
        CHECK(!ivec.cbegin());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());

        auto d = ivec.data();
        CHECK(ivec.reserve(2));
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

        CHECK(ivec.shrink_to_fit());
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

        const pod_vector<int32_t, Alloc> ivec2 = { 1, 2, 3, 4 };
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

        pod_vector<wchar_t, Alloc> svec;
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

        CHECK_FALSE(svec.resize(svec.capacity()));
        CHECK(svec.size() == svec.capacity());

        for (auto& s : svec) {
            CHECK(s == 0);
        }

        s1 = L"asdf";
        pod_vector<wchar_t, Alloc> cvec(s1.begin(), s1.end());
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

        CHECK_FALSE(cvec.resize(1));
        CHECK(cvec.size() == 1);
        CHECK(cvec.front() == L'b');
        CHECK(cvec.back() == L'b');

        // 0 is implicitly castable to nullptr_t which can be an iterator in our case
        pod_vector<int32_t, Alloc> nullptr_test(2, 0);
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
    clear_alloc_counters();
}

template <typename Alloc>
void swap_test()
{
    using namespace itlib;
    {
        pod_vector<int32_t, Alloc>
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
    clear_alloc_counters();
}

template <typename Alloc>
void empty_test()
{
    using namespace itlib;

    {
        pod_vector<int32_t, Alloc> foo = { 1, 2, 3, 4 };
        foo = pod_vector<int32_t, Alloc>();
        CHECK(foo.empty());
        CHECK(foo.capacity() == 0);
        CHECK(foo.data() == nullptr);

        foo = { 1, 2, 3, 4 };
        pod_vector<int32_t, Alloc> empty;

        foo = empty;
        CHECK(foo.empty());
        CHECK(foo.capacity() == 4);
        CHECK(foo.data() != nullptr);

        CHECK(foo.shrink_to_fit());
        CHECK(foo.capacity() == 0);
        CHECK(foo.data() == nullptr);
    }

    CHECK(mallocs == frees);
    clear_alloc_counters();
}

template <typename Alloc>
void realloc_test()
{
    using namespace itlib;

    using ii_t = uint16_t;
    constexpr size_t wcs = wasteful_copy_size / sizeof(ii_t);

    // reserve
    {
        pod_vector<ii_t, Alloc> bv(wcs - 5);
        CHECK(bv.capacity() == wcs - 5);
        bv.clear();
        CHECK(bv.capacity() == wcs - 5);

        reallocs = 0;

        bv.reserve(wcs);

        CHECK(reallocs == 1);

        bv.reserve(bv.capacity() + wcs + 1);

        CHECK(reallocs == 1);
    }

    clear_alloc_counters();

    // insert
    {
        pod_vector<ii_t, Alloc> bv(5);
        pod_vector<ii_t, Alloc> bv2(wcs, 10);

        reallocs = 0;
        bv.insert(bv.begin(), bv2.begin(), bv2.end());
        CHECK(reallocs == 1);
    }

    {
        pod_vector<ii_t, Alloc> bv(25);
        pod_vector<ii_t, Alloc> bv2(wcs + 1, 10);

        reallocs = 0;
        bv.insert(bv.begin(), bv2.begin(), bv2.end());
        CHECK(reallocs == 0);
    }

    CHECK(mallocs == frees);
    clear_alloc_counters();
}

std::vector<std::vector<uint8_t>> n_bufs;

enum class align_alloc_type
{
    expand,
    fail_expand,
    realloc,
    fail_realloc
};

template <size_t N, align_alloc_type Type>
struct n_align_allocator
{
    static_assert((N & (N - 1)) == 0, "must be pow2 align");
    using size_type = size_t;

    static void* malloc_at(std::vector<uint8_t>& buf, size_type size)
    {
        REQUIRE(buf.empty());
        buf.resize(size + N + 8*N);
        auto addr = reinterpret_cast<uintptr_t>(buf.data());
        auto offset = N - addr % N;
        auto fixed = addr + offset;
        auto ret = fixed + (rand() % 8) * N;
        return reinterpret_cast<void*>(ret);
    }

    static void* malloc(size_type size)
    {
        REQUIRE(size != 0);
        for (auto& b : n_bufs)
        {
            if (!b.empty()) continue;
            return malloc_at(b, size);
        }

        n_bufs.emplace_back();
        return malloc_at(n_bufs.back(), size);
    }

    static std::vector<uint8_t>& find_buf(void* ptr)
    {
        for (auto& b : n_bufs)
        {
            if (ptr >= b.data() && ptr < b.data() + b.size()) return b;
        }

        REQUIRE(false);
        return n_bufs.front();
    }

    static void free(void* ptr)
    {
        if (!ptr) return;

        auto& buf = find_buf(ptr);
        buf.clear();
    }

    static constexpr bool has_expand()
    {
        return Type == align_alloc_type::expand || Type == align_alloc_type::fail_expand;
    }

    static void* realloc(void* old, size_type new_size)
    {
        if (!old) return malloc(new_size);

        auto& buf = find_buf(old);
        auto off = reinterpret_cast<uint8_t*>(old) - buf.data();
        if (Type == align_alloc_type::realloc && buf.capacity() - off > new_size)
        {
            buf.resize(buf.capacity());
            return old;
        }

        std::vector<uint8_t> nb;
        auto ret = malloc_at(nb, new_size);
        auto nbytes = buf.size() - off;
        if (nbytes > new_size) nbytes = new_size;
        memcpy(ret, old, new_size);
        std::swap(nb, buf);
        return ret;
    }

    static bool expand(void* ptr, size_type new_size)
    {
        REQUIRE(ptr);

        auto& buf = find_buf(ptr);
        auto off = reinterpret_cast<uint8_t*>(ptr) - buf.data();
        if (Type == align_alloc_type::expand && buf.capacity() - off > new_size)
        {
            buf.resize(buf.capacity());
            return true;
        }
        return false;
    }

    static constexpr size_type max_size() { return ~size_type(0); }
    static constexpr bool zero_fill_new() { return true; }

    static constexpr size_type alloc_align()
    {
        return N;
    }
};

template <typename Alloc>
struct counting_allocator_wrapper
{
    Alloc a;
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

    static constexpr bool has_expand()
    {
        return Alloc::has_expand();
    }

    bool expand(void* ptr, size_type new_size)
    {
        REQUIRE(ptr);
        if (a.expand(ptr, new_size))
        {
            ++frees;
            ++mallocs;
            ++reallocs;
            return true;
        }
        return false;
    }

    void free(void* mem)
    {
        if (mem) ++frees;
        a.free(mem);
    }
    size_type max_size() const
    {
        return a.max_size();
    }

    constexpr bool zero_fill_new() const
    {
        return a.zero_fill_new();
    }

    constexpr size_type realloc_wasteful_copy_size() const
    {
        return wasteful_copy_size;
    }

    static constexpr size_type alloc_align()
    {
        return Alloc::alloc_align();
    }
};

using default_allocator = counting_allocator_wrapper<itlib::impl::pod_allocator>;

using one_alloc_r = counting_allocator_wrapper<n_align_allocator<1, align_alloc_type::realloc>>;
using one_alloc_fr = counting_allocator_wrapper<n_align_allocator<1, align_alloc_type::fail_realloc>>;
using two_alloc_r = counting_allocator_wrapper<n_align_allocator<2, align_alloc_type::realloc>>;
using m_alloc_r = counting_allocator_wrapper<n_align_allocator<alignof(max_align_t), align_alloc_type::realloc>>;
using m_alloc_fr = counting_allocator_wrapper<n_align_allocator<alignof(max_align_t), align_alloc_type::fail_realloc>>;

using one_alloc_e = counting_allocator_wrapper<n_align_allocator<1, align_alloc_type::expand>>;
using one_alloc_fe = counting_allocator_wrapper<n_align_allocator<1, align_alloc_type::fail_expand>>;
using two_alloc_e = counting_allocator_wrapper<n_align_allocator<2, align_alloc_type::expand>>;
using m_alloc_e = counting_allocator_wrapper<n_align_allocator<alignof(max_align_t), align_alloc_type::expand>>;
using m_alloc_fe = counting_allocator_wrapper<n_align_allocator<alignof(max_align_t), align_alloc_type::fail_expand>>;


TEST_CASE("basic")
{
    basic_test<default_allocator>();
    basic_test<one_alloc_r>();
    basic_test<one_alloc_fr>();
    basic_test<two_alloc_r>();
    basic_test<m_alloc_r>();
    basic_test<m_alloc_fr>();
    basic_test<one_alloc_e>();
    basic_test<one_alloc_fe>();
    basic_test<two_alloc_e>();
    basic_test<m_alloc_e>();
    basic_test<m_alloc_fe>();
}

TEST_CASE("swap")
{
    swap_test<default_allocator>();
    swap_test<one_alloc_r>();
    swap_test<one_alloc_fr>();
    swap_test<two_alloc_r>();
    swap_test<m_alloc_r>();
    swap_test<m_alloc_fr>();
    swap_test<one_alloc_e>();
    swap_test<one_alloc_fe>();
    swap_test<two_alloc_e>();
    swap_test<m_alloc_e>();
    swap_test<m_alloc_fe>();
}

TEST_CASE("empty")
{
    empty_test<default_allocator>();
    empty_test<one_alloc_r>();
    empty_test<one_alloc_fr>();
    empty_test<two_alloc_r>();
    empty_test<m_alloc_r>();
    empty_test<m_alloc_fr>();
    empty_test<one_alloc_e>();
    empty_test<one_alloc_fe>();
    empty_test<two_alloc_e>();
    empty_test<m_alloc_e>();
    empty_test<m_alloc_fe>();
}

TEST_CASE("reallocs")
{
    realloc_test<two_alloc_r>();
}

template <typename T, typename Alloc>
void align_test()
{
    using namespace itlib;
    {
        auto ptrval = [](void* ptr) -> uintptr_t { return reinterpret_cast<uintptr_t>(ptr); };

        pod_vector<T, Alloc> x;
        CHECK(x.data() == nullptr);
        CHECK(x.resize(2));
        CHECK(ptrval(x.data()) % alignof(T) == 0);
        x.reserve(10);
        CHECK(ptrval(x.data()) % alignof(T) == 0);
        x.insert(x.begin(), 1, {});
        CHECK(x.size() == 3);
    }
    clear_alloc_counters();
}

struct alignas(64) avx_512 { double d[8]; };

TEST_CASE("align")
{
    align_test<avx_512, default_allocator>();
    align_test<double, one_alloc_r>();
    align_test<avx_512, one_alloc_r>();
    align_test<avx_512, one_alloc_fr>();
    align_test<avx_512, one_alloc_e>();
    align_test<avx_512, one_alloc_fe>();
}

template <typename T>
using cpodvec = itlib::pod_vector<T, default_allocator>;

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
    clear_alloc_counters();
}

struct three { uint8_t x, y, z; };
struct two { uint16_t a; };

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

    clear_alloc_counters();
}

