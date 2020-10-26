#include "doctest.hpp"

#include <itlib/time_t.hpp>

TEST_CASE("[time_t] basic and arithmetic")
{
    {
        itlib::time_t t;
        CHECK(t.seconds_since_epoch() == 0);
    }

    const auto ms1800 = std::chrono::milliseconds(1800);

    itlib::time_t t(100);

    auto sum = t + ms1800;
    CHECK(sum.seconds_since_epoch() == 101);
    CHECK(sum > t);
    CHECK(sum >= t);
    sum = t - ms1800;
    CHECK(sum.seconds_since_epoch() == 99);
    CHECK(sum < t);
    CHECK(sum <= t);
    auto tt = sum;
    sum += ms1800;
    CHECK(sum == t);
    CHECK(sum <= t);
    CHECK(sum >= t);
    sum -= ms1800;
    CHECK(sum == tt);
    CHECK(sum <= tt);
    CHECK(sum >= tt);

    itlib::time_t t2(200);

    auto diff = t2 - t;
    CHECK(diff == std::chrono::seconds(100));
}

TEST_CASE("[time_t] tm")
{
    itlib::time_t t(1000100000);

    auto tm = t.gmtime();
    CHECK(tm.tm_sec == 20);
    CHECK(tm.tm_min == 33);
    CHECK(tm.tm_hour == 5);
    CHECK(tm.tm_mday == 10);
    CHECK(tm.tm_mon == 8);
    CHECK(tm.tm_year == 101);
    CHECK(tm.tm_wday == 1);
    CHECK(tm.tm_isdst == 0);

    auto lt = t.localtime();
    CHECK(lt.tm_sec == 20);
    CHECK((lt.tm_min == 33 || lt.tm_min == 3));
    CHECK((lt.tm_mday = 8 || lt.tm_mday == 9));
    CHECK(lt.tm_mon <= 8);
    CHECK(lt.tm_year == 101);
    CHECK((lt.tm_wday == 0 || lt.tm_wday == 1));

    std::locale::global(std::locale("en_US.utf8"));
    auto fmt = itlib::strftime("%A %D %T", tm);
    CHECK(fmt.length() == 24);
    CHECK(fmt == "Monday 09/10/01 05:33:20");
}
