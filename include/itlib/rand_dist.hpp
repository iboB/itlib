// itlib-rand_dist v1.00 alpha
//
// Alternative random distributions compatible with std::random
//
// SPDX-License-Identifier: MIT
// MIT License:
// Copyright(c) 2025 Borislav Stanimirov
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
//  1.00 (2025-10-31) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines several random distributions to extend the existing standard.
//
// In the list of distributions they can be tagged in several ways:
// * new  - not present in the standard library
// * alt  - alternative drop-in replacement for a standard distribution
// * det  - deterministic - produce a predictable sequence of values when used
//          with a deterministic random engine
// * pure - does not contain a mutable state: usable from multiple threads as
//          long as the RNG is used in a thread-safe manner (e.g. thread_local
//          RNGs)
//
// Distributions:
//
// uniform_uint_max_distribution<U> (new, det, pure)
//      Uniform distribution for unsigned integers in the range [0, max]
//
// uniform_int_distribution<I> (alt, det, pure)
//      Uniform distribution for integers in the range [min, max]
//
// fast_uniform_real_distribution<F> (new, pure)
//      Uniform distribution for floating point numbers in the range [min, max)
//      Guaranteed to draw a single value from the RNG
//      Note that it's not deterministic in the same sense as the others, as it
//      relies on floating point arithmetic which may have produce different
//      results depending on compiler options.
//      It is "almost" deterministic in that it will produce the same output
//      sequence when run without -ffast-math or similar optimizations.
//
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cmath>

// the standard requires random engines to have constexpr min() and max()
// if you want to use these distributions with unorthodox non-constexpr engines,
// define this macro to empty before including this file
#if !defined(ITLIB_RAND_DIST_CONSTEXPR)
#define ITLIB_RAND_DIST_CONSTEXPR constexpr
#endif

namespace itlib {

namespace impl {
template <typename R>
constexpr void do_rng_checks(R&) {
    static_assert(std::is_unsigned_v<typename R::result_type>, "random engine result_type must be unsigned");
    static_assert(R::max() > R::min(), "random engine must have non-zero range");

    // why the fuck is std::mersenne_twister_engine::operator() noexcept(false)??
    // static_assert(noexcept(std::declval<R&>()()), "random engine operator() must be noexcept");
}

} // namespace impl

// uniform distribution for unsigned integers in [0, max]
template <typename U = uint32_t>
struct uniform_uint_max_distribution {
    static_assert(std::is_unsigned_v<U>, "unsigned integer type required");
    using result_type = U;

    constexpr explicit uniform_uint_max_distribution(U max = std::numeric_limits<U>::max()) noexcept
        : m_max(max)
    {}

    constexpr U min() const noexcept { return 0; }
    constexpr U max() const noexcept { return m_max; }

    template <typename R>
    constexpr static U draw(U max, R& rng) {
        impl::do_rng_checks(rng);
        using r_t = typename R::result_type;
        ITLIB_RAND_DIST_CONSTEXPR r_t rng_range = R::max() - R::min();

        if ITLIB_RAND_DIST_CONSTEXPR(rng_range < std::numeric_limits<U>::max()) {
            // desired max might be bigger than rng's range
            if (max <= U(rng_range)) {
                // it fits
                return draw_in_rng_range(max, rng);
            }
            else {
                // we must draw multiple times

                // we represent max as a number in base (rng_range + 1)
                // we draw digit by digit, keeping track if we are within the limit
                // if we go outside the limit, we reject and stat over

                ITLIB_RAND_DIST_CONSTEXPR U base = U(rng_range) + 1;

                // collect base_(rng_range+1) digits of max
                // array size enough even if rng_range is 2
                // (we *could* be fancy and compute this more precisely, but stack is cheap)
                r_t max_digits[sizeof(U) * 8];
                int max_digit_count = 0;
                {
                    U temp = max;
                    while (temp > 0) {
                        max_digits[max_digit_count++] = r_t(temp % base);
                        temp /= base;
                    }
                }

                // rejection loop
                while (true) {
                    // start with most significant digit
                    U result = 0;
                    bool tight = true;
                    for (int i = max_digit_count - 1; ; --i) {
                        const auto random_digit = rng_range_draw(rng); // unconstrained draw

                        if (tight) {
                            if (random_digit > max_digits[i]) {
                                break;
                            }
                            if (random_digit < max_digits[i]) {
                                tight = false; // we are now free to choose any digit
                            }
                        }

                        result = result * base + U(random_digit);

                        if (i == 0) {
                            // done
                            return result;
                        }
                    }
                }
            }
        }
        else {
            // rng range is guaranteed to cover desired max
            return draw_in_rng_range(max, rng);
        }
    }

    template <typename R>
    constexpr U operator()(R& rng) const {
        return draw(m_max, rng);
    }

private:
    template <typename R>
    constexpr static auto rng_range_draw(R& rng) -> typename R::result_type {
        return rng() - R::min();
    }

    // draw from rng ASSUMING max <= rng_range!!!
    // use rejection sampling to avoid modulo bias
    template <typename R>
    constexpr static U draw_in_rng_range(U umax, R& rng) {
        if (umax == 0) return 0;

        using r_t = typename R::result_type;
        ITLIB_RAND_DIST_CONSTEXPR r_t rng_range = R::max() - R::min();
        const auto max = r_t(umax);

        if (rng_range == max) {
            return U(rng_range_draw(rng));
        }

        // after the check above, now we have a gurantee that max < rng_range

        // this means that result_range_size will not overflow and reject_count will not underflow
        const auto result_range_size = r_t(max + 1);

        // mathematically we should use just rng_range + 1 here, but if rng_range is limits<r_t>::max(),
        // it will overflow and wrap to 0 leading to reject_count == 0 and no rejections ever.
        const auto reject_count = r_t(rng_range - result_range_size + 1) % result_range_size;

        const auto accept_max = r_t(rng_range - reject_count);
        while (true) {
            if (auto v = rng_range_draw(rng); v <= accept_max) {
                return U(v % result_range_size);
            }
            // food for thought:
            // is it worth checking for broken (malicious?) RNG that might lead to an infinite loop here?
        }
    }

    const U m_max;
};

template <typename I>
struct uniform_int_distribution {
    static_assert(std::is_integral_v<I>, "integral type required");
    using result_type = I;

    using U = std::make_unsigned_t<I>;

    constexpr uniform_int_distribution(I min, I max) noexcept
        : m_min(U(min))
        , m_range(U(max) - U(min)) // as per the standard: UB if max < min
    {}

    constexpr explicit uniform_int_distribution(I max = std::numeric_limits<I>::max()) noexcept
        : uniform_int_distribution(0, max)
    {}

    constexpr I min() const noexcept { return I(m_min); }
    constexpr I max() const noexcept { return I(U(m_min + m_range.max())); }

    template <typename R>
    constexpr static I draw(I min, I max, R& rng) {
        // multiple seemingly redundant casts to U to handle 8-bit types which auto-promote to int in expressions
        const auto umin = U(min);
        const auto urange = U(U(max) - umin); // as per the standard: UB if max < min
        return I(U(umin + uniform_uint_max_distribution<U>::draw(urange, rng)));
    }

    template <typename R>
    constexpr I operator()(R& rng) const {
        return I(U(m_min + m_range(rng)));
    }

private:
    const U m_min;
    const uniform_uint_max_distribution<U> m_range;
};

// return floating point in the range [min, max)
// guaranteed to draw a single value from the RNG (potentially at the cost of entropy or stretching)
template <typename F>
struct fast_uniform_real_distribution {
    static_assert(std::is_floating_point_v<F>, "floating point type required");
    static_assert(std::numeric_limits<F>::radix == 2, "only binary floating point types are supported");
    static_assert(std::numeric_limits<F>::digits <= 64, "max floating point precision must fit uint64_t");

    using result_type = F;

    constexpr fast_uniform_real_distribution(F min = F(0), F max = F(1)) noexcept
        : m_min(min)
        , m_scale(max - min)
    {}
    constexpr F min() const noexcept { return m_min; }
    constexpr F scale() const noexcept { return m_scale; }
    constexpr F max() const noexcept { return m_min + m_scale; }

    // draws from [0, 1)
    template <typename R>
    constexpr static F draw_01(R& rng) {
        impl::do_rng_checks(rng);

        // (1 << d) - 1 is more readable, but might overflow
        constexpr uint64_t max_int = ~uint64_t(0) >> (64 - std::numeric_limits<F>::digits);

        using r_t = typename R::result_type;
        ITLIB_RAND_DIST_CONSTEXPR uint64_t rng_range = R::max() - R::min();
        if ITLIB_RAND_DIST_CONSTEXPR(rng_range >= max_int) {
            // rng_range is enough to saturate our float precision
            // we slice off the needed bits (and hope that rng is uniform over all bits)
            const auto random_value = r_t(rng() - R::min()) & r_t(max_int);

            // ideally we would use this here:
            // return std::ldexp(F(random_value), -std::numeric_limits<F>::digits);
            // ... but in reality no compiler would would optimize it well enough
            // every single one does a `call ldexp[f]`!
            // at best they would do value * exp2[f](-digits) which is literally the same as below
            // well, not literally, as exp2 only gets an overload with C++23, but still...
            return F(random_value) / F(max_int + 1);
        }
        else {
            // "stretching" to rng_range
            // some F values are unreachable
            return F(rng() - R::min()) / (F(rng_range) + 1);
        }
    }

    template <typename R>
    constexpr static F draw(F min, F max, R& rng) {
        // see comment below about fma
        //return std::fma(max - min, draw_01(rng), min);
        return min + (max - min) * draw_01(rng);
    }

    template <typename R>
    constexpr F operator()(R& rng) const {
        // in the interest of precision we should use fma here:
        // return std::fma(m_scale, draw_01(rng), m_min);
        // unfortunately it's slower than simple multiply+add on most hardware
        // std::uniform_real_distribution uses multiply+add as well
        // we want to be fast, so we do it as well
        return m_min + m_scale * draw_01(rng);
    }

private:
    const F m_min;
    const F m_scale;
};

} // namespace itlib
