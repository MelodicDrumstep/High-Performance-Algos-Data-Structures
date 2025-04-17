#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>
#include <immintrin.h>

#include "aligned_allocator.hpp"

using Vector = std::vector<int32_t, AlignedAllocator<int32_t>>;

/**
 * @brief Naive baseline, without copying the input array first.
 */
Vector prefix_sum_baseline(const Vector & elements) {
    Vector result(elements.size());
    result[0] = elements[0];
    for(int32_t i = 1; i < elements.size(); i++) {
        result[i] = elements[i] + result[i - 1];
    }
    return result;
}

/**
 * @brief Another naive baseline, copying the input array first.
 */
Vector prefix_sum_baseline2(const Vector & elements) {
    Vector result(elements);
    for(int32_t i = 1; i < elements.size(); i++) {
        result[i] += result[i - 1];
    }
    return result;
}

/**
 * @brief Use std::partial_sum.
 */
Vector prefix_sum_std(const Vector & elements) {
    Vector result(elements.size());
    std::partial_sum(elements.begin(), elements.end(), result.begin());
    return result;
}

// compute the local prefix sum for each elements (the sum of the 4 elements)
void prefix_8_int32(int32_t *p) {
    __m256i x = _mm256_load_si256(reinterpret_cast<__m256i*>(p));
    // a, b, c, d, e, f, g, h
    x = _mm256_add_epi32(x, _mm256_slli_si256(x, 4));
    // _mm256_slli_si256(x, 4) = 0, a, b, c, 0, e, f, g (128-bit lane)!!!
    // a, a + b, b + c, c + d, e, e + f, f + g, g + h
    x = _mm256_add_epi32(x, _mm256_slli_si256(x, 8));
    // _mm256_slli_si256(x, 8) = 0, 0, a, a + b, 0, 0, e, e + f (128-bit lane)!!!
    // a, a + b, a + b + c, a + b + c + d, e, e + f, e + f + g, e + f + g + h
    _mm256_store_si256(reinterpret_cast<__m256i*>(p), x);
}

/**
 * @brief Add the acculated prefix to the 4 int32 elements
 * @tparam prefetch Do software prefetch or not
 * @tparam BlockSize the block size to be prefetched, only meaningful if "prefetch == true"
 */
template <bool prefetch = false, int32_t BlockSize = 4>
__m128i accumulate_4_int32(int32_t *p, __m128i s) {
    if constexpr (prefetch) {
        __builtin_prefetch(p + BlockSize);
    }
    __m128i d = reinterpret_cast<__m128i>(_mm_broadcast_ss(reinterpret_cast<float*>(p + 3)));
    __m128i x = _mm_load_si128(reinterpret_cast<__m128i*>(p));
    x = _mm_add_epi32(s, x);
    _mm_store_si128(reinterpret_cast<__m128i*>(p), x);
    return _mm_add_epi32(s, d);
}

/**
 * @brief Use SIMD to do batch processing. 
 * 1. Group each 4 int32 elements into one group, 
 * use SIMD instructions to compute the local prefix sum 
 *  (computing this for 8 elements at a time)
 * 
 */
Vector prefix_sum_SIMD(const Vector & elements) {
    Vector result(elements);
    for (int32_t i = 0; i < elements.size(); i += 8) {
        prefix_8_int32(&result[i]);
    }
    
    __m128i s = _mm_setzero_si128();
    // s maintains the last prefix sum value of the last 4-int32 block 
    
    for (int32_t i = 0; i < elements.size(); i += 4) {
        s = accumulate_4_int32<false>(&result[i], s);
    }
    // accumulate the 4-int32 block and update s

    return result;
}

// compute the prefix sum value inside a block using "prefix_8_int32"
// and "accumulate_4_int32". This function will be called for block processing version.
template <bool prefetch, int32_t BlockSize>
__m128i prefix_sum_inside_block(int32_t * a, __m128i s) {
    for(int32_t i = 0; i < BlockSize; i += 8) {
        prefix_8_int32(a + i);
    }
    for(int32_t i = 0; i < BlockSize; i += 4) {
        s = accumulate_4_int32<prefetch, BlockSize>(a + i, s);
    }
    return s;
}

/**
 * @brief Use block processing for better cache hit rate. For each block, we 
 * calculate the prefix sum using "prefix_sum_inside_block", rather than compute the 
 * local prefix sum for each elements and then do accumulation, which leads to high cache
 * miss rate when accessing the array the second time.
 */
template <bool prefetch>
Vector prefix_sum_SIMD_blocking(const Vector & elements) {
    constexpr int32_t BlockSize = 1024;
    // make sure the array size is larger than the BlockSize
    Vector result(elements);
    __m128i s = _mm_setzero_si128();
    for(int32_t i = 0; i < elements.size(); i += BlockSize) {
        s = prefix_sum_inside_block<prefetch, BlockSize>(result.data() + i, s);
    }
    return result;
}

/**
 * @brief This is just a special idea: For small block sizes, the pipeline tends to stall.
 * And for large block sizes, the cache miss rate is higher.
 * (even if we do block processing, the hardware prefetching will be more inefficient) 
 * Then we can interleave the small block size with large block size to avoid the bad 
 * results from both of them.
 */
template <bool prefetch>
Vector prefix_sum_SIMD_blocking_interleaving(const Vector & elements) {
    constexpr int32_t BlockSize = 64;
    Vector result(elements);
    __m128i s = _mm_setzero_si128();
    for(int32_t i = 0; i < BlockSize; i += 8) {
        prefix_8_int32(result.data() + i);
    }
    for(int32_t i = BlockSize; i < elements.size(); i += 8) {
        prefix_8_int32(result.data() + i);
        s = accumulate_4_int32<prefetch>(result.data() + i - BlockSize, s);
        s = accumulate_4_int32<prefetch>(result.data() + i - BlockSize + 4, s);
    }
    for(int32_t i = elements.size() - BlockSize; i < elements.size(); i += 4) {
        s = accumulate_4_int32<prefetch>(result.data() + i, s);
    }
    return result;
}