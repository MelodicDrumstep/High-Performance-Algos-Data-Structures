#include <chrono>
#include <random>
#include <iostream>

#include "prefix_sum.hpp"
#include "test_utils.hpp"

// For simplicity, the tested functions assume the size of the array is a multiple of 8.
// And in reality when it's not, the prefix sum of the rest of the elements should
// be calculated in the naive way.

constexpr int32_t UpperBound = 100000;

struct ElementsBlock {
    Vector elements;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;


template <typename Func>
double testPrefixSum(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements] = input_param2elements.at(input_param);
    Vector result;
    for(int32_t i = 0; i < WarmupTimes; i++) {
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
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    // std::cout << "Function '" << funcName << "' took " << duration.count() << " µs to complete." << std::endl;
    // std::cout << "result.front() is " << result.front() << std::endl;
    // std::cout << "result[result.size() / 2 + 3] is " << result[result.size() / 2 + 3] << std::endl;
    // std::cout << "result.back() is " << result.back() << std::endl;
    return duration.count() * 1.0 / TestTimes;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    InputParam2ElementBlockMap input_param2elements;
    auto & input_params = test_manager.getInputParams();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(UpperBound / 2, UpperBound);

    for(int32_t input_param : input_params) {
        Vector elements(input_param);
        for(int32_t i = 0; i < input_param; i++) {
            elements[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements)});
    }

    test_manager.launchTest("prefix_sum_baseline", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_baseline, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_baseline2", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_baseline2, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_std", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_std, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_SIMD", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_SIMD, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_SIMD_blocking", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_SIMD_blocking<false>, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_SIMD_blocking_prefetching", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_SIMD_blocking<true>, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_SIMD_blocking_interleaving", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_SIMD_blocking_interleaving<false>, input_param, input_param2elements);
    });
    test_manager.launchTest("prefix_sum_SIMD_blocking_interleaving_prefetching", [&input_param2elements](int32_t input_param) {
        return testPrefixSum(prefix_sum_SIMD_blocking_interleaving<true>, input_param, input_param2elements);
    });
    test_manager.dump();
}