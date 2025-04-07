#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <cstdint>

int32_t gcd_baseline_recursion(int32_t a, int32_t b) {
    if(b == 0) {
        return a;
    }
    return gcd_baseline_recursion(b, a % b);
}

int32_t gcd_baseline_loop(int32_t a, int32_t b) {
    while(b > 0) {
        a %= b;
        std::swap(a, b);
    }
    return a;
}

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