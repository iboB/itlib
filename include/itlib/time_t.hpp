// itlib-time_t v1.03
//
// A thin wrapper of std::time_t which provides thread safe std::tm getters and
// type-safe (std::chrono::duration-based) arithmetic
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2020-2026 Borislav Stanimirov
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files(the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and / or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions :
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//
//                  VERSION HISTORY
//
//  1.03 (2026-10-06) Reimplement strftime: correctly guard against valid
//                    non-empty formats which produce empty output
//  1.02 (2023-04-29) Fix MSVC warning for assignment in while
//  1.01 (2021-04-29) Added named ctors: now, from_gmtime, from_localtime
//  1.00 (2020-10-36) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines the class itlib::time_t which is a thin wrapper of std::time_t
//
// It provides multiplatform thread-safe ops to convert to std::tm
// * std::tm itlib::time_t::gmtime() const
// * std::tm itlib::time_t::localtime() const
//
// It also provides type-safe arithmetic
// * itlib::time_t operators +,-,+= and -= with std::chrono_duration
// * std::chrono_duration operator-(itlib::time_t a, itlib::time_t b)
//
// The file also defines the function
// std::string itlib::strftime(const char* fmt, const std::tm& tm);
// It works exactly as std::strftime but returns a std::string with the
// appropriate size
//
//                  TESTS
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once

#include <ctime>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>

namespace itlib
{

class time_t
{
public:
    using timestamp_type = int64_t;
    using duration_type = std::chrono::duration<timestamp_type>;

    time_t() = default;
    time_t(const time_t&) = default;
    time_t& operator=(const time_t&) = default;

    explicit time_t(const std::time_t& st)
    {
        m_t = static_cast<timestamp_type>(st);
    }
    explicit operator std::time_t() const
    {
        return static_cast<std::time_t>(m_t);
    }

    timestamp_type seconds_since_epoch() const { return m_t; }

    static time_t from_seconds(timestamp_type s) { return time_t(s); }

    static time_t now() { return from_seconds(std::time(nullptr)); }

    // non-const argument - gets normalized internally
    static time_t from_gmtime(std::tm& gmtm)
    {
        auto tt =
#ifdef _WIN32
            _mkgmtime(&gmtm);
#else
            timegm(&gmtm);
#endif
        return from_seconds(tt);
    }

    // non-const argument - gets normalized internally
    static time_t from_localtime(std::tm& localtm)
    {
        return from_seconds(mktime(&localtm));
    }

    // cmp
    friend bool operator==(const time_t& a, const time_t& b) { return a.m_t == b.m_t; }
    friend bool operator!=(const time_t& a, const time_t& b) { return a.m_t != b.m_t; }
    friend bool operator<(const time_t& a, const time_t& b) { return a.m_t < b.m_t; }
    friend bool operator<=(const time_t& a, const time_t& b) { return a.m_t <= b.m_t; }
    friend bool operator>(const time_t& a, const time_t& b) { return a.m_t > b.m_t; }
    friend bool operator>=(const time_t& a, const time_t& b) { return a.m_t >= b.m_t; }

    // arithmetic
    template <typename Rep, typename Period>
    time_t& operator+=(const std::chrono::duration<Rep, Period>& d) { m_t += dc(d); return *this; }
    template <typename Rep, typename Period>
    time_t operator+(const std::chrono::duration<Rep, Period>& d) const { return time_t(m_t + dc(d)); }
    template <typename Rep, typename Period>
    time_t& operator-=(const std::chrono::duration<Rep, Period>& d) { m_t -= dc(d); return *this;  }
    template <typename Rep, typename Period>
    time_t operator-(const std::chrono::duration<Rep, Period>& d) const { return time_t(m_t - dc(d)); }

    friend duration_type operator-(const time_t& a, const time_t& b) { return duration_type(a.m_t - b.m_t); }

    // ops
    std::tm gmtime() const
    {
        std::tm ret = {};
        auto mt = std::time_t(*this);
#if defined(_WIN32)
        gmtime_s(&ret, &mt);
#else
        gmtime_r(&mt, &ret);
#endif
        return ret;
    }

    std::tm localtime() const
    {
        std::tm ret = {};
        auto mt = std::time_t(*this);
#if defined(_WIN32)
        localtime_s(&ret, &mt);
#else
        localtime_r(&mt, &ret);
#endif
        return ret;
    }

private:
    template <typename Rep, typename Period>
    static timestamp_type dc(const std::chrono::duration<Rep, Period>& d)
    {
        return std::chrono::duration_cast<duration_type>(d).count();
    }

    timestamp_type m_t = 0;
};

inline std::string strftime(const char* format, const std::tm& tm)
{
    const auto flen = std::strlen(format);
    if (flen <= 1) {
        // empty or single char format string - no need to actually format anything
        return std::string(format, flen);
    }

    // now, the problem is: how to guard for valid non-empty formats, which lead to an empty output
    // since we allocate the result anyway, we can piggy-back on the allocation and write
    // the format string with a space suffix at the end

    // but first let's devise a buffer reserve strategy
    std::string ret;
    if (flen < 32) {
        // small format string: likely only format sequences and a handful of literals
        const auto initial_size = flen * 4;
        if (initial_size < ret.capacity()) {
            // no need to resize to anything smaller than capacity: it's zero allocations anyway
            ret.resize(ret.capacity());
        }
        else {
            ret.resize(initial_size);
        }
    }
    else {
        // large format string: likely a lot of literal text
        ret.resize(128 + flen + 1);
    }
    auto fmtcopy = &ret.back() - flen;
    std::memcpy(fmtcopy, format, flen);
    ret.back() = ' '; // guarantee 1 byte of output in a legit empty result

    auto len = std::strftime(&ret.front(), ret.size() - flen - 1, fmtcopy, &tm);
    if (len == 1) {
        // legit empty result
        return {};
    }
    if (len != 0) {
        // lucky! our buffer front was enough
        ret.resize(len - 1);
        return ret;
    }

    // now we know for sure that the buffer was too small, and we can do a safe resize loop with the original fmt

    for (;;) {
        ret.resize(2 * ret.size());
        len = std::strftime(&ret.front(), ret.size(), format, &tm);
        if (len != 0) {
            ret.resize(len);
            return ret;
        }
        if (ret.size() > flen * 256) {
            // some platforms return 0 for invalid format strings, so we need to guard against that
            // otherwise we could loop until we run out of memory
            // 256 times the format string is a generous limit, we figure
            return {};
        }
    }
}

}
