#include "doctest.hpp"

#include <itlib/flat_map.hpp>

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

TEST_CASE("[flat_map] test")
{
    using namespace itlib;

    flat_map<int, float> ifmap;
    CHECK(ifmap.empty());
    CHECK(ifmap.size() == 0);
    CHECK(ifmap.capacity() == 0);
    CHECK(ifmap.begin() == ifmap.end());

    ifmap[1] = 3.2f;
    CHECK(ifmap.size() == 1);

    auto ifit = ifmap.begin();
    CHECK(ifit->first == 1);
    CHECK(ifit->second == 3.2f);
    CHECK(ifmap[1] == 3.2f);
    CHECK(ifmap.at(1) == 3.2f);
    CHECK(ifmap.count(1) == 1);
    CHECK(ifmap.count(5) == 0);

    ++ifit;
    CHECK(ifit == ifmap.end());

    auto res = ifmap.insert(std::make_pair(6, 3.14f));
    CHECK(res.second);
    CHECK(res.first == ifmap.begin() + 1);

    res = ifmap.emplace(3, 5.5f);
    CHECK(res.second);
    CHECK(res.first == ifmap.begin() + 1);

    res = ifmap.emplace(6, 8.f);
    CHECK(!res.second);
    CHECK(res.first == ifmap.begin() + 2);

    ifmap[2] = 5;
    ifmap[52] = 15;
    ifmap[12] = 1;
    CHECK(ifmap.size() == 6);

    auto cmp = [](const flat_map<int, float>::value_type& a, const flat_map<int, float>::value_type& b) -> bool
    {
        return a.first < b.first;
    };

    CHECK(std::is_sorted(ifmap.begin(), ifmap.end(), cmp));

    ifmap.erase(12);
    CHECK(ifmap.size() == 5);

    CHECK(std::is_sorted(ifmap.begin(), ifmap.end(), cmp));

    ifit = ifmap.find(12);
    CHECK(ifit == ifmap.end());

    ifit = ifmap.find(6);
    CHECK(ifit != ifmap.end());
    ifmap.erase(ifit);

    CHECK(ifmap.size() == 4);
    CHECK(std::is_sorted(ifmap.begin(), ifmap.end(), cmp));
    ifit = ifmap.find(6);
    CHECK(ifit == ifmap.end());

    //

    flat_map<std::string, int> simap;

    CHECK(simap["123"] == 0);

    CHECK(simap.begin()->first.c_str() == "123");

    ++simap["asd"];

    auto siit = simap.find("asd");
    CHECK(siit != simap.end());
    CHECK(siit->second == 1);
    CHECK(siit == simap.begin() + 1);

    CHECK(simap.count("bababa") == 0);
    CHECK(simap.count("asd") == 1);

    std::string asd = "asd";
    CHECK(simap.at(asd) == simap.at("asd"));

    simap["0The quick brown fox jumps over the lazy dog"] = 555;
    CHECK(simap.begin()->first[1] == 'T');
    const void* cstr = simap.begin()->first.c_str();

    auto simap2 = std::move(simap);
    CHECK(simap.empty());
    CHECK(simap2.begin()->first.c_str() == cstr);

    simap = std::move(simap2);
    CHECK(simap2.empty());
    CHECK(simap.begin()->first.c_str() == cstr);

    CHECK(simap2 != simap);
    simap2 = simap;
    CHECK(simap2 == simap);

    // no == comparable tests
    flat_map<int_wrap, int, int_wrap::compare> iwmap;
    iwmap[5] = 1;
    iwmap[20] = 15;
    iwmap[10] = 5;

    auto iwi = iwmap.emplace(3, 4);
    CHECK(iwi.second == true);
    CHECK(iwi.first == iwmap.begin());

    CHECK(iwmap.begin()->first.val == 3);
    CHECK(iwmap.begin()->second == 4);
    CHECK(iwmap.rbegin()->first.val == 20);
    CHECK(iwmap.rbegin()->second == 15);
    CHECK(iwmap.at(10) == 5);

    iwi = iwmap.insert(std::pair<int_wrap, int>(11, 6));
    CHECK(iwi.second == true);
    CHECK(iwi.first + 2 == iwmap.end());

    CHECK(iwmap[11] == 6);

    iwi = iwmap.emplace(10, 55);
    CHECK(iwi.second == false);
    CHECK(iwi.first->second == 5);

    CHECK(iwmap.find(18) == iwmap.end());
    CHECK(iwmap.find(11) != iwmap.end());

    const auto ciwmap = iwmap;

    CHECK(ciwmap.begin()->first.val == 3);
    CHECK(ciwmap.begin()->second == 4);
    CHECK(ciwmap.rbegin()->first.val == 20);
    CHECK(ciwmap.rbegin()->second == 15);
    CHECK(ciwmap.at(10) == 5);

    CHECK(ciwmap.find(18) == ciwmap.end());
    CHECK(ciwmap.find(11) != ciwmap.end());

    // swap
    flat_map<int, int> m1, m2;
    m1.reserve(10);
    m1[1] = 2;
    m1[2] = 5;
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

TEST_CASE("[flat_map] initialize")
{
    using namespace itlib;
    flat_map<int, int> m1 = {{5, 4}, {3, 44}, {23, 11}};
    CHECK(m1.size() == 3);
    CHECK(m1.container() == std::vector<std::pair<int, int>>{{3, 44}, {5, 4}, {23, 11}});

    flat_map<std::string, bool> m2 = {{"za", true}, {"b", false}, {"ccc", true}, {"azz", false}};
    CHECK(m2.size() == 4);
    auto& c = m2.container();
    CHECK(c[0].first == "azz");
    CHECK(c[1].first == "b");
    CHECK(c[2].first == "ccc");
    CHECK(c[3].first == "za");

    flat_map<int, int> m3 = {{5, 4}, {23, 11}, {3, 44}, {5, 4}, {23, 11}};
    CHECK(m3.size() == 3);
    CHECK(m3.container() == std::vector<std::pair<int, int>>{{3, 44}, {5, 4}, {23, 11}});
}

#include <itlib/static_vector.hpp>

TEST_CASE("[flat_map] static_vector test")
{
    using namespace itlib;

    flat_map<int, char, std::less<int>, static_vector<std::pair<int, char>, 10>> smap;
    CHECK(smap.empty());
    CHECK(smap.size() == 0);
    CHECK(smap.capacity() == 10);
    CHECK(smap.begin() == smap.end());

    smap[1] = 3;
    CHECK(smap.size() == 1);

    auto ifit = smap.begin();
    CHECK(ifit->first == 1);
    CHECK(ifit->second == 3);
    CHECK(smap[1] == 3);
    CHECK(smap.at(1) == 3);
    CHECK(smap.count(1) == 1);
    CHECK(smap.count(5) == 0);

    ++ifit;
    CHECK(ifit == smap.end());

    auto res = smap.insert(std::make_pair(6, 3));
    CHECK(res.second);
    CHECK(res.first == smap.begin() + 1);

    res = smap.emplace(3, 5);
    CHECK(res.second);
    CHECK(res.first == smap.begin() + 1);

    res = smap.emplace(6, 8);
    CHECK(!res.second);
    CHECK(res.first == smap.begin() + 2);

    smap[2] = 5;
    smap[52] = 15;
    smap[12] = 1;
    CHECK(smap.size() == 6);

    auto cmp = [](const flat_map<int, float>::value_type& a, const flat_map<int, float>::value_type& b) -> bool
    {
        return a.first < b.first;
    };

    CHECK(std::is_sorted(smap.begin(), smap.end(), cmp));

    smap.erase(12);
    CHECK(smap.size() == 5);

    CHECK(std::is_sorted(smap.begin(), smap.end(), cmp));

    ifit = smap.find(12);
    CHECK(ifit == smap.end());

    ifit = smap.find(6);
    CHECK(ifit != smap.end());
    smap.erase(ifit);

    CHECK(smap.size() == 4);
    CHECK(std::is_sorted(smap.begin(), smap.end(), cmp));
    ifit = smap.find(6);
    CHECK(ifit == smap.end());
}
