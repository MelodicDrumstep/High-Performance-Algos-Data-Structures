#include <gtest/gtest.h>
#include <unordered_map>
#include <random>
#include <string>
#include "flat_hash_map_v0.hpp"
#include "flat_hash_map_v1.hpp"
#include "flat_hash_map_v2.hpp"

using namespace hpds;

// #define DEBUG_FHM_TEST

#define ChosenFlatHashMap FlatHashMapV1c

// Basic functionality tests
TEST(FlatHashMapTest, BasicOperations) {
    ChosenFlatHashMap<int, std::string> map;
    
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

// Test with string keys
TEST(FlatHashMapTest, StringKeys) {
    ChosenFlatHashMap<std::string, int> fhm;
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

TEST(FlatHashMapTest, DuplicateInsertion) {
    ChosenFlatHashMap<int, std::string> map;
    
    auto [it1, inserted1] = map.insert({1, "one"});
    EXPECT_TRUE(inserted1);
    EXPECT_EQ(it1->second, "one");

    auto [it2, inserted2] = map.insert({1, "uno"});
    EXPECT_FALSE(inserted2);  // Should not insert again
    EXPECT_EQ(it2->second, "one");  // Value should remain unchanged
}

TEST(FlatHashMapTest, IteratorBehavior) {
    ChosenFlatHashMap<int, std::string> map;
    map[1] = "one";
    map[2] = "two";
    map[3] = "three";

    int found = 0;
    for (int key : {1, 2, 3}) {
        auto it = map.find(key);
        ASSERT_NE(it, map.end());
        ++found;
    }
    EXPECT_EQ(found, 3);
}

TEST(FlatHashMapTest, ClearBehavior) {
    ChosenFlatHashMap<int, std::string> map;
    map[1] = "one";
    map[2] = "two";

    map.clear();
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.find(1), map.end());

    // After clear, reinsert and check correctness
    map[1] = "one";
    EXPECT_EQ(map[1], "one");
}

TEST(FlatHashMapTest, CustomHashFunction) {
    struct ModHash {
        size_t operator()(int key) const {
            return key % 10;
        }
    };

    ChosenFlatHashMap<int, std::string, 16, ModHash> map;
    map[15] = "fifteen";
    map[25] = "twenty-five";  // Same hash as 15

    EXPECT_EQ(map[15], "fifteen");
    EXPECT_EQ(map[25], "twenty-five");

    // Ensure collision handled properly
    EXPECT_EQ(map.size(), 2);
}

TEST(FlatHashMapTest, RehashAndLoadFactor) {
    ChosenFlatHashMap<int, std::string, 4> map;
    map.set_max_load_factor(0.25);  // Force early rehash

    map[1] = "a";
    map[2] = "b";  // Should trigger rehash here
    map[3] = "c";

    EXPECT_GE(map.get_capacity(), 8);  // Ensure capacity has grown
    EXPECT_EQ(map[1], "a");
    EXPECT_EQ(map[2], "b");
    EXPECT_EQ(map[3], "c");
}

TEST(FlatHashMapTest, StressInsertions) {
    ChosenFlatHashMap<int, int> map;
    constexpr int N = 10000;

    for (int i = 0; i < N; ++i) {
        map[i] = i * 10;
    }

    EXPECT_EQ(map.size(), N);

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(map[i], i * 10);
    }
}

TEST(FlatHashMapTest, AtConstCorrectness) {
    const ChosenFlatHashMap<int, std::string> map_const = [] {
        ChosenFlatHashMap<int, std::string> temp;
        temp.insert({42, "answer"});
        return temp;
    }();

    EXPECT_EQ(map_const.at(42), "answer");
    EXPECT_THROW(map_const.at(100), std::out_of_range);
}

// Comparison test with std::unordered_map
TEST(FlatHashMapTest, ComparisonWithStdUnorderedMap) {
    ChosenFlatHashMap<int, int, 256> fhm;
    std::unordered_map<int, int> std_map;
    
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    // Insert random elements
    for (int i = 0; i < 1000; ++i) {
        int key = dis(gen);
        int value = dis(gen);
        fhm[key] = value;
        std_map[key] = value;

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

TEST(FlatHashMapTest, RandomFuzzAgainstUnorderedMap) {
    ChosenFlatHashMap<int, std::string> flat_map;
    std::unordered_map<int, std::string> std_map;

    constexpr int num_operations = 10000;
    std::mt19937 rng(42);  // Fix the random seed
    std::uniform_int_distribution<int> key_dist(0, 10000);
    std::uniform_int_distribution<int> op_dist(0, 2);  // 0 = insert, 1 = erase, 2 = find

    for (int i = 0; i < num_operations; ++i) {
        int key = key_dist(rng);
        int op = op_dist(rng);

        if (op == 0) {
            // Insert a key with value "val<key>"
            std::string value = "val" + std::to_string(key);

            #ifdef DEBUG_FHM_TEST
            std::cout << "Insert key : " << key << ", value : " << value << std::endl;
            #endif

            flat_map[key] = value;
            std_map[key] = value;
        } else if (op == 1) {
            #ifdef DEBUG_FHM_TEST
            std::cout << "Erase key : " << key << std::endl;
            #endif

            // Erase the key
            bool flat_erased = flat_map.erase(key) > 0;
            bool std_erased = std_map.erase(key) > 0;
            EXPECT_EQ(flat_erased, std_erased) << "Mismatch on erase for key " << key;
        } else {
            #ifdef DEBUG_FHM_TEST
            std::cout << "Find key : " << key << std::endl;
            #endif

            // Find the key and compare values
            auto flat_it = flat_map.find(key);
            auto std_it = std_map.find(key);

            if (flat_it == flat_map.end()) {
                EXPECT_EQ(std_it, std_map.end()) << "ChosenFlatHashMap miss, but std::unordered_map hit for key " << key;
            } else {
                EXPECT_NE(std_it, std_map.end()) << "ChosenFlatHashMap hit, but std::unordered_map miss for key " << key;
                EXPECT_EQ(flat_it->second, std_it->second) << "Value mismatch for key " << key;
            }
        }

        // Occasionally verify full map size and contents
        if (i % (num_operations / 10) == 0) {
            EXPECT_EQ(flat_map.size(), std_map.size()) << "Size mismatch at step " << i;
            for (const auto& [key, value] : std_map) {

                #ifdef DEBUG_FHM_TEST
                std::cout << "At for key : " << key << std::endl;
                #endif

                EXPECT_EQ(flat_map.at(key), value) << "Mismatch at key " << key << " during full map check";
            }
        }
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}