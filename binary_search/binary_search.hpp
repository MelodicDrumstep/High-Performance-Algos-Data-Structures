#pragma once

#include <vector>
#include <array>
#include <cstdint>

int32_t binary_search_baseline(std::vector<int32_t> & elements, int32_t target) {
    int l = 0, r = elements.size() - 1;
    while (l < r) {
        int m = (l + r) / 2;
        if (elements[m] >= target)
            r = m;
        else
            l = m + 1;
    }
    return elements[l];
}

