#include <cstdint>
#include <array>
#include <stdexcept>
#include <utility>

/**
 * A compile-time fixed-size bitmap supporting constexpr operations.
 * @tparam N Number of bits (must be positive).
 */
template <size_t N>
class ConstexprBitmap {
    static_assert(N > 0, "Bitmap size must be positive");

    // Each uint64_t stores 64 bits. Calculate the number of blocks needed.
    static constexpr size_t StorageSize = (N + 63) / 64;
    std::array<uint64_t, StorageSize> data{};  // Underlying storage

public:
    // --- Constructors ---
    /**
     * Default constructor (initializes all bits to 0).
     */
    constexpr ConstexprBitmap() = default;

    /**
     * Initialize from a uint64_t (only the lowest N bits are used).
     * @param value Initial value for the first 64 bits (remaining bits set to 0).
     */
    constexpr explicit ConstexprBitmap(uint64_t value) {
        if constexpr (StorageSize >= 1) {
            data[0] = value;
            for (size_t i = 1; i < StorageSize; ++i) {
                data[i] = 0;  // Clear remaining blocks
            }
        }
    }

    /**
     * Initialize from a boolean array.
     * @param values Array of bool where `true` = 1, `false` = 0.
     */
    constexpr explicit ConstexprBitmap(const std::array<bool, N>& values) {
        for (size_t i = 0; i < N; ++i) {
            set(i, values[i]);  // Set bits according to the input array
        }
    }

    // --- Bit Operations ---
    /**
     * Check if a bit is set (1).
     * @param pos Bit position (0-indexed).
     * @return `true` if the bit is 1, `false` otherwise.
     * @throws std::out_of_range if pos >= N.
     */
    constexpr bool test(size_t pos) const {
        if (pos >= N) throw std::out_of_range("Bitmap index out of range");
        return (data[pos / 64] & (1ULL << (pos % 64))) != 0;
    }

    /**
     * Set or clear a bit.
     * @param pos Bit position (0-indexed).
     * @param value `true` to set (1), `false` to clear (0).
     * @throws std::out_of_range if pos >= N.
     */
    constexpr void set(size_t pos, bool value = true) {
        if (pos >= N) throw std::out_of_range("Bitmap index out of range");
        const size_t block = pos / 64;
        const size_t offset = pos % 64;
        if (value) {
            data[block] |= (1ULL << offset);   // Set bit
        } else {
            data[block] &= ~(1ULL << offset);  // Clear bit
        }
    }

    /**
     * Clear a bit (set to 0).
     * @param pos Bit position (0-indexed).
     */
    constexpr void reset(size_t pos) {
        set(pos, false);
    }

    /**
     * Toggle a bit (0 ↔ 1).
     * @param pos Bit position (0-indexed).
     * @throws std::out_of_range if pos >= N.
     */
    constexpr void flip(size_t pos) {
        if (pos >= N) throw std::out_of_range("Bitmap index out of range");
        const size_t block = pos / 64;
        const size_t offset = pos % 64;
        data[block] ^= (1ULL << offset);  // XOR to flip
    }

    // --- Queries ---
    /**
     * Count the number of set bits (1s).
     * @return Number of 1s in the bitmap.
     */
    constexpr size_t count() const {
        size_t cnt = 0;
        for (uint64_t block : data) {
            cnt += __builtin_popcountll(block);  // Use compiler intrinsic for efficiency
        }
        return cnt;
    }

    /**
     * Check if all bits are set (1).
     * @return `true` if all bits are 1, `false` otherwise.
     */
    constexpr bool all() const {
        for (size_t i = 0; i < N; ++i) {
            if (!test(i)) return false;
        }
        return true;
    }

    /**
     * Check if any bit is set (1).
     * @return `true` if at least one bit is 1, `false` otherwise.
     */
    constexpr bool any() const {
        for (uint64_t block : data) {
            if (block != 0) return true;
        }
        return false;
    }

    /**
     * Check if no bits are set (all 0s).
     * @return `true` if all bits are 0, `false` otherwise.
     */
    constexpr bool none() const {
        return !any();
    }

    // --- Conversions ---
    /**
     * Convert to uint64_t (only valid if N <= 64).
     * @return The first 64 bits as an integer.
     */
    constexpr uint64_t to_uint64() const {
        static_assert(N <= 64, "Bitmap too large for uint64_t");
        return data[0];
    }

    /**
     * Convert to a binary string (e.g., "1010").
     * @return String representation of the bitmap.
     */
    constexpr std::string to_string() const {
        std::string result;
        for (size_t i = 0; i < N; ++i) {
            result += test(i) ? '1' : '0';
        }
        return result;
    }
};