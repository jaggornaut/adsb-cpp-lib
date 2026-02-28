#pragma once

#include "adsb/message/ADSBMessage.hpp"

namespace adsb::message {
    class VelocityMessage : public ADSBMessage {
    public:
        /**
         * @brief Constructs a VelocityMessage object.
         */
        VelocityMessage(const std::string& icao, int type_code, const std::vector<int>& payload);

        double get_speed() const { return m_speed; }
        double get_heading() const { return m_heading; }
        int get_vertical_rate() const { return m_vertical_rate; }

        /**
         * @brief Returns a string representation of the message.
         * @return A formatted string for debugging.
         */
        std::string to_string() const override;

    private:
        void decode_payload();

        double m_speed;
        double m_heading;
        int m_vertical_rate;
    };
}