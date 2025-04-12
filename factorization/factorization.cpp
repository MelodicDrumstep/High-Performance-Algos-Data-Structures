#include <chrono>
#include <random>
#include <iostream>

#include <factorization.hpp>

constexpr int32_t UpperBound = 500;
constexpr int32_t TestElements = 10000;
constexpr int32_t WarmupElements = TestElements;

using TestArray = std::array<int32_t, WarmupElements + TestElements>;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

template <typename Func>
void testFactorization(Func && func, const std::string& funcName, const TestArray & elements) {
    int32_t result;
    for(int32_t i = 0; i < WarmupElements; i++) {
        result = func(elements[i]);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestElements; i++) {
        result = func(elements[i]);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result is " << result << std::endl;
}

int main() {
    TestArray elements;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(1, UpperBound);
    for (int32_t i = 0; i < TestElements; i++) {
        elements[i] = dist(gen);
    }

    testFactorization(find_factor_baseline, "find_factor_baseline", elements);
    testFactorization(find_factor_brute_pruning, "find_factor_brute_pruning", elements);
    testFactorization(find_factor_lookup_table, "find_factor_lookup_table", elements);
    testFactorization(find_factor_wheel, "find_factor_wheel", elements);
    testFactorization(find_factor_wheel2, "find_factor_wheel2", elements);
    testFactorization(find_factor_prime_table, "find_factor_prime_table", elements);
    testFactorization(find_factor_prime_table_lemire, "find_factor_prime_table_lemire", elements);
    testFactorization(find_factor_Pollard_Pho, "find_factor_Pollard_Pho", elements);
    testFactorization(find_factor_Pollard_Brent, "find_factor_Pollard_Brent", elements);
    // testFactorization(find_factor_Pollard_Brent_batch, "find_factor_Pollard_Brent_batch", elements);
    testFactorization(find_factor_Pollard_Brent_batch_opt, "find_factor_Pollard_Brent_batch_opt", elements);
}