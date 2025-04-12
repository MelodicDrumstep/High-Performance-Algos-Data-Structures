#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <bitset>
#include <random>

#include "constexpr_bitmap.hpp"

int randint(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(a, b);
    return dis(gen);
}

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
 *  let m = (2^64 - 1) / y + 1
 *  Then mod(x, y) = (m * x * y) >> 64
 */
struct PrecalculationPrimeTableLemire {
    constexpr static int64_t N = (1 << 16);
    std::array<uint64_t, 6542> primes_magic;
    // number of primes in [2, (1 << 16)]

    constexpr PrecalculationPrimeTableLemire() : primes_magic{} {
        ConstexprBitmap<N> prime_bitmap;
        // Unfortunately, std::bitset is not convenient for compile-time computing
        // So I write one constexpr bitmap
        // to reduce the compile-time memory overhead
        int32_t next_prime_idx = 0;
        for(int32_t i = 2; i < N; i++) {
            if(!prime_bitmap.test(i)) {
                primes_magic[next_prime_idx++] = static_cast<uint64_t>(-1) / i + 1;
                // we want to avoid integer division instruction overhead
                // Then we can store the reciprocal of the prime numbers rather than the prime number themselves
                // and we just store (2^64 - 1) / prime + 1
                for(int32_t j = 2 * i; j < N; j += i) {
                    prime_bitmap.set(j);
                }
            }
        }
    }
};

uint64_t find_factor_prime_table_lemire(uint64_t n) {
    static constexpr PrecalculationPrimeTableLemire prime_magic_table;
    for(uint64_t prime_magic : prime_magic_table.primes_magic) {
        if(prime_magic * n < prime_magic) {
            // the condition is "n % prime == 0"
            // prime_magic = (2^64 - 1)/ prime + 1
            return static_cast<uint64_t>(-1) / prime_magic + 1;
            // retrieve the prime number by (2^64 - 1) / prime_magic + 1
        }
    }
    return 1;
}

/**
 * @brief A random algorithm that may return 1 when n is factorizable.
 */
uint64_t find_factor_Pollard_Pho(uint64_t n) {
    auto gcd = [](int32_t a, int32_t b) {
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
    };

    // We can run a multi
    // constexpr int32_t TestTimes = 10;
    // for(int32_t i = 0; i < TestTimes; i++) {
        uint64_t c = randint(1, n - 1);
        auto f = [n, c](uint64_t x) {
            return (static_cast<__uint128_t>(x) * x + c) % n;
        };
        uint64_t x = f(0);
        uint64_t y = f(f(0));
        while(x != y) {
            uint64_t g = gcd(abs(x - y), n);
            if(g > 1) {
                return g;
            }
            x = f(x);
            y = f(f(y));
            // tortise and hare algorithm to determine if
            // there's a cycle
        }
    // }
    return 1;
}