#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <random>
#include "stable_vector.hpp"

#define DEBUG_STABLE_VECTOR_TEST

using namespace hpds;

// Helper function to print test results
void print_test_result(const std::string& test_name, bool passed) {
    std::cout << "Test " << test_name << ": " << (passed ? "PASSED" : "FAILED") << std::endl;
}

// Test basic operations
void test_basic_operations() {
    std::cout << "\n=== Testing Basic Operations ===" << std::endl;
    
    StableVector<int> vec;
    bool all_passed = true;
    
    // Test empty and size
    all_passed &= vec.empty();
    all_passed &= (vec.size() == 0);
    print_test_result("Empty and size check", all_passed);
    
    // Test push_back
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    all_passed = (vec.size() == 3);
    print_test_result("Push back", all_passed);

    vec.expand_capacity_to(1024);

    // Test element access
    all_passed = (vec[0] == 1 && vec[1] == 2 && vec[2] == 3);

    // DEBUGING
    #ifdef DEBUG_STABLE_VECTOR_TEST
    std::cout << "[test_basic_operations] vec[0]: " << vec[0] << ", vec[1]: " << vec[1] << ", vec[2]: " << vec[2] << std::endl;
    #endif
    // DEBUGING
    
    print_test_result("Element access", all_passed);
    
    // Test front and back
    all_passed = (vec.front() == 1 && vec.back() == 3);

    // DEBUGING
    #ifdef DEBUG_STABLE_VECTOR_TEST
    std::cout << "[test_basic_operations] vec.front(): " << vec.front() << ", vec.back(): " << vec.back() << std::endl;
    #endif
    // DEBUGING

    print_test_result("Front and back", all_passed);
    
    // Test clear
    vec.clear();
    all_passed = (vec.empty() && vec.size() == 0);
    print_test_result("Clear", all_passed);
}

// Test iterator stability
void test_iterator_stability() {
    std::cout << "\n=== Testing Iterator Stability ===" << std::endl;
    
    StableVector<std::string> vec;
    bool all_passed = true;
    
    // Insert elements
    vec.push_back("first");
    vec.push_back("second");
    vec.push_back("third");

    auto it = vec.begin();
    vec.emplace_back("fourth");

    all_passed &= (*it == "first");
    all_passed &= (*(it + 1) == "second");
    all_passed &= (*(it + 2) == "third");
    all_passed &= (*(it + 3) == "fourth");
    print_test_result("Iterator stability", all_passed);
}

// Test with custom type
struct TestStruct {
    int value;
    std::string name;
    TestStruct() : value(0), name("") {}

    TestStruct(int v, const std::string& n) : value(v), name(n) {}
    
    bool operator==(const TestStruct& other) const {
        return value == other.value && name == other.name;
    }
};

void test_custom_type() {
    std::cout << "\n=== Testing Custom Type ===" << std::endl;
    
    StableVector<TestStruct> vec;
    bool all_passed = true;
    
    // Insert new element
    vec.emplace_back(0, "zero");

    // Get iterator
    auto it = vec.begin();
    
    // Insert elements
    vec.push_back(TestStruct(1, "one"));
    vec.push_back(TestStruct(2, "two"));

    // Verify iterator stability
    all_passed &= (*it == TestStruct(0, "zero"));
    all_passed &= (*(it + 1) == TestStruct(1, "one"));
    all_passed &= (*(it + 2) == TestStruct(2, "two"));
    print_test_result("Custom type iterator stability", all_passed);
    
    // Verify all elements
    all_passed &= (vec[0].value == 0 && vec[0].name == "zero");
    all_passed &= (vec[1].value == 1 && vec[1].name == "one");
    all_passed &= (vec[2].value == 2 && vec[2].name == "two");
    print_test_result("Custom type element access", all_passed);
}

// Comparison test with std::vector
void test_comparison_with_std_vector() {
    std::cout << "\n=== Testing Comparison with std::vector ===" << std::endl;
    
    StableVector<int> stable_vec;
    std::vector<int> std_vec;
    bool all_passed = true;
    
    // Test push_back operations
    for (int i = 0; i < 1000; ++i) {
        stable_vec.push_back(i);
        std_vec.push_back(i);
        all_passed &= (stable_vec.size() == std_vec.size());
        all_passed &= (stable_vec.back() == std_vec.back());
    }
    print_test_result("Push back comparison", all_passed);
    
    // Test iterator operations
    all_passed = true;
    auto stable_it = stable_vec.begin();
    auto std_it = std_vec.begin();
    
    for (size_t i = 0; i < 1000; ++i) {
        all_passed &= (*stable_it == *std_it);
        ++stable_it;
        ++std_it;
    }
    print_test_result("Iterator comparison", all_passed);
    
    // Test random access
    all_passed = true;
    for (size_t i = 0; i < 1000; ++i) {
        all_passed &= (stable_vec[i] == std_vec[i]);
    }
    print_test_result("Random access comparison", all_passed);
    
    // Test front and back
    all_passed = (stable_vec.front() == std_vec.front() && 
                 stable_vec.back() == std_vec.back());
    print_test_result("Front and back comparison", all_passed);
    
    // Test clear
    stable_vec.clear();
    std_vec.clear();
    all_passed = (stable_vec.empty() == std_vec.empty() && 
                 stable_vec.size() == std_vec.size());
    print_test_result("Clear comparison", all_passed);
}

// Test iterator stability with random insertions
void test_iterator_stability_with_random_insertions() {
    std::cout << "\n=== Testing Iterator Stability with Random Insertions ===" << std::endl;
    
    StableVector<int> vec;
    std::vector<int> std_vec;
    bool all_passed = true;
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000000);
    
    // Insert initial elements
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
        std_vec.push_back(i);
    }
    
    // Store iterators to some elements
    auto it1 = vec.begin() + 10;  // Points to 10
    auto it2 = vec.begin() + 50;  // Points to 50
    auto it3 = vec.begin() + 90;  // Points to 90
    
    // Store the values these iterators point to
    int val1 = *it1;
    int val2 = *it2;
    int val3 = *it3;
    
    // Insert many random numbers
    for (int i = 0; i < 10000; ++i) {
        int random_num = dis(gen);
        vec.push_back(random_num);
        std_vec.push_back(random_num);
        
        // Periodically check if iterators still point to correct values
        if (i % 1000 == 0) {
            all_passed &= (*it1 == val1);
            all_passed &= (*it2 == val2);
            all_passed &= (*it3 == val3);
            
            // Also verify the elements are in correct positions in the vector
            all_passed &= (vec[10] == val1);
            all_passed &= (vec[50] == val2);
            all_passed &= (vec[90] == val3);
        }
    }
    
    // Final check of iterator stability
    all_passed &= (*it1 == val1);
    all_passed &= (*it2 == val2);
    all_passed &= (*it3 == val3);
    print_test_result("Iterator stability after 10000 random insertions", all_passed);
    
    // Verify the entire vector matches std::vector
    all_passed = true;
    for (size_t i = 0; i < vec.size(); ++i) {
        all_passed &= (vec[i] == std_vec[i]);
    }
    print_test_result("Vector content matches std::vector after random insertions", all_passed);
}

int main() {
    std::cout << "Starting StableVector Tests...\n" << std::endl;
    
    test_basic_operations();
    test_iterator_stability();
    test_custom_type();
    test_comparison_with_std_vector();
    test_iterator_stability_with_random_insertions();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
} 