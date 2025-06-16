#include <chrono>
#include <random>
#include <iostream>
#include <functional>

// NOTE: This program is not finished yet.

#include "stable_vector.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

constexpr int32_t WarmupTimes = 20000;
constexpr int32_t TestTimes = 10000000;

// // take the func_name as a parameter for debugging and correctness testing
// template <bool Transform = false, typename Func>
// double testStableVector(Func && func, std::string_view func_name, int32_t input_param, 
//         const std::vector<int32_t> & elements, const std::vector<int32_t> & targets) {
//     for(int32_t i = 0; i < WarmupTimes; i++) {
//         OptRef result = func(elements, targets[i]);
//         if(!result.has_value()) {
//             throw std::runtime_error("[testStableVector for " + std::string(func_name) + "] result is empty, expected " + 
//                 std::to_string(targets[i]));
//         }
//         doNotOptimizeAway(result.value());
//         if(result != targets[i]) {
//             throw std::runtime_error("[testStableVector for " + std::string(func_name) + "] result is wrong. result is " + 
//                 std::to_string(result.value()) + ", expected " + std::to_string(targets[i]));
//         }
//     }
//     auto start_time = std::chrono::high_resolution_clock::now();
//     for(int32_t i = 0; i < TestTimes; i++) {
//         OptRef result = func(elements, targets[i]);
//         doNotOptimizeAway(result.value());
//     }
//     auto end_time = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
//     // std::cout << "Function '" << func_name << "' took " << duration.count() << " µs to complete." << std::endl;
//     return duration.count() * 1.0 / TestTimes;
//     // std::cout << "result is " << result << std::endl;
// }

int main(int argc, char **argv) {
    // if(argc != 2) {
    //     throw std::runtime_error("Usage : ./executable config_path");
    // }
    // TestManager test_manager(argv[1]);

    // auto & input_params = test_manager.getInputParams();

    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<int32_t> dist(0, UpperBound * 2 - 1);

    // for(int32_t input_param : input_params) {
    //     std::uniform_int_distribution<int32_t> dist2(0, input_param - 1);
    //     std::vector<int32_t> elements(input_param);
    //     std::vector<int32_t> targets(TestTimes);
    //     for(int32_t i = 0; i < input_param; i++) {
    //         elements[i] = dist(gen);
    //     }
    //     std::sort(elements.begin(), elements.end());
    // }

    // launchFuncTest(stable_vector_baseline<false>, stable_vector_baseline);
    // launchFuncTest(stable_vector_std<false>, stable_vector_std);


    // launchFuncTestAligned(stable_vector_baseline<true>, stable_vector_baseline_aligned);
    // launchFuncTestAligned(stable_vector_std<true>, stable_vector_std_aligned);

    // test_manager.dump();
    return 0;
}