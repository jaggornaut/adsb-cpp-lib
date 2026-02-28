#pragma once

#include <vector>

namespace adsb::utils {
    /**
     * @brief Converts a sequence of bits to an integer.
     *
     * @param begin A constant iterator to the beginning of the bit sequence.
     * @param end A constant iterator to the end of the bit sequence.
     * @return The resulting integer value.
     */
    int bits_to_int(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);
}