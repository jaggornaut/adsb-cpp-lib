#pragma once

#include "adsb/types.hpp"

#include <memory>
#include <vector>

#include "message/ADSBMessage.hpp"
#include "message/AirbornePositionMessage.hpp"


namespace adsb::decoder {

    /**
     * @brief Decodes a raw 112-bit ADS-B message.
     *
     * @param raw_bits A vector of 112 integers (0 or 1) representing the message bits.
     * @return A unique_ptr to the decoded ADSBMessage object.
     * Returns `nullptr` if the message is invalid or the CRC fails.
     */
    std::unique_ptr<message::ADSBMessage> decode(const std::vector<int>& raw_bits);

    /**
     * @brief Calculates the global position from a pair of airborne position messages.
     *
     * @param msg_a The first AirbornePositionMessage.
     * @param msg_b The second AirbornePositionMessage.
     * @param ref_pos The reference position of the receiver.
     * @return A PositionResult struct. The `is_valid` field is true on success.
     */
    types::PositionResult calculate_global_position(
        const message::AirbornePositionMessage& msg_a,
        const message::AirbornePositionMessage& msg_b,
        const types::GlobalPosition& ref_pos);

}
