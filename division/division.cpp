#include <chrono>
#include <random>
#include <iostream>

#include <division.hpp>

constexpr int32_t UpperBound = 100000;
constexpr int32_t TestElements = 10000;
constexpr int32_t WarmupElements = TestElements;

using TestArray = std::array<uint32_t, WarmupElements + TestElements>;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testDivision(Func && func, const std::string& funcName, const TestArray & elements_a, const TestArray & elements_b) {
    DivResult result;
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
    // std::cout << "result is {" << result.quotient << ", " << result.remainder << "}" << std::endl;
}

int main() {
    TestArray elements_a;
    TestArray elements_b;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(1, UpperBound);
    for (int32_t i = 0; i < TestElements; i++) {
        elements_a[i] = dist(gen);
        elements_b[i] = dist(gen);
    }

    testDivision(division_baseline, "division_baseline", elements_a, elements_b);
    testDivision(division_baseline2, "division_baseline2", elements_a, elements_b);
    testDivision(division_Barrett_reduction, "division_Barrett_reduction", elements_a, elements_b);
    testDivision(division_Lemire_reduction, "division_Lemire_reduction", elements_a, elements_b);
}