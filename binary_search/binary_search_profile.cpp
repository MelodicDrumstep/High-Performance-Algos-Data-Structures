#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <chrono>
#include <cstdint>
#include <algorithm>

#include "binary_search.hpp"

constexpr int WarmupTimes = 3;
constexpr int TestTimes = 10;
constexpr int NumQueries = 1000;

using std::cout;
using std::endl;

void show_help() {
    cout << "Usage: ./binary_search_profile <array_size> <implementation>\n"
         << "Available implementations:\n"
         << "  baseline\n"
         << "  std\n"
         << "  opt1_branchless\n"
         << "  opt2_branchless2\n"
         << "  opt3_branchless3\n"
         << "  opt4_prefetch\n"
         << "  opt5_eytzinger\n"
         << "  opt6_eytzinger_branchless\n"
         << "  opt7_eytzinger_prefetch1\n"
         << "  opt8_eytzinger_prefetch2\n"
         << "  opt9_branch_removal\n"
         << endl;
}

template <typename Func, typename Vec>
void warmup(Func&& func, const Vec& arr, const std::vector<int32_t>& queries) {
    for (int i = 0; i < WarmupTimes; ++i) {
        volatile int found = 0;
        for (auto key : queries) found += func(arr, key).has_value();
    }
}

template <typename Func, typename Vec>
void actualTest(Func&& func, const Vec& arr, const std::vector<int32_t>& queries) {
    for (int i = 0; i < TestTimes; ++i) {
        volatile int found = 0;
        for (auto key : queries) found += func(arr, key).has_value();
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        show_help();
        return 1;
    }

    int32_t n;
    try {
        n = std::stoi(argv[1]);
    } catch (...) {
        std::cerr << "Error: array_size must be an integer" << std::endl;
        return 1;
    }

    std::string impl = argv[2];

    // Generate sorted array
    std::vector<int32_t> arr(n);
    for (int32_t i = 0; i < n; ++i) arr[i] = i * 2;

    // Generate random queries (half present, half not)
    std::vector<int32_t> queries;
    std::mt19937 gen(42);
    std::uniform_int_distribution<int32_t> dist(0, n * 2);
    for (int i = 0; i < NumQueries; ++i) {
        if (i % 2 == 0) {
            queries.push_back(arr[dist(gen) % n]);
        } else {
            queries.push_back(dist(gen) | 1);
        }
    }

    // Use aligned vector for aligned implementations
    AlignedVector arr_aligned(arr.begin(), arr.end());

    // Eytzinger transformation if needed
    auto arr_eytzinger = eytzinger_transformation(arr);
    AlignedVector arr_eytzinger_aligned(arr_eytzinger.begin(), arr_eytzinger.end());

    if (impl == "baseline") {
        warmup(binary_search_baseline<false>, arr, queries);
        actualTest(binary_search_baseline<false>, arr, queries);
        cout << "Sample result: " << binary_search_baseline<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "std") {
        warmup(binary_search_std<false>, arr, queries);
        actualTest(binary_search_std<false>, arr, queries);
        cout << "Sample result: " << binary_search_std<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "opt1_branchless") {
        warmup(binary_search_opt1_branchless<false>, arr, queries);
        actualTest(binary_search_opt1_branchless<false>, arr, queries);
        cout << "Sample result: " << binary_search_opt1_branchless<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "opt2_branchless2") {
        warmup(binary_search_opt2_branchless2<false>, arr, queries);
        actualTest(binary_search_opt2_branchless2<false>, arr, queries);
        cout << "Sample result: " << binary_search_opt2_branchless2<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "opt3_branchless3") {
        warmup(binary_search_opt3_branchless3<false>, arr, queries);
        actualTest(binary_search_opt3_branchless3<false>, arr, queries);
        cout << "Sample result: " << binary_search_opt3_branchless3<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "opt4_prefetch") {
        warmup(binary_search_opt4_prefetch<false>, arr, queries);
        actualTest(binary_search_opt4_prefetch<false>, arr, queries);
        cout << "Sample result: " << binary_search_opt4_prefetch<false>(arr, queries[0]).has_value() << endl;
    } else if (impl == "opt5_eytzinger") {
        warmup(binary_search_opt5_eytzinger<false>, arr_eytzinger, queries);
        actualTest(binary_search_opt5_eytzinger<false>, arr_eytzinger, queries);
        cout << "Sample result: " << binary_search_opt5_eytzinger<false>(arr_eytzinger, queries[0]).has_value() << endl;
    } else if (impl == "opt6_eytzinger_branchless") {
        warmup(binary_search_opt6_eytzinger_branchless<false>, arr_eytzinger, queries);
        actualTest(binary_search_opt6_eytzinger_branchless<false>, arr_eytzinger, queries);
        cout << "Sample result: " << binary_search_opt6_eytzinger_branchless<false>(arr_eytzinger, queries[0]).has_value() << endl;
    } else if (impl == "opt7_eytzinger_prefetch1") {
        warmup(binary_search_opt7_eytzinger_prefetch1<1, false>, arr_eytzinger, queries);
        actualTest(binary_search_opt7_eytzinger_prefetch1<1, false>, arr_eytzinger, queries);
        cout << "Sample result: " << binary_search_opt7_eytzinger_prefetch1<1, false>(arr_eytzinger, queries[0]).has_value() << endl;
    } else if (impl == "opt8_eytzinger_prefetch2") {
        warmup(binary_search_opt8_eytzinger_prefetch2<1, false>, arr_eytzinger, queries);
        actualTest(binary_search_opt8_eytzinger_prefetch2<1, false>, arr_eytzinger, queries);
        cout << "Sample result: " << binary_search_opt8_eytzinger_prefetch2<1, false>(arr_eytzinger, queries[0]).has_value() << endl;
    } else if (impl == "opt9_branch_removal") {
        warmup(binary_search_opt9_branch_removal<1, false>, arr_eytzinger, queries);
        actualTest(binary_search_opt9_branch_removal<1, false>, arr_eytzinger, queries);
        cout << "Sample result: " << binary_search_opt9_branch_removal<1, false>(arr_eytzinger, queries[0]).has_value() << endl;
    } else {
        std::cerr << "Unknown implementation: " << impl << std::endl;
        show_help();
        return 1;
    }
    return 0;
}