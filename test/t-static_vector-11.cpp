#include <doctest/doctest.h>

#include <itlib/static_vector.hpp>

#include <cstring>
#include <string>

TEST_CASE("[static_vector] test")
{
    using namespace itlib;
    using namespace std;

    static_vector<int, 10> ivec;

    static_assert(sizeof(ivec) == sizeof(int) * 10 + sizeof(size_t), "static_vector must have size of N*t + size_t");

    CHECK(ivec.size() == 0);
    CHECK(ivec.capacity() == 10);
    CHECK(ivec.max_size() == 10);
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
    CHECK(iret == ivec.begin());
    iret = ivec.insert(ivec.begin() + 2, 90);
    CHECK(iret == ivec.begin() + 2);
    iret = ivec.insert(ivec.begin() + 4, 17);
    CHECK(iret == ivec.begin() + 4);
    iret = ivec.insert(ivec.end(), 6);
    CHECK(iret == ivec.end() - 1);

    int ints[] = { 53, 12, 90, 3, 17, 6 };
    CHECK(ivec.size() == 6);
    CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

    ivec.pop_back();
    CHECK(ivec.size() == 5);
    CHECK(memcmp(ivec.data(), ints, sizeof(ints) - sizeof(int)) == 0);

    ivec.resize(6);
    CHECK(ivec.size() == 6);
    ints[5] = 0;
    CHECK(memcmp(ivec.data(), ints, sizeof(ints)) == 0);

    const static_vector<int, 5> ivec2 = { 1, 2, 3, 4 };
    CHECK(ivec2.size() == 4);
    CHECK(*ivec2.begin() == 1);
    CHECK(ivec2[1] == 2);
    CHECK(ivec2.at(2) == 3);
    CHECK(*ivec2.rbegin() == 4);

    auto eret = ivec.erase(ivec.begin());
    CHECK(ivec.size() == 5);
    CHECK(ivec.front() == 12);
    CHECK(memcmp(ivec.data(), ints + 1, ivec.size() * sizeof(int)) == 0);
    CHECK(eret == ivec.begin());

    eret = ivec.erase(ivec.begin() + 2);
    CHECK(ivec.size() == 4);
    CHECK(ivec[2] == 17);
    CHECK(eret == ivec.begin() + 2);

    static_vector<string, 11> svec = { "as", "df" };
    CHECK(svec.size() == 2);
    string s1 = "the quick brown fox jumped over the lazy dog 1234567890";
    auto& esret = svec.emplace_back(s1);
    CHECK(svec.back() == s1);
    CHECK(esret == s1);
    CHECK(&esret == &svec.back());

    auto svec1 = svec;
    static_assert(sizeof(svec1) == sizeof(string) * 11 + sizeof(size_t), "static_vector must have size of N*t + size_t");
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

#if !defined(__EMSCRIPTEN__) // emscripten doesn't allow exceptions by default
    CHECK_THROWS_AS(svec.push_back("asd"), std::out_of_range);
    CHECK(svec.size() == svec.capacity());
    CHECK_THROWS_AS(svec.resize(55), std::out_of_range);
    CHECK(svec.size() == svec.capacity());
    CHECK_THROWS_AS(svec.insert(svec.begin(), "55"), std::out_of_range);
    CHECK(svec.size() == svec.capacity());
    CHECK_THROWS_AS(svec.emplace(svec.begin(), "55"), std::out_of_range);
    CHECK(svec.size() == svec.capacity());
#endif

    // self usurp
    svec = svec;
    CHECK(svec.size() == svec.capacity());

    // swap
    svec = {"1", "2", "3"};
    svec2 = {"4", "5", "6", "7"};

    svec.swap(svec2);

    CHECK(svec.size() == 4);
    CHECK(svec2.size() == 3);
    CHECK(svec2.front() == "1");
    CHECK(svec.back() == "7");

    svec = { "a", "b", "c" };
    svec2.swap(svec);

    CHECK(svec2.size() == svec.size());
    CHECK(svec2.back() == "c");
    CHECK(svec.front() == "1");

    // resize
    svec.resize(6, "xxx");
    REQUIRE(svec.size() == 6);
    CHECK(svec[3] == "xxx");
    CHECK(svec.back() == svec[4]);
    svec.resize(3, "xxx");
    CHECK(svec.size() == 3);

    svec.resize(5);
    REQUIRE(svec.size() == 5);
    CHECK(svec[3].empty());
    CHECK(svec.back() == "");
    svec.resize(3);
    CHECK(svec.size() == 3);

    CHECK(svec == static_vector<std::string, 3>{"1", "2", "3"});
}

struct foo
{
    int a;
    bool operator==(const foo& other) const { return a == other.a; }
};

TEST_CASE("[static_vector] compare")
{
    itlib::static_vector<int, 10> ivec1 = {1, 2, 3};
    itlib::static_vector<int, 5> ivec2 = {1, 2, 3};
    CHECK(ivec1 == ivec2);
    ivec2[1] = 8;
    CHECK(ivec1 != ivec2);

    itlib::static_vector<foo, 10> fvec1 = {{1}, {2}, {3}};
    itlib::static_vector<foo, 5> fvec2 = {{1}, {2}, {3}};
    CHECK(fvec1 == fvec2);
    fvec2[1].a = 8;
    CHECK(fvec1 != fvec2);
}

TEST_CASE("[static_vector] assign")
{
    int foo[] = { 1, 2, 3 };
    signed char bar[] = { 8, 9, 10, 11 };

    itlib::static_vector<int, 10> a(std::begin(foo), std::end(foo));
    REQUIRE(a.size() == 3);
    CHECK(a[0] == 1);
    CHECK(a[1] == 2);
    CHECK(a[2] == 3);

    a.assign(std::begin(bar), std::end(bar));
    REQUIRE(a.size() == 4);
    CHECK(a[0] == 8);
    CHECK(a[1] == 9);
    CHECK(a[2] == 10);
    CHECK(a[3] == 11);

    a.assign(2, 4);
    REQUIRE(a.size() == 2);
    CHECK(a[0] == 4);
    CHECK(a[1] == 4);

    a.assign({7, 6, 5});
    REQUIRE(a.size() == 3);
    CHECK(a[0] == 7);
    CHECK(a[1] == 6);
    CHECK(a[2] == 5);
}

TEST_CASE("[static_vector] insert/erase")
{
    using svec = itlib::static_vector<std::string, 20>;

    const svec helper = {"0", "1"};
    {
        svec e;
        auto it = e.insert(e.begin(), {"0", "1"});
        CHECK(it == e.begin());
        CHECK(e == helper);
    }
    {
        svec e;
        auto it = e.insert(e.end(), helper.begin(), helper.end());
        CHECK(it == e.begin());
        CHECK(e == helper);
    }

    svec vec = {"5", "8", "9"};
    auto it = vec.insert(vec.begin(), {"2", "3", "4"});
    CHECK(it == vec.begin());

    it = vec.insert(vec.begin(), helper.begin(), helper.end());
    CHECK(it == vec.begin());
    it = vec.insert(vec.begin(), helper.begin(), helper.begin());
    CHECK(it == vec.begin());

    it = vec.insert(vec.begin() + 6, 0, "zzz");
    CHECK(it == vec.begin() + 6);
    it = vec.insert(vec.begin() + 6, 2, "xxx");
    CHECK(it == vec.begin() + 6);

    std::string ten = "10";
    it = vec.insert(vec.end(), std::move(ten));
    CHECK(it == vec.begin() + 10);
    it = vec.insert(vec.end(), {"11", "12"});
    CHECK(it == vec.begin() + 11);

    REQUIRE(vec.size() == 13);

    for (int i = 0; i <= 5; ++i) CHECK(vec[i] == std::to_string(i));
    for (int i = 6; i < 8; ++i) CHECK(vec[i] == "xxx");
    for (int i = 8; i < 13; ++i) CHECK(vec[i] == std::to_string(i));

    it = vec.erase(vec.begin() + 6, vec.begin() + 8);
    CHECK(vec.begin() + 6);
    REQUIRE(vec.size() == 11);
    for (int i = 0; i <= 5; ++i) CHECK(vec[i] == std::to_string(i));
    for (int i = 8; i < 13; ++i) CHECK(vec[i-2] == std::to_string(i));

    it = vec.erase(vec.end() - 4, vec.end());
    CHECK(it == vec.end());
    REQUIRE(vec.size() == 7);
    for (int i = 0; i <= 5; ++i) CHECK(vec[i] == std::to_string(i));
    CHECK(vec.back() == "8");

    it = vec.erase(vec.begin(), vec.begin() + 3);
    CHECK(it == vec.begin());
    REQUIRE(vec.size() == 4);
    CHECK(vec == svec{"3", "4", "5", "8"});

    it = vec.erase(vec.begin(), vec.end());
    CHECK(it == vec.end());
    CHECK(vec.empty());

    it = vec.erase(vec.begin(), vec.end()); // must be safe
    CHECK(it == vec.end());
}
