#pragma once

#include <vector>
#include <cstdint>

template <int32_t UpperBound>
int32_t sum_baseline(std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        if(e < UpperBound) {
            sum += e;
        }
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication(std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += (e < UpperBound) * e;
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication_tenary(std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += ((e < UpperBound) ? e : 0);
    }
    return sum;
}

template <int32_t UpperBound>
int32_t sum_predication_masking(std::vector<int32_t> & elements) {
    int32_t sum = 0;
    for(auto e : elements) {
        sum += (((e < UpperBound) >> 31) - 1) & e;
    }
    return sum;
}