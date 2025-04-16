#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <cstddef>

template <typename T>
struct AlignedAllocator {
    using value_type = T;

    AlignedAllocator() = default;

    template <typename U>
    AlignedAllocator(const AlignedAllocator<U> &) {}

    T * allocate(std::size_t n) {
        void * ptr = nullptr;
        if(posix_memalign(&ptr, 32, n * sizeof(T)) != 0) {
            throw std::bad_alloc();
        }
        return static_cast<T *>(ptr);
    }

    void deallocate(T * ptr, std::size_t n) {
        free(ptr);
    }
};

template <typename T, typename U>
bool operator==(const AlignedAllocator<T> &, const AlignedAllocator<U> &) {
    return true;
}

template <typename T, typename U>
bool operator!=(const AlignedAllocator<T> &, const AlignedAllocator<U> &) {
    return false;
}