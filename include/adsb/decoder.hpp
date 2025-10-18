#pragma once

#include "adsb/types.hpp"

#include <memory>
#include <vector>

class ADSBMessage;
class AirbornePositionMessage;

namespace decoder {

    /**
     * @brief Decodes a raw 112-bit ADS-B message.
     *
     * @param raw_bits A vector of 112 integers (0 or 1) representing the message bits.
     * @return A unique_ptr to the decoded ADSBMessage object.
     * Returns `nullptr` if the message is invalid or the CRC fails.
     */
    std::unique_ptr<ADSBMessage> decode(const std::vector<int>& raw_bits);

    /**
     * @brief Calculates the global position from a pair of airborne position messages.
     *
     * @param msg_a The first AirbornePositionMessage.
     * @param msg_b The second AirbornePositionMessage.
     * @param ref_pos The reference position of the receiver.
     * @return A PositionResult struct. The `is_valid` field is true on success.
     */
    PositionResult calculate_global_position(
        const AirbornePositionMessage& msg_a,
        const AirbornePositionMessage& msg_b,
        const GlobalPosition& ref_pos);

}