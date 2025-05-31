#pragma once

#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <utility>

template <typename K, typename V, 
          typename Hash = std::hash<K>,
          typename KeyEqual = std::equal_to<K>,
          typename Allocator = std::allocator<std::pair<const K, V>>>
class FlatHashMap {
    using value_type = std::pair<const K, V>;
    // Follow the naming convention of STL containers
    // Therefore it's not named as "ValueType"
    // using IteratorT = ;
public:
    FlatHashMap();
    FlatHashMap(const FlatHashMap & other);
    FlatHashMap(FlatHashMap && other) noexcept;
    // template <typename InputIterator>
    // FlatHashMap(InputIterator first, InputIterator last);

    bool empty() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

    const Pair & at(const K & key) const;
    Pair & operator[](const K & key);

    std::pair<IteratorT, bool> insert(const value_type & pair);
    std::pair<IteratorT, bool> insert(value_tyupe && pair);

    // template <typename Pair>
    // std::pair<IteratorT, bool> insert(Pair && pair);
    
    // IteratorT insert(ConstIterator hint, const value_type & pair);
    // IteratorT insert(ConstIterator hint, value_type && pair);

    // template <typename Pair>
    // IteratorT insert(ConstIterator hint, Pair && pair);

    // template <typename InputIt>
    // void insert(InputIt first, InputIt last);

    // void insert(std::initializer_list<value_type> ilist);

    std::size_t erase(const K & key);
    IteratorT erase(ConstIterator position);

    void clear() noexcept;

    IteratorT find(const K & key);
    ConstIteratorT find(const K & key) const;

    std::size_t count(const K & key) const;

    float load_factor() const noexcept;
    void set_max_load_factor(float max_load_factor);

    void rehash(std::size_t num_buckets);

private:
};