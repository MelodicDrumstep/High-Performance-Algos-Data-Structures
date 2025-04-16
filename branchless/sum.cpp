#include <chrono>
#include <random>
#include <iostream>

#include "sum.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

template <typename Func, typename RandomNumberGenerator>
double testSum(Func && func, RandomNumberGenerator && rng, int32_t input_param) {
    std::vector<int32_t> elements(input_param);
    for (int32_t i = 0; i < input_param; i++) {
        elements[i] = rng();
    }
    int32_t result;
    for(int32_t i = 0; i < WarmupTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        result = func(elements);
        doNotOptimizeAway(result);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    return duration.count() * 1.0 / TestTimes;
    // std::cout << "result is " << result << std::endl;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(0, UpperBound * 2 - 1);

    test_manager.launchTest("sum_baseline", [&dist, &gen](int32_t input_param) {
        return testSum(sum_baseline<UpperBound>, [&dist, &gen](){ return dist(gen); }, input_param);
    });
    test_manager.launchTest("sum_predication", [&dist, &gen](int32_t input_param) {
        return testSum(sum_predication<UpperBound>, [&dist, &gen](){ return dist(gen); }, input_param);
    });
    test_manager.launchTest("sum_predication_tenary", [&dist, &gen](int32_t input_param) {
        return testSum(sum_predication_tenary<UpperBound>, [&dist, &gen](){ return dist(gen); }, input_param);
    });
    test_manager.launchTest("sum_predication_masking", [&dist, &gen](int32_t input_param) {
        return testSum(sum_predication_masking<UpperBound>, [&dist, &gen](){ return dist(gen); }, input_param);
    });
    test_manager.dump();
}