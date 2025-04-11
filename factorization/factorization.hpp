#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <bitset>

#include "constexpr_bitmap.hpp"

uint64_t find_factor_baseline(uint64_t n) {
    for(uint64_t d = 2; d < n; d++) {
        if(n % d == 0) {
            return d;
        }
    }
    return 1;
}

uint64_t find_factor_brute_pruning(uint64_t n) {
    for(uint64_t d = 2; d * d <= n; d++) {
        if(n % d == 0) {
            return d;
        }
    }
    return 1;
}

template <int32_t N = (1 << 16)>
struct PrecalculationLookupTable {
    std::array<uint8_t, N> divisor;

    constexpr PrecalculationLookupTable() : divisor {} {
        for(int32_t i = 0; i < N; i++) {
            divisor[i] = 1;
        }
        for(int32_t i = 2; i * i < N; i++) {
            if(divisor[i] == 1) {
                // no need to enter the loop if
                // divisor[i] != 1, which indicates that
                // it's divisor has modifies its value
                // and the divisor value of the 
                // multiples of this number has all
                // been updated.
                for(int32_t k = i * i; k < N; k += i) {
                    // Why starting from i * i rather than i * 2?
                    // because, remember that "i >= 2" right?
                    // imagine "i == 5",
                    // then i * 2, i * 3, i * 4 has already
                    // been marked by 2, 3, 4
                    // Then we only need to start from i * i.
                    divisor[k] = i;
                }
            }
        }
    }
};

uint64_t find_factor_lookup_table(uint64_t n) {
    static constexpr PrecalculationLookupTable<> P {};
    return P.divisor[n];
}

uint64_t find_factor_wheel(uint64_t n) {
    if(n % 2 == 0) {
        return 2;
    }
    for(uint64_t d = 3; d * d <= n; d += 2) {
        if(n % d == 0) {
            return d;
        }
    }
    return 1;
}

uint64_t find_factor_wheel2(uint64_t n) {
    for(uint64_t d : {2, 3, 5}) {
        if(n % d == 0) {
            return d;
        }
    }
    constexpr uint64_t offsets[] = {0, 4, 6, 10, 12, 16, 22, 24};
    for(uint64_t d = 7; d * d <= n; d += 30) {
        for(uint64_t offset : offsets) {
            uint64_t x = d + offset;
            if(n % x == 0) {
                return x;
            }
        }
    }
    return 1;
}

struct PrecalculationPrimeTable {
    constexpr static int64_t N = (1 << 16);
    std::array<uint16_t, 6542> primes;
    // number of primes in [2, (1 << 16)]

    constexpr PrecalculationPrimeTable() : primes{} {
        ConstexprBitmap<N> prime_bitmap;
        // Unfortunately, std::bitset is not convenient for compile-time computing
        // So I write one constexpr bitmap
        // to reduce the compile-time memory overhead
        int32_t next_prime_idx = 0;
        for(int32_t i = 2; i < N; i++) {
            if(!prime_bitmap.test(i)) {
                primes[next_prime_idx++] = i;
                for(int32_t j = 2 * i; j < N; j += i) {
                    prime_bitmap.set(j);
                }
            }
        }
    }
};

uint64_t find_factor_prime_table(uint64_t n) {
    static constexpr PrecalculationPrimeTable prime_table;
    for(uint16_t prime : prime_table.primes) {
        if(n % prime == 0) {
            return prime;
        }
    }
    return 1;
}

/**
 * Lemire Reduction is a way that we can accelerate integer division / modulo operation
 * floor(x / y) = floor((x \cdot m) / 2^s) = floor((x \cdot ceil(2^s / y)) / 2^s)
 * r = floor(((x \cdot ceil(2^s / y) mod 2^s) \cdot y) / 2^s)
 *  And we take s = 64 for uint64_t
 * Then mod(x, y) = () >> 64;
 */
struct PrecalculationPrimeTableLemire {
    constexpr static int64_t N = (1 << 16);
    std::array<uint16_t, 6542> primes_magic;
    // number of primes in [2, (1 << 16)]

    constexpr PrecalculationPrimeTableLemire() : primes_magic{} {
        ConstexprBitmap<N> prime_bitmap;
        // Unfortunately, std::bitset is not convenient for compile-time computing
        // So I write one constexpr bitmap
        // to reduce the compile-time memory overhead
        int32_t next_prime_idx = 0;
        for(int32_t i = 2; i < N; i++) {
            if(!prime_bitmap.test(i)) {
                primes_magic[next_prime_idx++] = uint64_t(-1) / i + 1;
                for(int32_t j = 2 * i; j < N; j += i) {
                    prime_bitmap.set(j);
                }
            }
        }
    }
};

uint64_t find_factor_prime_table_lemire(uint64_t n) {
    static constexpr PrecalculationPrimeTableLemire prime_magic_table;
    for(uint16_t prime_magic : prime_magic_table.primes_magic) {
        if(prime_magic * n < prime_magic) {
            // figure out why
            return uint64_t(-1) / prime_magic + 1;
        }
    }
    return 1;
}