#include <chrono>
#include <random>
#include <iostream>

#include "matmul.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 10000;

// #define DEBUG_MATMUL

struct ElementsBlock {
    Vector elements_a;
    Vector elements_b;
};

using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock>;

template <typename Func>
double testMatmul(Func && func, std::string_view func_name, int32_t input_param, 
        const InputParam2ElementBlockMap & input_param2elements) {
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
    std::cout << "\nFunction '" << func_name << "' took " << duration.count() << " µs to complete." << std::endl;
    
    std::cout << "result.front() is " << result.front() << std::endl;
    std::cout << "result[result.size() / 2 + 3] is " << result[result.size() / 2 + 3] << std::endl;
    std::cout << "result.back() is " << result.back() << std::endl;
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

    #ifdef DEBUG_MATMUL
        for(int32_t input_param : input_params) {
            size_t array_size = input_param * input_param;
            Vector elements_a(array_size);
            Vector elements_b(array_size);
            for(int32_t i = 0; i < array_size; i++) {
                elements_a[i] = 1.0;
                elements_b[i] = 1.0;
            }
            input_param2elements.emplace(input_param, ElementsBlock{std::move(elements_a), std::move(elements_b)});
        }
    #else 
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
    #endif

    #define launchFuncTest(system_func_name, string_func_name) \
        test_manager.launchTest(#string_func_name, [&input_param2elements](int32_t input_param) {   \
            return testMatmul(system_func_name, #string_func_name, input_param, input_param2elements);    \
        });

    launchFuncTest(matmul_baseline<false>, matmul_baseline);
    launchFuncTest(matmul_baseline<true>, matmul_baseline_restricted);
    // launchFuncTest(matmul_baseline_loop_interchange<false>, matmul_baseline_loop_interchange);
    launchFuncTest(matmul_opt1_loop_interchange<true>, matmul_opt1_loop_interchange);
    // launchFuncTest(matmul_baseline_loop_interchange_invariant<false>, matmul_baseline_loop_interchange_invariant);
    launchFuncTest(matmul_opt2_invariant<true>, matmul_opt2_invariant);
    launchFuncTest(matmul_opt3_register_reuse<true>, matmul_opt3_register_reuse);
    launchFuncTest(matmul_opt4_register_reuse2<true>, matmul_opt4_register_reuse2);
    launchFuncTest(matmul_opt5_4x4<true>, matmul_opt5_4x4);
    launchFuncTest(matmul_opt6_blocking_4x4<true>, matmul_opt6_blocking_4x4);
    launchFuncTest(matmul_opt7_4x4_vectorization<true>, matmul_opt7_4x4_vectorization);
    launchFuncTest(matmul_opt8_blocking_4x4_vectorization<true>, matmul_opt8_blocking_4x4_vectorization);
    launchFuncTest(matmul_opt9_packing<true>, matmul_opt9_packing);
    launchFuncTest(matmul_opt10_packing2<true>, matmul_opt10_packing2);
    // launchFuncTest(matmul_baseline_loop_interchange_unroll4<false>, matmul_baseline_loop_interchange_unroll4);
    // launchFuncTest(matmul_baseline_loop_interchange_unroll4<true>, matmul_baseline_loop_interchange_unroll4_restricted);
    launchFuncTest(matmul_transpose<true>, matmul_transpose);
    launchFuncTest(matmul_vectorization, matmul_vectorization);
    launchFuncTest(matmul_kernel_blocking, matmul_kernel_blocking);
    
    test_manager.dump();
}