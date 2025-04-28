#pragma once

#include <vector>
#include <array>
#include <cstdint>

int32_t binary_search_baseline(std::vector<int32_t> & elements, int32_t target) {
    int l = 0, r = elements.size() - 1;
    while (l <= r) {
        int m = (l + r) / 2;
        if (elements[m] == target) {
            return m;
        }
        else if (elements[m] < target) {
            l = m + 1;
        }
        else {
            r = m - 1;
        }
    }
    return -1;
}

int32_t binary_search_std(std::vector<int32_t> & elements, int32_t target) {
    auto it = std::lower_bound(elements.begin(), elements.end(), target);
    if (it != elements.end() && *it == target) {
        return *it;
    }
    return -1; // Return -1 if target is not found
}

int32_t binary_search_branchless(std::vector<int32_t> & elements, int32_t target) {
    int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        if (base[half - 1] < target) {
            base += half;
            len = len - half;
        }
        else {
            len = half;
        }
    }
    return (base - elements.data());
}

int32_t binary_search_opt1_branchless(std::vector<int32_t> & elements, int32_t target) {
    int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        if (base[half - 1] < target) {
            base += half;
        }
        len -= half;
    }
    return (base - elements.data());
}

int32_t binary_search_opt2_branchless2(std::vector<int32_t> & elements, int32_t target) {
    int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        base += (base[half - 1] < target) * half;
        len -= half;
    }
    return (base - elements.data());
}

int32_t binary_search_opt3_prefetch(std::vector<int32_t> & elements, int32_t target) {
    int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        __builtin_prefetch(&base[len / 2 - 1]);
        __builtin_prefetch(&base[half + len / 2 - 1]);
        base += (base[half - 1] < target) * half;
        len -= half;
    }
    return (base - elements.data());
}

void recursive_transformation(std::vector<int32_t> & result, const std::vector<int32_t> & elements, int32_t & i, int32_t k) {
    if(k < elements.size()) {
        recursive_transformation(2 * k);
        result[k] = elements[i++];
        recursive_transformation(2 * k + 1);
    }
}

std::vector<int32_t> eytzinger_transformation(const std::vector<int32_t> & elements) {
    std::vector<int32_t> result(elements.size() + 1);
    // the result array in 1-indexed for performance consideration
    int32_t i = 0;
    recursive_transformation(result, elements, i, 1);
    return result;
}