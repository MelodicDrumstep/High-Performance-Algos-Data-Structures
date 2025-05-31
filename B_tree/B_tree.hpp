#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <functional>
#include <optional>
#include <concepts>
#include <type_traits>
#include <immintrin.h>
#include <limits>
#include "aligned_allocator.hpp"

// Use the aligned allocator for better performance with SIMD
using AlignedVector = std::vector<int32_t, AlignedAllocator<int32_t>>;

template <bool Aligned>
using VecType = std::conditional_t<Aligned, AlignedVector, std::vector<int32_t>>;

template <typename T>
using OptRef = std::optional<std::reference_wrapper<T>>;

class BTreeEytzingerTransformer {
public:
    /**
    * @param result Logically it's a [nblocks * B] 2D array, implemented as a 1-D std::vector for performance.
    */
    void recursive_transformation_helper(auto & result, auto & elements, int32_t k) {
        // std::cout << "[recursive_transformation_helper] k: " << k << std::endl;
        if(k < nblocks_) {
            for(int32_t i = 0; i < B; i++) {
                // std::cout << "[recursive_transformation_helper] k: " << k << ", i: " 
                //     << i << ", getBTreeIndex(k, i): " << getBTreeIndex(k, i) << std::endl;
                recursive_transformation_helper(result, elements, getBTreeIndex(k, i));
                // std::cout << "Result size: " << result.size() << std::endl;
                // std::cout << "k * B + i: " << k * B + i << std::endl;
                result[k * B + i] = (original_sequential_index_ < elements.size() ? elements[original_sequential_index_++] : std::numeric_limits<int32_t>::max());
            }
            recursive_transformation_helper(result, elements, getBTreeIndex(k, B));
        }
    }

    auto transform(const auto & elements) {
        nblocks_ = size2nblocks(elements.size());
        std::remove_cvref_t<decltype(elements)> result(elements.size() + 1);
        recursive_transformation_helper(result, elements, 0);
        return result;
    }

    constexpr static int32_t B = 16;

    static int32_t getBTreeIndex(int32_t k, int32_t i) {
        return k * (B + 1) + i + 1;
    }

    static int32_t size2nblocks(int32_t size) {
        return (size + B - 1) / B;
    }

private:
    int32_t nblocks_;
    int32_t original_sequential_index_ {0};
};

int32_t cmp(__m256i x_vec, const int32_t * y_ptr) {
    __m256i y_vec = _mm256_load_si256(reinterpret_cast<const __m256i *>(y_ptr));
    __m256i mask = _mm256_cmpgt_epi32(x_vec, y_vec);
    return _mm256_movemask_ps(reinterpret_cast<__m256>(mask));
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_B_tree(const VecType<Aligned> & elements_transformed, int32_t target) {
    constexpr int32_t B = BTreeEytzingerTransformer::B;
    int32_t k = 0;
    const int32_t * res = nullptr;
    __m256i x = _mm256_set1_epi32(target);
    int32_t nblocks = BTreeEytzingerTransformer::size2nblocks(elements_transformed.size());
    while(k < nblocks) {
        int32_t mask = ~(cmp(x, &elements_transformed[k * B]) + 
            (cmp(x, &elements_transformed[k * B + 8]) << 8));
        int32_t i = __builtin_ffs(mask) - 1;
        if(i < B) {
            res = &elements_transformed[k * B + i];
        }
        k = BTreeEytzingerTransformer::getBTreeIndex(k, i);
    }
    if(!res) {
        return std::nullopt;
    }
    return *res;
}