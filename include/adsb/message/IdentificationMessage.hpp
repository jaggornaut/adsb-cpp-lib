#pragma once

#include "adsb/message/ADSBMessage.hpp"
#include <string>
#include <vector>

namespace adsb::message {
    class IdentificationMessage : public ADSBMessage {
    public:
        /**
         * @enum class EmitterCategory
         * @brief Defines the aircraft category based on TC=1-4 messages.
         */
        enum class EmitterCategory {
            NO_INFO_A,                      // A0
            LIGHT,                          // A1
            SMALL,                          // A2
            LARGE,                          // A3
            HIGH_VORTEX_LARGE,              // A4
            HEAVY,                          // A5
            HIGH_PERFORMANCE,               // A6
            ROTORCRAFT,                     // A7

            NO_INFO_B,                      // B0
            GLIDER_SAILPLANE,               // B1
            LIGHTER_THAN_AIR,               // B2
            PARACHUTIST_SKYDIVER,           // B3
            ULTRALIGHT_HANG_GLIDER,         // B4
            RESERVED_B5,                    // B5
            UNMANNED_AERIAL_VEHICLE,        // B6
            SPACE_TRANS_ATMOSPHERIC,        // B7

            NO_INFO_C,                      // C0
            SURFACE_EMERGENCY_VEHICLE,      // C1
            SURFACE_SERVICE_VEHICLE,        // C2
            POINT_OBSTACLE,                 // C3
            CLUSTER_OBSTACLE,               // C4
            LINE_OBSTACLE,                  // C5

            UNKNOWN
        };

        /**
         * @brief Constructs an IdentificationMessage object.
         */
        IdentificationMessage(const std::string& icao, int type_code, const std::vector<int>& payload);

        const std::string& get_flight_name() const { return m_flight_name; }
        EmitterCategory get_category() const { return m_category; }

        /**
         * @brief Returns a string representation of the message.
         * @return A single-line formatted string for debugging.
         */
        std::string to_string() const override;

    private:
        void decode_payload();

        EmitterCategory decode_category(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);
        std::string decode_flight_name(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);

        std::string m_flight_name;
        EmitterCategory m_category;
    };

    /**
     * @brief Utility function to convert an EmitterCategory to a string.
     * @param category The category enum to convert.
     * @return A string representation.
     */
    std::string to_string(IdentificationMessage::EmitterCategory category);
}