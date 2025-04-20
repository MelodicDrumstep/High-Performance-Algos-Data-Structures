#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>
#include <immintrin.h>
#include <cstdlib>
#include <cstring>
#include <mmintrin.h>
#include <xmmintrin.h>  // SSE
#include <pmmintrin.h>  // SSE2
#include <emmintrin.h>  // SSE3
#include <cstdalign>

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
    std::memset(c, 0, n * n * sizeof(float));
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
    std::memset(c, 0, n * n * sizeof(float));
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
    std::memset(c, 0, n * n * sizeof(float));
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
    std::memset(c, 0, n * n * sizeof(float));
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
    std::memset(c, 0, n * n * sizeof(float));
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j += 4) {
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
void kernel_add_dot_block_4x4(int32_t m, int32_t n, int32_t k, const float * a, int32_t lda,
    const float * b, int32_t ldb, float_ptr_wrapper<UseRestrict> c, int32_t ldc) {
    #define declare_c_value(row, col) \
    float c_##row##col = 0.0;

    #define declare_a_value(row) \
        float a_value_##row = a[(i + row) * lda + k];

    #define update_c_value(row, col) \
        c_##row##col += a_value_##row * b[k * ldb + j + col];

    #define store_back_c(row, col) \
        c[(i + row) * ldc + j + col] += c_##row##col;

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

    for(int32_t i = 0; i < n; i += 4) {
        for(int32_t j = 0; j < n; j += 4) {
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

template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc) {
    // Loop over blocks of 4x4 sub-matrices
    for (int32_t i = 0; i < m; i += 4) {
        for (int32_t j = 0; j < n; j += 4) {
            // Declare SIMD registers to accumulate results (using 128-bit AVX)
            // Reset the accumulation registers for this 4x4 block
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            for (int32_t p = 0; p < k; p++) {
                // Load values from matrix A for rows i, i+1, i+2, i+3
                __m128 a_row_0 = _mm_set1_ps(a[(i + 0) * lda + p]);
                __m128 a_row_1 = _mm_set1_ps(a[(i + 1) * lda + p]);
                __m128 a_row_2 = _mm_set1_ps(a[(i + 2) * lda + p]);
                __m128 a_row_3 = _mm_set1_ps(a[(i + 3) * lda + p]);

                // Load values from matrix B for columns j, j+1, j+2, j+3
                __m128 b_col = _mm_load_ps(&b[p * ldb + j]);

                // Perform the multiply-accumulate (FMA) operation for each element
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store the result back to matrix C
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt5_4x4(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    kernel_add_dot_block_4x4<UseRestrict>(n, n, n, a, n, b, n, c, n);
}

template <bool UseRestrict = false>
void matmul_opt6_blocking_4x4(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 256;
    constexpr int32_t KBlockSize = 128;

    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            kernel_add_dot_block_4x4<UseRestrict>(m_block_size, n, k_block_size, a + i * n + k, n,
                b + k * n, n, c + i * n, n);
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt7_4x4_vectorization(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    kernel_add_dot_block_4x4_vectorization<UseRestrict>(n, n, n, a, n, b, n, c, n);
}

template <bool UseRestrict = false>
void matmul_opt8_blocking_4x4_vectorization(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;
    constexpr int32_t KBlockSize = 32;

    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            kernel_add_dot_block_4x4_vectorization<UseRestrict>(m_block_size, n, k_block_size, a + i * n + k,
                n, b + k * n, n, c + i * n, n);
        }
    }
}

template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization_packed(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc, float * packed_a) {
    // Loop over blocks of 4x4 sub-matrices
    for (int32_t i = 0; i < m; i += 4) {
        // pack a here
        float * pack_current_pos = packed_a + i * k;
        const float * a_0_ptr = a + i * lda;
        const float * a_1_ptr = a + (i + 1) * lda;
        const float * a_2_ptr = a + (i + 2) * lda;
        const float * a_3_ptr = a + (i + 3) * lda;
        for(int32_t pack_idx = 0; pack_idx < k; pack_idx++) {
            *(pack_current_pos++) = *(a_0_ptr++);
            *(pack_current_pos++) = *(a_1_ptr++);
            *(pack_current_pos++) = *(a_2_ptr++);
            *(pack_current_pos++) = *(a_3_ptr++);
        }
        pack_current_pos = packed_a + i * k;

        for (int32_t j = 0; j < n; j += 4) {
            // Declare SIMD registers to accumulate results (using 128-bit AVX)
            // Reset the accumulation registers for this 4x4 block
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            for (int32_t p = 0; p < k; p++) {
                // Load values from matrix A for rows i, i+1, i+2, i+3
                __m128 a_row_0 = _mm_set1_ps(pack_current_pos[p * 4]);
                __m128 a_row_1 = _mm_set1_ps(pack_current_pos[p * 4 + 1]);
                __m128 a_row_2 = _mm_set1_ps(pack_current_pos[p * 4 + 2]);
                __m128 a_row_3 = _mm_set1_ps(pack_current_pos[p * 4 + 3]);
                // Load values from matrix B for columns j, j+1, j+2, j+3
                __m128 b_col = _mm_load_ps(&b[p * ldb + j]);

                // Perform the multiply-accumulate (FMA) operation for each element
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store the result back to matrix C
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt9_packing(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;
    constexpr int32_t KBlockSize = 32;

    alignas(32) float packed_a[KBlockSize * MBlockSize];

    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            kernel_add_dot_block_4x4_vectorization_packed<UseRestrict>(m_block_size, n, k_block_size, 
                a + i* k, n, b + k * n, n, c + i * n, n, packed_a);
        }
    }
}

template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization_packed2(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc, float * packed_a, float * packed_b) {
    // Loop over blocks of 4x4 sub-matrices
    for (int32_t i = 0; i < m; i += 4) {
        // pack a here
        float * pack_a_current_pos = packed_a + i * k;
        const float * a_0_ptr = a + i * lda;
        const float * a_1_ptr = a + (i + 1) * lda;
        const float * a_2_ptr = a + (i + 2) * lda;
        const float * a_3_ptr = a + (i + 3) * lda;
        for(int32_t pack_idx = 0; pack_idx < k; pack_idx++) {
            *(pack_a_current_pos++) = *(a_0_ptr++);
            *(pack_a_current_pos++) = *(a_1_ptr++);
            *(pack_a_current_pos++) = *(a_2_ptr++);
            *(pack_a_current_pos++) = *(a_3_ptr++);
        }
        pack_a_current_pos = packed_a + i * k;
        
        for (int32_t j = 0; j < n; j += 4) {
            // Declare SIMD registers to accumulate results (using 128-bit AVX)
            // Reset the accumulation registers for this 4x4 block
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            float * pack_b_current_pos = packed_b;
            for(int32_t p = 0; p < k; p++) {
                *(pack_b_current_pos++) = b[p * ldb + j];
                *(pack_b_current_pos++) = b[p * ldb + j + 1];
                *(pack_b_current_pos++) = b[p * ldb + j + 2];
                *(pack_b_current_pos++) = b[p * ldb + j + 3];
            }

            for (int32_t p = 0; p < k; p++) {
                // Load values from matrix A for rows i, i+1, i+2, i+3
                __m128 a_row_0 = _mm_set1_ps(pack_a_current_pos[p * 4]);
                __m128 a_row_1 = _mm_set1_ps(pack_a_current_pos[p * 4 + 1]);
                __m128 a_row_2 = _mm_set1_ps(pack_a_current_pos[p * 4 + 2]);
                __m128 a_row_3 = _mm_set1_ps(pack_a_current_pos[p * 4 + 3]);
                // Load values from matrix B for columns j, j+1, j+2, j+3
                __m128 b_col = _mm_load_ps(&(packed_b[p * 4]));

                // Perform the multiply-accumulate (FMA) operation for each element
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store the result back to matrix C
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

template <bool UseRestrict = false>
void matmul_opt10_packing2(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;
    constexpr int32_t KBlockSize = 32;

    alignas(32) float packed_a[KBlockSize * MBlockSize];
    alignas(32) float packed_b[4 * KBlockSize];

    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            kernel_add_dot_block_4x4_vectorization_packed2<UseRestrict>(m_block_size, n, k_block_size, 
                a + i* k, n, b + k * n, n, c + i * n, n, packed_a, packed_b);
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
    std::memset(c, 0, n * n * sizeof(float));
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
    std::memset(c, 0, n * n * sizeof(float));
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

    std::memset(c, 0, n * n * sizeof(float));

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
//     std::memset(c, 0, n * n * sizeof(float));
    
//     for(int32_t i3 = 0; i3 < )
// }