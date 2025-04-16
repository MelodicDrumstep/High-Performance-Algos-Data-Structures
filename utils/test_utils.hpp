#pragma once

#include <fstream>
#include <map>
#include <string_view>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

constexpr int32_t WarmupTimes = 2000;
constexpr int32_t TestTimes = 10000;

template <typename T> inline void doNotOptimizeAway(T&& datum) {
    asm volatile ("" : "+r" (datum));
}

class TestManager {
public:
    TestManager(std::string_view config_file_path) {
        std::ifstream input_config_file;
        input_config_file.open(std::string(config_file_path));
        if(!input_config_file.is_open()) {
            throw std::runtime_error("Could not open the file " + std::string(config_file_path));
        }

        json config;
        input_config_file >> config;

        test_name_ = config["test_name"].get<std::string>();
        input_params_ = std::move(config["input_params"].get<std::vector<int32_t>>());
        output_file_path_ = config["output_file_path"].get<std::string>();
    }
    
    template <typename Func>
    void launchTest(const std::string & case_name, Func && func) {
        std::vector<double> test_result;
        for(int32_t input_param : input_params_) {
            double time_count = func(input_param);
            test_result.push_back(time_count);
        }

        test_results_.emplace_back(case_name, std::move(test_result));
    }

    void dump() {
        json output_json;
        output_json["test_name"] = test_name_;
        output_json["input_params"] = input_params_;

        // Auto convert the unit
        constexpr double us_threshold = 1'000.0;    // 1μs = 1000ns
        constexpr double ms_threshold = 1'000'000.0; // 1ms = 1000000ns
        
        // find the maximum value
        double max_time = std::numeric_limits<double>::max();
        for (auto& [_, times] : test_results_) {
            auto it = std::min_element(times.begin(), times.end());
            if (it != times.end()) max_time = std::min(max_time, *it);
        }
        
        std::string unit = "ns";
        double scale = 1.0;
        
        if (max_time >= ms_threshold) {
            unit = "ms";
            scale = 1.0 / 1'000'000;
        } else if (max_time >= us_threshold) {
            unit = "us";
            scale = 1.0 / 1'000;
        }

        for(auto & [case_name, result] : test_results_) {
            for(auto & value : result) {
                // adjustment
                value *= scale;
            }
            output_json[case_name] = result;
        }
        output_json["unit"] = unit;
        std::ofstream output_file(output_file_path_);
        output_file << output_json.dump(4);
        output_file.close();
    }

private:
    struct TestResultNode {
        TestResultNode(const std::string & arg_name, std::vector<double> && arg_result)
            : name(arg_name), result(std::move(arg_result)) {}
        std::string name;
        std::vector<double> result;
    };

    std::string test_name_;
    std::string output_file_path_;

    std::vector<int32_t> input_params_;
    std::vector<TestResultNode> test_results_;
};