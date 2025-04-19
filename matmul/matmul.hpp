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
    std::memset(c, 0, n * n);
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            for(int32_t k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt1_loop_interchange(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n);
    for(int32_t i = 0; i < n; i++) {
        for(int32_t k = 0; k < n; k++) {
            for(int32_t j = 0; j < n; j++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt2_invariant(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n);
    for(int32_t i = 0; i < n; i++) {
        for(int32_t k = 0; k < n; k++) {
            float a_value = a[i * n + k];
            for(int32_t j = 0; j < n; j++) {
                c[i * n + j] += a_value * b[k * n + j];
            }
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt3_register_reuse(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n);
    for(int32_t j = 0; j < n; j += 4) {
        for(int32_t i = 0; i < n; i++) {
            float c_00 = 0.0;
            float c_01 = 0.0;
            float c_02 = 0.0;
            float c_03 = 0.0;
            for(int32_t k = 0; k < n; k++) {
                // we assume n is a multiple of 4, if not
                // padding is required
                float a_value = a[i * n + k];
                c_00 += a_value * b[k * n + j];
                c_01 += a_value * b[k * n + j + 1];
                c_02 += a_value * b[k * n + j + 2];
                c_03 += a_value * b[k * n + j + 3];
            }

            c[i * n + j] += c_00;
            c[i * n + j + 1] += c_01;
            c[i * n + j + 2] += c_02;
            c[i * n + j + 3] += c_03;
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt4_register_reuse2(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n);
    for(int32_t j = 0; j < n; j += 4) {
        for(int32_t i = 0; i < n; i++) {
            float c_00 = 0.0;
            float c_01 = 0.0;
            float c_02 = 0.0;
            float c_03 = 0.0;
            for(int32_t k = 0; k < n; k++) {
                // we assume n is a multiple of 4, if not
                // padding is required
                float a_value = a[i * n + k];
                size_t b_cached_index = k * n + j;
                c_00 += a_value * b[b_cached_index];
                c_01 += a_value * b[b_cached_index + 1];
                c_02 += a_value * b[b_cached_index + 2];
                c_03 += a_value * b[b_cached_index + 3];
            }

            size_t c_cached_index = i * n + j;
            c[c_cached_index] += c_00;
            c[c_cached_index + 1] += c_01;
            c[c_cached_index + 2] += c_02;
            c[c_cached_index + 3] += c_03;
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt5_4x4(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n);

    #define declare_c_value(row, col) \
    float c_##row##col = 0.0;

    #define declare_a_value(row) \
        float a_value_##row = a[(i + row) * n + k];

    #define update_c_value(row, col) \
        c_##row##col += a_value_##row * b[k * n + j + col];

    #define store_back_c(row, col) \
        c[(i + row) * n + j + col] += c_##row##col;

    #define EXPAND(...) __VA_ARGS__

    #define LOOP_0_H(ACTION, ...) 
    #define LOOP_1_H(ACTION, ...) ACTION(0, __VA_ARGS__) EXPAND(LOOP_0_H(ACTION, __VA_ARGS__))
    #define LOOP_2_H(ACTION, ...) ACTION(1, __VA_ARGS__) EXPAND(LOOP_1_H(ACTION, __VA_ARGS__))
    #define LOOP_3_H(ACTION, ...) ACTION(2, __VA_ARGS__) EXPAND(LOOP_2_H(ACTION, __VA_ARGS__))
    #define LOOP_4_H(ACTION, ...) ACTION(3, __VA_ARGS__) EXPAND(LOOP_3_H(ACTION, __VA_ARGS__))

    #define LOOP_4_4(ACTION) \
        EXPAND(LOOP_4_H(ACTION, 0)) \
        EXPAND(LOOP_4_H(ACTION, 1)) \
        EXPAND(LOOP_4_H(ACTION, 2)) \
        EXPAND(LOOP_4_H(ACTION, 3))

    #define LOOP_4(ACTION) \
        ACTION(0)\
        ACTION(1)\
        ACTION(2)\
        ACTION(3)    

    for(int32_t j = 0; j < n; j += 4) {
        for(int32_t i = 0; i < n; i += 4) {
            LOOP_4_4(declare_c_value)
            for(int32_t k = 0; k < n; k++) {
                // we assume n is a multiple of 4, if not
                // padding is required
                LOOP_4(declare_a_value)
                LOOP_4_4(update_c_value)
            }
            LOOP_4_4(store_back_c)
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
    std::memset(c, 0, n * n);
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
    std::memset(c, 0, n * n);
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

template <int32_t h, int32_t w>
void kernel_h_w_matmul(float * a, simd_vec_256 * b, simd_vec_256 * c, 
        int32_t x, int32_t y, int32_t l, int32_t r, int32_t n) {
    constexpr int32_t w_in_vector = w / BlockSizeInElements;
    simd_vec_256 t[h][w_in_vector] {};

    for(int32_t k = l; k < r; k++ ){
        for(int32_t i = 0; i < h; i++) {
            simd_vec_256 alpha = simd_vec_256 {} + a[(x + i) * n + k];

            for(int32_t j = 0; j < w_in_vector; j++) {
                t[i][j] += alpha * b[(k * n + y)  / BlockSizeInElements + j];
            }
        }
    }

    for(int32_t i = 0; i < h; i++) {
        for(int32_t j = 0; j < w_in_vector; j++) {
            c[((x + i) * n + y) / BlockSizeInElements + j] += t[i][j];
        }
    }
}

void matmul_kernel_blocking(const float * a, const float * b, float * c, int32_t n) {
    constexpr int32_t h = 6;
    constexpr int32_t w = 16;
    constexpr int32_t w_in_vector = w / BlockSizeInElements;

    std::memset(c, 0, n * n);

    int32_t nx = (n + h - 1) / h * h;
    int32_t ny = (n + w - 1) / w * w;
    // do padding

    float * pad_a = alloc<float, 32>(nx * ny);
    float * pad_b = alloc<float, 32>(nx * ny);
    float * pad_c = alloc<float, 32>(nx * ny);

    for(int32_t i = 0; i < n; i++) {
        std::memcpy(pad_a + i * ny, a + i * n, 4 * n);
        std::memcpy(pad_b + i * ny, b + i * n, 4 * n);
        // no need for transposing B here
    }

    for(int32_t x = 0; x < nx; x += h) {
        for(int32_t y = 0; y < ny; y += w) {
            kernel_h_w_matmul<h, w>(pad_a, reinterpret_cast<simd_vec_256 *>(pad_b), 
            reinterpret_cast<simd_vec_256 *>(pad_c), x, y, 0, n, ny);
        }
    }

    for(int32_t i = 0; i < n; i++) {
        std::memcpy(c + i * n, pad_c + i * ny, 4 * n);
    }

    std::free(pad_a);
    std::free(pad_b);
    std::free(pad_c);
}

// template <int32_t L1CacheSize, int32_t L2CacheSize, int32_t L3CacheSize>
// void matmul_kernel_blocking2(const float * a, const float * b, float * c, int32_t n) {
//     std::memset(c, 0, n * n);
    
//     for(int32_t i3 = 0; i3 < )
// }