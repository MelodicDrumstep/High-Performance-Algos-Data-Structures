#include <gtest/gtest.h>
#include <unordered_map>
#include <random>
#include <string>
#include "flat_hash_map.hpp"

using namespace hpds;

#define DEBUG_FHM_TEST

// Basic functionality tests
TEST(FlatHashMapTest, BasicOperations) {
    FlatHashMap<int, std::string> map;
    
    // Test empty and size
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    
    // Test insert
    auto [it1, inserted1] = map.insert({1, "one"});
    EXPECT_TRUE(inserted1);
    EXPECT_EQ(it1->first, 1);
    EXPECT_EQ(it1->second, "one");
    EXPECT_EQ(map.size(), 1);
    
    // Test find
    auto it2 = map.find(1);
    EXPECT_NE(it2, map.end());
    EXPECT_EQ(it2->first, 1);
    EXPECT_EQ(it2->second, "one");
    
    // Test operator[]
    map[2] = "two";
    EXPECT_EQ(map[2], "two");
    EXPECT_EQ(map.size(), 2);
    
    // Test at
    EXPECT_EQ(map.at(1), "one");
    EXPECT_THROW(map.at(3), std::out_of_range);
    
    // Test erase
    EXPECT_EQ(map.erase(1), 1);
    EXPECT_EQ(map.size(), 1);
    EXPECT_EQ(map.erase(1), 0);
    
    // Test clear
    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
}

// Comparison test with std::unordered_map
TEST(FlatHashMapTest, ComparisonWithStdUnorderedMap) {
    FlatHashMap<int, int, 256> fhm;
    std::unordered_map<int, int> std_map;
    
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    // Insert random elements
    for (int i = 0; i < 1000; ++i) {
        int key = dis(gen);
        int value = dis(gen);

        // #ifdef DEBUG_FHM_TEST
        // std::cout << "Insertion test. Before insertion. key is " << key << ", value is " << value << "\n";
        // std::cout << "fhm key existence : " << (fhm.find(key) != fhm.end()) << "\n";
        // std::cout << "std_map key existence : " << (std_map.find(key) != std_map.end()) << "\n";
        // std::cout << "fhm size is " << fhm.size() << std::endl;
        // std::cout << "fhm capacity is " << fhm.get_capacity() << std::endl;
        // #endif

        fhm[key] = value;
        std_map[key] = value;

        // #ifdef DEBUG_FHM_TEST
        // std::cout << "Insertion test. After insertion. fhm size is " << fhm.size() << std::endl;
        // std::cout << "fhm capacity is " << fhm.get_capacity() << std::endl;
        // #endif
        
        // Verify consistency after each insertion
        EXPECT_EQ(fhm.size(), std_map.size());
        EXPECT_EQ(fhm[key], std_map[key]);
    }
    
    // Test find operations
    for (int i = 0; i < 1000; ++i) {
        int key = dis(gen);
        auto fhm_it = fhm.find(key);
        auto std_it = std_map.find(key);
        
        if (fhm_it == fhm.end()) {
            EXPECT_EQ(std_it, std_map.end());
        } else {
            EXPECT_NE(std_it, std_map.end());
            EXPECT_EQ(fhm_it->first, std_it->first);
            EXPECT_EQ(fhm_it->second, std_it->second);
        }
    }
    
    // Test erase operations
    for (int i = 0; i < 500; ++i) {
        int key = dis(gen);
        size_t fhm_erased = fhm.erase(key);
        size_t std_erased = std_map.erase(key);
        
        EXPECT_EQ(fhm_erased, std_erased);
        EXPECT_EQ(fhm.size(), std_map.size());
    }
}

// Test with string keys
TEST(FlatHashMapTest, StringKeys) {
    FlatHashMap<std::string, int> fhm;
    std::unordered_map<std::string, int> std_map;
    
    // Insert some string keys
    std::vector<std::string> keys = {"apple", "banana", "cherry", "date", "elderberry"};
    for (size_t i = 0; i < keys.size(); ++i) {
        fhm[keys[i]] = i;
        std_map[keys[i]] = i;
    }
    
    // Verify all keys
    for (const auto& key : keys) {
        EXPECT_EQ(fhm[key], std_map[key]);
    }
    
    // Test non-existent key
    EXPECT_EQ(fhm.find("fig"), fhm.end());
    EXPECT_EQ(std_map.find("fig"), std_map.end());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 