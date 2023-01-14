#include <doctest/doctest.h>

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
    static_assert(std::is_same<typename flat_set<int>::container_type, std::vector<int>>::value, "default container is vector");
    static_assert(sizeof(iset) == sizeof(std::vector<int>), "empty base optimization must work");

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

    CHECK(*sset.begin() == "123");

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

TEST_CASE("[flat_set] initialize")
{
    using namespace itlib;
    flat_set<int> m1 = {5, 3, 23};
    CHECK(m1.size() == 3);
    CHECK(m1.container() == std::vector<int>{3, 5, 23});

    flat_set<std::string> m2 = {"za", "b", "ccc", "azz"};
    CHECK(m2.size() == 4);
    auto& c = m2.container();
    CHECK(c[0] == "azz");
    CHECK(c[1] == "b");
    CHECK(c[2] == "ccc");
    CHECK(c[3] == "za");

    flat_set<std::string> m3 = {"ba", "aa", "ba", "gg", "ba"};
    CHECK(m3.size() == 3);
    auto& c3 = m3.container();
    CHECK(c3[0] == "aa");
    CHECK(c3[1] == "ba");
    CHECK(c3[2] == "gg");
}

TEST_CASE("[flat_map] ranges")
{
    using namespace itlib;
    flat_set<std::string> m = {"abc", "def", "geh", "xxx"};
    auto abc = m.find("abc");
    CHECK(*abc == "abc");
    auto def = m.find("def");
    CHECK(*def == "def");
    auto geh = m.find("geh");
    CHECK(*geh == "geh");
    auto xxx = m.find("xxx");
    CHECK(*xxx == "xxx");

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

TEST_CASE("[flat_set] custom cmp")
{
    // stateful comparator
    struct distance_from_constant
    {
        distance_from_constant(int m) : middle(m) {}
        int middle = 0;
        bool operator()(const int& a, const int& b) const { return std::abs(a - middle) < std::abs(b - middle); }
    };

    itlib::flat_set<int, distance_from_constant> dist({0, 9, 10, 11, 12, 20}, distance_from_constant{10});
    CHECK(dist.size() == 4);

    dist.clear();
    dist.emplace(5);
    dist.emplace(10);
    auto dr = dist.emplace(15);
    CHECK_FALSE(dr.second);

    CHECK(dist.size() == 2);
    CHECK(dist.container() == std::vector<int>{10, 5});

    auto f = dist.find(15);
    CHECK(f != dist.end());
    CHECK(*f == 5);
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
