#include <chrono>
#include <random>
#include <iostream>

#include "prefix_sum.hpp"

constexpr int32_t N = 4096;
constexpr int32_t UpperBound = 100000;
constexpr int32_t WramupTimes = 5000;
constexpr int32_t TestTimes = 10000;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testPrefixSum(Func && func, const std::string& funcName, const Vector & elements) {
    Vector result;
    for(int32_t i = 0; i < WramupTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result[0]);
        doNotOptimizeAway(result[result.size() - 1]);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements);
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
    Vector elements(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(UpperBound / 2, UpperBound);
    for (int32_t i = 0; i < N; i++) {
        elements[i] = dist(gen);
    }

    testPrefixSum(prefix_sum_baseline, "prefix_sum_baseline", elements);
    testPrefixSum(prefix_sum_baseline2, "prefix_sum_baseline2", elements);
    testPrefixSum(prefix_sum_std, "prefix_sum_std", elements);
    testPrefixSum(prefix_sum_SIMD, "prefix_sum_SIMD", elements);
    testPrefixSum(prefix_sum_SIMD_blocking, "prefix_sum_SIMD_blocking", elements);
}