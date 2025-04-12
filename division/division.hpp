#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>

struct DivResult {
    uint32_t quotient;
    uint32_t remainder;
};

DivResult division_baseline(uint32_t a, uint32_t b) {
    return {a / b, a % b};
}

DivResult division_baseline2(uint32_t a, uint32_t b) {
    uint32_t quotient = a / b;
    return {quotient, a - b * quotient};
}

// Barrett reduction for 32-bit division
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

DivResult division_Lemire_reduction(uint32_t a, uint32_t b) {
    uint64_t m = static_cast<uint64_t>(-1) / b + 1;
    // ceil(2^64 / y)

    uint32_t quotient = (static_cast<__uint128_t>(m) * a) >> 64;
    uint32_t remainder = (static_cast<__uint128_t>(m * a) * b) >> 64;
    return {quotient, remainder};
}