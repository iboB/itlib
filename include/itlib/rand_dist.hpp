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
    static U roll(U max, R& rng) noexcept {
        using r_t = typename R::result_type;
        static_assert(std::is_unsigned_v<r_t>, "random engine result_type must be unsigned");
        constexpr r_t rng_range = R::max() - R::min();

        if constexpr (rng_range < std::numeric_limits<U>::max()) {
            // desired max might be bigger than rng's range
            // check if it fits
            // note that the elso condition is compatible and will work either way,
            // we still check here to potentially save the extra multiplication by `multiplier` (which would be 1)
            if (max <= U(rng_range)) {
                // it fits
                return roll_with_rejection(max, rng);
            }
            else {
                // we must roll multiple times

                // we represent max as a number in base (rng_range + 1)
                // we roll digit by digit, keeping track if we are within the limit
                // if we are, we consrain the last (highest) digit more
                constexpr U base = U(rng_range) + 1;
                U result = 0;
                U multiplier = 1;
                bool over_limit = false;
                while (max >= base) {
                    const U m_digit = max % base;
                    max /= base;
                    U r_digit = range_roll(rng);
                    result += multiplier * r_digit;
                    multiplier *= base;
                    if (!over_limit && r_digit > m_digit) {
                        // rolled digit is too big, must re-roll everything
                        over_limit = true;
                    }
                }
                // adjust max for highest digit
                max -= over_limit;
                // and roll
                result += multiplier * roll_with_rejection(max, rng);

                return result;
            }
        }
        else {
            return roll_with_rejection(max, rng);
        }
    }

    template <typename R>
    U operator()(R& rng) const noexcept {
        return roll(m_max, rng);
    }

private:
    template <typename R>
    static auto range_roll(R& rng) noexcept -> typename R::result_type {
        return rng() - R::min();
    }

    // rejection sample rng ASSUMING max is within R's range
    template <typename R>
    static U roll_with_rejection(U umax, R& rng) noexcept {
        if (umax == 0) return 0;

        using r_t = typename R::result_type;
        constexpr r_t rng_range = R::max() - R::min();
        const auto max = r_t(umax);

        if (rng_range == max) {
            return range_roll(rng);
        }

        const auto reject_limit = rng_range - (rng_range % (max + 1));
        while (true) {
            const auto v = range_roll(rng);
            if (v < reject_limit) {
                return v % (max + 1);
            }
        }
    }

    const U m_max;
};

template <typename I>
struct uniform_int_distribution {
    static_assert(std::is_integral_v<I>, "integral type required");
    using result_type = I;

    using UT = std::make_unsigned_t<I>;

    constexpr uniform_int_distribution(I min, I max) noexcept
        : m_min(UT(min))
        , m_max(UT(max) - UT(min))
    {}

    constexpr explicit uniform_int_distribution(I max = std::numeric_limits<I>::max()) noexcept
        : uniform_int_distribution(0, max)
    {}

    constexpr I min() const noexcept { return I(m_min); }
    constexpr I max() const noexcept { return I(m_max.max()); }

    template <typename R>
    static I roll(I min, I max, R& rng) noexcept {
        const auto umax = UT(max) - UT(min);
        return I(min + uniform_uint_max_distribution<UT>::roll(umax, rng));
    }

    template <typename R>
    I operator()(R& rng) const noexcept {
        return I(m_min + m_max(rng));
    }

private:
    const UT m_min;
    const uniform_uint_max_distribution<UT> m_max;
};

} // namespace itlib
