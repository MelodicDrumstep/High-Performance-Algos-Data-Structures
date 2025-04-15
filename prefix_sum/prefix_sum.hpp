#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>
#include <immintrin.h>

#include "aligned_allocator.hpp"

using Vector = std::vector<int32_t, AlignedAllocator<int32_t>>;

Vector prefix_sum_baseline(const Vector & elements) {
    Vector result(elements.size());
    result[0] = elements[0];
    for(int32_t i = 1; i < elements.size(); i++) {
        result[i] = elements[i] + result[i - 1];
    }
    return result;
}

Vector prefix_sum_baseline2(const Vector & elements) {
    Vector result(elements);
    for(int32_t i = 1; i < elements.size(); i++) {
        result[i] += result[i - 1];
    }
    return result;
}

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

// Add the acculated prefix to the 4 int32 elements
__m128i accumulate_4_int32(int32_t *p, __m128i s) {
    __m128i d = reinterpret_cast<__m128i>(_mm_broadcast_ss(reinterpret_cast<float*>(p + 3)));
    __m128i x = _mm_load_si128(reinterpret_cast<__m128i*>(p));
    x = _mm_add_epi32(s, x);
    _mm_store_si128(reinterpret_cast<__m128i*>(p), x);
    return _mm_add_epi32(s, d);
}

Vector prefix_sum_SIMD(const Vector & elements) {
    Vector result(elements);
    for (int32_t i = 0; i < elements.size(); i += 8)
    prefix_8_int32(&result[i]);
    
    __m128i s = _mm_setzero_si128();
    // s maintains the last prefix sum value of the last 4-int32 block 
    
    for (int32_t i = 0; i < elements.size(); i += 4)
        s = accumulate_4_int32(&result[i], s);
        // accumulate the 4-int32 block and update s

    return result;
}

template <int32_t BlockSize>
__m128i local_prefix(int32_t * a, __m128i s) {
    for(int32_t i = 0; i < BlockSize; i += 8) {
        prefix_8_int32(a + i);
    }
    for(int32_t i = 0; i < BlockSize; i += 4) {
        s = accumulate_4_int32(a + i, s);
    }
    return s;
}

Vector prefix_sum_SIMD_blocking(const Vector & elements) {
    constexpr int32_t BlockSize = 4096;
    Vector result(elements);
    __m128i s = _mm_setzero_si128();
    for(int32_t i = 0; i < elements.size(); i += BlockSize) {
        s = local_prefix<BlockSize>(result.data() + i, s);
    }
    return result;
}