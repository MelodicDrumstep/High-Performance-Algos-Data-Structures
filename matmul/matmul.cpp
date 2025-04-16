#include <chrono>
#include <random>
#include <iostream>

#include "matmul.hpp"

constexpr int32_t N = 32;
// For simplicity, the tested functions assume the size of the array is a multiple of 8.
// And in reality when it's not, the prefix sum of the rest of the elements should
// be calculated in the naive way.

constexpr int32_t UpperBound = 10000;
constexpr int32_t WramupTimes = 5000;
constexpr int32_t TestTimes = 10000;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testMatmul(Func && func, const std::string& funcName, const Vector & elements_a, const Vector & elements_b) {
    Vector result(N * N);
    for(int32_t i = 0; i < WramupTimes; i++) {
        func(elements_a.data(), elements_b.data(), result.data(), N);
        doNotOptimizeAway(result[0]);
        doNotOptimizeAway(result[result.size() - 1]);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        func(elements_a.data(), elements_b.data(), result.data(), N);
        doNotOptimizeAway(result[0]);
        doNotOptimizeAway(result[result.size() - 1]);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    std::cout << "result.front() is " << result.front() << std::endl;
    std::cout << "result[result.size() / 2 + 3] is " << result[result.size() / 2 + 3] << std::endl;
    std::cout << "result.back() is " << result.back() << std::endl;
}

int main() {
    Vector elements_a(N * N);
    Vector elements_b(N * N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0, static_cast<float>(UpperBound));
    for (int32_t i = 0; i < N * N; i++) {
        elements_a[i] = dist(gen);
        elements_b[i] = dist(gen);
    }

    testMatmul(matmul_baseline<false>, "matmul_baseline", elements_a, elements_b);
    testMatmul(matmul_baseline<true>, "matmul_baseline_restricted", elements_a, elements_b);
    testMatmul(matmul_transpose<false>, "matmul_transpose", elements_a, elements_b);
    testMatmul(matmul_transpose<true>, "matmul_transpose_restricted", elements_a, elements_b);
    testMatmul(matmul_vectorization, "matmul_vectorization", elements_a, elements_b);
    // testMatmul(matmul_simd_no_copy, "matmul_simd_no_copy", elements_a, elements_b);
    // testMatmul(matmul_block_transpose, "matmul_block_transpose", elements_a, elements_b);
}