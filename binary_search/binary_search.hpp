#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <functional>
#include <optional>
#include <concepts>
#include <type_traits>

#include "aligned_allocator.hpp"

// Use the aligned allocator for better performance with SIMD
using AlignedVector = std::vector<int32_t, AlignedAllocator<int32_t>>;

template <bool Aligned>
using VecType = std::conditional_t<Aligned, AlignedVector, std::vector<int32_t>>;

// template <typename T>
// concept VecT = std::is_same_v<std::vector<int32_t>, T> || std::is_same_v<AlignedVector, T>;

template <typename T>
using OptRef = std::optional<std::reference_wrapper<T>>;

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_baseline(const VecType<Aligned> & elements, int32_t target) {
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
    return std::nullopt;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_std(const VecType<Aligned> & elements, int32_t target) {
    auto it = std::lower_bound(elements.begin(), elements.end(), target);
    if(it == elements.end()) {
        return std::nullopt;
    }
    return *it;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt1_branchless(const VecType<Aligned> & elements, int32_t target) {
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
    if (*base != target) {
        return std::nullopt;
    }
    return *base;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt2_branchless2(const VecType<Aligned> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        if (*(base + half - 1) < target) {
            base += half;
        }
        len -= half;
    }
    if (*base != target) {
        return std::nullopt;
    }
    return *base;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt3_branchless3(const VecType<Aligned> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        base += (*(base + half - 1) < target) * half;
        len -= half;
    }
    if (*base != target) {
        return std::nullopt;
    }
    return *base;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt4_prefetch(const VecType<Aligned> & elements, int32_t target) {
    const int32_t * base = elements.data();
    int32_t len = elements.size();
    while (len > 1) {
        int32_t half = len / 2;
        __builtin_prefetch(&base[len / 2 - 1]);
        __builtin_prefetch(&base[half + len / 2 - 1]);
        base += (*(base + half - 1) < target) * half;
        len -= half;
    }
    if (*base != target) {
        return std::nullopt;
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
void recursive_eytzinger_transformation_helper(auto & result, auto & elements, 
        int32_t & original_sequential_index, int32_t k) {
    if(k <= elements.size()) {
        recursive_eytzinger_transformation_helper(result, elements, original_sequential_index, 2 * k);
        result[k] = elements[original_sequential_index++];
        recursive_eytzinger_transformation_helper(result, elements, original_sequential_index, 2 * k + 1);
    }
}

auto eytzinger_transformation(const auto & elements) {
    std::remove_cvref_t<decltype(elements)> result(elements.size() + 1);
    // the result array in 1-indexed for performance consideration
    int32_t original_sequential_index = 0;
    recursive_eytzinger_transformation_helper(result, elements, original_sequential_index, 1);
    return result;
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt5_eytzinger(const VecType<Aligned> & elements_eytzinger, int32_t target) {
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
    return std::nullopt;
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt6_eytzinger_branchless(const VecType<Aligned> & elements_eytzinger, int32_t target) {
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
    if(elements_eytzinger[k] != target) {
        return std::nullopt;
    }
    return elements_eytzinger[k];
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
template <int32_t PrefetchStrideInElements, bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt7_eytzinger_prefetch(const VecType<Aligned> & elements_eytzinger, int32_t target) {
    int32_t k = 1;
    while(k < elements_eytzinger.size()) {
        __builtin_prefetch(elements_eytzinger.data() + k * PrefetchStrideInElements * sizeof(int32_t));
        k = 2 * k + (elements_eytzinger[k] < target);
    }
    k >>= __builtin_ffs(~k);
    if(elements_eytzinger[k] != target) {
        return std::nullopt;
    }
    return elements_eytzinger[k];
}

/**
 * @param elements_eytzinger Assume this array is 1-indexed.
 */
template <int32_t PrefetchStrideInElements, bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_opt8_branch_removal(const VecType<Aligned> & elements_eytzinger, int32_t target) {
    int32_t iters = std::__lg(elements_eytzinger.size());
    int32_t k = 1;
    for(int32_t i = 0; i < iters; i++) {
        __builtin_prefetch(elements_eytzinger.data() + k * PrefetchStrideInElements * sizeof(int32_t));
        k = 2 * k + (elements_eytzinger[k] < target);
    }
    // We remove the last round in the loop
    // and use a "cmove" like operation
    // because the last round is more possible to lead to branch miss
    int32_t val = (k < elements_eytzinger.size() ? elements_eytzinger[k] : 0);
    k = 2 * k + (val < target);

    k >>= __builtin_ffs(~k);
    if(elements_eytzinger[k] != target) {
        return std::nullopt;
    }
    return elements_eytzinger[k];
}