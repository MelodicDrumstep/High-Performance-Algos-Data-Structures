#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <concepts>

namespace hpds {
 
template <int32_t BlockSize>
concept ValidBlockSize = (BlockSize % 8 == 0) && (BlockSize <= 64) && (BlockSize > 0);

template <std::size_t InitCapacity, ValidBlockSize BlockSize>
class Bitmap {
    using ElementT = std::conditional_t<InitCapacity == 8, uint8_t,
                                        InitCapacity == 16, uint16_t,
                                        InitCapacity == 32, uint32_t,
                                        InitCapacity == 64, uint64_t>;
public:
    

    void resize();
private:
    std::vector<ElementT> elements_;
};

}