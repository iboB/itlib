#include "doctest.hpp"

#include <itlib/small_vector.hpp>

#include <utility>
#include <cstring>

size_t allocations = 0;
size_t deallocations = 0;
size_t allocated_bytes = 0;
size_t deallocated_bytes = 0;
size_t constructions = 0;
size_t destructions = 0;

template <typename T>
class counting_allocator : public std::allocator<T>
{
};

namespace std
{

template <typename T>
class allocator_traits<counting_allocator<T>> /* hacky */ : public allocator_traits<std::allocator<T>>
{
public:
    typedef std::allocator_traits<std::allocator<T>> super;
    typedef counting_allocator<T> Alloc;

    static T* allocate(Alloc& a, size_t n, typename std::allocator_traits<super>::const_pointer hint = 0)
    {
        ++allocations;
        allocated_bytes += n * sizeof(T);
        return super::allocate(a, n, hint);
    }

    static void deallocate(Alloc& a, T* p, size_t n)
    {
        ++deallocations;
        deallocated_bytes += n * sizeof(T);
        return super::deallocate(a, p, n);
    }

    template< class U, class... Args >
    static void construct(Alloc& a, U* p, Args&&... args)
    {
        ++constructions;
        return super::construct(a, p, std::forward<Args>(args)...);
    }

    template< class U >
    static void destroy(Alloc& a, U* p)
    {
        ++destructions;
        return super::destroy(a, p);
    }

    static Alloc select_on_container_copy_construction(const Alloc& a)
    {
        return a;
    }
};

}

TEST_CASE("[small_vector] static")
{
    using namespace itlib;
    using namespace std;

    static_assert(sizeof(small_vector<void*, 10>) - sizeof(small_vector<void*, 3>) == sizeof(void*) * 7, "small_vector needs to have a static buffer");
    {
        small_vector<int, 10, 0, counting_allocator<int>> ivec;
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
        iret = ivec.insert(ivec.begin(), { 1, 2 });
        CHECK(iret == ivec.begin());

        int ints[] = { 1, 2, 53, 12, 90, 3, 17, 6 };
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

        const small_vector<int, 5, 0, counting_allocator<int>> ivec2 = { 1, 2, 3, 4 };
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

        small_vector<string, 11, 0, counting_allocator<string>> svec;
        svec.assign({ "as", "df" });
        CHECK(svec.size() == 2);
        string s1 = "the quick brown fox jumped over the lazy dog 1234567890";
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
        small_vector<char, 10, 10, counting_allocator<char>> cvec(s1.begin(), s1.end());
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
        small_vector<int, 4, 4> nullptr_test(2, 0);
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

    CHECK(allocations == 0);
    CHECK(deallocations == 0);
    CHECK(allocated_bytes == 0);
    CHECK(deallocated_bytes == 0);
    CHECK(constructions == destructions);

    constructions = destructions = 0;
}


TEST_CASE("[small_vector] dynamic")
{
    using namespace itlib;
    using namespace std;
    {
        small_vector<int, 1, 0, counting_allocator<int>> ivec;
        CHECK(ivec.size() == 0);
        CHECK(ivec.capacity() == 1);
        CHECK(ivec.begin() == ivec.end());
        CHECK(ivec.cbegin() == ivec.cend());
        CHECK(ivec.empty());

        auto d = ivec.data();
        ivec.reserve(2);
        CHECK(ivec.capacity() == 2);
        CHECK(d != ivec.data());
        CHECK(allocations == 1);

        ivec.resize(3, 8);
        CHECK(ivec.capacity() == 3);
        CHECK(ivec.size() == 3);
        CHECK(ivec.front() == 8);
        CHECK(ivec.back() == 8);
        CHECK(d != ivec.data());
        CHECK(allocations == 2);

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

        const small_vector<int, 1, 0, counting_allocator<int>> ivec2 = { 1, 2, 3, 4 };
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

        small_vector<string, 1, 0, counting_allocator<string>> svec;
        svec.assign({ "as", "df" });
        CHECK(svec.size() == 2);
        string s1 = "the quick brown fox jumped over the lazy dog 1234567890";
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
        small_vector<char, 1, 0, counting_allocator<char>> cvec(s1.begin(), s1.end());
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

    CHECK(allocations == deallocations);
    CHECK(allocated_bytes == deallocated_bytes);
    CHECK(constructions == destructions);

    allocations = deallocations = allocated_bytes = deallocated_bytes = constructions = destructions = 0;
}

TEST_CASE("[small_vector] static-dynamic")
{
    using namespace itlib;
    using namespace std;

    {
        small_vector<int, 5, 3, counting_allocator<int>> ivec;
        auto d = ivec.data();
        ivec.reserve(20);
        CHECK(ivec.data() == d);

        ivec.push_back(1);
        ivec.push_back(2);
        ivec.push_back(3);

        CHECK(ivec.data() == d);

        auto lastsize = ivec.size();
        auto iret = ivec.insert(ivec.end(), 3u, 8);
        CHECK(iret == ivec.begin() + lastsize);

        CHECK(ivec.size() == 6);
        CHECK(ivec.capacity() == 20);

        auto dd = ivec.data();

        ivec.erase(ivec.begin(), ivec.begin() + 6);
        CHECK(ivec.data() == d);
        CHECK(ivec.empty());

        ivec.resize(19, 11);
        CHECK(ivec.size() == 19);
        CHECK(ivec.capacity() == 20);
        CHECK(ivec.data() == dd);

        ivec.resize(4);
        CHECK(ivec.size() == 4);
        CHECK(ivec.capacity() == 20);
        CHECK(ivec.data() == dd);

        ivec.revert_to_static();
        CHECK(ivec.size() == 4);
        CHECK(ivec.capacity() == 5);
        CHECK(ivec.data() == d);

        ivec.reserve(10);
        CHECK(ivec.size() == 4);
        CHECK(ivec.capacity() == 20);
        CHECK(ivec.data() == dd);

        ivec.shrink_to_fit();
        CHECK(ivec.size() == 4);
        CHECK(ivec.capacity() == 5);
        CHECK(ivec.data() == d);

        ivec.reserve(10);
        CHECK(ivec.size() == 4);
        CHECK(ivec.capacity() == 10);
        CHECK(ivec.data() != d);

        dd = ivec.data();
        iret = ivec.insert(ivec.begin() + 3, 5u, 88);
        CHECK(ivec.size() == 9);
        CHECK(ivec.capacity() == 10);
        CHECK(ivec.data() == dd);
        CHECK(ivec[2] == 11);
        CHECK(ivec[7] == 88);
        CHECK(ivec[8] == 11);
        CHECK(iret == ivec.begin() + 3);

        small_vector<int, 3, 4, counting_allocator<int>> ivec2(ivec.begin(), ivec.end());
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
        CHECK(ivec.data() == d);
        CHECK(eret == ivec.end());

        eret = ivec2.erase(ivec2.begin() + 1, ivec2.end() - 2);
        CHECK(ivec2.size() == 3);
        CHECK(ivec2.capacity() == 3);
        CHECK(eret == ivec2.begin() + 1);
    }

    CHECK(allocations == deallocations);
    CHECK(allocated_bytes == deallocated_bytes);
    CHECK(constructions == destructions);

    allocations = deallocations = allocated_bytes = deallocated_bytes = constructions = destructions = 0;
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
