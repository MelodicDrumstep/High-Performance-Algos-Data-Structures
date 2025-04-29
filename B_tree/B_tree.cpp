#include <chrono>
#include <random>
#include <iostream>
#include <functional>

#include "B_tree.hpp"
#include "test_utils.hpp"

constexpr int32_t UpperBound = 100;

template <bool Aligned = false>
struct ElementsBlock {
    VecType<Aligned> elements;
    std::vector<int32_t> targets;
};

template <bool Aligned = false>
using InputParam2ElementBlockMap = std::unordered_map<int32_t, ElementsBlock<Aligned>>;

template <typename T>
concept ElementBlockMapT = std::is_same_v<InputParam2ElementBlockMap<false>, T> 
    || std::is_same_v<InputParam2ElementBlockMap<true>, T>;

// take the func_name as a parameter for debugging and correctness testing
template <bool Transform = false, typename Func, ElementBlockMapT Map>
double testBinarySearch(Func && func, std::string_view func_name, int32_t input_param, 
        const Map & input_param2elements) {
    auto & elements_block = input_param2elements.at(input_param);
    auto elements(elements_block.elements);
    const std::vector<int32_t> & targets = elements_block.targets;
    if constexpr (Transform) {
        elements = eytzinger_transformation(elements);
    }
    for(int32_t i = 0; i < WarmupTimes; i++) {
        OptRef result = func(elements, targets[i]);
        if(!result.has_value()) {
            throw std::runtime_error("[testBinarySearch for " + std::string(func_name) + "] result is empty, expected " + 
                std::to_string(targets[i]));
        }
        doNotOptimizeAway(result.value());
        if(result != targets[i]) {
            throw std::runtime_error("[testBinarySearch for " + std::string(func_name) + "] result is wrong. result is " + 
                std::to_string(result.value()) + ", expected " + std::to_string(targets[i]));
        }
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int32_t i = 0; i < TestTimes; i++) {
        OptRef result = func(elements, targets[i]);
        doNotOptimizeAway(result.value());
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    // std::cout << "Function '" << func_name << "' took " << duration.count() << " µs to complete." << std::endl;
    return duration.count() * 1.0 / TestTimes;
    // std::cout << "result is " << result << std::endl;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Usage : ./executable config_path");
    }
    TestManager test_manager(argv[1]);

    InputParam2ElementBlockMap<false> input_param2elements;
    InputParam2ElementBlockMap<true> input_param2elements_aligned;
    auto & input_params = test_manager.getInputParams();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32_t> dist(0, UpperBound * 2 - 1);

    for(int32_t input_param : input_params) {
        std::uniform_int_distribution<int32_t> dist2(0, input_param - 1);
        {
            std::vector<int32_t> elements(input_param);
            std::vector<int32_t> targets(TestTimes);
            for(int32_t i = 0; i < input_param; i++) {
                elements[i] = dist(gen);
            }
            std::sort(elements.begin(), elements.end());
            for(int32_t i = 0; i < TestTimes; i++) {
                targets[i] = elements[dist2(gen)];
            }
            input_param2elements.emplace(input_param, ElementsBlock<false>{std::move(elements), std::move(targets)});
        }

        {
            AlignedVector elements(input_param);
            std::vector<int32_t> targets(TestTimes);
            for(int32_t i = 0; i < input_param; i++) {
                elements[i] = dist(gen);
            }
            std::sort(elements.begin(), elements.end());
            for(int32_t i = 0; i < TestTimes; i++) {
                targets[i] = elements[dist2(gen)];
            }
            input_param2elements_aligned.emplace(input_param, ElementsBlock<true>{std::move(elements), std::move(targets)});
        }
    }
    

    #define launchFuncTest(system_func_name, output_func_name) \
        test_manager.launchTest(#output_func_name, [&input_param2elements](int32_t input_param) {   \
            return testBinarySearch<false>(system_func_name, #output_func_name, input_param, input_param2elements);    \
        });

    #define launchFuncTestTransform(system_func_name, output_func_name) \
    test_manager.launchTest(#output_func_name, [&input_param2elements](int32_t input_param) {   \
        return testBinarySearch<true>(system_func_name, #output_func_name, input_param, input_param2elements);    \
    });

    #define launchFuncTestAligned(system_func_name, output_func_name) \
        test_manager.launchTest(#output_func_name, [&input_param2elements_aligned](int32_t input_param) {   \
            return testBinarySearch<false>(system_func_name, #output_func_name, input_param, input_param2elements_aligned);    \
        });

    #define launchFuncTestTransformAligned(system_func_name, output_func_name) \
    test_manager.launchTest(#output_func_name, [&input_param2elements_aligned](int32_t input_param) {   \
        return testBinarySearch<true>(system_func_name, #output_func_name, input_param, input_param2elements_aligned);    \
    });

    launchFuncTest(binary_search_baseline<false>, binary_search_baseline);
    launchFuncTest(binary_search_std<false>, binary_search_std);
    launchFuncTest(binary_search_opt1_branchless<false>, binary_search_opt1_branchless);
    launchFuncTest(binary_search_opt2_branchless2<false>, binary_search_opt2_branchless2);
    launchFuncTest(binary_search_opt3_branchless3<false>, binary_search_opt3_branchless3);
    launchFuncTest(binary_search_opt4_prefetch<false>, binary_search_opt4_prefetch);
    launchFuncTestTransform(binary_search_opt5_eytzinger<false>, binary_search_opt5_eytzinger);
    launchFuncTestTransform(binary_search_opt6_eytzinger_branchless<false>, binary_search_opt6_eytzinger_branchless);
    launchFuncTestTransform((binary_search_opt7_eytzinger_prefetch<4, false>), binary_search_opt7_eytzinger_prefetch);
    launchFuncTestTransform((binary_search_opt8_branch_removal<8, false>), binary_search_opt8_branch_removal);

    launchFuncTestAligned(binary_search_baseline<true>, binary_search_baseline_aligned);
    launchFuncTestAligned(binary_search_std<true>, binary_search_std_aligned);
    launchFuncTestAligned(binary_search_opt1_branchless<true>, binary_search_opt1_branchless_aligned);
    launchFuncTestAligned(binary_search_opt2_branchless2<true>, binary_search_opt2_branchless2_aligned);
    launchFuncTestAligned(binary_search_opt3_branchless3<true>, binary_search_opt3_branchless3_aligned);
    launchFuncTestAligned(binary_search_opt4_prefetch<true>, binary_search_opt4_prefetch_aligned);
    launchFuncTestTransformAligned(binary_search_opt5_eytzinger<true>, binary_search_opt5_eytzinger_aligned);
    launchFuncTestTransformAligned(binary_search_opt6_eytzinger_branchless<true>, binary_search_opt6_eytzinger_branchless_aligned);
    launchFuncTestTransformAligned((binary_search_opt7_eytzinger_prefetch<4, true>), binary_search_opt7_eytzinger_prefetch_aligned);
    launchFuncTestTransformAligned((binary_search_opt8_branch_removal<8, true>), binary_search_opt8_branch_removal_aligned);

    test_manager.dump();
}