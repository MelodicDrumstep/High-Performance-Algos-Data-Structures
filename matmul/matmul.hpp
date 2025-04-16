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

constexpr static int32_t VectorSizeInBytes = 32; // 8 floats, using __m256
constexpr static int32_t BlockSizeInElements = VectorSizeInBytes / sizeof(float); // 8 floats inside one block
using simd_vec_256 = float __attribute__ ((vector_size(VectorSizeInBytes))); // using gcc vector type extension

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
    int32_t num_blocks = (n + BlockSizeInElements - 1) / BlockSizeInElements;
    simd_vec_256 * blocks_a = alloc<simd_vec_256, VectorSizeInBytes>(n * num_blocks);
    simd_vec_256 * blocks_b = alloc<simd_vec_256, VectorSizeInBytes>(n * num_blocks);
    // allocate an aligned space

    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            blocks_a[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = a[i * n + j];
            blocks_b[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = b[j * n + i];
        }
    }
    // copy the matrices to the aligned space, transpose matrix B

    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            simd_vec_256 s{};
            for(int32_t k = 0; k < num_blocks; k++) {
                // Use vector multiplication (SIMD instruction)
                s += blocks_a[i * num_blocks + k] * blocks_b[j * num_blocks + k];
            }
            // horizontal accumulation
            for(int32_t k = 0; k < BlockSizeInElements; k++) {
                c[i * n + j] += s[k];
            }
        }
    }
    std::free(blocks_a);
    std::free(blocks_b);
}

void kernel(float * a, simd_vec_256 * b, simd_vec_256 * c, int32_t x, int32_t y0,
            int32_t l, int32_t r, int32_t n) {
    constexpr int32_t h = 6;
    constexpr int32_t w = 16;
    constexpr int32_t w_in_vector = w / sizeof(float);
        
    simd_vec_256 t[6][2] {};

    for(int32_t k = l; k < r; k++ ){
        for(int32_t i = 0; i < h; i++) {
            simd_vec_256 alpha = simd_vec_256 {} + a[(x + i ) * n + k];

            for(int32_t j = 0; j < w_in_vector; j++) {
                t[i][j] += alpha * b[(k * n + y)  / sizeof(float) + j];
            }
        }
    }

    for(int32_t i = 0; i < h; i++) {
        for(int32_t j = 0; j < w_in_vector; j++) {
            c[((x + i) * n + y) / sizeof(float) + j] += t[i][j];
        }
    }

}

void matmul_blocking(const float * a, const float * b, float * c, int32_t n) {
    constexpr int32_t h = 6;
    constexpr int32_t w = 16;
    constexpr int32_t w_in_vector = w / sizeof(float);
        
    int32_t nx = (n + h - 1) / h * h;
    int32_t ny = (n + w - 1) / w * w;

    float * blocks_a = alloc<float, 32>(nx * ny);
    float * blocks_b = alloc<float, 32>(nx * ny);

    for(int32_t i = 0; i < n; i++) {
        std::memcpy(blocks_a + i * ny, a + i * n, 4 * n);
        std::memcpy(blocks_a + i * ny, a + i * n, 4 * n);
    }


}