#pragma once

#include <vector>
#include <array>
#include <cstdint>

const int32_t & binary_search_baseline(const std::vector<int32_t> & elements, int32_t target) {
    int l = 0, r = elements.size() - 1;
    while (l <= r) {
        int m = (l + r) / 2;
        if (elements[m] == target) {
            return elements[m];
        }
        else if (elements[m] < target) {
            l = m + 1;
        }
        else {
            r = m - 1;
        }
    }
    return elements[0];
}

const int32_t & binary_search_std(const std::vector<int32_t> & elements, int32_t target) {
    auto it = std::lower_bound(elements.begin(), elements.end(), target);
    return *it;
}

const int32_t & binary_search_opt1_branchless(const std::vector<int32_t> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        if (*(base + half - 1) < target) {
            base += half;
            len = len - half;
        }
        else {
            len = half;
        }
    }
    return *base;
}

const int32_t & binary_search_opt2_branchless2(const std::vector<int32_t> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        if (*(base + half - 1) < target) {
            base += half;
        }
        len -= half;
    }
    return *base;
}

const int32_t & binary_search_opt3_branchless3(const std::vector<int32_t> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        base += (*(base + half - 1) < target) * half;
        len -= half;
    }
    return *base;
}

const int32_t & binary_search_opt4_prefetch(const std::vector<int32_t> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        __builtin_prefetch(&base[len / 2 - 1]);
        __builtin_prefetch(&base[half + len / 2 - 1]);
        base += (*(base + half - 1) < target) * half;
        len -= half;
    }
    return *base;
}

/**
 * @brief Tranforme the sorted vector into eytzinger layout recursively.
    It takes a lot of time for me to comprehend this function.
    Let's capture some key points:
    1. This transformation write "elements.size()" elements without repetition
    2. For each node "k", when result [k] is set, the left sub-tree of this node must
    have already been set. Since we pop elements in the original sorted array sequentially,
    we can conlude that, for each node "result[k]", it's larger
    than the left sub-tree and smaller than the right sub-tree.
    Therefore the transformation is well-formed.
    It's really clever!
 */
void recursive_transformation(std::vector<int32_t> & result, const std::vector<int32_t> & elements, int32_t & i, int32_t k) {
    if(k < elements.size()) {
        recursive_transformation(result, elements, i, 2 * k);
        result[k] = elements[i++];
        recursive_transformation(result, elements, i, 2 * k + 1);
    }
}

std::vector<int32_t> eytzinger_transformation(const std::vector<int32_t> & elements) {
    std::vector<int32_t> result(elements.size() + 1);
    // the result array in 1-indexed for performance consideration
    int32_t i = 0;
    recursive_transformation(result, elements, i, 1);
    return result;
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
const int32_t & binary_search_opt5_eytzinger(const std::vector<int32_t> & elements_eytzinger, int32_t target) {

    int32_t k = 1;
    while(k < elements_eytzinger.size()) {
        if(elements_eytzinger[k] == target) {
            return elements_eytzinger[k];
        }
        if(elements_eytzinger[k] < target) {
            k = 2 * k + 1;
        } else {
            k = 2 * k;
        }
    }
    return elements_eytzinger[k];
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
const int32_t & binary_search_opt6_eytzinger_branchless(const std::vector<int32_t> & elements_eytzinger, int32_t target) {
    int32_t k = 1;
    while(k < elements_eytzinger.size()) {
        // avoid branch. But also leads to a problem:
        // We don't know which k leads to the target value!
        k = 2 * k + (elements_eytzinger[k] < target);
    }
    // Key observation : If the target has already been matched
    // at this round "(elements_eytzinger[k] < target) == 0", we will go left
    // But at the following rounds we must go right since in the left sub-tree of the target node
    // there's nothing larger than the target.
    // So the pattern of the final steps is : one step left, multiple steps right.
    // Therefore we can count the trailing ones to get back the matched "k".
    k >>= __builtin_ffs(~k);
    // This is AMAZAING!!
    // for each iteration, the binary representation of k is appended with "bit 0" if we go left
    // and appended with "bit 1" if we go right.
    // And we need to get rid of the redundant bits since "k" matches the target.
    // Then we use __builtin_ffs to find the first 1 from the LSB in "~k".
    // And we right shift "k" by that value to retrieve the answer.
    return elements_eytzinger[k];
}