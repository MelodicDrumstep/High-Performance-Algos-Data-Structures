#include <chrono>
#include <random>
#include <iostream>

#include "argmin.hpp"

constexpr int32_t N = 10000;
constexpr int32_t UpperBound = 100000;
constexpr int32_t WramupTimes = 5000;
constexpr int32_t TestTimes = 10000;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testArgmin(Func && func, const std::string& funcName, const Vector & elements) {
    int32_t result;
    for(int32_t i = 0; i < WramupTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    std::cout << "result is " << result << std::endl;
    std::cout << "min value is " << elements[result] << "\n";
}

int main() {
    Vector elements(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(UpperBound / 2, UpperBound);
    for (int32_t i = 0; i < N; i++) {
        elements[i] = dist(gen);
    }

    testArgmin(argmin_baseline, "argmin_baseline", elements);
    testArgmin(argmin_baseline_with_hint, "argmin_baseline_with_hint", elements);
    testArgmin(argmin_std, "argmin_std", elements);
    testArgmin(argmin_vectorize, "argmin_vectorize", elements);
    testArgmin(argmin_vectorize2, "argmin_vectorize2", elements);
    testArgmin(argmin_vectorize2_with_hint, "argmin_vectorize2_with_hint", elements);
    testArgmin(argmin_vectorize2_unroll2, "argmin_vectorize2_unroll2", elements);
    testArgmin(argmin_vectorize2_unroll4, "argmin_vectorize2_unroll4", elements);
    testArgmin(argmin_blocking_breakdown, "argmin_blocking_breakdown", elements);
}