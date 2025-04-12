#pragma once

#include <cstdint>

struct Montgomery {
    uint64_t n, nr;
    
    Montgomery(uint64_t n) : n(n) {
        nr = 1;
        for (int i = 0; i < 6; i++)
            nr *= 2 - n * nr;
    }

    uint64_t reduce(__uint128_t x) const {
        uint64_t q = uint64_t(x) * nr;
        uint64_t m = ((__uint128_t) q * n) >> 64;
        return (x >> 64) + n - m;
    }

    uint64_t multiply(uint64_t x, uint64_t y) {
        return reduce((__uint128_t) x * y);
    }
};