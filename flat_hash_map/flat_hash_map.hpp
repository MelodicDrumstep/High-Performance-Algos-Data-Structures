#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <utility>
#include <optional>
#include <cassert>
#include <cstddef>

//#define DEBUG_FHM

namespace hpds {
// hpds is for High-Performance Data Structures

template <typename K, typename V,
          std::size_t InitCapacity = 256,
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
requires ((InitCapacity > 0) && ((InitCapacity & (InitCapacity - 1)) == 0))
class FlatHashMap {
public:
    // Keep struct alignment padding in mind
    struct ElementT {
        bool is_valid = false;
        std::pair<K, V> pair;
    };
    
    friend class IteratorT;

    class IteratorT {
    public:
        friend class FlatHashMap;
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
        bool & getValidFlagField() {
            // A fatal bug happened here, here's the reason for the bug
            // 1. I don't consider struct alignment rule
            // 2. I don't convert pair_ptr to uint8_t * before pointer arithmatic
            return *(reinterpret_cast<bool *>(reinterpret_cast<uint8_t *>(pair_ptr_) - offsetof(ElementT, pair)));
        }
        std::pair<K, V> * pair_ptr_;
    };

    FlatHashMap() : elements_(InitCapacity), capacity_(InitCapacity) {}
    FlatHashMap(const FlatHashMap & other) = default;
    FlatHashMap(FlatHashMap && other) noexcept = default;

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

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
bool FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::empty() const noexcept {
    return size_ == 0;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::size() const noexcept {
    return size_;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::capacity() const noexcept {
    return capacity_;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
const V & FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::at(const K & key) const {
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t cnt = 0;
    do {
        auto & element = elements_.at(pos);
        if(element.is_valid) {
            // DEBUGING
            #ifdef DEBUG_FHM
            std::cout << "[FlatHashMap::at] key " << key << " is not found. pos is " << pos << std::endl;
            #endif
            // DEBUGING
            if(element.pair.first == key) {
                // DBEUGING
                #ifdef DEBUG_FHM
                if(key == 195) {
                    std::cout << "[FlatHashMap::at] key is 195. find the element at pos " << pos << "\n";
                }
                #endif
                // DEBUGING

                break;
            }
        }
        pos = (pos + 1) % capacity_;
        cnt++;
        // make sure capacity_ is a power of 2 to optimize this
    } while (cnt < capacity_);

    if(cnt == capacity_) {
        throw std::out_of_range("[FlatHashMap::at] key is not found");
    }
    return elements_.at(pos).pair.second;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
V & FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::operator[](const K & key) {
    if(load_factor() > max_load_factor_) {
        expand_and_rehash();
    }
    std::size_t pos = Hash()(key) % capacity_;
    do {
        auto & element = elements_[pos];
        if((!element.is_valid)) {
            // How does the statement "map[k] = v" automatically update the size of the hashmap?
            // We can define the semantic of operator[] in a smart way.
            // If map[k] does not exists, we insert a dump element and increment the size.
            // And we return the dump element.
            element.is_valid = true;
            element.pair.first = key;

            // // DEBUGING
            // element.pair.second = 1;
            // // DEBUGING
            size_++;

            // DBEUGING
            #ifdef DEBUG_FHM
            if(key == 195) {
                std::cout << "[FlatHashMap::operator[]] key is 195. Insert dump element at pos " << pos << "\n";
            }
            #endif
            // DEBUGING

            break;
        }
        else if(element.pair.first == key) {
            // DBEUGING
            #ifdef DEBUG_FHM
            if(key == 195) {
                std::cout << "[FlatHashMap::operator[]] key is 195. The key already exists.\n";
            }
            #endif
            // DEBUGING

            break;
        }
        pos = (pos + 1) % capacity_;
    } while (true);
    return elements_[pos].pair.second;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
auto FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::find(const K & key) -> IteratorT {
    std::size_t pos = Hash()(key) % capacity_;
    std::size_t cnt = 0;
    do {
        auto & element = elements_[pos];
        if((element.is_valid) && (element.pair.first == key)) {
            // DBEUGING
            #ifdef DEBUG_FHM
            if(key == 195) {
                std::cout << "[FlatHashMap::find] key is 195. element at pos " << pos << " is what we want to find. Break\n";
            }
            #endif
            // DEBUGING
        
            return IteratorT(&element.pair);
        }
        pos = (pos + 1) % capacity_;
        cnt++;
    } while (cnt < capacity_);
    return end();
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
auto FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::insert(const std::pair<const K, V> & pair) -> std::pair<IteratorT, bool> {
    if(load_factor() > max_load_factor_) {
        // #ifdef DEBUG_FHM
        //     std::cout << "size is " << size_ << "capacity is " << capacity_ << std::endl;
        //     std::cout << "[insert] load_factor() > max_load_factor_" << std::endl;
        // #endif
        expand_and_rehash();
    }

    const K key = pair.first;
    std::size_t pos = Hash()(key) % capacity_;
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

    elements_[pos] = {true, {pair.first, pair.second}};
    size_++;
    return {IteratorT(&(elements_[pos].pair)), true};
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
std::size_t FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::erase(const K & key) {
    IteratorT it = find(key);
    if(it == end()) {
        return 0;
    }

    // DEBUGING
    #ifdef DEBUG_FHM
    if(key != it -> first) {
        std::cout << "[FlatHashMap::erase] key != it -> first!! Error\n";
    }
    #endif
    // DEBUGING

    assert(key == it -> first);

    // assert(static_cast<uint8_t>(it.getValidFlagField()) == 0x1);
    it.getValidFlagField() = false;
    size_--;
    return 1;
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
void FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::expand_and_rehash() {
    ContainerT new_elements(elements_.size() * 2);
    capacity_ *= 2;
    for(auto & element : elements_) {
        if(element.is_valid) {

            // DBEUGING
            #ifdef DEBUG_FHM
            if(element.pair.first == 195) {
                std::cout << "[FlatHashMap::expand_and_rehash] key is 195\n";
            }
            #endif
            // DEBUGING

            std::size_t new_pos = Hash()(element.pair.first) % capacity_;
            while(true) {
                // Don't forget to apply linear probe when resizing
                auto & new_element = new_elements[new_pos];
                if(!new_element.is_valid) {
                    new_element = element;

                    // DBEUGING
                    #ifdef DEBUG_FHM
                    if(element.pair.first == 195) {
                        std::cout << "[FlatHashMap::expand_and_rehash] Find position. size is " << size_ <<
                            ", capacity is " << capacity_ << ", pos is " << new_pos << ", new_element.is_valid is " << 
                            new_element.is_valid << std::endl;
                    }
                    #endif
                    // DEBUGING

                    break;
                }
                new_pos = (new_pos + 1) % capacity_;
            }
        }
    }
    elements_ = std::move(new_elements);
}

template <typename K, typename V,
          std::size_t InitCapacity,
          typename Hash,
          typename KeyEqual,
          typename Allocator>
void FlatHashMap<K, V, InitCapacity, Hash, KeyEqual, Allocator>::clear() {
    capacity_ = InitCapacity;
    elements_.clear();
    elements_.resize(InitCapacity);
    size_ = 0;
}

}