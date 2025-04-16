#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>
#include <immintrin.h>
#include <cstdlib>
#include <cstring>

#include "aligned_allocator.hpp"

using Vector = std::vector<float, AlignedAllocator<float>>;

template <bool UseRestrict = false>
using float_ptr_wrapper = std::conditional_t<UseRestrict, float * __restrict__, float *>;

/**
 * @brief Naive 3 layer loop implementation.
 */
template <bool UseRestrict = false>
void matmul_baseline(const float * a,
                     const float * b, 
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            for(int32_t k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

/**
 * @brief Transpose b first, for better cache hit rate.
 */
template <bool UseRestrict = false>
void matmul_transpose(const float * a,
                      const float * b, 
                      float_ptr_wrapper<UseRestrict> c, 
                      int32_t n) {
    std::vector<float> b_transpose(n * n);
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            b_transpose[i * n + j] = b[j * n + i];
        }
    }
    
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            for(int32_t k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b_transpose[j * n + k];
            }
        }
    }
}

template <typename T, int32_t AlignedSize>
T * alloc(int32_t n) {
    T * ptr = reinterpret_cast<T *>(std::aligned_alloc(AlignedSize, AlignedSize * n));
    std::memset(ptr, 0, AlignedSize * n);
    return ptr;
}

/**
 * @brief Use gcc vector type for SIMD processing.
 */
void matmul_vectorization(const float * a, const float * b, float * c, int32_t n) {
    constexpr static int32_t VectorSizeInBytes = 32; // 8 floats
    constexpr static int32_t BlockSizeInElements = VectorSizeInBytes / sizeof(float);
    using vec = float __attribute__ ((vector_size(VectorSizeInBytes)));

    int32_t num_blocks = (n + BlockSizeInElements - 1) / BlockSizeInElements;
    vec * blocks_a = alloc<vec, VectorSizeInBytes>(n * num_blocks);
    vec * blocks_b = alloc<vec, VectorSizeInBytes>(n * num_blocks);

    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            blocks_a[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = a[i * n + j];
            blocks_b[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = b[j * n + i];
        }
    }

    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            vec s{};
            for(int32_t k = 0; k < num_blocks; k++) {
                s += blocks_a[i * num_blocks + k] * blocks_b[j * num_blocks + k];
            }
            for(int32_t k = 0; k < BlockSizeInElements; k++) {
                c[i * n + j] += s[k];
            }
        }
    }
    std::free(blocks_a);
    std::free(blocks_b);
}