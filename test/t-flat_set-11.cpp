#include "doctest.hpp"

#include <itlib/flat_set.hpp>

// struct with no operator==
struct int_wrap
{
    int_wrap() = default;
    int_wrap(int i) : val(i) {}
    int val;

    struct compare
    {
        bool operator()(const int_wrap& a, const int_wrap& b) const
        {
            return a.val < b.val;
        }
    };
};

TEST_CASE("[flat_set] test")
{
    using namespace itlib;

    flat_set<int> iset;
    CHECK(iset.empty());
    CHECK(iset.size() == 0);
    CHECK(iset.capacity() == 0);
    CHECK(iset.begin() == iset.end());

    iset.insert(1);
    CHECK(iset.size() == 1);

    auto ifit = iset.begin();
    CHECK(*ifit == 1);
    CHECK(iset.count(1) == 1);
    CHECK(iset.count(5) == 0);

    ++ifit;
    CHECK(ifit == iset.end());

    auto res = iset.insert(6);
    CHECK(res.second);
    CHECK(res.first == iset.begin() + 1);

    res = iset.emplace(3);
    CHECK(res.second);
    CHECK(res.first == iset.begin() + 1);

    res = iset.emplace(6);
    CHECK(!res.second);
    CHECK(res.first == iset.begin() + 2);

    iset.insert(5);
    iset.insert(52);
    iset.insert(12);
    CHECK(iset.size() == 6);

    CHECK(std::is_sorted(iset.begin(), iset.end()));

    iset.erase(12);
    CHECK(iset.size() == 5);

    CHECK(std::is_sorted(iset.begin(), iset.end()));

    ifit = iset.find(12);
    CHECK(ifit == iset.end());

    ifit = iset.find(6);
    CHECK(ifit != iset.end());
    iset.erase(ifit);

    CHECK(iset.size() == 4);
    CHECK(std::is_sorted(iset.begin(), iset.end()));
    ifit = iset.find(6);
    CHECK(ifit == iset.end());

    //

    flat_set<std::string> sset;

    sset.emplace("123");

    CHECK(sset.begin()->c_str() == "123");

    sset.insert("asd");

    auto siit = sset.find("asd");
    CHECK(siit != sset.end());
    CHECK(siit == sset.begin() + 1);

    CHECK(sset.count("bababa") == 0);
    CHECK(sset.count("asd") == 1);

    std::string asd = "asd";
    CHECK(sset.find(asd) == sset.begin() + 1);

    sset.emplace("0The quick brown fox jumps over the lazy dog");
    CHECK(sset.begin()->c_str()[1] == 'T');
    const void* cstr = sset.begin()->c_str();

    auto sset2 = std::move(sset);
    CHECK(sset.empty());
    CHECK(sset2.begin()->c_str() == cstr);

    sset = std::move(sset2);
    CHECK(sset2.empty());
    CHECK(sset.begin()->c_str() == cstr);

    CHECK(sset2 != sset);
    sset2 = sset;
    CHECK(sset2 == sset);

    // no == comparable tests
    flat_set<int_wrap, int_wrap::compare> iwset;
    iwset.emplace(5);
    iwset.emplace(20);
    iwset.emplace(10);

    auto iwi = iwset.emplace(3);
    CHECK(iwi.second == true);
    CHECK(iwi.first == iwset.begin());

    CHECK(iwset.begin()->val == 3);
    CHECK(iwset.rbegin()->val == 20);
    CHECK(iwset.find(10)->val == 10);

    iwi = iwset.insert(11);
    CHECK(iwi.second == true);
    CHECK(iwi.first + 2 == iwset.end());

    CHECK(iwset.find(11) != iwset.end());

    iwi = iwset.emplace(10);
    CHECK(iwi.second == false);
    CHECK(iwi.first->val == 10);

    CHECK(iwset.find(18) == iwset.end());
    CHECK(iwset.find(11) != iwset.end());

    const auto ciwset = iwset;

    CHECK(ciwset.begin()->val == 3);
    CHECK(ciwset.rbegin()->val == 20);

    CHECK(ciwset.find(18) == ciwset.end());
    CHECK(ciwset.find(11) != ciwset.end());

    // swap
    flat_set<int> m1, m2;
    m1.reserve(10);
    m1.emplace(1);
    m1.insert(2);
    auto m1c = m1.capacity();

    CHECK(m2.capacity() == 0);
    m1.swap(m2);

    CHECK(m2.size() == 2);
    CHECK(m2.capacity() == m1c);
    CHECK(m1.capacity() == 0);

    // self usurp
    m2 = m2;
    CHECK(m2.size() == 2);
    CHECK(m2.capacity() == m1c);
}

#include <itlib/static_vector.hpp>

TEST_CASE("[flat_set] static_vector test")
{
    using namespace itlib;

    flat_set<int, std::less<int>, static_vector<int, 10>> sset;
    CHECK(sset.empty());
    CHECK(sset.size() == 0);
    CHECK(sset.capacity() == 10);
    CHECK(sset.begin() == sset.end());

    sset.insert(1);
    CHECK(sset.size() == 1);

    auto ifit = sset.begin();
    CHECK(*ifit == 1);
    CHECK(sset.count(1) == 1);
    CHECK(sset.count(5) == 0);

    ++ifit;
    CHECK(ifit == sset.end());

    auto res = sset.insert(6);
    CHECK(res.second);
    CHECK(res.first == sset.begin() + 1);

    res = sset.emplace(3);
    CHECK(res.second);
    CHECK(res.first == sset.begin() + 1);

    res = sset.emplace(6);
    CHECK(!res.second);
    CHECK(res.first == sset.begin() + 2);

    sset.emplace(2);
    sset.emplace(52);
    sset.emplace(12);
    CHECK(sset.size() == 6);

    CHECK(std::is_sorted(sset.begin(), sset.end()));

    sset.erase(12);
    CHECK(sset.size() == 5);

    CHECK(std::is_sorted(sset.begin(), sset.end()));

    ifit = sset.find(12);
    CHECK(ifit == sset.end());

    ifit = sset.find(6);
    CHECK(ifit != sset.end());
    sset.erase(ifit);

    CHECK(sset.size() == 4);
    CHECK(std::is_sorted(sset.begin(), sset.end()));
    ifit = sset.find(6);
    CHECK(ifit == sset.end());
}
