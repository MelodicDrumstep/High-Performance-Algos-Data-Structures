#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>

/**
 * @brief Naive version, just use recursion.
 *  gcd(a, 0) == a
 *  gcd(a, b) == gcd(b, a % b)
 */
int32_t gcd_baseline_recursion(int32_t a, int32_t b) {
    if(b == 0) {
        return a;
    }
    return gcd_baseline_recursion(b, a % b);
}

/**
 * @brief Use loop rather than recursion.
 */
int32_t gcd_baseline_loop(int32_t a, int32_t b) {
    while(b > 0) {
        a %= b;
        std::swap(a, b);
    }
    return a;
}

/**
 * @brief Really smart observation.
 * 1. gcd(0, b) == b, gcd(a, 0) == a
 * 2. gcd(2a, 2b) == 2gcd(a, b)
 * 3. gcd(2a, b) == gcd(a, b) if b is odd, gcd(a, 2b) == gcd(a, b) if a is odd
 * 4. gcd(a, b) == gcd(|a - b|, min(a, b)) if a and b are both odd
 */
int32_t gcd_binary(int32_t a, int32_t b) {
    if(a == 0) {
        return b;
    }
    if ((b == 0) || (a == b)) {
        return a;
    }
    if(a % 2 == 0) {
        if(b % 2 == 0) {
            return 2 * gcd_binary(a / 2, b / 2);
        }
        return gcd_binary(a / 2, b);
    } else {
        if(b % 2 == 0) {
            return gcd_binary(a, b / 2);
        }
        return gcd_binary(std::abs(a - b), std::min(a, b));
    }
}

/**
 * @brief Actually the last version is not fast. Too many branches left there.
 * We can have some insights:
 * 1. If we need to divide the input by 2(right shift by 1) for many times, we 
 * can merge them together to avoid overhead. We can do this by using "__builtin_ctz" instruction
 * to retrieve the trailing zeros in the element. And right shift the corresponding bits.
 * 2. Actually the second codition "gcd(2a, 2b) == 2gcd(a, b)" can only happen at the beginning,
 * because after we merge the divisions, either a or b will become odd. Then we can do the merged
 * division and the beginning before entering the loop. And inside the loop both a and b will be odd.
 * 3. When both a and b are odd, min(a, b) is odd too. Therefore if we assign a = |a - b|, b = min(a, b),
 * only a can become even at this time. Then we just retrieve the trailing zeros and convert a to odd again.
 * We keep looping until a become zero.
 */
int32_t gcd_binary_opt1(int32_t a, int32_t b) {
    if(a == 0) {
        return b;
    }
    if(b == 0) {
        return a;
    }
    int32_t az = __builtin_ctz(a);
    int32_t bz = __builtin_ctz(b);
    // retrieve the trailing zeros.
    // It's the minimum times we need for the original a and b to be divided by 2
    // to turn into odd.
    int32_t shift = std::min(az, bz);
    // shift variable indicates that the result needs to multiply by "2^{shift}"
    a >>= az;
    b >>= bz;
    // a and b are both odd now

    while(a != 0) {
        int32_t diff = a - b;
        b = std::min(a, b);
        a = std::abs(diff);
        a >>= __builtin_ctz(a);
        // a becomes odd now
    }

    return b << shift;
    // multiply by "2^{shift}"
    // Notice that all multiplication and division is performed by shifting,
    // which is fast
}

/**
 * @brief A really smart optimization for "gcd_binary_opt2".
 * Notice there's a data hazard for "a >>= __builtin_ctz(a);"
 * And we can mitigate this by using "a >>= __builtin_ctz(diff);" instead!
 * It yields the same result but erasing the hazard.
 */
int32_t gcd_binary_opt2(int32_t a, int32_t b) {
    if(a == 0) {
        return b;
    }
    if(b == 0) {
        return a;
    }
    int32_t az = __builtin_ctz(a);
    int32_t bz = __builtin_ctz(b);
    int32_t shift = std::min(az, bz);
    a >>= az;
    b >>= bz;

    while(a != 0) {
        int32_t diff = a - b;
        b = std::min(a, b);
        a = std::abs(diff);
        a >>= __builtin_ctz(diff);
    }

    return b << shift;
}