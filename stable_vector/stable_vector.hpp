#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <functional>
#include <optional>
#include <concepts>
#include <type_traits>

// #define DEBUG_STABLE_VECTOR

namespace hpds {

template <typename T, std::size_t ChunkSize = 256, std::size_t InitialCapacity = 256>
requires ((((ChunkSize) & (ChunkSize - 1)) == 0))
// make sure ChunkSize is a power of 2, for the compiler to optimize
// out modular operations
class StableVector {
    class Chunk {
    public:
        void push_back(const T & element) {
            // DEBUGING
            #ifdef DEBUG_STABLE_VECTOR
            std::cout << "[Chunk::push_back] chunk_size: " << chunk_size << std::endl;
            #endif
            // DEBUGING

            if(chunk_size >= ChunkSize) {
                throw std::out_of_range("[Chunk::push_back]");
            }
            // TODO: I think copy like this is the same fast as placement new
            // But I can measure on that
            // (placement new may have a nullptr check depending on the compiler implementation)
            elements[chunk_size] = element;
            chunk_size++;
        }

        template <typename ... Args>
        void emplace_back(Args && ... args) {
            if(chunk_size >= ChunkSize) {
                throw std::out_of_range("[Chunk::push_back]");
            }
            new (elements.data() + chunk_size) T(std::forward<Args>(args)...);
            chunk_size++;
        }

        T & operator[](std::size_t index) {
            return elements[index];
        }

        const T & at (std::size_t index) const {
            return elements.at(index);
        }

    private:
        std::array<T, ChunkSize> elements;
        // make sure T has default constructor
        std::size_t chunk_size = 0;
    };

    class Iterator {
    public:
        Iterator(StableVector<T, ChunkSize, InitialCapacity> & stable_vec, std::size_t index) : stable_vec_(stable_vec), index_(index) {}
        Iterator(const Iterator & other) : stable_vec_(other.stable_vec_), index_(other.index_) {}
        
        T & operator*() {
            return stable_vec_[index_];
        }

        const T & operator*() const {
            return stable_vec_[index_];
        }

        T * operator->() {
            return &stable_vec_[index_];
        }

        const T * operator->() const {
            return &stable_vec_[index_];
        }

        Iterator & operator++() {
            index_++;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            index_++;
            return tmp;
        }
        
        Iterator & operator--() {
            index_--;
            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            index_--;
            return tmp;
        }

        Iterator operator+(std::size_t n) const {
            return Iterator(stable_vec_, index_ + n);
        }

        Iterator operator-(std::size_t n) const {
            return Iterator(stable_vec_, index_ - n);
        }

        bool operator==(const Iterator & other) const {
            if(index_ >= stable_vec_.size() && other.index_ >= stable_vec_.size()) {
                // both is end iterator
                return true;
            }
            return index_ == other.index_;
        }

        bool operator!=(const Iterator & other) const {
            return !operator==(other);
        }
    private:
        StableVector<T, ChunkSize, InitialCapacity> & stable_vec_;
        std::size_t index_;
    };

public:
    StableVector() {
        chunks_.reserve(InitialCapacity / ChunkSize);
        for (size_t i = 0; i < InitialCapacity / ChunkSize; ++i) {
            chunks_.push_back(std::make_unique<Chunk>());
        }
    }
    StableVector(const StableVector&) = delete;
    StableVector(StableVector&&) = default;
    StableVector& operator=(const StableVector&) = delete;
    StableVector& operator=(StableVector&&) = default;
    ~StableVector() = default;

    T & operator[](std::size_t index) {
        return (*chunks_[index / ChunkSize])[index % ChunkSize];
    }

    const T & at(std::size_t index) {
        if(index >= size_) {
            throw std::out_of_range("[StableVector::at] index out of range");
        }
        return chunks_.at(index / ChunkSize) -> at(index % ChunkSize);
    }

    void push_back(const T & element) {
        get_last_chunk_for_insert().push_back(element);
        size_++;
    }

    template <typename ... Args>
    void emplace_back(Args && ... args) {
        get_last_chunk_for_insert().emplace_back(std::forward<Args>(args)...);
        size_++;
    }

    void expand_capacity_to(std::size_t capacity) {
        if((capacity / ChunkSize) > (size_ / ChunkSize)) {
            // std::unique_ptr is not copyable, so we need to use resize
            // and then copy the elements
            chunks_.resize(capacity / ChunkSize);
            for(size_t i = size_ / ChunkSize; i < capacity / ChunkSize; i++) {
                chunks_[i] = std::make_unique<Chunk>();
            }
        }
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return (size_ == 0);
    }

    Iterator begin() {
        return Iterator(*this, 0);
    }

    Iterator end() {
        return Iterator(*this, size_);
    }

    T & front() {
        if(size_ == 0) {
            throw std::out_of_range("[StableVector::front]");
        }
        return (*(chunks_[0]))[0];
    }

    T & back() {
        if(size_ == 0) {
            throw std::out_of_range("[StableVector::back]");
        }
        return (*(chunks_[(size_ - 1) / ChunkSize]))[(size_ - 1) % ChunkSize];
    }

    void clear() {
        chunks_.clear();
        chunks_.reserve(InitialCapacity / ChunkSize);
        for (size_t i = 0; i < InitialCapacity / ChunkSize; ++i) {
            chunks_.push_back(std::make_unique<Chunk>());
        }
        size_ = 0;
    }

private:
    Chunk & get_last_chunk_for_insert() {
        // DEBUGING
        #ifdef DEBUG_STABLE_VECTOR
        std::cout << "[get_last_chunk_for_insert] size_: " << size_ << ", chunks_.size(): " << chunks_.size() << std::endl;
        #endif
        // DEBUGING

        if(((size_ % ChunkSize) == 0) && ((size_ / ChunkSize) == chunks_.size())) {
            // no more space, need to insert a new chunk
            chunks_.push_back(std::make_unique<Chunk>());
        }

        return *(chunks_[size_ / ChunkSize]);
    }

    std::vector<std::unique_ptr<Chunk>> chunks_;
    std::size_t size_ = 0;
};

}