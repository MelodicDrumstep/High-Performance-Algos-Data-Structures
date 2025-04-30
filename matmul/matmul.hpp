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

// Use the aligned allocator for better performance with SIMD
using Vector = std::vector<float, AlignedAllocator<float>>;

// Constants for SIMD operations
constexpr static int32_t VectorSizeInBytes = 32; // 8 floats, using __m256
constexpr static int32_t BlockSizeInElements = VectorSizeInBytes / sizeof(float); // 8 floats inside one block
using simd_vec_256 = float __attribute__ ((vector_size(VectorSizeInBytes))); // using gcc vector type extension

// Template to optionally use restrict keyword for better compiler optimization
template <bool UseRestrict = false>
using float_ptr_wrapper = std::conditional_t<UseRestrict, float * __restrict__, float *>;


template <int32_t h, int32_t w>
void kernel_h_w_matmul(float * a, simd_vec_256 * b, simd_vec_256 * c, 
        int32_t x, int32_t y, int32_t l, int32_t r, int32_t n);

/**
 * @brief Naive 3 layer loop implementation of matrix multiplication.
 * This is the baseline implementation with no optimizations.
 * Time complexity: O(n^3)
 * Space complexity: O(1)
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

/**
 * @brief Optimized version using loop interchange.
 * Swaps the order of loops to improve cache locality.
 * Better cache performance by accessing memory in a more sequential manner.
 */
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

/**
 * @brief Optimized version using loop invariant code motion.
 * Moves invariant calculations outside the inner loop.
 * Reduces redundant calculations and improves performance.
 */
template <bool UseRestrict = false>
void matmul_opt2_invariant(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    for(int32_t i = 0; i < n; i++) {
        for(int32_t k = 0; k < n; k++) {
            float a_value = a[i * n + k];  // Loop invariant moved outside
            for(int32_t j = 0; j < n; j++) {
                c[i * n + j] += a_value * b[k * n + j];
            }
        }
    }
}

/**
 * @brief Optimized version using register reuse.
 * Unrolls the inner loop and uses registers to store intermediate results.
 * Reduces memory accesses and improves cache utilization.
 */
template <bool UseRestrict = false>
void matmul_opt3_register_reuse(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    for(int32_t j = 0; j < n; j += 4) {  // Process 4 elements at a time
        for(int32_t i = 0; i < n; i++) {
            float c_00 = 0.0;  // Use registers for intermediate results
            float c_01 = 0.0;
            float c_02 = 0.0;
            float c_03 = 0.0;
            for(int32_t k = 0; k < n; k++) {
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

/**
 * @brief Optimized version using matrix transposition.
 * Transposes matrix B to improve cache locality.
 * Better memory access pattern for the second matrix.
 */
template <bool UseRestrict = false>
void matmul_transpose(const float * a,
                      const float * b,
                      float_ptr_wrapper<UseRestrict> c,
                      int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    // Transpose matrix B for better cache locality
    std::vector<float> b_transpose(n * n);
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            b_transpose[i * n + j] = b[j * n + i];
        }
    }
    
    // Perform multiplication with transposed matrix
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            for(int32_t k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b_transpose[j * n + k];
            }
        }
    }
}

/**
 * @brief Helper function for aligned memory allocation.
 * Ensures memory alignment for SIMD operations.
 */
template <typename T, int32_t AlignedSize>
T * alloc(int32_t n) {
    T * ptr = reinterpret_cast<T *>(std::aligned_alloc(AlignedSize, AlignedSize * n));
    std::memset(ptr, 0, AlignedSize * n);
    return ptr;
}

/**
 * @brief Optimized version using vectorization with GCC vector types.
 * Uses GCC's vector type extension for SIMD operations.
 * Provides a more portable way to use SIMD instructions.
 */
void matmul_vectorization(const float * a, const float * b, float * c, int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    int32_t num_blocks = (n + BlockSizeInElements - 1) / BlockSizeInElements;
    simd_vec_256 * blocks_a = alloc<simd_vec_256, VectorSizeInBytes>(n * num_blocks);
    simd_vec_256 * blocks_b = alloc<simd_vec_256, VectorSizeInBytes>(n * num_blocks);

    // Copy and transpose matrices for better memory access
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            blocks_a[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = a[i * n + j];
            blocks_b[i * num_blocks + j / BlockSizeInElements][j % BlockSizeInElements] = b[j * n + i];
        }
    }

    // Perform vectorized multiplication
    for(int32_t i = 0; i < n; i++) {
        for(int32_t j = 0; j < n; j++) {
            simd_vec_256 s{};
            for(int32_t k = 0; k < num_blocks; k++) {
                s += blocks_a[i * num_blocks + k] * blocks_b[j * num_blocks + k];
            }
            // Accumulate results
            for(int32_t k = 0; k < BlockSizeInElements; k++) {
                c[i * n + j] += s[k];
            }
        }
    }
    std::free(blocks_a);
    std::free(blocks_b);
}

/**
 * @brief Optimized version using kernel blocking.
 * Divides the matrix into smaller blocks for better cache utilization.
 * Uses a combination of blocking and vectorization.
 */
void matmul_kernel_blocking(const float * a, const float * b, float * c, int32_t n) {
    constexpr int32_t h = 6;  // Block height
    constexpr int32_t w = 16; // Block width
    constexpr int32_t w_in_vector = w / BlockSizeInElements;

    std::memset(c, 0, n * n * sizeof(float));

    // Calculate padded dimensions
    int32_t nx = (n + h - 1) / h * h;
    int32_t ny = (n + w - 1) / w * w;

    // Allocate padded matrices
    float * pad_a = alloc<float, 32>(nx * ny);
    float * pad_b = alloc<float, 32>(nx * ny);
    float * pad_c = alloc<float, 32>(nx * ny);

    // Copy input matrices with padding
    for(int32_t i = 0; i < n; i++) {
        std::memcpy(pad_a + i * ny, a + i * n, 4 * n);
        std::memcpy(pad_b + i * ny, b + i * n, 4 * n);
    }

    // Process matrix in blocks
    for(int32_t x = 0; x < nx; x += h) {
        for(int32_t y = 0; y < ny; y += w) {
            kernel_h_w_matmul<h, w>(pad_a, reinterpret_cast<simd_vec_256 *>(pad_b), 
            reinterpret_cast<simd_vec_256 *>(pad_c), x, y, 0, n, ny);
        }
    }

    // Copy results back
    for(int32_t i = 0; i < n; i++) {
        std::memcpy(c + i * n, pad_c + i * ny, 4 * n);
    }

    std::free(pad_a);
    std::free(pad_b);
    std::free(pad_c);
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

/**
 * @brief Optimized version using 4x4 blocking.
 * Divides the matrix into 4x4 blocks for better cache utilization.
 * Uses SIMD instructions for vectorized operations.
 */
template <bool UseRestrict = false>
void kernel_add_dot_block_4x4(int32_t m, int32_t n, int32_t k, const float * a, int32_t lda,
    const float * b, int32_t ldb, float_ptr_wrapper<UseRestrict> c, int32_t ldc) {
    // Macros for code generation to handle 4x4 blocks
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

    // Process matrix in 4x4 blocks
    for(int32_t i = 0; i < n; i += 4) {
        for(int32_t j = 0; j < n; j += 4) {
            // Initialize 4x4 block of C
            LOOP_4_4(declare_c_value)
            for(int32_t k = 0; k < n; k++) {
                // Load values from A
                LOOP_4(declare_a_value)
                // Update C values
                LOOP_4_4(update_c_value)
            }
            // Store results back
            LOOP_4_4(store_back_c)
        }
    }
}

/**
 * @brief Optimized version using vectorization with 4x4 blocking.
 * Uses SIMD instructions (SSE/AVX) for parallel processing.
 * Combines blocking with vectorized operations for maximum performance.
 */
template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc) {
    // Process matrix in 4x4 blocks
    for (int32_t i = 0; i < m; i += 4) {
        for (int32_t j = 0; j < n; j += 4) {
            // Initialize SIMD registers for accumulation
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            for (int32_t p = 0; p < k; p++) {
                // Load values from A using SIMD
                __m128 a_row_0 = _mm_set1_ps(a[(i + 0) * lda + p]);
                __m128 a_row_1 = _mm_set1_ps(a[(i + 1) * lda + p]);
                __m128 a_row_2 = _mm_set1_ps(a[(i + 2) * lda + p]);
                __m128 a_row_3 = _mm_set1_ps(a[(i + 3) * lda + p]);

                // Load values from B using SIMD
                __m128 b_col = _mm_load_ps(&b[p * ldb + j]);

                // Perform SIMD multiply-accumulate
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store results back using SIMD
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

/**
 * @brief Optimized version using 4x4 blocking with vectorization.
 * Wrapper function that calls the kernel implementation with full matrix dimensions.
 * Uses SIMD instructions for parallel processing of 4x4 blocks.
 */
template <bool UseRestrict = false>
void matmul_opt5_4x4(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    kernel_add_dot_block_4x4<UseRestrict>(n, n, n, a, n, b, n, c, n);
}

/**
 * @brief Optimized version using larger block sizes (256x128) with 4x4 blocking.
 * Divides the matrix into larger blocks to better utilize cache hierarchy.
 * Uses fixed block sizes that are tuned for typical cache sizes.
 */
template <bool UseRestrict = false>
void matmul_opt6_blocking_4x4(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 256;  // Block size for rows
    constexpr int32_t KBlockSize = 128;  // Block size for columns

    // Process matrix in larger blocks
    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            // Process each block using 4x4 kernel
            kernel_add_dot_block_4x4<UseRestrict>(m_block_size, n, k_block_size, a + i * n + k, n,
                b + k * n, n, c + i * n, n);
        }
    }
}

/**
 * @brief Optimized version using 4x4 blocking with vectorization.
 * Wrapper function that calls the vectorized kernel implementation.
 * Uses SIMD instructions for parallel processing of 4x4 blocks.
 */
template <bool UseRestrict = false>
void matmul_opt7_4x4_vectorization(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    kernel_add_dot_block_4x4_vectorization<UseRestrict>(n, n, n, a, n, b, n, c, n);
}

/**
 * @brief Optimized version using blocking with vectorization and smaller blocks.
 * Uses 32x32 blocks to better fit in L1 cache.
 * Combines blocking with SIMD vectorization for maximum performance.
 */
template <bool UseRestrict = false>
void matmul_opt8_blocking_4x4_vectorization(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;  // Smaller block size for L1 cache
    constexpr int32_t KBlockSize = 32;

    // Process matrix in smaller blocks
    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            // Process each block using vectorized 4x4 kernel
            kernel_add_dot_block_4x4_vectorization<UseRestrict>(m_block_size, n, k_block_size, a + i * n + k,
                n, b + k * n, n, c + i * n, n);
        }
    }
}

/**
 * @brief Optimized version using packed data layout with vectorization.
 * Packs matrix A into a contiguous format for better memory access.
 * Uses SIMD instructions for parallel processing.
 */
template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization_packed(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc, float * packed_a) {
    // Process matrix in 4x4 blocks
    for (int32_t i = 0; i < m; i += 4) {
        // Pack 4 rows of A into contiguous memory
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
            // Initialize SIMD registers for accumulation
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            for (int32_t p = 0; p < k; p++) {
                // Load packed values from A using SIMD
                __m128 a_row_0 = _mm_set1_ps(pack_current_pos[p * 4]);
                __m128 a_row_1 = _mm_set1_ps(pack_current_pos[p * 4 + 1]);
                __m128 a_row_2 = _mm_set1_ps(pack_current_pos[p * 4 + 2]);
                __m128 a_row_3 = _mm_set1_ps(pack_current_pos[p * 4 + 3]);
                // Load values from B using SIMD
                __m128 b_col = _mm_load_ps(&b[p * ldb + j]);

                // Perform SIMD multiply-accumulate
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store results back using SIMD
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

/**
 * @brief Optimized version using packed data layout.
 * Wrapper function that calls the packed kernel implementation.
 * Uses aligned memory for better cache performance.
 */
template <bool UseRestrict = false>
void matmul_opt9_packing(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;
    constexpr int32_t KBlockSize = 32;

    // Allocate aligned memory for packed matrix
    alignas(32) float packed_a[KBlockSize * MBlockSize];

    // Process matrix in blocks
    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            // Process each block using packed kernel
            kernel_add_dot_block_4x4_vectorization_packed<UseRestrict>(m_block_size, n, k_block_size, 
                a + i* k, n, b + k * n, n, c + i * n, n, packed_a);
        }
    }
}

/**
 * @brief Optimized version using double packing (both matrices).
 * Packs both matrices A and B into contiguous format.
 * Uses SIMD instructions for parallel processing.
 */
template <bool UseRestrict = false>
void kernel_add_dot_block_4x4_vectorization_packed2(int32_t m, int32_t n, int32_t k, const float* a, int32_t lda,
    const float* b, int32_t ldb, float* c, int32_t ldc, float * packed_a, float * packed_b) {
    // Process matrix in 4x4 blocks
    for (int32_t i = 0; i < m; i += 4) {
        // Pack 4 rows of A into contiguous memory
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
            // Initialize SIMD registers for accumulation
            __m128 c_00_c_01_c_02_c_03 = _mm_setzero_ps();
            __m128 c_10_c_11_c_12_c_13 = _mm_setzero_ps();
            __m128 c_20_c_21_c_22_c_23 = _mm_setzero_ps();
            __m128 c_30_c_31_c_32_c_33 = _mm_setzero_ps();

            // Pack 4 columns of B into contiguous memory
            float * pack_b_current_pos = packed_b;
            for(int32_t p = 0; p < k; p++) {
                *(pack_b_current_pos++) = b[p * ldb + j];
                *(pack_b_current_pos++) = b[p * ldb + j + 1];
                *(pack_b_current_pos++) = b[p * ldb + j + 2];
                *(pack_b_current_pos++) = b[p * ldb + j + 3];
            }

            for (int32_t p = 0; p < k; p++) {
                // Load packed values from A using SIMD
                __m128 a_row_0 = _mm_set1_ps(pack_a_current_pos[p * 4]);
                __m128 a_row_1 = _mm_set1_ps(pack_a_current_pos[p * 4 + 1]);
                __m128 a_row_2 = _mm_set1_ps(pack_a_current_pos[p * 4 + 2]);
                __m128 a_row_3 = _mm_set1_ps(pack_a_current_pos[p * 4 + 3]);
                // Load packed values from B using SIMD
                __m128 b_col = _mm_load_ps(&(packed_b[p * 4]));

                // Perform SIMD multiply-accumulate
                c_00_c_01_c_02_c_03 = _mm_add_ps(_mm_mul_ps(a_row_0, b_col), c_00_c_01_c_02_c_03);
                c_10_c_11_c_12_c_13 = _mm_add_ps(_mm_mul_ps(a_row_1, b_col), c_10_c_11_c_12_c_13);
                c_20_c_21_c_22_c_23 = _mm_add_ps(_mm_mul_ps(a_row_2, b_col), c_20_c_21_c_22_c_23);
                c_30_c_31_c_32_c_33 = _mm_add_ps(_mm_mul_ps(a_row_3, b_col), c_30_c_31_c_32_c_33);
            }

            // Store results back using SIMD
            _mm_store_ps(&c[(i + 0) * ldc + j], c_00_c_01_c_02_c_03);
            _mm_store_ps(&c[(i + 1) * ldc + j], c_10_c_11_c_12_c_13);
            _mm_store_ps(&c[(i + 2) * ldc + j], c_20_c_21_c_22_c_23);
            _mm_store_ps(&c[(i + 3) * ldc + j], c_30_c_31_c_32_c_33);
        }
    }
}

/**
 * @brief Optimized version using double packing with aligned memory.
 * Wrapper function that calls the double-packed kernel implementation.
 * Uses aligned memory for both packed matrices.
 */
template <bool UseRestrict = false>
void matmul_opt10_packing2(const float * a,
                     const float * b,
                     float_ptr_wrapper<UseRestrict> c, 
                     int32_t n) {
    std::memset(c, 0, n * n * sizeof(float));
    constexpr int32_t MBlockSize = 32;
    constexpr int32_t KBlockSize = 32;

    // Allocate aligned memory for both packed matrices
    alignas(32) float packed_a[KBlockSize * MBlockSize];
    alignas(32) float packed_b[4 * KBlockSize];

    // Process matrix in blocks
    for(int32_t k = 0; k < n; k += KBlockSize) {
        int32_t k_block_size = std::min(n - k, KBlockSize);
        for(int32_t i = 0; i < n; i += MBlockSize) {
            int32_t m_block_size = std::min(n - i, MBlockSize);
            // Process each block using double-packed kernel
            kernel_add_dot_block_4x4_vectorization_packed2<UseRestrict>(m_block_size, n, k_block_size, 
                a + i* k, n, b + k * n, n, c + i * n, n, packed_a, packed_b);
        }
    }
}