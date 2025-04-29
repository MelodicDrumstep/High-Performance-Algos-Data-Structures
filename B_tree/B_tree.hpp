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

constexpr static int32_t B = 16;

static int32_t go(int32_t k, int32_t i) {
    return k * (B + 1) + i + 1;
}

template <bool Aligned>
__attribute__((noinline))
OptRef<const int32_t> binary_search_B_tree(const VecType<Aligned> & elements, int32_t target) {

}

class BTreeEytzingerTransformer {
public:
    BTreeEytzingerTransformer(int32_t nblocks)
        : nblocks_(nblocks) {}

    /**
    * @param result Logically it's a [nblocks * B] 2D array, implemented as a 1-D std::vector for performance.
    */
    void recursive_transformation_helper(auto & result, auto & elements, int32_t k) {
        if(k < nblocks_) {
            for(int32_t i = 0; i < B; i++) {
                recursive_transformation_helper(result, elements, go(k, i));
                result[k * B + i] = (original_sequential_index_ < elements.size() ? elements[original_sequential_index_++] : INT_MAX);
            }
        }
        recursive_transformation_helper(result, elements, go(k, B));
    }

    auto transform(const auto & elements) {
        std::remove_cvref_t<decltype(elements)> result(elements.size() + 1);
        // the result array in 1-indexed for performance consideration
        recursive_transformation_helper(result, elements, 1, (elements.size() + B - 1) / b);
        return result;
    }
private:
    int32_t nblocks_;
    int32_t original_sequential_index_ {0};
};