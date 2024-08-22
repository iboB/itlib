// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

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

#if !defined(__APPLE__) && !defined(__MINGW32__)
    // macs have some weird problem with std::locale
    // but I don't have the time (or patience) to debug it now :)
    std::locale::global(std::locale("en_US.UTF8"));
    auto fmt = itlib::strftime("%A %D %T", tm);
    CHECK(fmt.length() == 24);
    CHECK(fmt == "Monday 09/10/01 05:33:20");
#endif
}

TEST_CASE("[time_t] from/to tm")
{
    std::tm tm = {};
    tm.tm_year = 120;
    tm.tm_mon = 11;
    tm.tm_mday = 10;
    tm.tm_hour = 15;
    tm.tm_min = 30;
    tm.tm_sec = 30;

    CHECK(itlib::time_t::from_gmtime(tm).seconds_since_epoch() == 1607614230);

    itlib::time_t now = itlib::time_t::now();

    auto eq = [](const std::tm& a, const std::tm& b) {
        return a.tm_sec == b.tm_sec
            && a.tm_min == b.tm_min
            && a.tm_hour == b.tm_hour
            && a.tm_mday == b.tm_mday
            && a.tm_mon == b.tm_mon
            && a.tm_year == b.tm_year
            && a.tm_wday == b.tm_wday
            && a.tm_yday == b.tm_yday
            && a.tm_isdst == b.tm_isdst;
    };

    {
        auto gmtm = now.gmtime();
        auto gmtmc = gmtm;
        auto fromgm = itlib::time_t::from_gmtime(gmtm);
        CHECK(fromgm == now);
        CHECK(eq(gmtm, gmtmc)); // gmtm shouldn't be normalized it should be produced normalized by gmtime
    }

    {
        auto localtm = now.localtime();
        auto localtmc = localtm;
        auto fromlocal = itlib::time_t::from_localtime(localtm);
        CHECK(fromlocal == now);
        CHECK(eq(localtm, localtmc)); // localtm shouldn't be normalized it should be produced normalized by localtime
    }
}
