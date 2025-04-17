#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>
#include <immintrin.h>

#include "aligned_allocator.hpp"

using Vector = std::vector<int32_t, AlignedAllocator<int32_t>>;

/**
 * @brief Naive way to iterate through all of the elements
 */
int32_t argmin_baseline(const Vector & elements) {
    int32_t index = 0;
    for(int32_t i = 0; i < elements.size(); i++) {
        if(elements[i] < elements[index]) {
            index = i;
        }
    }
    return index;
}

/**
 * @brief Add a hint to indicate that the if condition is rare to be hitten.
 */
int32_t argmin_baseline_with_hint(const Vector & elements) {
    int32_t index = 0;
    for(int32_t i = 0; i < elements.size(); i++) {
        if(__builtin_expect(elements[i] < elements[index], false)) {
            index = i;
        }
    }
    return index;
}

/**
 * @brief Standard library algorithm
 */
int32_t argmin_std(const Vector & elements) {
    return std::min_element(elements.begin(), elements.end()) - elements.begin(); 
}

/**
 * @brief Use avx256 to do batch processing. We assume the address is 32-bytes aligned.
 * We group the elements into 8 groups, and compute the minmum value / index in each group.
 * At last, we do a reduction to find out the answer.
 */
int32_t argmin_vectorize(const Vector & elements) {
    __m256i cur = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
    __m256i min = _mm256_set1_epi32(INT32_MAX);
    __m256i idx = _mm256_setzero_si256();
    const __m256i constant_eight = _mm256_set1_epi32(8);

    int32_t elements_idx = 0;
    for(; elements_idx + 8 < elements.size(); elements_idx += 8) {
        __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx));
        __m256i mask = _mm256_cmpgt_epi32(min, x);
        idx = _mm256_blendv_epi8(idx, cur, mask);
        min = _mm256_min_epi32(x, min);
        cur = _mm256_add_epi32(cur, constant_eight);
    }

    std::array<int32_t, 8> min_array, idx_array;
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(min_array.data()), min);
    _mm256_storeu_si256(reinterpret_cast<__m256i *>(idx_array.data()), idx);

    int32_t k = 0;
    int32_t m = min_array[0];

    // Handle the remaining elements here when the size of the array
    // is not a multiple of 8
    for(; elements_idx < elements.size(); elements_idx++) {
        if(elements[elements_idx] < m) {
            k = elements_idx;
            m = elements[elements_idx];
        }
    }

    for(int32_t i = 1; i < 8; i++) {
        if(min_array[i] < m) {
            k = i;
            m = min_array[i];
        }
    }

    return idx_array[k];
}

/**
 * @brief Use avx256 to do batch processing. We assume the address is 32-bytes aligned.
 * Now we don't maintain 8 groups min value / index. Each time we find out there's a value smaller than
 * the current minimum in the next 8 elements, we search through these elements to update the minimum element and
 * the minimum index.
 */
int32_t argmin_vectorize2(const Vector & elements) {
    int32_t min = INT32_MAX;
    int32_t idx = 0;
    __m256i p = _mm256_set1_epi32(min);

    int32_t elements_idx = 0;
    for(; elements_idx + 8 < elements.size(); elements_idx += 8) {
        __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx));
        __m256i mask = _mm256_cmpgt_epi32(p, y);
        if(!_mm256_testz_si256(mask, mask)) {
            for(int32_t j = elements_idx; j < elements_idx + 8; j++) {
                if(elements[j] < min) {
                    idx = j;
                    min = elements[j];
                }
            }
            p = _mm256_set1_epi32(min);
        }
    }

    // Handle the remaining elements here when the size of the array
    // is not a multiple of 8
    for(; elements_idx < elements.size(); elements_idx++) {
        if(elements[elements_idx] < min) {
            idx = elements_idx;
            min = elements[elements_idx];
        }
    }
    return idx;
}

/**
 * @brief Add hint to indicate that the if condition is rare to be hitten.
 */
int32_t argmin_vectorize2_with_hint(const Vector & elements) {
    int32_t min = INT32_MAX;
    int32_t idx = 0;
    __m256i p = _mm256_set1_epi32(min);

    int32_t elements_idx = 0;
    for(; elements_idx + 8 < elements.size(); elements_idx += 8) {
        __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx));
        __m256i mask = _mm256_cmpgt_epi32(p, y);
        if(__builtin_expect(!_mm256_testz_si256(mask, mask), false)) {
            for(int32_t j = elements_idx; j < elements_idx + 8; j++) {
                if(elements[j] < min) {
                    idx = j;
                    min = elements[j];
                }
            }
            p = _mm256_set1_epi32(min);
        }
    }

    // Handle the remaining elements here when the size of the array
    // is not a multiple of 8
    for(; elements_idx < elements.size(); elements_idx++) {
        if(elements[elements_idx] < min) {
            idx = elements_idx;
            min = elements[elements_idx];
        }
    }
    return idx;
}

/**
 * @brief Unroll the loop by 2 for "argmin_vectorize2" function.
 */
int32_t argmin_vectorize2_unroll2(const Vector & elements) {
    int32_t min = INT32_MAX;
    int32_t idx = 0;
    __m256i p = _mm256_set1_epi32(min);

    int32_t elements_idx = 0;
    for(; elements_idx + 16 < elements.size(); elements_idx += 16) {
        __m256i y1 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx));
        __m256i y2 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx + 8));
        __m256i y = _mm256_min_epi32(y1, y2);
        __m256i mask = _mm256_cmpgt_epi32(p, y);
        if(!_mm256_testz_si256(mask, mask)) {
            for(int32_t j = elements_idx; j < elements_idx + 16; j++) {
                if(elements[j] < min) {
                    idx = j;
                    min = elements[j];
                }
            }
            p = _mm256_set1_epi32(min);
        }
    }

    // Handle the remaining elements here when the size of the array
    // is not a multiple of 16
    for(; elements_idx < elements.size(); elements_idx++) {
        if(elements[elements_idx] < min) {
            idx = elements_idx;
            min = elements[elements_idx];
        }
    }
    return idx;
}

/**
 * @brief Unroll the loop by 4 for "argmin_vectorize2" function.
 */
int32_t argmin_vectorize2_unroll4(const Vector & elements) {
    int32_t min = INT32_MAX;
    int32_t idx = 0;
    __m256i p = _mm256_set1_epi32(min);

    int32_t elements_idx = 0;
    for(; elements_idx + 32 < elements.size(); elements_idx += 32) {
        __m256i y1 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx));
        __m256i y2 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx + 8));
        __m256i y3 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx + 16));
        __m256i y4 = _mm256_load_si256(reinterpret_cast<const __m256i *>(elements.data() + elements_idx + 24));
        y1 = _mm256_min_epi32(y1, y2);
        y3 = _mm256_min_epi32(y3, y4);
        __m256i y = _mm256_min_epi32(y1, y3);
        __m256i mask = _mm256_cmpgt_epi32(p, y);
        if(!_mm256_testz_si256(mask, mask)) {
            for(int32_t j = elements_idx; j < elements_idx + 32; j++) {
                if(elements[j] < min) {
                    idx = j;
                    min = elements[j];
                }
            }
            p = _mm256_set1_epi32(min);
        }
    }

    // Handle the remaining elements here when the size of the array
    // is not a multiple of 32
    for(; elements_idx < elements.size(); elements_idx++) {
        if(elements[elements_idx] < min) {
            idx = elements_idx;
            min = elements[elements_idx];
        }
    }
    return idx;
}

// Compact the m256i mask into unsigned mask
unsigned get_mask(const __m256i m) {
    return _mm256_movemask_ps(reinterpret_cast<const __m256>(m));
}

__m256i cmp(__m256i x, const int32_t *p) {
    __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
    return _mm256_cmpeq_epi32(x, y);
}

__m256i min(__m256i x, const int32_t *p) {
    __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
    return _mm256_min_epi32(x, y);
}

int32_t find(const int32_t * a, int32_t n, int32_t needle) {
    __m256i x = _mm256_set1_epi32(needle);

    // we have to make sure that n is a multiple of 32
    for (int32_t i = 0; i < n; i += 32) {
        __m256i m1 = cmp(x, a + i);
        __m256i m2 = cmp(x, a + i + 8);
        __m256i m3 = cmp(x, a + i + 16);
        __m256i m4 = cmp(x, a + i + 24);

        // Compact the 4 comparison results into one __m256i
        // for testing if there's a value equal to needle
        __m256i m12 = _mm256_or_si256(m1, m2);
        __m256i m34 = _mm256_or_si256(m3, m4);
        __m256i m = _mm256_or_si256(m12, m34);
        if (!_mm256_testz_si256(m, m)) {
            // If something match needle, process further

            // we just use some bit trick to make each mask bit
            // matches the corresponding position
            unsigned mask = (get_mask(m4) << 24)
                          + (get_mask(m3) << 16)
                          + (get_mask(m2) << 8)
                          +  get_mask(m1);
            // Therefore, counting the trailing zero will get us
            // the position of the matched element inside the 32-bytes block
            return i + __builtin_ctz(mask);
        }
    }

    return -1;
}

__m256i hmin(__m256i x) {
    // 2  1  4  3  6  5  8  7
    // re-arrange the elements for pairwise comparison
    __m256i y = reinterpret_cast<__m256i>(_mm256_permute_ps( (__m256) x, 1 + (0 << 2) + (3 << 4) + (2 << 6)));
    x = _mm256_min_epi32(x, y);
    // 2  1  4  3  6  5  8  7 
    // 2-nd re-arrange
    y = reinterpret_cast<__m256i>(_mm256_permute_pd( (__m256d) x, 5));
    x = _mm256_min_epi32(x, y);
    // 5  6  7  8  1  2  3  4
    // 3-rd re-arrange
    y = _mm256_permute2x128_si256(x, y, 1);
    x = _mm256_min_epi32(x, y);
    return x;
}

/**
 * @brief Breakdown the argmin algorithm into 2 steps:
 * 1. do block processing, firstly find the minimum value and the block base index
 * 2. search in the block to find the target index
 */
int32_t argmin_blocking_breakdown(const Vector & elements) {
    constexpr int32_t BlockSize = 128;
    int32_t idx = 0;
    __m256i m, m1, m2;
    m = m1 = m2 = _mm256_set1_epi32(INT32_MAX);

    for (int32_t i = 0; i + BlockSize < elements.size(); i += BlockSize) {
        for (int32_t j = i; j < i + BlockSize; j += 16) {
            // unroll by 2
            m1 = min(m1, elements.data() + j);
            m2 = min(m2, elements.data() + j + 8);
        }
        __m256i t = _mm256_min_epi32(m1, m2);
        __m256i mask = _mm256_cmpgt_epi32(m, t);
        if (!_mm256_testz_si256(mask, mask)) {
            idx = i;
            m = hmin(t);
        }
    }

    // handle the remaining elements when the size is not a multiple of 16
    int32_t remaining = elements.size() % BlockSize;
    if (remaining > 0) {
        for (int32_t i = elements.size() - remaining; i < elements.size(); i += 8) {
            if (i + 8 <= elements.size()) {
                m1 = min(m1, elements.data() + i);
            }
            if (i + 16 <= elements.size()) {
                m2 = min(m2, elements.data() + i + 8);
            }
        }

        __m256i t = _mm256_min_epi32(m1, m2);
        __m256i mask = _mm256_cmpgt_epi32(m, t);
        if (!_mm256_testz_si256(mask, mask)) {
            idx = elements.size() - remaining;
            m = hmin(t);
        }
    }

    // finally, find the exact position index inside the block
    return idx + find(elements.data() + idx, BlockSize, _mm256_extract_epi32(m, 0));
}