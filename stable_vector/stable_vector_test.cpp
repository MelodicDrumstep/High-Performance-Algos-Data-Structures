#include <iostream>
#include <string>
#include <cassert>
#include "stable_vector.hpp"

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

    vec.resize(1408);

    // Test element access
    all_passed = (vec[0] == 1 && vec[1] == 2 && vec[2] == 3);
    print_test_result("Element access", all_passed);
    
    // // Test front and back
    // all_passed = (vec.front() == 1 && vec.back() == 3);
    // print_test_result("Front and back", all_passed);
    
    // // Test clear
    // vec.clear();
    // all_passed = (vec.empty() && vec.size() == 0);
    // print_test_result("Clear", all_passed);
}

// // Test iterator stability
// void test_iterator_stability() {
//     std::cout << "\n=== Testing Iterator Stability ===" << std::endl;
    
//     StableVector<std::string> vec;
//     bool all_passed = true;
    
//     // Insert elements
//     vec.push_back("first");
//     vec.push_back("second");
//     vec.push_back("third");
    
//     // Get iterators
//     auto it1 = vec.begin();
//     auto it2 = vec.begin() + 1;
//     auto it3 = vec.begin() + 2;
    
//     // Store values
//     std::string val1 = *it1;
//     std::string val2 = *it2;
//     std::string val3 = *it3;
    
//     // Insert element in the middle
//     vec.insert(vec.begin() + 1, "inserted");
    
//     // Verify iterators still point to the same elements
//     all_passed &= (*it1 == val1);
//     all_passed &= (*it2 == val2);
//     all_passed &= (*it3 == val3);
//     print_test_result("Iterator stability after insert", all_passed);
// }

// // Test erase operations
// void test_erase_operations() {
//     std::cout << "\n=== Testing Erase Operations ===" << std::endl;
    
//     StableVector<int> vec;
//     bool all_passed = true;
    
//     // Insert elements
//     for (int i = 0; i < 5; ++i) {
//         vec.push_back(i);
//     }
    
//     // Get iterator to middle element
//     auto it = vec.begin() + 2;
//     int value = *it;
    
//     // Erase element
//     vec.erase(vec.begin() + 1);
    
//     // Verify iterator still points to the same element
//     all_passed &= (*it == value);
//     all_passed &= (vec.size() == 4);
//     print_test_result("Iterator stability after erase", all_passed);
    
//     // Verify other elements
//     all_passed &= (vec[0] == 0);
//     all_passed &= (vec[1] == 2);
//     all_passed &= (vec[2] == 3);
//     all_passed &= (vec[3] == 4);
//     print_test_result("Element order after erase", all_passed);
// }

// // Test with custom type
// struct TestStruct {
//     int value;
//     std::string name;
    
//     TestStruct(int v, const std::string& n) : value(v), name(n) {}
    
//     bool operator==(const TestStruct& other) const {
//         return value == other.value && name == other.name;
//     }
// };

// void test_custom_type() {
//     std::cout << "\n=== Testing Custom Type ===" << std::endl;
    
//     StableVector<TestStruct> vec;
//     bool all_passed = true;
    
//     // Insert elements
//     vec.push_back(TestStruct(1, "one"));
//     vec.push_back(TestStruct(2, "two"));
    
//     // Get iterator
//     auto it = vec.begin();
    
//     // Insert new element
//     vec.insert(vec.begin(), TestStruct(0, "zero"));
    
//     // Verify iterator stability
//     all_passed &= (it->value == 1);
//     all_passed &= (it->name == "one");
//     print_test_result("Custom type iterator stability", all_passed);
    
//     // Verify all elements
//     all_passed &= (vec[0].value == 0 && vec[0].name == "zero");
//     all_passed &= (vec[1].value == 1 && vec[1].name == "one");
//     all_passed &= (vec[2].value == 2 && vec[2].name == "two");
//     print_test_result("Custom type element access", all_passed);
// }

int main() {
    std::cout << "Starting StableVector Tests...\n" << std::endl;
    
    test_basic_operations();
    // test_iterator_stability();
    // test_erase_operations();
    // test_custom_type();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
} 