#include <doctest/doctest.h>

#include <itlib/small_vector.hpp>

#include <doctest/util/counting_allocator.hpp>

#include <utility>
#include <string>
#include <cstring>

using itlib::small_vector;

template <typename T, size_t N, size_t M>
using small_vector_ca = small_vector<T, N, M, doctest::util::counting_allocator<T>>;

TEST_CASE("[small_vector] static")
{
    static_assert(sizeof(small_vector<void*, 10>) - sizeof(small_vector<void*, 3>) == sizeof(void*) * 7, "small_vector needs to have a static buffer");

    doctest::util::counting_allocator<void> ca;

    {
        small_vector_ca<int, 10, 0> ivec(ca);
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 10);
        CHECK(ivec.begin() == ivec.end());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());

        auto d = ivec.data();
        ivec.reserve(9);
        CHECK(ivec.capacity() == 10);
        CHECK(d == ivec.data());

        ivec.resize(2, 8);
        CHECK(ivec.size() == 2);
        CHECK(ivec.front() == 8);
        CHECK(ivec.back() == 8);
        CHECK(d == ivec.data());

        ivec.clear();
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 10);
        CHECK(ivec.begin() == ivec.end());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());
        CHECK(d == ivec.data());

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

        auto& back = ivec.emplace_back(3);
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
        CHECK(back == 3);
        CHECK(&back == &ivec.back());

        auto iret = ivec.insert(ivec.begin(), 53);
        CHECK(iret == ivec.begin());
        iret = ivec.insert(ivec.begin() + 2, 90);
        CHECK(iret == ivec.begin() + 2);
        iret = ivec.insert(ivec.begin() + 4, 17);
        CHECK(iret == ivec.begin() + 4);
        iret = ivec.insert(ivec.end(), 6);
        CHECK(iret == ivec.end() - 1);
        iret = ivec.insert(ivec.begin(), {1, 2});
        CHECK(iret == ivec.begin());

        int ints[] = {1, 2, 53, 12, 90, 3, 17, 6};
        CHECK(ivec.size() == 8);
        CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

        ivec.shrink_to_fit();
        CHECK(ivec.size() == 8);
        CHECK(ivec.capacity() == 10);
        CHECK(d == ivec.data());

        ivec.revert_to_static();
        CHECK(ivec.size() == 8);
        CHECK(ivec.capacity() == 10);
        CHECK(d == ivec.data());

        ivec.pop_back();
        CHECK(ivec.size() == 7);
        CHECK(memcmp(ivec.data(), ints, sizeof(ints) - sizeof(int)) == 0);

        ivec.resize(8);
        CHECK(ivec.size() == 8);
        ints[7] = 0;
        CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

        const small_vector_ca<int, 5, 0> ivec2 = {1, 2, 3, 4};
        CHECK(ivec2.size() == 4);
        CHECK(*ivec2.begin() == 1);
        CHECK(ivec2[1] == 2);
        CHECK(ivec2.at(2) == 3);
        CHECK(*ivec2.rbegin() == 4);

        auto eret = ivec.erase(ivec.begin());
        CHECK(ivec.size() == 7);
        CHECK(ivec.front() == 2);
        CHECK(memcmp(ivec.data(), ints + 1, ivec.size() * sizeof(int)) == 0);
        CHECK(eret == ivec.begin());

        eret = ivec.erase(ivec.begin() + 2, ivec.begin() + 4);
        CHECK(ivec.size() == 5);
        CHECK(ivec[3] == 17);
        CHECK(eret == ivec.begin() + 2);

        // empty erase
        eret = ivec.erase(ivec.begin() + 1, ivec.begin() + 1);
        CHECK(ivec.size() == 5);
        CHECK(eret == ivec.begin() + 1);

        small_vector_ca<std::string, 11, 0> svec(ca);
        svec.assign({"as", "df"});
        CHECK(svec.size() == 2);
        std::string s1 = "the quick brown fox jumped over the lazy dog 1234567890";
        auto& rs = svec.emplace_back(s1);
        CHECK(svec.back() == s1);
        CHECK(rs == s1);
        CHECK(&rs == &svec.back());

        auto svec1 = svec;
        CHECK(svec1 == svec);

        const void* cstr = svec.back().c_str();
        auto svec2 = std::move(svec);
        CHECK(svec2.size() == 3);
        CHECK(svec2.back() == s1);

        CHECK(svec.empty());
        CHECK(svec2.back().c_str() == cstr);

        svec = std::move(svec2);
        CHECK(svec2.empty());
        CHECK(svec.back().c_str() == cstr);

        svec2 = svec;
        CHECK(svec2.back() == s1);
        CHECK(svec.back() == s1);
        CHECK(svec == svec2);

        auto isret = svec.insert(svec.begin(), s1);
        CHECK(svec.size() == 4);
        CHECK(svec.back().c_str() == cstr);
        CHECK(svec.front() == svec.back());
        CHECK(isret == svec.begin());

        cstr = s1.c_str();
        isret = svec.emplace(svec.begin() + 2, std::move(s1));
        CHECK(svec.size() == 5);
        CHECK(svec.front() == svec[2]);
        CHECK(svec[2].c_str() == cstr);
        CHECK(isret == svec.begin() + 2);

        svec.clear();
        CHECK(svec.empty());
        svec2.clear();
        CHECK(svec2.empty());
        CHECK(svec == svec2);

        svec.resize(svec.capacity());
        CHECK(svec.size() == svec.capacity());

        for (auto& s : svec)
        {
            CHECK(s.empty());
        }

        s1 = "asdf";
        small_vector_ca<char, 10, 10> cvec(s1.begin(), s1.end(), ca);
        CHECK(cvec.size() == 4);
        CHECK(cvec.front() == 'a');
        CHECK(cvec.back() == 'f');

        cvec.clear();
        CHECK(cvec.size() == 0);
        CHECK(cvec.empty());

        s1 = "baz";
        cvec.assign(s1.begin(), s1.end());
        CHECK(cvec.size() == 3);
        CHECK(cvec.front() == 'b');
        CHECK(cvec.back() == 'z');

        // 0 is implicitly castable to nullptr_t which can be an iterator in our case
        small_vector_ca<int, 4, 4> nullptr_test(2, 0, ca);
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

    auto& p = ca.payload();
    CHECK(p.allocations == 0);
    CHECK(p.deallocations == 0);
    CHECK(p.allocated_bytes == 0);
    CHECK(p.deallocated_bytes == 0);
}

TEST_CASE("[small_vector] dynamic")
{
    small_vector_ca<int, 1, 0> ivec;
    CHECK(ivec.size() == 0);
    CHECK(ivec.capacity() == 1);
    CHECK(ivec.begin() == ivec.end());
    CHECK(ivec.cbegin() == ivec.cend());
    CHECK(ivec.empty());

    auto d = ivec.data();
    ivec.reserve(2);
    CHECK(ivec.capacity() == 2);
    CHECK(d != ivec.data());
    CHECK(ivec.get_allocator().payload().allocations == 1);

    ivec.resize(3, 8);
    CHECK(ivec.capacity() == 3);
    CHECK(ivec.size() == 3);
    CHECK(ivec.front() == 8);
    CHECK(ivec.back() == 8);
    CHECK(d != ivec.data());
    CHECK(ivec.get_allocator().payload().allocations == 2);

    ivec.clear();
    CHECK(ivec.size() == 0);
    CHECK(ivec.capacity() == 3);
    CHECK(d != ivec.data());
    CHECK(ivec.begin() == ivec.end());
    CHECK(ivec.cbegin() == ivec.cend());
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

    auto& back = ivec.emplace_back(3);
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
    CHECK(back == 3);
    CHECK(&back == &ivec.back());

    auto iret = ivec.insert(ivec.begin(), 53);
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

    int ints[] = { 1, 2, 53, 12, 90, 3, 17, 6 };
    CHECK(ivec.capacity() >= 8);
    CHECK(ivec.size() == 8);
    CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

    ivec.pop_back();
    CHECK(ivec.size() == 7);
    CHECK(memcmp(ivec.data(), ints, sizeof(ints) - sizeof(int)) == 0);

    ivec.resize(8);
    CHECK(ivec.size() == 8);
    ints[7] = 0;
    CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

    const small_vector_ca<int, 1, 0> ivec2 = { 1, 2, 3, 4 };
    CHECK(ivec2.size() == 4);
    CHECK(*ivec2.begin() == 1);
    CHECK(ivec2[1] == 2);
    CHECK(ivec2.at(2) == 3);
    CHECK(*ivec2.rbegin() == 4);

    auto eret = ivec.erase(ivec.begin());
    CHECK(ivec.size() == 7);
    CHECK(ivec.front() == 2);
    CHECK(memcmp(ivec.data(), ints + 1, ivec.size() * sizeof(int)) == 0);
    CHECK(eret == ivec.begin());

    eret = ivec.erase(ivec.begin() + 2, ivec.begin() + 4);
    CHECK(ivec.size() == 5);
    CHECK(ivec[3] == 17);
    CHECK(eret == ivec.begin() + 2);

    // empty erase
    eret = ivec.erase(ivec.begin() + 1, ivec.begin() + 1);
    CHECK(ivec.size() == 5);
    CHECK(eret == ivec.begin() + 1);

    small_vector_ca<std::string, 1, 0> svec;
    svec.assign({ "as", "df" });
    CHECK(svec.size() == 2);
    std::string s1 = "the quick brown fox jumped over the lazy dog 1234567890";
    auto& rs = svec.emplace_back(s1);
    CHECK(svec.back() == s1);
    CHECK(rs == s1);
    CHECK(&rs == &svec.back());

    auto svec1 = svec;
    CHECK(svec1 == svec);

    const void* cstr = svec.back().c_str();
    auto svec2 = std::move(svec);
    CHECK(svec2.size() == 3);
    CHECK(svec2.back() == s1);

    CHECK(svec.empty());
    CHECK(svec2.back().c_str() == cstr);

    svec = std::move(svec2);
    CHECK(svec2.empty());
    CHECK(svec.back().c_str() == cstr);

    svec2 = svec;
    CHECK(svec2.back() == s1);
    CHECK(svec.back() == s1);
    CHECK(svec == svec2);

    auto isret = svec.insert(svec.begin(), s1);
    CHECK(svec.size() == 4);
    CHECK(svec.back().c_str() == cstr);
    CHECK(svec.front() == svec.back());
    CHECK(isret == svec.begin());

    cstr = s1.c_str();
    isret = svec.emplace(svec.begin() + 2, std::move(s1));
    CHECK(svec.size() == 5);
    CHECK(svec.front() == svec[2]);
    CHECK(svec[2].c_str() == cstr);
    CHECK(isret == svec.begin() + 2);

    svec.clear();
    CHECK(svec.empty());
    svec2.clear();
    CHECK(svec2.empty());
    CHECK(svec == svec2);

    svec.resize(svec.capacity());
    CHECK(svec.size() == svec.capacity());

    for (auto& s : svec)
    {
        CHECK(s.empty());
    }

    s1 = "asdf";
    small_vector_ca<char, 1, 0> cvec(s1.begin(), s1.end());
    CHECK(cvec.size() == 4);
    CHECK(cvec.front() == 'a');
    CHECK(cvec.back() == 'f');

    cvec.clear();
    CHECK(cvec.size() == 0);
    CHECK(cvec.empty());

    s1 = "baz";
    cvec.assign(s1.begin(), s1.end());
    CHECK(cvec.size() == 3);
    CHECK(cvec.front() == 'b');
    CHECK(cvec.back() == 'z');
}

TEST_CASE("[small_vector] static-dynamic")
{
    small_vector_ca<int, 5, 3> ivec;
    const auto static_data = ivec.data();

    ivec.push_back(1);
    ivec.push_back(2);
    ivec.push_back(3);

    CHECK(ivec.is_static());
    CHECK(ivec.data() == static_data);

    auto lastsize = ivec.size();
    auto iret = ivec.insert(ivec.end(), 3u, 8);
    CHECK(iret == ivec.begin() + lastsize);

    CHECK(ivec.size() == 6);
    CHECK(ivec.data() != static_data);

    ivec.reserve(20);

    auto dyn_data = ivec.data();
    CHECK(dyn_data != static_data);

    ivec.erase(ivec.begin(), ivec.begin() + 6);
    CHECK(ivec.data() == static_data);
    CHECK(ivec.empty());

    ivec.resize(19, 11);
    dyn_data = ivec.data();
    CHECK(dyn_data != static_data);
    CHECK(ivec.capacity() == 19);
    CHECK(ivec.size() == 19);

    ivec.resize(4);
    CHECK(ivec.size() == 4);
    CHECK(ivec.data() == dyn_data);

    ivec.revert_to_static();
    CHECK(ivec.size() == 4);
    CHECK(ivec.capacity() == 5);
    CHECK(ivec.data() == static_data);

    ivec.reserve(10);
    dyn_data = ivec.data();
    CHECK(dyn_data != static_data);
    CHECK(ivec.size() == 4);
    CHECK(ivec.capacity() == 10);

    ivec.shrink_to_fit();
    CHECK(ivec.size() == 4);
    CHECK(ivec.capacity() == 5);
    CHECK(ivec.data() == static_data);

    ivec.reserve(10);
    CHECK(ivec.size() == 4);
    CHECK(ivec.capacity() == 10);
    dyn_data = ivec.data();
    CHECK(dyn_data != static_data);

    iret = ivec.insert(ivec.begin() + 3, 5u, 88);
    CHECK(ivec.size() == 9);
    CHECK(ivec.capacity() == 10);
    CHECK(ivec.data() == dyn_data);
    CHECK(ivec[2] == 11);
    CHECK(ivec[7] == 88);
    CHECK(ivec[8] == 11);
    CHECK(iret == ivec.begin() + 3);

    small_vector_ca<int, 3, 4> ivec2(ivec.begin(), ivec.end());
    CHECK(ivec2.size() == 9);
    CHECK(ivec2.size() == 9);
    CHECK(ivec2.capacity() == 9);
    CHECK(ivec2[2] == 11);
    CHECK(ivec2[7] == 88);
    CHECK(ivec2[8] == 11);

    auto eret = ivec.erase(ivec.begin() + 1, ivec.end() - 2);
    CHECK(ivec.size() == 3);
    CHECK(eret == ivec.begin() + 1);

    eret = ivec.erase(ivec.end() - 1);
    CHECK(ivec.size() == 2);
    CHECK(ivec.capacity() == 5);
    CHECK(ivec.data() == static_data);
    CHECK(eret == ivec.end());

    eret = ivec2.erase(ivec2.begin() + 1, ivec2.end() - 2);
    CHECK(ivec2.size() == 3);
    CHECK(ivec2.capacity() == 3);
    CHECK(eret == ivec2.begin() + 1);

    small_vector_ca<int, 4, 3> ivec3(50);
    ivec3.resize(2);
    CHECK(ivec3.size() == 2);
    CHECK(ivec3.capacity() == 50);
    CHECK_FALSE(ivec3.is_static());

    CHECK(*ivec2.begin() == 11);
    CHECK(ivec2[1] == 88);
    CHECK(ivec2.back() == 11);

    auto p = ivec3.insert(ivec3.begin() + 1, ivec2.begin(), ivec2.end());
    CHECK(p == ivec3.begin() + 1);
    CHECK(ivec3.size() == 5);
    CHECK(ivec3.front() == 0);
    CHECK(ivec3.back() == 0);
    CHECK(ivec3[1] == 11);
    CHECK(ivec3[2] == 88);
    CHECK(ivec3[3] == 11);
}

struct foo
{
    int a;
    bool operator==(const foo& other) const { return a == other.a; }
};

TEST_CASE("[small_vector] compare")
{
    small_vector<int, 10> ivec1 = {1, 2, 3};
    small_vector<int, 2> ivec2 = {1, 2, 3};
    CHECK(ivec1 == ivec2);
    ivec2[1] = 8;
    CHECK(ivec1 != ivec2);

    small_vector<foo, 2> fvec1 = {{1}, {2}, {3}};
    small_vector<foo, 5> fvec2 = {{1}, {2}, {3}};
    CHECK(fvec1 == fvec2);
    fvec2[1].a = 8;
    CHECK(fvec1 != fvec2);
}

#if !defined(__EMSCRIPTEN__) // emscripten doesn't allow exceptions by default
TEST_CASE("[small_vector] out of range")
{
    using namespace itlib;
    small_vector<int, 5> ivec;
    ivec.resize(4);
    CHECK(ivec.capacity() == 5);

    CHECK_THROWS_AS(ivec.insert(ivec.begin() - 1, 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.insert(ivec.end() + 1, 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.begin() - 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.end() + 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.begin() - 1, ivec.begin() + 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.begin() + 2, ivec.end() + 1), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.end() + 1, ivec.end() + 3), std::out_of_range);
    CHECK(ivec.size() == 4);
    CHECK_THROWS_AS(ivec.erase(ivec.end() - 1, ivec.begin() + 1), std::out_of_range);
    CHECK(ivec.size() == 4);

}
#endif
