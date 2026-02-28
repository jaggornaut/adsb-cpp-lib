#include "adsb/utils.hpp"

#include <vector>

namespace adsb::utils {
    int bits_to_int(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end) {
        int n = 0;
        for (auto it = begin; it != end; ++it) {
            n = (n << 1) | *it;
        }
        return n;
    }
}
