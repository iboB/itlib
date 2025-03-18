// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

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
    static_assert(std::is_same<typename flat_map<int, float>::container_type, std::vector<std::pair<int, float>>>::value, "default container is vector");
    static_assert(sizeof(ifmap) == sizeof(std::vector<std::pair<int, float>>), "empty base optimization must work");

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

    CHECK(simap.begin()->first == "123");

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

TEST_CASE("[flat_map] ranges")
{
    using namespace itlib;
    flat_map<std::string, int> m = {{"abc", 1}, {"def", 44}, {"geh", 11}, {"xxx", 43}};
    auto abc = m.find("abc");
    CHECK(abc->first == "abc");
    auto def = m.find("def");
    CHECK(def->first == "def");
    auto geh = m.find("geh");
    CHECK(geh->first == "geh");
    auto xxx = m.find("xxx");
    CHECK(xxx->first == "xxx");

    CHECK(m.lower_bound("aaa") == abc);
    CHECK(m.lower_bound("abc") == abc);
    CHECK(m.lower_bound("bbb") == def);
    CHECK(m.lower_bound("xxx") == xxx);
    CHECK(m.lower_bound("xxz") == m.end());

    CHECK(m.upper_bound("aaa") == abc);
    CHECK(m.upper_bound("abc") == def);
    CHECK(m.upper_bound("bbb") == def);
    CHECK(m.upper_bound("xxx") == m.end());
    CHECK(m.upper_bound("xxz") == m.end());

    CHECK(m.equal_range("aaa") == std::make_pair(abc, abc));
    CHECK(m.equal_range("abc") == std::make_pair(abc, def));
    CHECK(m.equal_range("bbb") == std::make_pair(def, def));
    CHECK(m.equal_range("xxx") == std::make_pair(xxx, m.end()));
    CHECK(m.equal_range("xxz") == std::make_pair(m.end(), m.end()));
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

    std::vector<std::pair<int, int>> ints = {{5, 4}, {23, 11}, {3, 44}, {5, 4}, {3, 44}};
    flat_map<int, int> m4(ints.begin(), ints.end());
    CHECK(m4.container() == m3.container());
}

TEST_CASE("[flat_map] custom cmp")
{
    // stateful comparator
    struct distance_from_constant
    {
        distance_from_constant(int m) : middle(m) {}
        int middle = 0;
        bool operator()(const int& a, const int& b) const { return std::abs(a - middle) < std::abs(b - middle); }
    };

    itlib::flat_map<int, std::string, distance_from_constant> dist(
        {{0, "a"}, {9, "b"}, {10, "c"}, {11, "d"}, {12, "e"}, {20, "f"}},
        distance_from_constant{10});
    CHECK(dist.size() == 4);

    dist.clear();
    dist.insert({5, "five"});
    dist[10] = "ten";
    auto dr = dist.insert({15, "xxx"});
    CHECK_FALSE(dr.second);

    CHECK(dist.size() == 2);
    CHECK(dist.container() == std::vector<std::pair<int, std::string>>{{10, "ten"}, {5, "five"}});

    auto f = dist.find(15);
    CHECK(f != dist.end());
    CHECK(f->second == "five");

    CHECK(dist[15] == "five");

    CHECK(dist.size() == 2);
}

TEST_CASE("[flat_map] at bounds check")
{
    itlib::flat_map<int, char> map = {{1, 'a'}, {4, 'd'}, {3, 'c'}};
    CHECK(map.at(1) == 'a');
    CHECK(map.at(3) == 'c');
    CHECK(map.at(4) == 'd');
    CHECK_THROWS_AS_MESSAGE(map.at(0), std::out_of_range, "itlib::flat_map out of range");
    CHECK_THROWS_AS_MESSAGE(map.at(2), std::out_of_range, "itlib::flat_map out of range");
    CHECK_THROWS_AS_MESSAGE(map.at(5), std::out_of_range, "itlib::flat_map out of range");

    const auto& cmap = map;
    CHECK(cmap.at(1) == 'a');
    CHECK(cmap.at(3) == 'c');
    CHECK(cmap.at(4) == 'd');
    CHECK_THROWS_AS_MESSAGE(cmap.at(0), std::out_of_range, "itlib::flat_map out of range");
    CHECK_THROWS_AS_MESSAGE(cmap.at(2), std::out_of_range, "itlib::flat_map out of range");
    CHECK_THROWS_AS_MESSAGE(cmap.at(5), std::out_of_range, "itlib::flat_map out of range");
}

#include <itlib/static_vector.hpp>

TEST_CASE("[flat_map] static_vector test")
{
    using namespace itlib;

    flat_map<int, char, std::less<int>, static_vector<std::pair<int, char>, 10>> smap
        = {{0, 'a'}, {1, 'b'}, {3, 'd'}, {1, 'b'}, {1, 'b'}, {2, 'c'}};
    CHECK(smap.size() == 4);
    CHECK(smap[0] == 'a');
    CHECK(smap.at(1) == 'b');
    CHECK(smap.find(2)->second == 'c');
    CHECK(smap.container().back().second == 'd');
    CHECK(smap.find(10) == smap.end());

    smap.clear();
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

    auto res = smap.insert(std::make_pair(6, '\003'));
    CHECK(res.second);
    CHECK(res.first == smap.begin() + 1);

    res = smap.emplace(3, '\005');
    CHECK(res.second);
    CHECK(res.first == smap.begin() + 1);

    res = smap.emplace(6, '\010');
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

TEST_CASE("emplace_hint")
{
    itlib::flat_map<int, int> m = {{1, 1}, {2, 2}, {3, 3}};
    auto it = m.emplace_hint(m.begin(), 4, 4);
    CHECK(it == m.end() - 1);
    it = m.emplace_hint(m.begin(), 0, 0);
    CHECK(it == m.begin());
    it = m.emplace_hint(m.end(), 7, 7);
    CHECK(it == m.end() - 1);
    it = m.emplace_hint(m.end(), 3, 3);
    CHECK(it == m.begin() + 3);
    it = m.emplace_hint(m.begin(), 3, 3);
    CHECK(it == m.begin() + 3);
    it = m.emplace_hint(m.begin() + 5, 5, 5);
    CHECK(it == m.end() - 2);
}

TEST_CASE("flat_map_ready_tag")
{
    using namespace itlib;

#define BROKEN_DATA {{5, 4}, {3, 44}, {23, 11}, {3, 44}, {5, 4}, {23, 11}}

    // Unsorted and with duplicated elements
    std::vector<std::pair<int, int>> broken_data = BROKEN_DATA;

    // Create flat_map with flat_map_ready_tag
    flat_map<int, int> fm_ready(broken_data, flat_map_ready_tag{});

    // Check that the container is unchanged
    CHECK(fm_ready.container() == broken_data);

    // Create flat_map with flat_map_ready_tag using initializer list
    flat_map<int, int> fm_ready_init(BROKEN_DATA, flat_map_ready_tag{});

    // Check that the container is unchanged
    CHECK(fm_ready_init.container() == broken_data);

    // Create flat_map with flat_map_ready_tag using iterators
    flat_map<int, int> fm_ready_iter(broken_data.begin(), broken_data.end(), flat_map_ready_tag{});

    // Check that the container is unchanged
    CHECK(fm_ready_iter.container() == broken_data);
}
