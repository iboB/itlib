// itlib-rand_dist v0.01 alpha
//
// Deterministic and pure random distributions compatible with std::random
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
//  0.01 (2025-10-24) Initial release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// It defines several random distribution types to extend the existing standard
//
//
// You can find unit tests in the official repo:
// https://github.com/iboB/itlib/blob/master/test/
//
#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

namespace itlib {

namespace impl {
template <typename R>
constexpr void do_rng_checks(R&) {
    static_assert(std::is_unsigned_v<typename R::result_type>, "random engine result_type must be unsigned");
    static_assert(R::max() > R::min(), "random engine must have non-zero range");
    static_assert(noexcept(std::declval<R&>()()), "random engine operator() must be noexcept");
};
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
    constexpr static U draw(U max, R& rng) noexcept {
        impl::do_rng_checks(rng);
        using r_t = typename R::result_type;
        constexpr r_t rng_range = R::max() - R::min();

        if constexpr (rng_range < std::numeric_limits<U>::max()) {
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

                constexpr U base = U(rng_range) + 1;

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
    constexpr U operator()(R& rng) const noexcept {
        return draw(m_max, rng);
    }

private:
    template <typename R>
    constexpr static auto rng_range_draw(R& rng) noexcept -> typename R::result_type {
        return rng() - R::min();
    }

    // draw from rng ASSUMING max <= rng_range!!!
    // use rejection sampling to avoid modulo bias
    template <typename R>
    constexpr static U draw_in_rng_range(U umax, R& rng) noexcept {
        if (umax == 0) return 0;

        using r_t = typename R::result_type;
        constexpr r_t rng_range = R::max() - R::min();
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
    constexpr static I draw(I min, I max, R& rng) noexcept {
        // multiple seemingly redundant casts to U to handle 8-bit types which auto-promote to int in expressions
        const auto umin = U(min);
        const auto urange = U(U(max) - umin); // as per the standard: UB if max < min
        return I(U(umin + uniform_uint_max_distribution<U>::draw(urange, rng)));
    }

    template <typename R>
    constexpr I operator()(R& rng) const noexcept {
        return I(U(m_min + m_range(rng)));
    }

private:
    const U m_min;
    const uniform_uint_max_distribution<U> m_range;
};

// return floating point in the range [-min, max)
// guaranteed to draw a single value from the RNG
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
    constexpr static F draw_01(R& rng) noexcept {
        impl::do_rng_checks(rng);

        // (1 << d) - 1 is more readable, but might overflow
        constexpr uint64_t max_int = ~uint64_t(0) >> (64 - std::numeric_limits<F>::digits);

        using r_t = typename R::result_type;
        constexpr uint64_t rng_range = R::max() - R::min();
        if constexpr (rng_range > max_int) {
            const auto random_value = r_t(rng() - R::min()) & r_t(max_int);
            return F(random_value) / F(max_int);
        }
        else {
            return F(rng() - R::min()) / F(rng_range);
        }
    }

    template <typename R>
    constexpr static F draw(F min, F max, R& rng) noexcept {
        return min + (max - min) * draw_01(rng);
    }

    template <typename R>
    constexpr F operator()(R& rng) const noexcept {
        return m_min + m_scale * draw_01(rng);
    }

private:
    const F m_min;
    const F m_scale;
};

} // namespace itlib
