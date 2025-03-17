#include <chrono>
#include <random>
#include <iostream>

#include <sum.hpp>

constexpr int32_t N = 100000;
constexpr int32_t UpperBound = 100;
constexpr int32_t WramupTimes = 5000;
constexpr int32_t TestTimes = 10000;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testSum(Func && func, const std::string& funcName) {
    int32_t result;
    for(int32_t i = 0; i < WramupTimes; i++) {
        result = func();
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func();
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
}

int main() {
    std::vector<int32_t> elements(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(0, UpperBound * 2 - 1);
    for (int32_t i = 0; i < N; i++) {
        elements[i] = dist(gen);
    }

    testSum([&elements](){ return sum_baseline<UpperBound>(elements); }, "sum_baseline");
    testSum([&elements](){ return sum_predication<UpperBound>(elements); }, "sum_predication");
    testSum([&elements](){ return sum_predication_tenary<UpperBound>(elements); }, "sum_predication_tenary");
    testSum([&elements](){ return sum_predication_masking<UpperBound>(elements); }, "sum_predication_masking");
}