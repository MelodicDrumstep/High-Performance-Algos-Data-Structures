#include <chrono>
#include <random>
#include <iostream>

#include "matmul.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 10000;

struct ElementsBlock {
    Vector elements_a;
    Vector elements_b;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testMatmul(Func && func, int32_t input_param, const InputParam2ElementBlockMap & input_param2elements) {
    auto & [elements_a, elements_b] = input_param2elements.at(input_param);
    Vector result(input_param * input_param, 0);
    for(int32_t i = 0; i < WarmupTimes; i++) {
        func(elements_a.data(), elements_b.data(), result.data(), input_param);
        doNotOptimizeAway(result[0]);
        doNotOptimizeAway(result[result.size() - 1]);
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        func(elements_a.data(), elements_b.data(), result.data(), input_param);
        doNotOptimizeAway(result[0]);
        doNotOptimizeAway(result[result.size() - 1]);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
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
        size_t array_size = input_param * input_param;
        Vector elements_a(array_size);
        Vector elements_b(array_size);
        for(int32_t i = 0; i < array_size; i++) {
            elements_a[i] = dist(gen);
            elements_b[i] = dist(gen);
        }
        input_param2elements.emplace(input_param, ElementsBlock{std::move(elements_a), std::move(elements_b)});
    }

    test_manager.launchTest("matmul_baseline", [&input_param2elements](int32_t input_param) {
        return testMatmul(matmul_baseline<false>, input_param, input_param2elements);
    });
    test_manager.launchTest("matmul_baseline_restricted", [&input_param2elements](int32_t input_param) {
        return testMatmul(matmul_baseline<true>, input_param, input_param2elements);
    });
    test_manager.launchTest("matmul_transpose", [&input_param2elements](int32_t input_param) {
        return testMatmul(matmul_transpose<false>, input_param, input_param2elements);
    });
    test_manager.launchTest("matmul_transpose_restricted", [&input_param2elements](int32_t input_param) {
        return testMatmul(matmul_transpose<true>, input_param, input_param2elements);
    });
    test_manager.launchTest("matmul_vectorization", [&input_param2elements](int32_t input_param) {
        return testMatmul(matmul_vectorization, input_param, input_param2elements);
    });
    test_manager.dump();
}