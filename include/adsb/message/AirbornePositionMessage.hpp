#pragma once

#include "adsb/message/ADSBMessage.hpp"

namespace adsb::message {
    class AirbornePositionMessage : public ADSBMessage {
    public:
        /**
         * @brief Constructs an AirbornePositionMessage object.
         */
        AirbornePositionMessage(const std::string& icao, int type_code, const std::vector<int>& payload);

        int get_surveillance_status() const { return m_surveillance_status; }
        int get_nic_supplement_b() const { return m_nic_supplement_b; }
        int get_altitude() const { return m_altitude; }
        bool has_time_utc_sync() const { return m_time_bit; }
        bool is_odd_frame() const { return m_is_odd; }
        int get_cpr_latitude_raw() const { return m_cpr_lat; }
        int get_cpr_longitude_raw() const { return m_cpr_lon; }

        /**
         * @brief Returns a string representation of the message.
         * @return A formatted string for debugging.
         */
        std::string to_string() const override;

    private:
        void decode_payload();
        int decode_altitude(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);

        int m_surveillance_status;
        int m_nic_supplement_b;
        int m_altitude;
        bool m_time_bit;
        bool m_is_odd;
        int m_cpr_lat;
        int m_cpr_lon;
    };
}