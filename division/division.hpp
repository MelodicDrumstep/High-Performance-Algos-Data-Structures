#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>

// type of the return result of the division function
struct DivResult {
    uint32_t quotient;
    uint32_t remainder;
};

/**
 * @brief baseline, normal operator
 */
DivResult division_baseline(uint32_t a, uint32_t b) {
    return {a / b, a % b};
}

/**
 * @brief in case of the compiler generate two "div" instruction for
    "/" and "%", I try to memorize the quotient result
    and try to see the change in the assembly code
 */
DivResult division_baseline2(uint32_t a, uint32_t b) {
    uint32_t quotient = a / b;
    return {quotient, a - b * quotient};
}

/**
 * @brief Barrett reduction for 32-bit division.
 * The core concept is to convert "division" into "multiplication".
 */
DivResult division_Barrett_reduction(uint32_t a, uint32_t b) {
    // Step 1: Precompute m = floor(2^64 / b)
    uint64_t m = (static_cast<uint64_t>(-1)) / b;

    // Step 2: Compute approximate quotient (q = floor((a * m) / 2^64))
    uint32_t q = (static_cast<__uint128_t>(a) * m) >> 64;

    // Step 3: Compute remainder (r = a - q * b)
    uint32_t r = a - q * b;

    // Step 4: Adjust remainder if necessary (r >= b)
    if (r >= b) {
        q += 1;
        r -= b;
    }
    return {q, r};
}

DivResult division_Barrett_reduction_precompute(uint32_t a, uint64_t b) {
    // Step 1: Precompute m = floor(2^64 / b)
    static uint64_t m = (static_cast<uint64_t>(-1)) / b;
    // I lazily use static variable here and it's dangerous to use it like this in real world
    // Only for testing purposes!

    // Step 2: Compute approximate quotient (q = floor((a * m) / 2^64))
    uint32_t q = (static_cast<__uint128_t>(a) * m) >> 64;

    // Step 3: Compute remainder (r = a - q * b)
    uint32_t r = a - q * b;

    // Step 4: Adjust remainder if necessary (r >= b)
    if (r >= b) {
        q += 1;
        r -= b;
    }
    return {q, r};
}

/**
 * @brief Lemire reduction for 32-bit division.
 * The intuition is similar to Barrett reduction, but it's more concise and efficient.
 * We can convert "a / b" into "a * m / 2^k". And we take k == 64 for uint32_t case. 
 * Then we can compute m as ceil(2^{64} / b), i.e. (2^{64} - 1) / b + 1.
 */
DivResult division_Lemire_reduction(uint32_t a, uint32_t b) {
    uint64_t m = static_cast<uint64_t>(-1) / b + 1;
    // ceil(2^64 / b)

    uint32_t quotient = (static_cast<__uint128_t>(m) * a) >> 64;
    uint32_t remainder = (static_cast<__uint128_t>(m * a) * b) >> 64;
    return {quotient, remainder};
}

DivResult division_Lemire_reduction_precompute(uint32_t a, uint32_t b) {
    static uint64_t m = static_cast<uint64_t>(-1) / b + 1;
    // ceil(2^64 / b)

    uint32_t quotient = (static_cast<__uint128_t>(m) * a) >> 64;
    uint32_t remainder = (static_cast<__uint128_t>(m * a) * b) >> 64;
    return {quotient, remainder};
}