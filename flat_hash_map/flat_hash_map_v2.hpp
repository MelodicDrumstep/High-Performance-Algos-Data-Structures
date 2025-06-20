#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include <optional>
#include <cassert>
#include <bitset>
#include <cstddef>

// #define DEBUG_FHM
// #define PRINT_FHM_CMP_POS

namespace hpds {
// hpds is for High-Performance Data Structures

/** 
 * @brief Add a "is_removed" bit to serve as the tombersome flag.
 */
template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity = 256,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
requires ((std::is_same_v<ValidAndPosStructType,uint8_t> || (std::is_same_v<ValidAndPosStructType, uint16_t>)
     || (std::is_same_v<ValidAndPosStructType, uint32_t>))
    && (InitCapacity > 0) && ((InitCapacity & (InitCapacity - 1)) == 0))
class FlatHashMapV2 {
public:
    // Keep struct alignment padding in mind
    struct ElementT {
        constexpr static std::size_t size_pos_in_bits = (sizeof(ValidAndPosStructType) * 8 - 2);

        ValidAndPosStructType is_valid : 1;
        ValidAndPosStructType is_removed : 1;
        ValidAndPosStructType pos : size_pos_in_bits;
        std::pair<K, V> pair;

        ElementT() : is_valid(0), is_removed(0), pos(0) {}
        void set(ValidAndPosStructType p, const K& key, const V& value) {
            is_valid = true;
            is_removed = false;
            pos = p;
            pair = std::pair<K, V>(key, value);
        }

        friend std::ostream & operator<<(std::ostream & cout, const ElementT & element) {
            return (cout << "{ is_valid : " << static_cast<int32_t>(element.is_valid)
                << ", is_removed : " << static_cast<int32_t>(element.is_removed)
                << ", pos : " << static_cast<int32_t>(element.pos) << ", key : " << element.pair.first 
                << ", value : " << element.pair.second << " }\n");
        }

        bool compare_pos(std::size_t start_pos) const {
            // DEBUGING
            #ifdef PRINT_FHM_CMP_POS
            static uint64_t cnt = 0;
            // std::cout << "pos in bits : " << std::bitset<8>(pos) << std::endl;
            // std::cout << "static_cast<ValidAndPosStructType>(start_pos) in bits : " 
            //     << std::bitset<8>(static_cast<ValidAndPosStructType>(start_pos)) << std::endl;
            // std::cout << "(static_cast<ValidAndPosStructType>(start_pos) 
            //     & (~(1 << size_pos_in_bits))) in bits : " 
            //     << std::bitset<8>((static_cast<ValidAndPosStructType>(start_pos) 
            //     & (~(1 << size_pos_in_bits)))) << std::endl;
            bool result = (pos == (static_cast<ValidAndPosStructType>(start_pos) 
                & ((1 << size_pos_in_bits) - 1)));
            if(result) {
                cnt++;
                if(cnt % 100 == 0) {
                    std::cout << "[FlatHashMapV2::compare_pos] cnt is " << cnt << std::endl;
                }
            }
            #endif
            // DEBUGING

            return (pos == (static_cast<ValidAndPosStructType>(start_pos) 
                & ((1 << size_pos_in_bits) - 1)));
            // magic mask
            // for ValidAndPosStructType == uint8_t, sizeof is 1
            // 1 << size_pos_in_bits is 10000000
            // ((1 << size_pos_in_bits) - 1) is 01111111
        }
    };
        
    friend class IteratorT;

    class IteratorT {
    public:
        friend class FlatHashMapV2;
        IteratorT(std::pair<K, V> * pair = nullptr) : pair_ptr_(pair) {}
        IteratorT(const IteratorT & other) = default;

        bool operator== (const IteratorT & other) const {
            return (pair_ptr_ == other.pair_ptr_);
        }

        std::pair<K, V> & operator*() {
            return *pair_ptr_;
        }
        std::pair<K, V> * operator->() {
            return pair_ptr_;
        }

    private:
        void invalidate() {
            // A fatal bug happened here, here's the reason for the bug
            // 1. I don't consider struct alignment rule
            // 2. I don't convert pair_ptr to uint8_t * before pointer arithmatic
            ElementT * element_ptr = (reinterpret_cast<ElementT *>(reinterpret_cast<uint8_t *>(pair_ptr_) - offsetof(ElementT, pair)));
            element_ptr -> is_valid = false;
            element_ptr -> is_removed = true;
        }
        std::pair<K, V> * pair_ptr_;
    };

    FlatHashMapV2() : elements_(InitCapacity), capacity_(InitCapacity) {}
    FlatHashMapV2(const FlatHashMapV2 & other) = default;
    FlatHashMapV2(FlatHashMapV2 && other) noexcept = default;

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

    const V & at(const K & key) const;
    V & operator[](const K & key);

    std::pair<IteratorT, bool> insert(const std::pair<const K, V> & pair);
    // std::pair<IteratorT, bool> insert(value_tyupe && pair);

    std::size_t erase(const K & key);
    // IteratorT erase(ConstIterator position);

    void clear();

    IteratorT find(const K & key);
    // ConstIteratorT find(const K & key) const;

    // std::size_t count(const K & key) const noexcept;

    IteratorT end() const {
        return IteratorT(nullptr);
    }

    float load_factor() const noexcept {
        return (size_ * 1.0f) / capacity_;
    }
    void set_max_load_factor(float max_load_factor) {
        max_load_factor_ = max_load_factor;
    }

    // For debug only
    std::size_t get_capacity() const {
        return capacity_;
    }

    // void rehash(std::size_t num_buckets);
private:
    void expand_and_rehash();

    using ContainerT = std::vector<ElementT>;

    ContainerT elements_;
    std::size_t size_{0}; // TODO: Test use std::size_t or std::ssize_t here
    std::size_t capacity_;
    float max_load_factor_{0.6};
};

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
bool FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::empty() const noexcept {
    return size_ == 0;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::size() const noexcept {
    return size_;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::capacity() const noexcept {
    return capacity_;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
const V & FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::at(const K & key) const {
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t start_pos = pos;
    std::size_t cnt = 0;
    do {
        auto & element = elements_.at(pos);
        if(element.is_valid) {
            // DEBUGING
            #ifdef DEBUG_FHM
            std::cout << "[FlatHashMapV2::at] key " << key << " is not found. pos is " << pos << std::endl;
            #endif
            // DEBUGING
            if(element.compare_pos(start_pos) && (element.pair.first == key)) {
                break;
            }
        }
        else if((element.is_removed) && element.compare_pos(start_pos) && (element.pair.first == key)) {
            // Use the tombersome flag to accelerate finding for removed elements
            throw std::out_of_range("[FlatHashMapV2::at] key is not found");
        }
        pos = (pos + 1) % capacity_;
        cnt++;
        // make sure capacity_ is a power of 2 to optimize this
    } while (cnt < capacity_);

    if(cnt == capacity_) {
        throw std::out_of_range("[FlatHashMapV2::at] key is not found");
    }
    return elements_.at(pos).pair.second;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
V & FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::operator[](const K & key) {
    if(load_factor() > max_load_factor_) {
        expand_and_rehash();
    }
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t start_pos = pos;

    // DEBUGING
    #ifdef DEBUG_FHM
    std::cout << "[FlatHashMap::operator[]] capacity is " << capacity_ << ", key is " << key 
        << ", start_pos is " << start_pos << "\n";
    #endif
    // DEBUGING

    // NOTE: In this naive implementation, I cannot just iterate through the element
    // from position "pos", and insert a dump element if we encounter an invalid element.
    // This is because such thing can happen: 
    // 1. We insert (k0, v0) which Hash(k0) % cap = h0. The pair is placed at position "h0".
    // 2. We insert (k1, v1) which Hash(k1) % cap = h0. The pair is placed at position "h0 + 1".
    // 3. We erase (k0, v0).
    // 4. We call operator[k1]. If we don't find out whether k1 exist or not first, we would insert dump element
    // at position "h0" and return. That is totally wrong. 
    // Therefore I call "find" function here.
    // And this is obviously non-efficient. Further optimizations will improve it.
    auto it = find(key);
    if(it != end()) {
        return it -> second;
    }

    do {
        auto & element = elements_[pos];

        // DEBUGING
        #ifdef DEBUG_FHM
        std::cout << "[FlatHashMap::operator[]] pos is " << static_cast<int32_t>(pos) << ", and element is " << element << "\n";
        #endif
        // DEBUGING

        if((!element.is_valid)) {
            // How does the statement "map[k] = v" automatically update the size of the hashmap?
            // We can define the semantic of operator[] in a smart way.
            // If map[k] does not exists, we insert a dump element and increment the size.
            // And we return the dump element.
            element.is_valid = true;
            element.pos = start_pos;
            element.pair.first = key;

            // // DEBUGING
            // element.pair.second = 1;
            // // DEBUGING
            size_++;
            break;
        }
        else if(element.compare_pos(start_pos) && (element.pair.first == key)) {
            break;
        }
        pos = (pos + 1) % capacity_;
    } while (true);
    return elements_[pos].pair.second;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
auto FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::find(const K & key) -> IteratorT {
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t start_pos = pos;
    std::size_t cnt = 0;
    do {
        auto & element = elements_[pos];

        // DEBUGING
        #ifdef DEBUG_FHM
        std::cout << "[FlatHashMapV2::find] pos is " << pos << ", element is " << element << "\n";
        std::cout << "element.compare_pos(start_pos) : " << element.compare_pos(start_pos) << "\n";
        #endif
        // DEBUGING

        if((element.is_valid) && element.compare_pos(start_pos) && (element.pair.first == key)) {
            return IteratorT(&element.pair);
        }
        else if((element.is_removed) && element.compare_pos(start_pos) && (element.pair.first== key)) {
            // Use the tombersome flag to accelerate finding for removed elements

            // DEBUGING
            #ifdef DEBUG_FHM
            std::cout << "[FlatHashMapV2::find] matched tombersome. return end()." << "\n";
            std::cout << "start_pos is " << start_pos << "\n";
            #endif
            // DEBUGING

            return end();
        }
        pos = (pos + 1) % capacity_;
        cnt++;
    } while (cnt < capacity_);
    return end();
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
auto FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::insert(const std::pair<const K, V> & pair) -> std::pair<IteratorT, bool> {
    if(load_factor() > max_load_factor_) {
        // #ifdef DEBUG_FHM
        //     std::cout << "size is " << size_ << "capacity is " << capacity_ << std::endl;
        //     std::cout << "[insert] load_factor() > max_load_factor_" << std::endl;
        // #endif
        expand_and_rehash();
    }

    const K & key = pair.first;
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t start_pos = pos;
    do {
        auto & element = elements_.at(pos);
        if(element.pair.first == key) {
            return {IteratorT(&element.pair), false};
        }
        if(!element.is_valid) {
            break;
        }
        pos = (pos + 1) % capacity_;
        // make sure capacity_ is a power of 2 to optimize this
    } while (true);

    // #ifdef DEBUG_FHM
    //     std::cout << "[insert] Setting" << std::endl;
    // #endif

    elements_[pos].set(start_pos, pair.first, pair.second);
    size_++;
    return {IteratorT(&(elements_[pos].pair)), true};
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::erase(const K & key) {
    IteratorT it = find(key);
    if(it == end()) {
        return 0;
    }

    // DEBUGING
    #ifdef DEBUG_FHM
    if(key != it -> first) {
        std::cout << "[FlatHashMapV2::erase] key != it -> first!! Error\n";
    }
    #endif
    // DEBUGING

    assert(key == it -> first);

    // assert(static_cast<uint8_t>(it.getValidFlagField()) == 0x1);
    it.invalidate();
    size_--;
    return 1;
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
void FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::expand_and_rehash() {
    ContainerT new_elements(elements_.size() * 2);
    capacity_ *= 2;
    for(auto & element : elements_) {
        if(element.is_valid) {
            std::size_t new_pos = Hash()(element.pair.first) % capacity_;
            std::size_t new_start_pos = new_pos;
            while(true) {
                // Don't forget to apply linear probe when resizing
                auto & new_element = new_elements[new_pos];
                if(!new_element.is_valid) {
                    new_element = element;
                    new_element.pos = new_start_pos;
                    break;
                }
                new_pos = (new_pos + 1) % capacity_;
            }
        }
    }
    elements_ = std::move(new_elements);
}

template <typename ValidAndPosStructType,
          typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
void FlatHashMapV2<ValidAndPosStructType, K, V, InitCapacity, Hash, KeyEqual, Allocator>::clear() {
    capacity_ = InitCapacity;
    elements_.clear();
    elements_.resize(InitCapacity);
    size_ = 0;
}

template <typename K, typename V,
          std::size_t InitCapacity = 256,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
using FlatHashMapV2a = FlatHashMapV2<uint8_t, K, V, InitCapacity, Hash, KeyEqual, Allocator>;

template <typename K, typename V,
          std::size_t InitCapacity = 256,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
using FlatHashMapV2b = FlatHashMapV2<uint16_t, K, V, InitCapacity, Hash, KeyEqual, Allocator>;

template <typename K, typename V,
          std::size_t InitCapacity = 256,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
using FlatHashMapV2c = FlatHashMapV2<uint32_t, K, V, InitCapacity, Hash, KeyEqual, Allocator>;

}