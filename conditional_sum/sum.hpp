#pragma once

#include <vector>
#include <cstdint>

template <int32_t UpperBound>
int32_t sum_baseline(const std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        if(e < UpperBound) {
            sum += e;
        }
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication(const std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += (e < UpperBound) * e;
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication_tenary(const std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += ((e < UpperBound) ? e : 0);
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication_masking(const std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += ((~((e < UpperBound) - 1))) & e;
    }
    return sum;
}