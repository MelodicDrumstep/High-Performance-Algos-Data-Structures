#include <chrono>
#include <random>
#include <iostream>

#include <gcd.hpp>

constexpr int32_t UpperBound = 100;
constexpr int32_t TestElements = 10000;
constexpr int32_t WarmupElements = TestElements;

using TestArray = std::array<int32_t, WarmupElements + TestElements>;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testGcd(Func && func, const std::string& funcName, const TestArray & elements_a, const TestArray & elements_b) {
    int32_t result;
    for(int32_t i = 0; i < WarmupElements; i++) {
        result = func(elements_a[i], elements_b[i]);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestElements; i++) {
        result = func(elements_a[i], elements_b[i]);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
}

int main() {
    TestArray elements_a;
    TestArray elements_b;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(1, UpperBound);
    for (int32_t i = 0; i < TestElements; i++) {
        elements_a[i] = dist(gen);
        elements_b[i] = dist(gen);
    }

    testGcd(gcd_baseline_recursion, "gcd_baseline_recursion", elements_a, elements_b);
    testGcd(gcd_baseline_loop, "gcd_baseline_loop", elements_a, elements_b);
    testGcd(gcd_binary, "gcd_binary", elements_a, elements_b);
    testGcd(gcd_binary_opt1, "gcd_binary_opt1", elements_a, elements_b);
    testGcd(gcd_binary_opt2, "gcd_binary_opt2", elements_a, elements_b);
}